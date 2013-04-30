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
#include "channel.h"

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_udp_channel_t,cain_sip_channel_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

struct cain_sip_udp_channel{
	cain_sip_channel_t base;
	int sock;
};

typedef struct cain_sip_udp_channel cain_sip_udp_channel_t;

static void udp_channel_uninit(cain_sip_udp_channel_t *obj){

}

static int udp_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	cain_sip_udp_channel_t *chan=(cain_sip_udp_channel_t *)obj;
	int err;
	err=sendto(chan->sock,buf,buflen,0,obj->current_peer->ai_addr,obj->current_peer->ai_addrlen);
	if (err==-1){
		cain_sip_error("channel [%p]: could not send UDP packet because [%s]",obj,cain_sip_get_socket_error_string());
		return -errno;
	}
	return err;
}

static int udp_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	cain_sip_udp_channel_t *chan=(cain_sip_udp_channel_t *)obj;
	int err;
	struct sockaddr_storage addr;
	socklen_t addrlen=sizeof(addr);
	err=recvfrom(chan->sock,buf,buflen,0,(struct sockaddr*)&addr,&addrlen);

	if (err==-1 && get_socket_error()!=CAINSIP_EWOULDBLOCK){
		cain_sip_error("Could not receive UDP packet: %s",cain_sip_get_socket_error_string());
		return -errno;
	}
	return err;
}

int udp_channel_connect(cain_sip_channel_t *obj, const struct addrinfo *ai){
	struct sockaddr_storage laddr;
	socklen_t lslen=sizeof(laddr);
	if (obj->local_ip==NULL){
		cain_sip_get_src_addr_for(ai->ai_addr,ai->ai_addrlen,(struct sockaddr*)&laddr,&lslen,obj->local_port);
		cain_sip_address_remove_v4_mapping((struct sockaddr*)&laddr,(struct sockaddr*)&laddr,&lslen);
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
	cain_sip_channel_init((cain_sip_channel_t*)obj,stack,bindip,localport,NULL,dest,port);
	obj->sock=sock;
	return (cain_sip_channel_t*)obj;
}

cain_sip_channel_t * cain_sip_channel_new_udp_with_addr(cain_sip_stack_t *stack, int sock, const char *bindip, int localport, const struct addrinfo *peer){
	cain_sip_udp_channel_t *obj=cain_sip_object_new(cain_sip_udp_channel_t);
	struct addrinfo ai,hints={0};
	char name[NI_MAXHOST];
	char serv[NI_MAXSERV];
	int err;

	obj->sock=sock;
	ai=*peer;
	err=getnameinfo(ai.ai_addr,ai.ai_addrlen,name,sizeof(name),serv,sizeof(serv),NI_NUMERICHOST|NI_NUMERICSERV);
	if (err!=0){
		cain_sip_error("cain_sip_channel_new_udp_with_addr(): getnameinfo() failed: %s",gai_strerror(err));
		cain_sip_object_unref(obj);
		return NULL;
	}
	cain_sip_channel_init((cain_sip_channel_t*)obj,stack,bindip,localport,NULL,name,atoi(serv));
	hints.ai_family=peer->ai_family;
	err=getaddrinfo(name,serv,&hints,&obj->base.current_peer); /*might be optimized someway ?*/
	if (err!=0){
		cain_sip_error("getaddrinfo() failed for udp channel [%p] error [%s]",obj,gai_strerror(err));
	}
	obj->base.peer_list=cain_sip_list_prepend(obj->base.peer_list,obj->base.current_peer);
	return (cain_sip_channel_t*)obj;
}

