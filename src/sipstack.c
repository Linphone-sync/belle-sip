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
#ifdef HAVE_GNUTLS
#include <gnutls/gnutls.h>
#endif
static void cain_sip_stack_destroy(cain_sip_stack_t *stack){
#ifdef HAVE_GNUTLS
	gnutls_global_deinit ();
#endif
	cain_sip_object_unref(stack->ml);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_stack_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_stack_t,cain_sip_object_t,cain_sip_stack_destroy,NULL,NULL,FALSE);
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
cain_sip_stack_t * cain_sip_stack_new(const char *properties){
	cain_sip_stack_t *stack=cain_sip_object_new(cain_sip_stack_t);
	stack->ml=cain_sip_main_loop_new ();
	stack->timer_config.T1=500;
	stack->timer_config.T2=4000;
	stack->timer_config.T4=5000;
#ifdef HAVE_OPENSSL
	SSL_library_init();
	SSL_load_error_strings();
	/*CRYPTO_set_id_callback(&threadid_cb);
	CRYPTO_set_locking_callback(&locking_function);*/
#endif
#ifdef HAVE_GNUTLS
	{
		int result;
		/*gnutls_global_set_log_level(9);*/
		gnutls_global_set_log_function(_gnutls_log_func);
		if ((result = gnutls_global_init ()) <0) {
			cain_sip_fatal("Cannot initialize gnu tls caused by [%s]",gnutls_strerror(result));
		}
	}
#endif
	return stack;
}

const cain_sip_timer_config_t *cain_sip_stack_get_timer_config(const cain_sip_stack_t *stack){
	return &stack->timer_config;
}

cain_sip_listening_point_t *cain_sip_stack_create_listening_point(cain_sip_stack_t *s, const char *ipaddress, int port, const char *transport){
	cain_sip_listening_point_t *lp=NULL;
	if (strcasecmp(transport,"UDP")==0) {
		lp=cain_sip_udp_listening_point_new(s,ipaddress,port);
	} else if (strcasecmp(transport,"TCP") == 0) {
		lp=cain_sip_stream_listening_point_new(s,ipaddress,port);
	}else if (strcasecmp(transport,"TLS") == 0) {
		lp=cain_sip_tls_listening_point_new(s,ipaddress,port);
	} else {
		cain_sip_fatal("Unsupported transport %s",transport);
	}
	return lp;
}

void cain_sip_stack_delete_listening_point(cain_sip_stack_t *s, cain_sip_listening_point_t *lp){
	cain_sip_object_unref(lp);
}

cain_sip_provider_t *cain_sip_stack_create_provider(cain_sip_stack_t *s, cain_sip_listening_point_t *lp){
	cain_sip_provider_t *p=cain_sip_provider_new(s,lp);
	return p;
}

void cain_sip_stack_delete_provider(cain_sip_stack_t *s, cain_sip_provider_t *p){
	cain_sip_object_unref(p);
}

cain_sip_main_loop_t * cain_sip_stack_get_main_loop(cain_sip_stack_t *stack){
	return stack->ml;
}

void cain_sip_stack_main(cain_sip_stack_t *stack){
	cain_sip_main_loop_run(stack->ml);
}

void cain_sip_stack_sleep(cain_sip_stack_t *stack, unsigned int milliseconds){
	cain_sip_main_loop_sleep (stack->ml,milliseconds);
}

void cain_sip_stack_get_next_hop(cain_sip_stack_t *stack, cain_sip_request_t *req, cain_sip_hop_t *hop){
	cain_sip_header_route_t *route=CAIN_SIP_HEADER_ROUTE(cain_sip_message_get_header(CAIN_SIP_MESSAGE(req),"route"));
	cain_sip_uri_t *uri;
	if (route!=NULL){
		uri=cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(route));
	}else{
		uri=cain_sip_request_get_uri(req);
	}
	hop->transport=cain_sip_uri_get_transport_param(uri);
	if (hop->transport==NULL) hop->transport="UDP";
	hop->host=cain_sip_uri_get_host(uri);
	hop->port=cain_sip_uri_get_listening_port(uri);
}


