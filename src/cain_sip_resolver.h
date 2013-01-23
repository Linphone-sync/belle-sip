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


#ifndef cain_sip_resolver_h
#define cain_sip_resolver_h

#include "cain_sip_internal.h"
#include "dns.h"


typedef struct cain_sip_resolver_context cain_sip_resolver_context_t;

#define CAIN_SIP_RESOLVER_CONTEXT(obj) CAIN_SIP_CAST(obj,cain_sip_resolver_context_t)

/**
 * Callback prototype for asynchronous DNS resolution. The result addrinfo must be taken and (possibly later) freed by 
 * the callee, using freeaddrinfo().
**/
typedef void (*cain_sip_resolver_callback_t)(void *data, const char *name, struct addrinfo *result);


struct cain_sip_resolver_context{
	cain_sip_source_t source;
	cain_sip_resolver_callback_t cb;
	void *cb_data;
	struct dns_resolv_conf *resconf;
	struct dns_hosts *hosts;
	struct dns_resolver *R;
	char *name;
	int port;
	struct addrinfo *ai;
	int family;
	uint8_t cancelled;
	uint8_t done;
};

int cain_sip_addrinfo_to_ip(const struct addrinfo *ai, char *ip, size_t ip_size, int *port);
struct addrinfo * cain_sip_ip_address_to_addrinfo(const char *ipaddress, int port);
unsigned long cain_sip_resolve(cain_sip_stack_t *stack, const char *name, int port, int family, cain_sip_resolver_callback_t cb , void *data, cain_sip_main_loop_t *ml);
void cain_sip_resolve_cancel(cain_sip_main_loop_t *ml, unsigned long id);

void cain_sip_get_src_addr_for(const struct sockaddr *dest, socklen_t destlen, struct sockaddr *src, socklen_t *srclen);


#endif
