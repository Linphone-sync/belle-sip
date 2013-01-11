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
#include "listeningpoint_internal.h"

#ifdef HAVE_OPENSSL
#include "gnutls/openssl.h"
#endif

#ifdef HAVE_GNUTLS
#include <gnutls/gnutls.h>
#endif

static int tls_ready=0;

#ifdef HAVE_GNUTLS
static void _gnutls_log_func( int level, const char* log) {
	cain_sip_log_level cain_sip_level;
	switch(level) {
	case 1: cain_sip_level=CAIN_SIP_LOG_ERROR;break;
	case 2: cain_sip_level=CAIN_SIP_LOG_WARNING;break;
	case 3: cain_sip_level=CAIN_SIP_LOG_MESSAGE;break;
	default:cain_sip_level=CAIN_SIP_LOG_MESSAGE;break;
	}
	cain_sip_log(cain_sip_level,"gnutls:%s",log);
}
#endif /*HAVE_GNUTLS*/

static void check_tls_init(void){
	if (tls_ready==0){
		#ifdef HAVE_OPENSSL
		SSL_library_init();
		SSL_load_error_strings();
		/*CRYPTO_set_id_callback(&threadid_cb);
		CRYPTO_set_locking_callback(&locking_function);*/
		#endif
		#ifdef HAVE_GNUTLS
		int result;
		/*gnutls_global_set_log_level(9);*/
		gnutls_global_set_log_function(_gnutls_log_func);
		if ((result = gnutls_global_init ()) <0) {
			cain_sip_fatal("Cannot initialize gnu tls caused by [%s]",gnutls_strerror(result));
		}
		#endif
	}
	tls_ready++;
}

static void check_tls_deinit(void){
	tls_ready--;
	if (tls_ready==0){
#ifdef HAVE_GNUTLS
		gnutls_global_deinit ();
#endif
	}
}

static void cain_sip_tls_listening_point_uninit(cain_sip_tls_listening_point_t *lp){
	check_tls_deinit();
}

static cain_sip_channel_t *tls_create_channel(cain_sip_listening_point_t *lp, const char *dest_ip, int port){
#ifdef HAVE_GNUTLS
	cain_sip_channel_t *chan=cain_sip_channel_new_tls(CAIN_SIP_TLS_LISTENING_POINT(lp)
														,cain_sip_uri_get_host(lp->listening_uri)
														,cain_sip_uri_get_port(lp->listening_uri)
														,dest_ip
														,port);
	return chan;
#else
	return NULL;
#endif
	
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_tls_listening_point_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_tls_listening_point_t)={
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_tls_listening_point_t, cain_sip_listening_point_t,TRUE),
			(cain_sip_object_destroy_t)cain_sip_tls_listening_point_uninit,
			NULL,
			NULL
		},
		"TLS",
		tls_create_channel
	}
};



cain_sip_listening_point_t * cain_sip_tls_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port){
	check_tls_init();
#ifdef HAVE_GNUTLS
	cain_sip_tls_listening_point_t *lp=cain_sip_object_new(cain_sip_tls_listening_point_t);
	cain_sip_listening_point_init((cain_sip_listening_point_t*)lp,s,ipaddress,port);
#ifdef HAVE_OPENSSL
	char ssl_error_string[128]; /*see openssl doc for size*/
	lp->ssl_context=SSL_CTX_new(TLSv1_client_method());
	if (!lp->ssl_context) {
		cain_sip_error("cain_sip_listening_point_t: SSL_CTX_new failed caused by [%s]",ERR_error_string(ERR_get_error(),ssl_error_string));
		cain_sip_object_unref(lp);
		return NULL;
	}
	/*SSL_CTX_set_cipher_list(lp->ssl_context,"LOW");*/
#endif /**/
	return CAIN_SIP_LISTENING_POINT(lp);
#else
	cain_sip_error("Cannot create tls listening point because not compile with TLS support");
	return NULL;
#endif
}
