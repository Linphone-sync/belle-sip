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

#ifndef LISTENINGPOINT_INTERNAL_H_
#define LISTENINGPOINT_INTERNAL_H_

#include "cain_sip_internal.h"
#ifdef HAVE_TLS
#include "gnutls/openssl.h"
#endif
/*
 Listening points: base, udp
*/

struct cain_sip_listening_point{
	cain_sip_object_t base;
	cain_sip_stack_t *stack;
	cain_sip_list_t *channels;
	char *addr;
	int port;
};

void cain_sip_listening_point_init(cain_sip_listening_point_t *lp, cain_sip_stack_t *s, const char *address, int port);
cain_sip_channel_t *_cain_sip_listening_point_get_channel(cain_sip_listening_point_t *lp,const char *peer_name, int peer_port, const struct addrinfo *addr);
cain_sip_channel_t *cain_sip_listening_point_create_channel(cain_sip_listening_point_t *ip, const char *dest, int port);
int cain_sip_listening_point_get_well_known_port(const char *transport);
cain_sip_channel_t *cain_sip_listening_point_get_channel(cain_sip_listening_point_t *lp,const char *peer_name, int peer_port);
void cain_sip_listening_point_add_channel(cain_sip_listening_point_t *lp, cain_sip_channel_t *chan);



/**udp*/
typedef struct cain_sip_udp_listening_point cain_sip_udp_listening_point_t;
cain_sip_channel_t * cain_sip_channel_new_udp(cain_sip_stack_t *stack, int sock, const char *bindip, int localport, const char *peername, int peerport);
cain_sip_channel_t * cain_sip_channel_new_udp_with_addr(cain_sip_stack_t *stack, int sock, const char *bindip, int localport, const struct addrinfo *ai);
cain_sip_listening_point_t * cain_sip_udp_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port);
CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_udp_listening_point_t,cain_sip_listening_point_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END
#define CAIN_SIP_UDP_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_udp_listening_point_t)

/*stream*/
typedef struct cain_sip_stream_listening_point cain_sip_stream_listening_point_t;
cain_sip_channel_t * cain_sip_channel_new_tcp(cain_sip_stack_t *stack, const char *bindip, int localport,const char *name, int port);
CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_stream_listening_point_t,cain_sip_listening_point_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END
#define CAIN_SIP_STREAM_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_stream_listening_point_t)
cain_sip_listening_point_t * cain_sip_stream_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port);

/*tls*/

typedef struct cain_sip_tls_listening_point cain_sip_tls_listening_point_t;
#ifdef HAVE_TLS
struct cain_sip_tls_listening_point{
	cain_sip_listening_point_t base;
	SSL_CTX *ssl_context;
};
#endif
CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_tls_listening_point_t,cain_sip_listening_point_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END
#define CAIN_SIP_TLS_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_tls_listening_point_t)
cain_sip_listening_point_t * cain_sip_tls_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port);
cain_sip_channel_t * cain_sip_channel_new_tls(cain_sip_tls_listening_point_t* lp, const char *bindip, int localport,const char *name, int port);


#endif /* LISTENINGPOINT_INTERNAL_H_ */
