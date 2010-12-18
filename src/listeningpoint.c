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

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#else

#endif

/*
 Channels: udp
*/

struct cain_sip_channel{
	cain_sip_object_t base;
	struct addrinfo peer;
	struct sockaddr_storage peer_addr;
};

typedef struct cain_sip_channel_vptr{
	int (*channel_send)(cain_sip_channel_t *obj, const void *buf, size_t buflen);
	int (*channel_recv)(cain_sip_channel_t *obj, void *buf, size_t buflen);
}cain_sip_channel_vptr_t;

static void cain_sip_channel_init(cain_sip_channel_t *obj){
	cain_sip_object_init_type(obj,cain_sip_channel_t);
	obj->peer.ai_addr=(struct sockaddr*)&obj->peer_addr;
}

int cain_sip_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_vptr_t)->channel_send(obj,buf,buflen);
}

int cain_sip_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	return CAIN_SIP_OBJECT_VPTR(obj,cain_sip_channel_vptr_t)->channel_recv(obj,buf,buflen);
}

const struct addrinfo * cain_sip_channel_get_peer(cain_sip_channel_t *obj){
	return &obj->peer;
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
	err=sendto(chan->sock,buf,buflen,0,obj->peer.ai_addr,obj->peer.ai_addrlen);
	if (err==-1){
		cain_sip_error("Could not send UDP packet: %s",strerror(errno));
	}
	return err;
}

static int udp_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	cain_sip_udp_channel_t *chan=(cain_sip_udp_channel_t *)obj;
	int err;
	obj->peer.ai_addrlen=sizeof(obj->peer_addr);
	err=recvfrom(chan->sock,buf,buflen,MSG_DONTWAIT,obj->peer.ai_addr,&obj->peer.ai_addrlen);
	if (err==-1 && errno!=EWOULDBLOCK){
		cain_sip_error("Could not receive UDP packet: %s",strerror(errno));
	}
	return err;
}

void cain_sip_udp_channel_set_dest(cain_sip_udp_channel_t *chan, const struct addrinfo *ai){
	memcpy(chan->base.peer.ai_addr,ai->ai_addr,ai->ai_addrlen);
	chan->base.peer.ai_addrlen=ai->ai_addrlen;
}

static cain_sip_channel_vptr_t udp_channel_vptr={
	udp_channel_send,
	udp_channel_recv
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

cain_sip_udp_channel_t *cain_sip_udp_channel_new(const char *addr, int port){
	cain_sip_udp_channel_t *obj=cain_sip_object_new_with_vptr(cain_sip_udp_channel_t,&udp_channel_vptr,udp_channel_uninit);
	cain_sip_channel_init((cain_sip_channel_t*)obj);
	obj->sock=create_udp_socket(addr,port);
	if (obj->sock==-1){
		cain_sip_object_unref(obj);
		return NULL;
	}
	return obj;
}

/*
 Listening points: base, udp
*/

struct cain_sip_listening_point{
	cain_sip_object_t base;
	cain_sip_stack_t *stack;
	char *transport;
	char *addr;
	int port;
};

typedef struct cain_sip_listening_point_vptr{
	cain_sip_channel_t * (*find_output_channel)(cain_sip_listening_point_t *lp,const struct addrinfo *dest);
} cain_sip_listening_point_vptr_t;

static void cain_sip_listening_point_init(cain_sip_listening_point_t *lp, cain_sip_stack_t *s, const char *transport, const char *address, int port){
	cain_sip_object_init_type(lp,cain_sip_listening_point_t);
	lp->transport=cain_sip_strdup(transport);
	lp->port=port;
	lp->addr=cain_sip_strdup(address);
	lp->stack=s;
}

static void cain_sip_listening_point_uninit(cain_sip_listening_point_t *lp){
	cain_sip_free(lp->addr);
	cain_sip_free(lp->transport);
}

const char *cain_sip_listening_point_get_ip_address(const cain_sip_listening_point_t *lp){
	return lp->addr;
}

int cain_sip_listening_point_get_port(const cain_sip_listening_point_t *lp){
	return lp->port;
}

const char *cain_sip_listening_point_get_transport(const cain_sip_listening_point_t *lp){
	return lp->transport;
}

cain_sip_channel_t *cain_sip_listening_point_find_output_channel (cain_sip_listening_point_t *lp,const struct addrinfo *dest){
	return ((cain_sip_listening_point_vptr_t*)lp->base.vptr)->find_output_channel(lp,dest);
}

struct cain_sip_udp_listening_point{
	cain_sip_listening_point_t base;
	cain_sip_udp_channel_t *channel;
};


static cain_sip_channel_t *udp_listening_point_find_output_channel(cain_sip_listening_point_t* obj,const struct addrinfo *dest){
	cain_sip_udp_listening_point_t *lp=(cain_sip_udp_listening_point_t*)obj;
	cain_sip_udp_channel_set_dest(lp->channel,dest);
	return CAIN_SIP_CHANNEL(lp);
}

static void cain_sip_udp_listening_point_uninit(cain_sip_udp_listening_point_t *lp){
	cain_sip_object_unref(lp->channel);
	cain_sip_listening_point_uninit((cain_sip_listening_point_t*)lp);
}

static cain_sip_listening_point_vptr_t udp_listening_point_vptr={
	udp_listening_point_find_output_channel
};


cain_sip_listening_point_t * cain_sip_udp_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port){
	cain_sip_udp_listening_point_t *lp=cain_sip_object_new_with_vptr(cain_sip_udp_listening_point_t,&udp_listening_point_vptr,cain_sip_udp_listening_point_uninit);
	cain_sip_listening_point_init((cain_sip_listening_point_t*)lp,s,"UDP",ipaddress,port);
	lp->channel=cain_sip_udp_channel_new(ipaddress,port);
	if (lp->channel==NULL){
		cain_sip_object_unref(lp);
		lp=NULL;
	}
	return CAIN_SIP_LISTENING_POINT(lp);
}



