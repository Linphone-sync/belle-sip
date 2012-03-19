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


#include <sys/socket.h>
#include <netinet/tcp.h>
#include "listeningpoint_internal.h"
#include "cain_sip_internal.h"
#include "cain-sip/mainloop.h"
#include "stream_channel.h"
#include "gnutls/openssl.h"

/*************tls********/

struct cain_sip_tls_channel{
	cain_sip_channel_t base;
	cain_sip_tls_listening_point_t* lp;
	SSL *ssl;
	struct sockaddr_storage ss;
};


static void tls_channel_uninit(cain_sip_tls_channel_t *obj){
	cain_sip_fd_t sock = cain_sip_source_get_fd((cain_sip_source_t*)obj);
	if (sock!=-1)
		close_socket(sock);
	 cain_sip_main_loop_remove_source(obj->base.stack->ml,(cain_sip_source_t*)obj);
	 cain_sip_object_unref(obj->lp);
}

static int tls_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	cain_sip_fd_t sock = cain_sip_source_get_fd((cain_sip_source_t*)obj);
	int err;
	err=send(sock,buf,buflen,0);
	if (err==-1){
		cain_sip_fatal("Could not send tls packet on channel [%p]: %s",obj,strerror(errno));
		return -errno;
	}
	return err;
}

static int tls_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	cain_sip_fd_t sock = cain_sip_source_get_fd((cain_sip_source_t*)obj);
	int err;
	err=recv(sock,buf,buflen,MSG_DONTWAIT);
	if (err==-1 && errno!=EWOULDBLOCK){
		cain_sip_error("Could not receive tls packet: %s",strerror(errno));
		return -errno;
	}
	return err;
}

int tls_channel_connect(cain_sip_channel_t *obj, const struct sockaddr *addr, socklen_t socklen){
	return stream_channel_connect(obj,addr,socklen);
}

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_tls_channel_t,cain_sip_channel_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_tls_channel_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_tls_channel_t)=
{
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_tls_channel_t,cain_sip_channel_t,FALSE),
			(cain_sip_object_destroy_t)tls_channel_uninit,
			NULL,
			NULL
		},
		"TLS",
		1, /*is_reliable*/
		tls_channel_connect,
		tls_channel_send,
		tls_channel_recv
	}
};

static int process_data(cain_sip_channel_t *obj,unsigned int revents){
	cain_sip_tls_channel_t* channel=(cain_sip_tls_channel_t*)obj;
	socklen_t addrlen=sizeof(channel->ss);
	char ssl_error_string[128];
	int result;
	cain_sip_fd_t fd=cain_sip_source_get_fd((cain_sip_source_t*)channel);
	if (obj->state == CAIN_SIP_CHANNEL_CONNECTING && (revents&CAIN_SIP_EVENT_WRITE)) {

		if (finalize_stream_connection(fd,(struct sockaddr*)&channel->ss,&addrlen)) {
			goto process_error;
		}
		/*connected, now etablishing TLS connection*/
		if (!channel->ssl) {
			channel->ssl=SSL_new(channel->lp->ssl_context);
			if (!channel->ssl) {
				cain_sip_error("Cannot create TLS channel context");
				goto process_error;
			}
		}
		cain_sip_source_set_events((cain_sip_source_t*)channel,CAIN_SIP_EVENT_READ|CAIN_SIP_EVENT_ERROR);

		if (!SSL_set_fd(channel->ssl,fd)) {
			;
			cain_sip_error("TLS connection failed to set fd caused by [%s]",ERR_error_string(ERR_get_error(),ssl_error_string));
			goto process_error;
		}
		result=SSL_connect(channel->ssl);
		result = SSL_get_error(channel->ssl, result);
		if (result == SSL_ERROR_NONE) {
			cain_sip_channel_set_ready(obj,(struct sockaddr*)&channel->ss,addrlen);
			return CAIN_SIP_CONTINUE;
		} else if (result == SSL_ERROR_WANT_READ || result == SSL_ERROR_WANT_WRITE) {
			cain_sip_message("TLS connection in progress for channel [%p]",channel);
			return CAIN_SIP_CONTINUE;
		} else {
			cain_sip_error("TLS connection failed caused by [%s]",ERR_error_string(result,ssl_error_string));
			goto process_error;
		}


	} else if ( obj->state == CAIN_SIP_CHANNEL_READY) {
		cain_sip_channel_process_data(obj,revents);
	} else {
		cain_sip_warning("Unexpected event [%i], for channel [%p]",revents,channel);
	}
	return CAIN_SIP_CONTINUE;
	process_error:
	cain_sip_error("Cannot connect to [%s://%s:%s]",cain_sip_channel_get_transport_name(obj),obj->peer_name,obj->peer_port);
	channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
	channel_process_queue(obj);
	return CAIN_SIP_STOP;
}
cain_sip_channel_t * cain_sip_channel_new_tls(cain_sip_tls_listening_point_t *lp,const char *bindip, int localport, const char *dest, int port){
	cain_sip_tls_channel_t *obj=cain_sip_object_new(cain_sip_tls_channel_t);
	cain_sip_channel_init((cain_sip_channel_t*)obj
							,((cain_sip_listening_point_t*)lp)->stack
							,socket(AF_INET, SOCK_STREAM, 0)
							,(cain_sip_source_func_t)process_data
							,bindip,localport,dest,port);
	cain_sip_object_ref(obj->lp=lp);
	return (cain_sip_channel_t*)obj;
}















