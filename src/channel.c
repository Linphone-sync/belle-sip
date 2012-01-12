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



static void cain_sip_channel_destroy(cain_sip_channel_t *obj){
	if (obj->peer) freeaddrinfo(obj->peer);
	cain_sip_free(obj->peer_name);
}

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_channel_t)=
{
	{
		CAIN_SIP_VPTR_INIT(cain_sip_channel_t,cain_sip_source_t),
		(cain_sip_object_destroy_t)cain_sip_channel_destroy,
		NULL, /*clone*/
		NULL, /*marshall*/
	}
};


static void cain_sip_channel_init(cain_sip_channel_t *obj, cain_sip_provider_t *prov, const char *peername, int peer_port){
	obj->peer_name=cain_sip_strdup(peername);
	obj->peer_port=peer_port;
	obj->peer=NULL;
	obj->prov=prov;
}

int cain_sip_channel_matches(const cain_sip_channel_t *obj, const char *peername, int peerport){
	return strcmp(peername,obj->peer_name)==0 && peerport==obj->peer_port;
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


static void send_message(cain_sip_channel_t *obj, cain_sip_message_t *msg){
	char buffer[cain_sip_network_buffer_size];
	int len=cain_sip_object_marshal((cain_sip_object_t*)msg,buffer,0,sizeof(buffer));
	if (len>0){
		int ret=cain_sip_channel_send(obj,buffer,len);
		if (ret==-1){
			cain_sip_io_error_event_t ev;
			obj->state=CAIN_SIP_CHANNEL_ERROR;
			ev.transport=CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->transport;
			ev.source=obj->prov;
			ev.port=obj->peer_port;
			ev.host=obj->peer_name;
			CAIN_SIP_PROVIDER_INVOKE_LISTENERS(obj->prov,process_io_error,&ev);
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

static void channel_res_done(void *data, const char *name, struct addrinfo *res){
	cain_sip_channel_t *obj=(cain_sip_channel_t*)data;
	obj->resolver_id=0;
	if (res){
		obj->peer=res;
		obj->state=CAIN_SIP_CHANNEL_RES_DONE;
	}else{
		cain_sip_io_error_event_t ev;
		obj->state=CAIN_SIP_CHANNEL_ERROR;
		ev.transport=CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->transport;
		ev.source=obj->prov;
		ev.port=obj->peer_port;
		ev.host=obj->peer_name;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(obj->prov,process_io_error,&ev);
	}
	channel_process_queue(obj);
}

int cain_sip_channel_resolve(cain_sip_channel_t *obj){
	obj->state=CAIN_SIP_CHANNEL_RES_IN_PROGRESS;
	obj->resolver_id=cain_sip_resolve(obj->peer_name, obj->peer_port, 0, channel_res_done, obj, obj->prov->stack->ml);
	return 0;
}

int cain_sip_channel_connect(cain_sip_channel_t *obj){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_t)->connect(obj,obj->peer->ai_addr,obj->peer->ai_addrlen);
}

int cain_sip_channel_queue_message(cain_sip_channel_t *obj, cain_sip_message_t *msg){
	if (msg!=NULL){
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
	obj->state=CAIN_SIP_CHANNEL_READY;
	channel_process_queue(obj);
	return 0;
}

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_udp_channel_t)=
{
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_udp_channel_t,cain_sip_channel_t),
			(cain_sip_object_destroy_t)udp_channel_uninit,
			NULL,
			NULL
		},
		"UDP",
		udp_channel_connect,
		udp_channel_send,
		udp_channel_recv
	}
};

static int create_udp_socket(const char *addr, int port){
	struct addrinfo hints={0};
	struct addrinfo *res=NULL;
	int err;
	int sock;
	char portnum[10];
	
	sock=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (sock==-1){
		cain_sip_error("Cannot create UDP socket: %s",strerror(errno));
		return -1;
	}
	snprintf(portnum,sizeof(portnum),"%i",port);
	err=getaddrinfo(addr,portnum,&hints,&res);
	if (err!=0){
		cain_sip_error("getaddrinfo() failed for %s port %i: %s",addr,port,strerror(errno));
		close(sock);
		return -1;
	}
	err=bind(sock,res->ai_addr,res->ai_addrlen);
	if (err==-1){
		cain_sip_error("udp bind() failed for %s port %i: %s",addr,port,strerror(errno));
		close(sock);
		return -1;
	}
	return sock;
}

cain_sip_channel_t * cain_sip_channel_new_udp_master(cain_sip_provider_t *prov, const char *localname, int localport){
	cain_sip_udp_channel_t *obj=cain_sip_object_new(cain_sip_udp_channel_t);
	cain_sip_channel_init((cain_sip_channel_t*)obj,prov,"",-1);
	obj->sock=create_udp_socket(localname, localport);
	return (cain_sip_channel_t*)obj;
}

cain_sip_channel_t * cain_sip_channel_new_udp_slave(cain_sip_channel_t *master, const char *peername, int peerport){
	cain_sip_udp_channel_t *obj=cain_sip_object_new(cain_sip_udp_channel_t);
	cain_sip_channel_init((cain_sip_channel_t*)obj,master->prov,peername,peerport);
	obj->sock=((cain_sip_udp_channel_t*)master)->sock;
	return (cain_sip_channel_t*)obj;
}


