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
#include "stream_channel.h"
#ifdef HAVE_GNUTLS
#include <gnutls/gnutls.h>
#elif HAVE_OPENSSL
#include "openssl/ssl.h"
#endif
/*************tls********/

static int tls_process_data(cain_sip_channel_t *obj,unsigned int revents);

struct cain_sip_tls_channel{
	cain_sip_stream_channel_t base;
	int socket_connected;
#ifdef HAVE_OPENSSL
	SSL *ssl;
#endif
#ifdef HAVE_GNUTLS
	gnutls_session_t session;
	gnutls_certificate_credentials_t xcred;
#endif
	struct sockaddr_storage ss;
};

static void tls_channel_close(cain_sip_tls_channel_t *obj){
#ifdef HAVE_GNUTLS
	gnutls_bye (obj->session, GNUTLS_SHUT_RDWR);
	gnutls_deinit (obj->session);
	gnutls_certificate_free_credentials (obj->xcred);
#endif
	stream_channel_close((cain_sip_channel_t*)obj);
}

static void tls_channel_uninit(cain_sip_tls_channel_t *obj){
	cain_sip_socket_t sock = cain_sip_source_get_socket((cain_sip_source_t*)obj);
	if (sock!=-1)
		tls_channel_close(obj);
}

static int tls_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	cain_sip_tls_channel_t* channel = (cain_sip_tls_channel_t*)obj;
	int err = -1;
#ifdef HAVE_GNUTLS
	/*fix me, can block, see gnutls doc*/
	err=gnutls_record_send (channel->session, buf, buflen);
	if (err<0){
		cain_sip_error("channel [%p]: could not send tls packet because [%s]",obj,gnutls_strerror(err));
		return err;
	}
#endif
	return err;
}

#ifdef HAVE_GNUTLS
static ssize_t tls_channel_pull_func(gnutls_transport_ptr_t obj, void* buff, size_t bufflen) {
	int err=recv(
		cain_sip_source_get_socket((cain_sip_source_t *)obj),buff,bufflen,0);
	if (err==-1 && get_socket_error()!=EWOULDBLOCK){
		cain_sip_error("tls_channel_pull_func: %s",cain_sip_get_socket_error_string());
	}
	return err;
}
#endif

static int tls_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	cain_sip_tls_channel_t* channel = (cain_sip_tls_channel_t*)obj;
	int err = 1;
#ifdef HAVE_GNUTLS
	err=gnutls_record_recv(channel->session,buf,buflen);
	if (err<0 ){
		cain_sip_error("Could not receive tls packet: %s",gnutls_strerror(err));
		return err;
	}
#endif
	return err;
}

int tls_channel_connect(cain_sip_channel_t *obj, const struct addrinfo *ai){
	int err= stream_channel_connect(obj,ai);
	if (err==0){
		cain_sip_socket_t sock=cain_sip_source_get_socket((cain_sip_source_t*)obj);
		cain_sip_channel_set_socket(obj,sock,(cain_sip_source_func_t)tls_process_data);
		return 0;
	}
	return -1;
}

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_tls_channel_t,cain_sip_stream_channel_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_tls_channel_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_tls_channel_t)=
{
	{
		{
			{
				CAIN_SIP_VPTR_INIT(cain_sip_tls_channel_t,cain_sip_stream_channel_t,FALSE),
				(cain_sip_object_destroy_t)tls_channel_uninit,
				NULL,
				NULL
			},
			"TLS",
			1, /*is_reliable*/
			tls_channel_connect,
			tls_channel_send,
			tls_channel_recv,
			(void (*)(cain_sip_channel_t*))tls_channel_close
		}
	}
};

static int tls_process_data(cain_sip_channel_t *obj,unsigned int revents){
	cain_sip_tls_channel_t* channel=(cain_sip_tls_channel_t*)obj;
	socklen_t addrlen=sizeof(channel->ss);
	int result;
#ifdef HAVE_OPENSSL
	char ssl_error_string[128];
#endif /*HAVE_OPENSSL*/
	cain_sip_socket_t fd=cain_sip_source_get_socket((cain_sip_source_t*)channel);
	if (obj->state == CAIN_SIP_CHANNEL_CONNECTING) {
		if (!channel->socket_connected) {
			if (finalize_stream_connection(fd,(struct sockaddr*)&channel->ss,&addrlen)) {
				goto process_error;
			}
			cain_sip_source_set_events((cain_sip_source_t*)channel,CAIN_SIP_EVENT_READ|CAIN_SIP_EVENT_ERROR);
			channel->socket_connected=1;
			cain_sip_message("Connected at TCP level.");
		}
		/*connected, now establishing TLS connection*/
#if HAVE_GNUTLS
		gnutls_transport_set_ptr2(channel->session, (gnutls_transport_ptr_t)channel,(gnutls_transport_ptr_t) (0xFFFFFFFFUL&fd));
		result = gnutls_handshake(channel->session);
		if ((result < 0 && gnutls_error_is_fatal (result) == 0)) {
			cain_sip_message("TLS connection in progress for channel [%p]",channel);
			return CAIN_SIP_CONTINUE;
		} else if (result<0) {
			cain_sip_error("TLS Handshake failed caused by [%s]",gnutls_strerror(result));
			goto process_error;
		} else {
			cain_sip_channel_set_ready(obj,(struct sockaddr*)&channel->ss,addrlen);
			return CAIN_SIP_CONTINUE;
		}
#elif HAVE_OPENSSL
		if (!channel->ssl) {
			channel->ssl=SSL_new(channel->lp->ssl_context);
			if (!channel->ssl) {
				cain_sip_error("Cannot create TLS channel context");
				goto process_error;
			}
		}
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
#endif /*HAVE_OPENSSL*/

	} else if ( obj->state == CAIN_SIP_CHANNEL_READY) {
		cain_sip_channel_process_data(obj,revents);
	} else {
		cain_sip_warning("Unexpected event [%i], for channel [%p]",revents,channel);
	}
	return CAIN_SIP_CONTINUE;
	process_error:
	cain_sip_error("Cannot connect to [%s://%s:%i]",cain_sip_channel_get_transport_name(obj),obj->peer_name,obj->peer_port);
	channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
	channel_process_queue(obj);
	return CAIN_SIP_STOP;
}

cain_sip_channel_t * cain_sip_channel_new_tls(cain_sip_tls_listening_point_t *lp,const char *bindip, int localport, const char *dest, int port){
	cain_sip_tls_channel_t *obj=cain_sip_object_new(cain_sip_tls_channel_t);
	cain_sip_channel_t* channel=(cain_sip_channel_t*)obj;
	int result;
	#ifdef HAVE_GNUTLS
	const char* err_pos;
	result = gnutls_init (&obj->session, GNUTLS_CLIENT);
	if (result<0) {
		cain_sip_error("Cannot initialize gnu tls session for channel [%p] caused by [%s]",obj,gnutls_strerror(result));
		goto error;
	}
	result = gnutls_certificate_allocate_credentials (&obj->xcred);
	if (result<0) {
		cain_sip_error("Cannot allocate_client_credentials for channel [%p] caused by [%s]",obj,gnutls_strerror(result));
		goto error;
	}
	/* Use default priorities */
	result = gnutls_priority_set_direct (obj->session, "NORMAL"/*"PERFORMANCE:+ANON-DH:!ARCFOUR-128"*/,&err_pos);
	if (result<0) {
		cain_sip_error("Cannot set direct priority for channel [%p] caused by [%s] at position [%s]",obj,gnutls_strerror(result),err_pos);
		goto error;
	}
	/* put the  credentials to the current session
	 */
	 result = gnutls_credentials_set (obj->session, GNUTLS_CRD_CERTIFICATE, obj->xcred);
	if (result<0) {
		cain_sip_error("Cannot set credential for channel [%p] caused by [%s]",obj,gnutls_strerror(result));
		goto error;
	}
	gnutls_transport_set_pull_function(obj->session,tls_channel_pull_func);
#endif
	cain_sip_channel_init(channel
							,((cain_sip_listening_point_t*)lp)->stack
							,bindip,localport,dest,port);
	return (cain_sip_channel_t*)obj;
error:
	cain_sip_error("Cannot create tls channel to [%s://%s:%i]",cain_sip_channel_get_transport_name(channel),channel->peer_name,channel->peer_port);
	cain_sip_object_unref(obj);
	return NULL;
}















