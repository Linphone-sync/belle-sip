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

static const char *channel_state_to_string(cain_sip_channel_state_t state){
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
	}
	return "BAD";
}

static void cain_sip_channel_destroy(cain_sip_channel_t *obj){
	if (obj->peer) freeaddrinfo(obj->peer);
	cain_sip_free(obj->peer_name);
	if (obj->local_ip) cain_sip_free(obj->local_ip);
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

void cain_sip_channel_process_data(cain_sip_channel_t *obj,unsigned int revents){
	CAIN_SIP_INVOKE_LISTENERS_ARG1_ARG2(obj->listeners,cain_sip_channel_listener_t,on_event,obj,revents);
}

static void cain_sip_channel_init(cain_sip_channel_t *obj, cain_sip_stack_t *stack, int fd, const char *bindip,int localport,const char *peername, int peer_port){
	obj->peer_name=cain_sip_strdup(peername);
	obj->peer_port=peer_port;
	obj->peer=NULL;
	obj->stack=stack;
	if (strcmp(bindip,"::0")!=0 && strcmp(bindip,"0.0.0.0")!=0)
		obj->local_ip=cain_sip_strdup(bindip);
	obj->local_port=localport;
		
	cain_sip_fd_source_init((cain_sip_source_t*)obj,(cain_sip_source_func_t)cain_sip_channel_process_data,obj,fd,CAIN_SIP_EVENT_READ|CAIN_SIP_EVENT_ERROR,-1);
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

const char * cain_sip_channel_get_transport_name(const cain_sip_channel_t *obj){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->transport;
}

int cain_sip_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->channel_send(obj,buf,buflen);
}

int cain_sip_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->channel_recv(obj,buf,buflen);
}

const struct addrinfo * cain_sip_channel_get_peer(cain_sip_channel_t *obj){
	return obj->peer;
}



static void channel_set_state(cain_sip_channel_t *obj, cain_sip_channel_state_t state){
	cain_sip_message("channel %p: state %s",obj,channel_state_to_string(state));
	obj->state=state;
	CAIN_SIP_INVOKE_LISTENERS_ARG1_ARG2(obj->listeners,cain_sip_channel_listener_t,on_state_changed,obj,state);
}

static void send_message(cain_sip_channel_t *obj, cain_sip_message_t *msg){
	char buffer[cain_sip_network_buffer_size];
	int len;
	CAIN_SIP_INVOKE_LISTENERS_ARG1_ARG2(obj->listeners,cain_sip_channel_listener_t,on_sending,obj,msg);
	len=cain_sip_object_marshal((cain_sip_object_t*)msg,buffer,0,sizeof(buffer));
	if (len>0){
		int ret=cain_sip_channel_send(obj,buffer,len);
		if (ret==-1){
			channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
		}else{
			cain_sip_message("channel %p: message sent: \n%s",obj,buffer);
		}
	}
}


static void channel_process_queue(cain_sip_channel_t *obj){
	if (obj->msg){
		switch(obj->state){
			case CAIN_SIP_CHANNEL_INIT:
				cain_sip_channel_resolve(obj);
			break;
			case CAIN_SIP_CHANNEL_RES_DONE:
				cain_sip_channel_connect(obj);
			break;
			case CAIN_SIP_CHANNEL_READY:
				send_message(obj, obj->msg);
			case CAIN_SIP_CHANNEL_ERROR:
				cain_sip_object_unref(obj->msg);
				obj->msg=NULL;
			break;
			default:
			break;
		}
	}
}

static void cain_sip_channel_set_ready(cain_sip_channel_t *obj, const struct sockaddr *addr, socklen_t slen){
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
	}else{
		channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
	}
	channel_process_queue(obj);
}

int cain_sip_channel_resolve(cain_sip_channel_t *obj){
	channel_set_state(obj,CAIN_SIP_CHANNEL_RES_IN_PROGRESS);
	obj->resolver_id=cain_sip_resolve(obj->peer_name, obj->peer_port, 0, channel_res_done, obj, obj->stack->ml);
	return 0;
}

int cain_sip_channel_connect(cain_sip_channel_t *obj){
	int ret=CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->connect(obj,obj->peer->ai_addr,obj->peer->ai_addrlen);
	return ret;
}

int cain_sip_channel_queue_message(cain_sip_channel_t *obj, cain_sip_message_t *msg){
	if (obj->msg!=NULL){
		cain_sip_error("Queue is not a queue.");
		return -1;
	}
	obj->msg=(cain_sip_message_t*)cain_sip_object_ref(msg);
	channel_process_queue(obj);
	return 0;
}

struct cain_sip_udp_channel{
	cain_sip_channel_t base;
	int sock;
};

typedef struct cain_sip_udp_channel cain_sip_udp_channel_t;

static void udp_channel_uninit(cain_sip_udp_channel_t *obj){
	if (obj->sock!=-1)
		close(obj->sock);
}

static int udp_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	cain_sip_udp_channel_t *chan=(cain_sip_udp_channel_t *)obj;
	int err;
	err=sendto(chan->sock,buf,buflen,0,obj->peer->ai_addr,obj->peer->ai_addrlen);
	if (err==-1){
		cain_sip_fatal("Could not send UDP packet: %s",strerror(errno));
		return -errno;
	}
	return err;
}

static int udp_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	cain_sip_udp_channel_t *chan=(cain_sip_udp_channel_t *)obj;
	int err;
	struct sockaddr_storage addr;
	socklen_t addrlen=sizeof(addr);
	err=recvfrom(chan->sock,buf,buflen,MSG_DONTWAIT,(struct sockaddr*)&addr,&addrlen);
	if (err==-1 && errno!=EWOULDBLOCK){
		cain_sip_error("Could not receive UDP packet: %s",strerror(errno));
		return -errno;
	}
	return err;
}

int udp_channel_connect(cain_sip_channel_t *obj, const struct sockaddr *addr, socklen_t socklen){
	struct sockaddr_storage laddr;
	socklen_t lslen=sizeof(laddr);
	if (obj->local_ip==NULL){
		cain_sip_get_src_addr_for(addr,socklen,(struct sockaddr*)&laddr,&lslen);
		if (lslen==sizeof(struct sockaddr_in6)){
			struct sockaddr_in6 *sin6=(struct sockaddr_in6*)&laddr;
			sin6->sin6_port=htons(obj->local_port);
		}else{
			struct sockaddr_in *sin=(struct sockaddr_in*)&laddr;
			sin->sin_port=htons(obj->local_port);
		}
	}
	cain_sip_channel_set_ready(obj,(struct sockaddr*)&laddr,lslen);
	return 0;
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_udp_channel_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_udp_channel_t)=
{
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_udp_channel_t,cain_sip_channel_t,FALSE),
			(cain_sip_object_destroy_t)udp_channel_uninit,
			NULL,
			NULL
		},
		"UDP",
		0, /*is_reliable*/
		udp_channel_connect,
		udp_channel_send,
		udp_channel_recv
	}
};

cain_sip_channel_t * cain_sip_channel_new_udp(cain_sip_stack_t *stack, int sock, const char *bindip, int localport, const char *dest, int port){
	cain_sip_udp_channel_t *obj=cain_sip_object_new(cain_sip_udp_channel_t);
	cain_sip_channel_init((cain_sip_channel_t*)obj,stack,sock,bindip,localport,dest,port);
	obj->sock=sock;
	return (cain_sip_channel_t*)obj;
}

cain_sip_channel_t * cain_sip_channel_new_udp_with_addr(cain_sip_stack_t *stack, int sock, const char *bindip, int localport, const struct addrinfo *peer){
	cain_sip_udp_channel_t *obj=cain_sip_object_new(cain_sip_udp_channel_t);
	struct addrinfo *ai=cain_sip_new0(struct addrinfo);
	char name[NI_MAXHOST];
	char serv[NI_MAXSERV];
	int err;
	
	obj->sock=sock;
	*ai=*peer;
	err=getnameinfo(ai->ai_addr,ai->ai_addrlen,name,sizeof(name),serv,sizeof(serv),NI_NUMERICHOST|NI_NUMERICSERV);
	if (err!=0){
		cain_sip_error("cain_sip_channel_new_udp_with_addr(): getnameinfo() failed: %s",gai_strerror(err));
		cain_sip_object_unref(obj);
		return NULL;
	}
	cain_sip_channel_init((cain_sip_channel_t*)obj,stack,sock,bindip,localport,name,atoi(serv));
	return (cain_sip_channel_t*)obj;
}



