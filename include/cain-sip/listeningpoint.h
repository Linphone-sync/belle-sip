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


#ifndef CAIN_SIP_TRANSPORT_H
#define CAIN_SIP_TRANSPORT_H

#ifndef WIN32
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
#else
#endif


typedef struct cain_sip_channel cain_sip_channel_t;


CAIN_SIP_BEGIN_DECLS

/**
* cain_sip_transport_t is an object representing a single communication channel (=unix socket), 
* unlike the cain_sip_listening_point_t that can owns several channels for TCP or TLS (incoming server child sockets or 
* outgoing client sockets).
**/

int cain_sip_channel_send(cain_sip_channel_t *t, const void *buf, size_t buflen);

int cain_sip_channel_recv(cain_sip_channel_t *t, void *buf, size_t buflen);

const struct addrinfo * cain_sip_channel_get_peer(cain_sip_channel_t *t);




const char *cain_sip_listening_point_get_ip_address(const cain_sip_listening_point_t *lp);
int cain_sip_listening_point_get_port(const cain_sip_listening_point_t *lp);
const char *cain_sip_listening_point_get_transport(const cain_sip_listening_point_t *ip);
const char *cain_sip_listening_point_get_ip_address(const  cain_sip_listening_point_t *ip);



CAIN_SIP_END_DECLS

#endif

