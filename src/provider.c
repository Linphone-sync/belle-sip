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



static void cain_sip_provider_uninit(cain_sip_provider_t *p){
	cain_sip_list_free(p->listeners);
	cain_sip_list_free(p->lps);
}

static void channel_state_changed(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, cain_sip_channel_state_t state){
}

static void cain_sip_provider_dispatch_message(cain_sip_provider_t *prov, cain_sip_message_t *msg){
	/*should find existing transaction*/

	if (cain_sip_message_is_request(msg)){
		cain_sip_request_event_t event;
		event.source=prov;
		event.server_transaction=NULL;
		event.request=(cain_sip_request_t*)msg;
		event.dialog=NULL;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov,process_request_event,&event);
	}else{
		cain_sip_response_event_t event;
		event.source=prov;
		event.client_transaction=NULL;
		event.dialog=NULL;
		event.response=(cain_sip_response_t*)msg;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov,process_response_event,&event);
	}
}

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
		cain_sip_header_via_set_received(via,received);
		cain_sip_header_via_set_rport(via,atoi(rport));
	}
}

static void fix_outgoing_via(cain_sip_provider_t *p, cain_sip_channel_t *chan, cain_sip_message_t *msg){
	cain_sip_header_via_t *via=CAIN_SIP_HEADER_VIA(cain_sip_message_get_header(msg,"via"));
	cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(via),"rport",NULL);
	if (cain_sip_header_via_get_host(via)==NULL){
		const char *local_ip;
		int local_port;
		local_ip=cain_sip_channel_get_local_address(chan,&local_port);
		cain_sip_header_via_set_host(via,local_ip);
		cain_sip_header_via_set_port(via,local_port);
		cain_sip_header_via_set_protocol(via,"SIP/2.0");
		cain_sip_header_via_set_transport(via,cain_sip_channel_get_transport_name(chan));
	}
	if (cain_sip_header_via_get_branch(via)==NULL){
		char *branchid=cain_sip_strdup_printf(CAIN_SIP_BRANCH_MAGIC_COOKIE "%x",cain_sip_random());
		cain_sip_header_via_set_branch(via,branchid);
		cain_sip_free(branchid);
	}
}

static void cain_sip_provider_read_message(cain_sip_provider_t *prov, cain_sip_channel_t *chan){
	char buffer[cain_sip_network_buffer_size];
	int err;
	err=cain_sip_channel_recv(chan,buffer,sizeof(buffer));
	if (err>0){
		cain_sip_message_t *msg;
		buffer[err]='\0';
		cain_sip_message("provider %p read message from %s:%i\n%s",prov,chan->peer_name,chan->peer_port,buffer);
		msg=cain_sip_message_parse(buffer);
		if (msg){
			if (cain_sip_message_is_request(msg)) fix_incoming_via(CAIN_SIP_REQUEST(msg),chan->peer);
			cain_sip_provider_dispatch_message(prov,msg);
		}else{
			cain_sip_error("Could not parse this message.");
		}
	}
}

static int channel_on_event(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, unsigned int revents){
	if (revents & CAIN_SIP_EVENT_READ){
		cain_sip_provider_read_message(CAIN_SIP_PROVIDER(obj),chan);
	}
	return 0;
}

static void channel_on_sending(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, cain_sip_message_t *msg){
	fix_outgoing_via((cain_sip_provider_t*)obj,chan,msg);
}

CAIN_SIP_IMPLEMENT_INTERFACE_BEGIN(cain_sip_provider_t,cain_sip_channel_listener_t)
	channel_state_changed,
	channel_on_event,
	channel_on_sending
CAIN_SIP_IMPLEMENT_INTERFACE_END

CAIN_SIP_DECLARE_IMPLEMENTED_INTERFACES_1(cain_sip_provider_t,cain_sip_channel_listener_t);
	
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_provider_t,cain_sip_object_t,cain_sip_provider_uninit,NULL,NULL,FALSE);

cain_sip_provider_t *cain_sip_provider_new(cain_sip_stack_t *s, cain_sip_listening_point_t *lp){
	cain_sip_provider_t *p=cain_sip_object_new(cain_sip_provider_t);
	p->stack=s;
	cain_sip_provider_add_listening_point(p,lp);
	return p;
}

int cain_sip_provider_add_listening_point(cain_sip_provider_t *p, cain_sip_listening_point_t *lp){
	p->lps=cain_sip_list_append(p->lps,lp);
	return 0;
}

cain_sip_listening_point_t *cain_sip_provider_get_listening_point(cain_sip_provider_t *p, const char *transport){
	cain_sip_list_t *l;
	for(l=p->lps;l!=NULL;l=l->next){
		cain_sip_listening_point_t *lp=(cain_sip_listening_point_t*)l->data;
		if (strcasecmp(cain_sip_listening_point_get_transport(lp),transport)==0)
			return lp;
	}
	return NULL;
}

const cain_sip_list_t *cain_sip_provider_get_listening_points(cain_sip_provider_t *p){
	return p->lps;
}

void cain_sip_provider_add_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l){
	p->listeners=cain_sip_list_append(p->listeners,l);
}

void cain_sip_provider_remove_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l){
	p->listeners=cain_sip_list_remove(p->listeners,l);
}

cain_sip_header_call_id_t * cain_sip_provider_get_new_call_id(cain_sip_provider_t *prov){
	cain_sip_header_call_id_t *cid=cain_sip_header_call_id_new();
	char tmp[32];
	snprintf(tmp,sizeof(tmp),"%u",cain_sip_random());
	cain_sip_header_call_id_set_call_id(cid,tmp);
	return cid;
}

cain_sip_client_transaction_t *cain_sip_provider_get_new_client_transaction(cain_sip_provider_t *prov, cain_sip_request_t *req){
	if (strcmp(cain_sip_request_get_method(req),"INVITE")==0)
		return (cain_sip_client_transaction_t*)cain_sip_ict_new(prov,req);
	else 
		return (cain_sip_client_transaction_t*)cain_sip_nict_new(prov,req);
}

cain_sip_server_transaction_t *cain_sip_provider_get_new_server_transaction(cain_sip_provider_t *prov, cain_sip_request_t *req){
	if (strcmp(cain_sip_request_get_method(req),"INVITE")==0)
		return (cain_sip_server_transaction_t*)cain_sip_ist_new(prov,req);
	else 
		return (cain_sip_server_transaction_t*)cain_sip_nist_new(prov,req);
}

cain_sip_stack_t *cain_sip_provider_get_sip_stack(cain_sip_provider_t *p){
	return p->stack;
}

cain_sip_channel_t * cain_sip_provider_get_channel(cain_sip_provider_t *p, const char *name, int port, const char *transport){
	cain_sip_list_t *l;
	cain_sip_listening_point_t *candidate=NULL,*lp;
	cain_sip_channel_t *chan;
	for(l=p->lps;l!=NULL;l=l->next){
		lp=(cain_sip_listening_point_t*)l->data;
		if (strcasecmp(cain_sip_listening_point_get_transport(lp),transport)==0){
			chan=cain_sip_listening_point_get_channel(lp,name,port);
			if (chan) return chan;
			candidate=lp;
		}
	}
	if (candidate){
		chan=cain_sip_listening_point_create_channel(candidate,name,port);
		if (chan==NULL) cain_sip_error("Could not create channel to %s:%s:%i",transport,name,port);
		else cain_sip_channel_add_listener(chan,CAIN_SIP_CHANNEL_LISTENER(p));
		return chan;
	}
	cain_sip_error("No listening point matching for transport %s",transport);
	return NULL;
}

void cain_sip_provider_send_request(cain_sip_provider_t *p, cain_sip_request_t *req){
	cain_sip_hop_t hop={0};
	cain_sip_channel_t *chan;
	cain_sip_stack_get_next_hop(p->stack,req,&hop);
	chan=cain_sip_provider_get_channel(p,hop.host, hop.port, hop.transport);
	if (chan) {
		cain_sip_channel_queue_message(chan,CAIN_SIP_MESSAGE(req));
	}
}

void cain_sip_provider_send_response(cain_sip_provider_t *p, cain_sip_response_t *resp){
	cain_sip_hop_t hop;
	cain_sip_channel_t *chan;
	cain_sip_response_get_return_hop(resp,&hop);
	chan=cain_sip_provider_get_channel(p,hop.host, hop.port, hop.transport);
	if (chan) cain_sip_channel_queue_message(chan,CAIN_SIP_MESSAGE(resp));
}

/*private provider API*/

void cain_sip_provider_set_transaction_terminated(cain_sip_provider_t *p, cain_sip_transaction_t *t){
	cain_sip_transaction_terminated_event_t ev;
	ev.source=p;
	ev.transaction=t;
	ev.is_server_transaction=CAIN_SIP_IS_INSTANCE_OF(t,cain_sip_server_transaction_t);
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS(p,process_transaction_terminated,&ev);
}

