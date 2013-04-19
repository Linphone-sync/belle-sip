/*
	cain-sip - SIP (RFC3261) library.
    Copyright (C) 2010-2013  Belledonne Communications SARL

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

#ifdef HAVE_TUNNEL

struct cain_sip_tunnel_listening_point{
	cain_sip_listening_point_t base;
	void *tunnelclient;
};


static cain_sip_channel_t *tunnel_create_channel(cain_sip_listening_point_t *lp, const cain_sip_hop_t *hop){
	cain_sip_channel_t *chan=cain_sip_channel_new_tunnel(lp->stack, ((cain_sip_tunnel_listening_point_t*)lp)->tunnelclient,
								cain_sip_uri_get_host(lp->listening_uri), cain_sip_uri_get_port(lp->listening_uri),
								hop->host, hop->port);
	return chan;
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_tunnel_listening_point_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_tunnel_listening_point_t)={
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_tunnel_listening_point_t, cain_sip_listening_point_t,TRUE),
			NULL,
			NULL,
			NULL
		},
		"UDP",
		tunnel_create_channel
	}
};


static void cain_sip_tunnel_listening_point_init(cain_sip_tunnel_listening_point_t *lp, cain_sip_stack_t *s, void *tunnelclient) {
	cain_sip_listening_point_init((cain_sip_listening_point_t*)lp,s,"0.0.0.0",5060);
	lp->tunnelclient = tunnelclient;
}


cain_sip_listening_point_t * cain_sip_tunnel_listening_point_new(cain_sip_stack_t *s, void *tunnelclient){
	cain_sip_tunnel_listening_point_t *lp=cain_sip_object_new(cain_sip_tunnel_listening_point_t);
	cain_sip_tunnel_listening_point_init(lp,s,tunnelclient);
	return (cain_sip_listening_point_t*)lp;
}

#endif
