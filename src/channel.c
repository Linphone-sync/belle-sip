/*
	cain-sip - SIP (RFC3261) library.
    Copyright (C) 2010  Belledonne Communications SARL

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "cain_sip_internal.h"

static void channel_prepare_continue(cain_sip_channel_t *obj);
static void channel_process_queue(cain_sip_channel_t *obj);

const char *cain_sip_channel_state_to_string(cain_sip_channel_state_t state){
	switch(state){
		case CAIN_SIP_CHANNEL_INIT:
			return "INIT";
		case CAIN_SIP_CHANNEL_RES_IN_PROGRESS:
			return "RES_IN_PROGRESS";
		case CAIN_SIP_CHANNEL_RES_DONE:
			return "RES_DONE";
		case CAIN_SIP_CHANNEL_CONNECTING:
			return "CONNECTING";
		case CAIN_SIP_CHANNEL_READY:
			return "READY";
		case CAIN_SIP_CHANNEL_ERROR:
			return "ERROR";
		case CAIN_SIP_CHANNEL_DISCONNECTED:
			return "DISCONNECTED";
	}
	return "BAD";
}

static cain_sip_list_t * for_each_weak_unref_free(cain_sip_list_t *l, cain_sip_object_destroy_notify_t notify, void *ptr){
	cain_sip_list_t *elem,*next;
	for(elem=l;elem!=NULL;elem=next){
		next=elem->next;
		cain_sip_object_weak_unref(elem->data,notify,ptr);
		cain_sip_free(elem);
	}
	return NULL;
}

static void cain_sip_channel_destroy(cain_sip_channel_t *obj){
	if (obj->peer) freeaddrinfo(obj->peer);
	cain_sip_free(obj->peer_name);
	if (obj->local_ip) cain_sip_free(obj->local_ip);
	obj->listeners=for_each_weak_unref_free(obj->listeners,(cain_sip_object_destroy_notify_t)cain_sip_channel_remove_listener,obj);
	if (obj->resolver_id) cain_sip_resolve_cancel(cain_sip_stack_get_main_loop(obj->stack),obj->resolver_id);
	cain_sip_message("channel [%p] destroyed",obj);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_channel_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_channel_t)=
{
	{
		CAIN_SIP_VPTR_INIT(cain_sip_channel_t,cain_sip_source_t,FALSE),
		(cain_sip_object_destroy_t)cain_sip_channel_destroy,
		NULL, /*clone*/
		NULL, /*marshall*/
	}
};
static void fix_incoming_via(cain_sip_request_t *msg, const struct addrinfo* origin){
	char received[NI_MAXHOST];
	char rport[NI_MAXSERV];
	cain_sip_header_via_t *via;
	int err=getnameinfo(origin->ai_addr,origin->ai_addrlen,received,sizeof(received),
	                rport,sizeof(rport),NI_NUMERICHOST|NI_NUMERICSERV);
	if (err!=0){
		cain_sip_error("fix_via: getnameinfo() failed: %s",gai_strerror(errno));
		return;
	}
	via=CAIN_SIP_HEADER_VIA(cain_sip_message_get_header((cain_sip_message_t*)msg,"via"));
	if (via){
		const char* host = cain_sip_header_via_get_host(via);
		
		if (strcmp(host,received)!=0)
				cain_sip_header_via_set_received(via,received);
			
		if (cain_sip_parameters_has_parameter(CAIN_SIP_PARAMETERS(via),"rport")){
			int port = cain_sip_header_via_get_listening_port(via);
			int rport_int=atoi(rport);
			if (rport_int!=port) cain_sip_header_via_set_rport(via,atoi(rport));
		}
	}
}
static int get_message_start_pos(char *buff, size_t bufflen) {
	/*FIXME still to optimize an better tested, specially REQUEST PATH and error path*/
	int i;
	int res=0;
	int status_code;
	char method[17];
	char saved_char1;
	char sip_version[10];
	int saved_char1_index;

	for(i=0; i<(int)bufflen-12;i++) { /*9=strlen( SIP/2.0\r\n)*/
		saved_char1_index=bufflen-1;
		saved_char1=buff[saved_char1_index]; /*make sure buff is null terminated*/
		buff[saved_char1_index]='\0';
		res=sscanf(buff+i,"SIP/2.0 %d ",&status_code);
		if (res!=1) {
			res= sscanf(buff+i,"%16s %*s %9s\r\n",method,sip_version)==2
					&& strcmp("SIP/2.0",sip_version)==0 ;
		}
		buff[saved_char1_index]=saved_char1;
		if (res==1) return i;
	}
	return -1;
}

static void cain_sip_channel_input_stream_reset(cain_sip_channel_input_stream_t* input_stream) {
	input_stream->read_ptr=input_stream->write_ptr=input_stream->buff;
	input_stream->state=WAITING_MESSAGE_START;
	input_stream->msg=NULL;
}
static size_t cain_sip_channel_input_stream_get_buff_length(cain_sip_channel_input_stream_t* input_stream) {
	return MAX_CHANNEL_BUFF_SIZE - (input_stream->write_ptr-input_stream->buff);
}

int cain_sip_channel_process_data(cain_sip_channel_t *obj,unsigned int revents){
	int num;
	int offset;
	int i;
	size_t read_size=0;
	cain_sip_header_content_length_t* content_length_header;
	int content_length;

	if (revents & CAIN_SIP_EVENT_READ) {
		if (obj->recv_error>0) {
			num=cain_sip_channel_recv(obj,obj->input_stream.write_ptr,cain_sip_channel_input_stream_get_buff_length(&obj->input_stream)-1);
			/*write ptr is only incremented if data were acquired from the transport*/
			obj->input_stream.write_ptr+=num;
			/*first null terminate the read buff*/
			*obj->input_stream.write_ptr='\0';
		} else {
			num=obj->recv_error;
		}
	} else if (!revents) {
		num=obj->input_stream.write_ptr-obj->input_stream.read_ptr;
	} else {
		cain_sip_error("Unexpected event [%i] on channel [%p]",revents,obj);
		num=-1; /*to trigger an error*/
	}
	if (num>0){


		if (obj->input_stream.state == WAITING_MESSAGE_START) {
			/*search for request*/
			if ((offset=get_message_start_pos(obj->input_stream.read_ptr,num)) >=0 ) {
				/*message found !*/
				if (offset>0) {
					cain_sip_warning("trashing [%i] bytes in front of sip message on channel [%p]",offset,obj);
					obj->input_stream.read_ptr+=offset;
				}
				obj->input_stream.state=MESSAGE_AQUISITION;
			} else {
				cain_sip_debug("Unexpected [%s] received on channel [%p], trashing",obj->input_stream.read_ptr,obj);
				cain_sip_channel_input_stream_reset(&obj->input_stream);
			}
		}

		if (obj->input_stream.state==MESSAGE_AQUISITION) {
			/*search for \r\n\r\n*/
			for (i=0;i<obj->input_stream.write_ptr-obj->input_stream.read_ptr;i++) {
				if (strncmp("\r\n\r\n",&obj->input_stream.read_ptr[i],4)==0) {
					/*end of message found*/
					cain_sip_message("channel [%p] read message from %s:%i\n%s",obj, obj->peer_name,obj->peer_port,obj->input_stream.read_ptr);
					obj->input_stream.msg=cain_sip_message_parse_raw(obj->input_stream.read_ptr
											,obj->input_stream.write_ptr-obj->input_stream.read_ptr
											,&read_size);
					obj->input_stream.read_ptr+=read_size;
					if (obj->input_stream.msg && read_size > 0){
						cain_sip_message("channel [%p] [%i] bytes parsed",obj,read_size);
						cain_sip_object_ref(obj->input_stream.msg);
						if (cain_sip_message_is_request(obj->input_stream.msg)) fix_incoming_via(CAIN_SIP_REQUEST(obj->input_stream.msg),obj->peer);
						/*check for body*/
						if ((content_length_header = (cain_sip_header_content_length_t*)cain_sip_message_get_header(obj->input_stream.msg,CAIN_SIP_CONTENT_LENGTH)) != NULL
								&& cain_sip_header_content_length_get_content_length(content_length_header)>0) {

							obj->input_stream.state=BODY_AQUISITION;
							break; /*don't avoid to exist from loop, because 2 response can be linked*/
						} else {
							/*no body*/
							goto message_ready;
						}

					}else{
						cain_sip_error("Could not parse [%s], resetting channel [%p]",obj->input_stream.read_ptr,obj);
						cain_sip_channel_input_stream_reset(&obj->input_stream);
					}
				}
			}
		}

		if (obj->input_stream.state==BODY_AQUISITION) {
			content_length=cain_sip_header_content_length_get_content_length((cain_sip_header_content_length_t*)cain_sip_message_get_header(obj->input_stream.msg,CAIN_SIP_CONTENT_LENGTH));
			if (content_length <= obj->input_stream.write_ptr-obj->input_stream.read_ptr) {
				/*great body completed*/
				cain_sip_message("channel [%p] read [%i] bytes of body from %s:%i\n%s"	,obj
																						,content_length
																						,obj->peer_name
																						,obj->peer_port
																						,obj->input_stream.read_ptr);
				cain_sip_message_set_body(obj->input_stream.msg,obj->input_stream.read_ptr,content_length);
				read_size+=content_length; /*read size is used in message ready to compute residu*/
				obj->input_stream.read_ptr+=content_length;
				goto message_ready;

			}
		}
		return CAIN_SIP_CONTINUE;
	message_ready:
		obj->incoming_messages=cain_sip_list_append(obj->incoming_messages,obj->input_stream.msg);
		obj->input_stream.msg=NULL;
		obj->input_stream.state=WAITING_MESSAGE_START;
		CAIN_SIP_INVOKE_LISTENERS_ARG1_ARG2(obj->listeners,cain_sip_channel_listener_t,on_event,obj,CAIN_SIP_EVENT_READ/*always a read event*/);
		if (obj->input_stream.write_ptr-obj->input_stream.read_ptr>0) {
			/*process residu*/
			cain_sip_channel_process_data(obj,0);
		} else {
			cain_sip_channel_input_stream_reset(&obj->input_stream); /*end of strem, back to home*/
		}
		return CAIN_SIP_CONTINUE;
	} else if (num == 0) {
		channel_set_state(obj,CAIN_SIP_CHANNEL_DISCONNECTED);
		cain_sip_channel_close(obj);
		return CAIN_SIP_STOP;
	} else {
		cain_sip_error("Receive error on channel [%p]",obj);
		channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
		cain_sip_channel_close(obj);
		return CAIN_SIP_STOP;
	}
	return CAIN_SIP_CONTINUE;
}


void cain_sip_channel_init(cain_sip_channel_t *obj, cain_sip_stack_t *stack,const char *bindip,int localport,const char *peername, int peer_port){
	obj->peer_name=cain_sip_strdup(peername);
	obj->peer_port=peer_port;
	obj->peer=NULL;
	obj->stack=stack;
	if (strcmp(bindip,"::0")!=0 && strcmp(bindip,"0.0.0.0")!=0)
		obj->local_ip=cain_sip_strdup(bindip);
	obj->local_port=localport;
	obj->recv_error=1;/*not set*/
	cain_sip_channel_input_stream_reset(&obj->input_stream);
}

void cain_sip_channel_set_socket(cain_sip_channel_t *obj, cain_sip_socket_t sock, cain_sip_source_func_t datafunc){
	cain_sip_socket_source_init((cain_sip_source_t*)obj
									, datafunc
									, obj
									, sock
									, CAIN_SIP_EVENT_READ|CAIN_SIP_EVENT_WRITE
									, -1);
}

void cain_sip_channel_add_listener(cain_sip_channel_t *obj, cain_sip_channel_listener_t *l){
	obj->listeners=cain_sip_list_append(obj->listeners,
	                cain_sip_object_weak_ref(l,
	                (cain_sip_object_destroy_notify_t)cain_sip_channel_remove_listener,obj));
}

void cain_sip_channel_remove_listener(cain_sip_channel_t *obj, cain_sip_channel_listener_t *l){
	cain_sip_object_weak_unref(l,(cain_sip_object_destroy_notify_t)cain_sip_channel_remove_listener,obj);
	obj->listeners=cain_sip_list_remove(obj->listeners,l);
}

int cain_sip_channel_matches(const cain_sip_channel_t *obj, const char *peername, int peerport, const struct addrinfo *addr){
	if (peername && strcmp(peername,obj->peer_name)==0 && peerport==obj->peer_port)
		return 1;
	if (addr && obj->peer) 
		return addr->ai_addrlen==obj->peer->ai_addrlen && memcmp(addr->ai_addr,obj->peer->ai_addr,addr->ai_addrlen)==0;
	return 0;
}

const char *cain_sip_channel_get_local_address(cain_sip_channel_t *obj, int *port){
	if (port) *port=obj->local_port;
	return obj->local_ip;
}

int cain_sip_channel_is_reliable(const cain_sip_channel_t *obj){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->reliable;
}

const char * cain_sip_channel_get_transport_name_lower_case(const cain_sip_channel_t *obj){
	const char* transport = cain_sip_channel_get_transport_name(obj);
	if (strcasecmp("udp",transport)==0) return "udp";
	else if (strcasecmp("tcp",transport)==0) return "tcp";
	else if (strcasecmp("tls",transport)==0) return "tls";
	else if (strcasecmp("dtls",transport)==0) return "dtls";
	else {
		cain_sip_message("Cannot convert [%s] to lower case",transport);
		return transport;
	}
}

const char * cain_sip_channel_get_transport_name(const cain_sip_channel_t *obj){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->transport;
}

int cain_sip_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->channel_send(obj,buf,buflen);
}

int cain_sip_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->channel_recv(obj,buf,buflen);
}

void cain_sip_channel_close(cain_sip_channel_t *obj){
	if (CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->close)
		CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->close(obj); /*udp channel don't have close function*/
}

const struct addrinfo * cain_sip_channel_get_peer(cain_sip_channel_t *obj){
	return obj->peer;
}

cain_sip_message_t* cain_sip_channel_pick_message(cain_sip_channel_t *obj) {
	cain_sip_message_t* result=NULL;
	cain_sip_list_t* front;
	if ((front=obj->incoming_messages)==NULL) {
		cain_sip_error("Cannot pickup incoming message, empty list");
	} else {
		result = (cain_sip_message_t*)obj->incoming_messages->data;
		obj->incoming_messages=cain_sip_list_remove_link(obj->incoming_messages,obj->incoming_messages);
		cain_sip_free(front);
	}
	return result;
}

static void channel_invoke_state_listener(cain_sip_channel_t *obj){
	cain_sip_list_t* list=cain_sip_list_copy(obj->listeners); /*copy list because error state alter this list (I.E by provider)*/
	CAIN_SIP_INVOKE_LISTENERS_ARG1_ARG2(list,cain_sip_channel_listener_t,on_state_changed,obj,obj->state);
	cain_sip_list_free(list);
}

void channel_set_state(cain_sip_channel_t *obj, cain_sip_channel_state_t state) {
	cain_sip_message("channel %p: state %s",obj,cain_sip_channel_state_to_string(state));
	obj->state=state;
	if (state==CAIN_SIP_CHANNEL_ERROR){
		/*Because error notification will in practice trigger the destruction of possible transactions and this channel,
		 * it is safer to invoke the listener outside the current call stack.
		 * Indeed the channel encounters network errors while being called for transmiting by a transaction.
		 */
		cain_sip_main_loop_do_later(obj->stack->ml,(cain_sip_callback_t)channel_invoke_state_listener,obj);
	}else
		channel_invoke_state_listener(obj);
}

static void _send_message(cain_sip_channel_t *obj, cain_sip_message_t *msg){
	char buffer[cain_sip_network_buffer_size];
	int len;
	int ret=0;
	
	CAIN_SIP_INVOKE_LISTENERS_ARG1_ARG2(obj->listeners,cain_sip_channel_listener_t,on_sending,obj,msg);
	len=cain_sip_object_marshal((cain_sip_object_t*)msg,buffer,0,sizeof(buffer));
	if (len>0){
		if (!obj->stack->send_error)
			ret=cain_sip_channel_send(obj,buffer,len);
		else
			/*debug case*/
			ret=obj->stack->send_error;

		if (ret<0){
			cain_sip_error("channel [%p]: could not send [%i] bytes from [%s://%s:%i]  to [%s:%i]"	,obj
				,len
				,cain_sip_channel_get_transport_name(obj)
				,obj->local_ip
				,obj->local_port
				,obj->peer_name
				,obj->peer_port);
			channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
			cain_sip_channel_close(obj);
		}else{
			cain_sip_message("channel [%p]: message sent to [%s://%s:%i] \n%s"
								,obj
								,cain_sip_channel_get_transport_name(obj)
								,obj->peer_name
								,obj->peer_port
								,buffer);
		}
	}
}

/* just to emulate network transmission delay */

typedef struct delayed_send{
	cain_sip_channel_t *chan;
	cain_sip_message_t *msg;
}delayed_send_t;

static int on_delayed_send_do(delayed_send_t *ds){
	cain_sip_message("on_delayed_send_do(): sending now");
	if (ds->chan->state==CAIN_SIP_CHANNEL_READY){
		_send_message(ds->chan,ds->msg);
	}
	cain_sip_object_unref(ds->chan);
	cain_sip_object_unref(ds->msg);
	cain_sip_free(ds);
	return FALSE;
}

static void send_message(cain_sip_channel_t *obj, cain_sip_message_t *msg){
	if (obj->stack->tx_delay>0){
		delayed_send_t *ds=cain_sip_new(delayed_send_t);
		ds->chan=(cain_sip_channel_t*)cain_sip_object_ref(obj);
		ds->msg=(cain_sip_message_t*)cain_sip_object_ref(msg);
		cain_sip_main_loop_add_timeout(obj->stack->ml,(cain_sip_source_func_t)on_delayed_send_do,ds,obj->stack->tx_delay);
		cain_sip_message("channel %p: message sending delayed by %i ms",obj,obj->stack->tx_delay);
	}else _send_message(obj,msg);
}

void cain_sip_channel_prepare(cain_sip_channel_t *obj){
	channel_prepare_continue(obj);
}

static void channel_push_outgoing(cain_sip_channel_t *obj, cain_sip_message_t *msg){
	obj->outgoing_messages=cain_sip_list_append(obj->outgoing_messages,msg);
}

static cain_sip_message_t *channel_pop_outgoing(cain_sip_channel_t *obj){
	cain_sip_message_t *msg=NULL;
	if (obj->outgoing_messages){
		msg=(cain_sip_message_t*)obj->outgoing_messages->data;
		obj->outgoing_messages=cain_sip_list_delete_link(obj->outgoing_messages,obj->outgoing_messages);
	}
	return msg;
}

static void channel_prepare_continue(cain_sip_channel_t *obj){
	switch(obj->state){
		case CAIN_SIP_CHANNEL_INIT:
			cain_sip_channel_resolve(obj);
		break;
		case CAIN_SIP_CHANNEL_RES_DONE:
			cain_sip_channel_connect(obj);
		break;
		case CAIN_SIP_CHANNEL_READY:
			channel_process_queue(obj);
		break;
		default:
		break;
	}
}

static void channel_process_queue(cain_sip_channel_t *obj){
	cain_sip_message_t *msg;
	cain_sip_object_ref(obj);/* we need to ref ourself because code below may trigger our destruction*/

	while((msg=channel_pop_outgoing(obj))!=NULL) {
		send_message(obj, msg);
		cain_sip_object_unref(msg);
	}

	cain_sip_object_unref(obj);
}

void cain_sip_channel_set_ready(cain_sip_channel_t *obj, const struct sockaddr *addr, socklen_t slen){
	char name[NI_MAXHOST];
	char serv[NI_MAXSERV];

	if (obj->local_ip==NULL){
		int err=getnameinfo(addr,slen,name,sizeof(name),serv,sizeof(serv),NI_NUMERICHOST|NI_NUMERICSERV);
		if (err!=0){
			cain_sip_error("cain_sip_channel_connect(): getnameinfo() failed: %s",gai_strerror(err));
		}else{
			obj->local_ip=cain_sip_strdup(name);
			obj->local_port=atoi(serv);
			cain_sip_message("Channel has local address %s:%s",name,serv);
		}
	}
	channel_set_state(obj,CAIN_SIP_CHANNEL_READY);
	channel_process_queue(obj);
}

static void channel_res_done(void *data, const char *name, struct addrinfo *res){
	cain_sip_channel_t *obj=(cain_sip_channel_t*)data;
	obj->resolver_id=0;
	if (res){
		obj->peer=res;
		channel_set_state(obj,CAIN_SIP_CHANNEL_RES_DONE);
		channel_prepare_continue(obj);
	}else{
		channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
	}
}

void cain_sip_channel_resolve(cain_sip_channel_t *obj){
	channel_set_state(obj,CAIN_SIP_CHANNEL_RES_IN_PROGRESS);
	obj->resolver_id=cain_sip_resolve(obj->stack, obj->peer_name, obj->peer_port, obj->lp->ai_family, channel_res_done, obj, obj->stack->ml);
	return ;
}

void cain_sip_channel_connect(cain_sip_channel_t *obj){
	channel_set_state(obj,CAIN_SIP_CHANNEL_CONNECTING);
	if(CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->connect(obj,obj->peer)) {
		cain_sip_error("Cannot connect to [%s://%s:%i]",cain_sip_channel_get_transport_name(obj),obj->peer_name,obj->peer_port);
		channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
	}
	return;
}

int cain_sip_channel_queue_message(cain_sip_channel_t *obj, cain_sip_message_t *msg){
	cain_sip_object_ref(msg);
	channel_push_outgoing(obj,msg);
	if (obj->state==CAIN_SIP_CHANNEL_INIT){
		cain_sip_channel_prepare(obj);
	}else if (obj->state==CAIN_SIP_CHANNEL_READY) {
		channel_process_queue(obj);
	}		
	return 0;
}

void cain_sip_channel_force_close(cain_sip_channel_t *obj){
	obj->force_close=1;
	/*first, every existing channel must be set to error*/
	channel_set_state(obj,CAIN_SIP_CHANNEL_DISCONNECTED);
	cain_sip_channel_close(obj);
}


