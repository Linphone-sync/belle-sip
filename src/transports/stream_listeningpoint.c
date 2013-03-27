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

static int on_new_connection(void *userdata, unsigned int events);


void cain_sip_stream_listening_point_destroy_server_socket(cain_sip_stream_listening_point_t *lp){
	if (lp->server_sock!=(cain_sip_socket_t)-1){
		close_socket(lp->server_sock);
		lp->server_sock=-1;
	}
	if (lp->source){
		cain_sip_main_loop_remove_source(lp->base.stack->ml,lp->source);
		cain_sip_object_unref(lp->source);
		lp->source=NULL;
	}
}

static void cain_sip_stream_listening_point_uninit(cain_sip_stream_listening_point_t *lp){
	cain_sip_stream_listening_point_destroy_server_socket(lp);
}

static cain_sip_channel_t *stream_create_channel(cain_sip_listening_point_t *lp, const cain_sip_hop_t *hop){
	cain_sip_channel_t *chan=cain_sip_stream_channel_new_client(lp->stack
							,cain_sip_uri_get_host(lp->listening_uri)
							,cain_sip_uri_get_port(lp->listening_uri)
							,hop->cname,hop->host,hop->port);
	return chan;
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_stream_listening_point_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_stream_listening_point_t)={
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_stream_listening_point_t, cain_sip_listening_point_t,TRUE),
			(cain_sip_object_destroy_t)cain_sip_stream_listening_point_uninit,
			NULL,
			NULL
		},
		"TCP",
		stream_create_channel
	}
};

static cain_sip_socket_t create_server_socket(const char *addr, int port, int *family){
	struct addrinfo hints={0};
	struct addrinfo *res=NULL;
	int err;
	cain_sip_socket_t sock;
	char portnum[10];
	int optval=1;

	snprintf(portnum,sizeof(portnum),"%i",port);
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=IPPROTO_TCP;
	hints.ai_flags=AI_NUMERICSERV;
	err=getaddrinfo(addr,portnum,&hints,&res);
	if (err!=0){
		cain_sip_error("getaddrinfo() failed for %s port %i: %s",addr,port,gai_strerror(err));
		return -1;
	}
	*family=res->ai_family;
	sock=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if (sock==-1){
		cain_sip_error("Cannot create UDP socket: %s",cain_sip_get_socket_error_string());
		freeaddrinfo(res);
		return -1;
	}
	err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
			(char*)&optval, sizeof (optval));
	if (err == -1){
		cain_sip_warning ("Fail to set SIP/UDP address reusable: %s.", cain_sip_get_socket_error_string());
	}
	
	err=bind(sock,res->ai_addr,res->ai_addrlen);
	if (err==-1){
		cain_sip_error("TCP bind() failed for %s port %i: %s",addr,port,cain_sip_get_socket_error_string());
		close_socket(sock);
		freeaddrinfo(res);
		return -1;
	}
	freeaddrinfo(res);
	err=listen(sock,64);
	if (err==-1){
		cain_sip_error("TCP listen() failed for %s port %i: %s",addr,port,cain_sip_get_socket_error_string());
		close_socket(sock);
	}
	return sock;
}

void cain_sip_stream_listening_point_setup_server_socket(cain_sip_stream_listening_point_t *obj, cain_sip_source_func_t on_new_connection_cb ){
	obj->server_sock=create_server_socket(cain_sip_uri_get_host(obj->base.listening_uri),
		cain_sip_uri_get_port(obj->base.listening_uri),&obj->base.ai_family);
	if (obj->server_sock==(cain_sip_socket_t)-1) return;
	obj->source=cain_sip_socket_source_new(on_new_connection_cb,obj,obj->server_sock,CAIN_SIP_EVENT_READ,-1);
	cain_sip_main_loop_add_source(obj->base.stack->ml,obj->source);
}

static int on_new_connection(void *userdata, unsigned int events){
	cain_sip_socket_t child;
	struct sockaddr_storage addr;
	socklen_t slen=sizeof(addr);
	cain_sip_stream_listening_point_t *lp=(cain_sip_stream_listening_point_t*)userdata;
	cain_sip_channel_t *chan;
	
	child=accept(lp->server_sock,(struct sockaddr*)&addr,&slen);
	if (child==(cain_sip_socket_t)-1){
		cain_sip_error("Listening point [%p] accept() failed on TCP server socket: %s",lp,cain_sip_get_socket_error_string());
		cain_sip_stream_listening_point_destroy_server_socket(lp);
		cain_sip_stream_listening_point_setup_server_socket(lp,on_new_connection);
		return CAIN_SIP_STOP;
	}
	cain_sip_message("New connection arriving !");
	chan=cain_sip_stream_channel_new_child(lp->base.stack,child,(struct sockaddr*)&addr,slen);
	if (chan) cain_sip_listening_point_add_channel((cain_sip_listening_point_t*)lp,chan);
	return CAIN_SIP_CONTINUE;
}

void cain_sip_stream_listening_point_init(cain_sip_stream_listening_point_t *obj, cain_sip_stack_t *s, const char *ipaddress, int port, cain_sip_source_func_t on_new_connection_cb ){
	cain_sip_listening_point_init((cain_sip_listening_point_t*)obj,s,ipaddress,port);
	cain_sip_stream_listening_point_setup_server_socket(obj, on_new_connection_cb);
}

cain_sip_listening_point_t * cain_sip_stream_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port){
	cain_sip_stream_listening_point_t *lp=cain_sip_object_new(cain_sip_stream_listening_point_t);
	cain_sip_stream_listening_point_init(lp,s,ipaddress,port,on_new_connection);
	if (lp->server_sock==(cain_sip_socket_t)-1){
		cain_sip_object_unref(lp);
		return NULL;
	}
	return CAIN_SIP_LISTENING_POINT(lp);
}

