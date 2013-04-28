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

#ifdef HAVE_TLS
#include "gnutls/openssl.h"
#endif

#ifdef HAVE_POLARSSL
#include <polarssl/ssl.h>
#endif

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_listening_point_t,cain_sip_object_t)
const char *transport;
cain_sip_channel_t * (*create_channel)(cain_sip_listening_point_t *, const cain_sip_hop_t *hop);
CAIN_SIP_DECLARE_CUSTOM_VPTR_END


#define CAIN_SIP_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_listening_point_t)


/*
 Listening points: base, udp
*/

struct cain_sip_listening_point{
	cain_sip_object_t base;
	cain_sip_stack_t *stack;
	cain_sip_list_t *channels;
	cain_sip_uri_t* listening_uri;
	cain_sip_source_t* keep_alive_timer;
	cain_sip_channel_listener_t* channel_listener; /*initial channel listener used for channel creation, specially for socket server*/
	int ai_family; /*AF_INET or AF_INET6*/
};

CAIN_SIP_BEGIN_DECLS
void cain_sip_listening_point_init(cain_sip_listening_point_t *lp, cain_sip_stack_t *s,  const char *address, int port);
cain_sip_channel_t *_cain_sip_listening_point_get_channel(cain_sip_listening_point_t *lp,const cain_sip_hop_t *hop, const struct addrinfo *addr);
cain_sip_channel_t *cain_sip_listening_point_create_channel(cain_sip_listening_point_t *ip, const cain_sip_hop_t *hop);
void cain_sip_listening_point_remove_channel(cain_sip_listening_point_t *lp, cain_sip_channel_t *chan);
int cain_sip_listening_point_get_well_known_port(const char *transport);
cain_sip_channel_t *cain_sip_listening_point_get_channel(cain_sip_listening_point_t *lp, const cain_sip_hop_t *hop);
void cain_sip_listening_point_add_channel(cain_sip_listening_point_t *lp, cain_sip_channel_t *chan);
void cain_sip_listening_point_set_channel_listener(cain_sip_listening_point_t *lp,cain_sip_channel_listener_t* channel_listener);
CAIN_SIP_END_DECLS

/**udp*/
typedef struct cain_sip_udp_listening_point cain_sip_udp_listening_point_t;
cain_sip_channel_t * cain_sip_channel_new_udp(cain_sip_stack_t *stack, int sock, const char *bindip, int localport, const char *peername, int peerport);
cain_sip_channel_t * cain_sip_channel_new_udp_with_addr(cain_sip_stack_t *stack, int sock, const char *bindip, int localport, const struct addrinfo *ai);
cain_sip_listening_point_t * cain_sip_udp_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port);
CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_udp_listening_point_t,cain_sip_listening_point_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END


/*stream*/
typedef struct cain_sip_stream_listening_point cain_sip_stream_listening_point_t;

struct cain_sip_stream_listening_point{
	cain_sip_listening_point_t base;
	cain_sip_socket_t server_sock;
	cain_sip_source_t *source;
};

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_stream_listening_point_t,cain_sip_listening_point_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

void cain_sip_stream_listening_point_setup_server_socket(cain_sip_stream_listening_point_t *obj, cain_sip_source_func_t on_new_connection_cb );
void cain_sip_stream_listening_point_destroy_server_socket(cain_sip_stream_listening_point_t *lp);
void cain_sip_stream_listening_point_init(cain_sip_stream_listening_point_t *obj, cain_sip_stack_t *s, const char *ipaddress, int port, cain_sip_source_func_t on_new_connection_cb );
cain_sip_listening_point_t * cain_sip_stream_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port);

/*tls*/

struct cain_sip_tls_listening_point{
	cain_sip_stream_listening_point_t base;
#ifdef HAVE_OPENSSL
	SSL_CTX *ssl_context;
#endif
	char *root_ca;
	int verify_exceptions;
};

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_tls_listening_point_t,cain_sip_listening_point_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END
#define CAIN_SIP_TLS_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_tls_listening_point_t)
cain_sip_listening_point_t * cain_sip_tls_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port);
cain_sip_channel_t * cain_sip_channel_new_tls(cain_sip_tls_listening_point_t* lp, const char *bindip, int localport,const char *cname, const char *name, int port);

/*tunnel*/
#ifdef HAVE_TUNNEL
typedef struct cain_sip_tunnel_listening_point cain_sip_tunnel_listening_point_t;
CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_tunnel_listening_point_t,cain_sip_listening_point_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END
#define CAIN_SIP_TUNNEL_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_tunnel_listening_point_t)
cain_sip_channel_t * cain_sip_channel_new_tunnel(cain_sip_stack_t *s, void *tunnelclient, const char *bindip, int localport, const char *name, int port);
#endif

#include "transports/stream_channel.h"

#endif /* LISTENINGPOINT_INTERNAL_H_ */

