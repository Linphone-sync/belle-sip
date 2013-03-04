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


cain_sip_hop_t* cain_sip_hop_new(const char* transport, const char* host,int port) {
	cain_sip_hop_t* hop = cain_sip_object_new(cain_sip_hop_t);
	if (transport) hop->transport=cain_sip_strdup(transport);
	if (host) hop->host=cain_sip_strdup(host);
	hop->port=port;
	return hop;
}

cain_sip_hop_t* cain_sip_hop_new_from_uri(const cain_sip_uri_t *uri){
	const char *host;
	host=cain_sip_uri_get_maddr_param(uri);
	if (!host) host=cain_sip_uri_get_host(uri);
	return cain_sip_hop_new(cain_sip_uri_get_transport_param(uri),
				host,
				cain_sip_uri_get_listening_port(uri));
}

static void cain_sip_hop_destroy(cain_sip_hop_t *hop){
	if (hop->host) {
		cain_sip_free(hop->host);
		hop->host=NULL;
	}
	if (hop->transport){
		cain_sip_free(hop->transport);
		hop->transport=NULL;
	}
}

static void cain_sip_hop_clone(cain_sip_hop_t *hop, const cain_sip_hop_t *orig){
	if (orig->host)
		hop->host=cain_sip_strdup(orig->host);
	if (orig->transport)
		hop->transport=cain_sip_strdup(orig->transport);
	
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_hop_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_hop_t,cain_sip_object_t,cain_sip_hop_destroy,cain_sip_hop_clone,NULL,TRUE);

static void cain_sip_stack_destroy(cain_sip_stack_t *stack){
	cain_sip_object_unref(stack->ml);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_stack_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_stack_t,cain_sip_object_t,cain_sip_stack_destroy,NULL,NULL,FALSE);

cain_sip_stack_t * cain_sip_stack_new(const char *properties){
	cain_sip_stack_t *stack=cain_sip_object_new(cain_sip_stack_t);
	stack->ml=cain_sip_main_loop_new ();
	stack->timer_config.T1=500;
	stack->timer_config.T2=4000;
	stack->timer_config.T4=5000;
	stack->transport_timeout=30000;
	stack->dns_timeout=15000;
	return stack;
}

const cain_sip_timer_config_t *cain_sip_stack_get_timer_config(const cain_sip_stack_t *stack){
	return &stack->timer_config;
}

int cain_sip_stack_get_transport_timeout(const cain_sip_stack_t *stack){
	return stack->transport_timeout;
}

int cain_sip_stack_get_dns_timeout(const cain_sip_stack_t *stack) {
	return stack->dns_timeout;
}

void cain_sip_stack_set_dns_timeout(cain_sip_stack_t *stack, int timeout) {
	stack->dns_timeout = timeout;
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

cain_sip_hop_t * cain_sip_stack_get_next_hop(cain_sip_stack_t *stack, cain_sip_request_t *req) {
	cain_sip_header_route_t *route=CAIN_SIP_HEADER_ROUTE(cain_sip_message_get_header(CAIN_SIP_MESSAGE(req),"route"));
	cain_sip_uri_t *uri;

	if (route!=NULL){
		uri=cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(route));
	}else{
		uri=cain_sip_request_get_uri(req);
	}
	return cain_sip_hop_new_from_uri(uri);
}



void cain_sip_stack_set_tx_delay(cain_sip_stack_t *stack, int delay_ms){
	stack->tx_delay=delay_ms;
}
void cain_sip_stack_set_send_error(cain_sip_stack_t *stack, int send_error){
	stack->send_error=send_error;
}

void cain_sip_stack_set_resolver_tx_delay(cain_sip_stack_t *stack, int delay_ms) {
	stack->resolver_tx_delay = delay_ms;
}

void cain_sip_stack_set_resolver_send_error(cain_sip_stack_t *stack, int send_error) {
	stack->resolver_send_error = send_error;
}

const char* cain_sip_version_to_string() {
	return PACKAGE_VERSION;
}

