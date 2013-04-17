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

#define DNS_H /* do not include dns.h in a CPP file! */
#include "cain_sip_internal.h"
#include "channel.h"

#ifdef HAVE_TUNNEL

#include <tunnel/client.hh>

#define TUNNEL_POLLING_DURATION	20 /* in ms */

using namespace belledonnecomm;

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_tunnel_channel_t, cain_sip_channel_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

struct cain_sip_tunnel_channel {
	cain_sip_channel_t base;
	cain_sip_source_t *pollingtimer;
	TunnelClient *tunnelclient;
	TunnelSocket *tunnelsocket;
};

typedef struct cain_sip_tunnel_channel cain_sip_tunnel_channel_t;


static int tunnel_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen) {
	cain_sip_tunnel_channel_t *chan = reinterpret_cast<cain_sip_tunnel_channel_t *>(obj);
	int err;
	err = chan->tunnelsocket->sendto(buf, buflen, obj->peer->ai_addr, obj->peer->ai_addrlen);
	return err;
}

static int tunnel_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen) {
	cain_sip_tunnel_channel_t *chan = reinterpret_cast<cain_sip_tunnel_channel_t *>(obj);
	int err;
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	err = chan->tunnelsocket->recvfrom(buf, buflen, (struct sockaddr *)&addr, addrlen);
	return err;
}

static int tunnel_channel_connect(cain_sip_channel_t *obj, const struct addrinfo *ai) {
	struct sockaddr_storage laddr;
	socklen_t lslen = sizeof(laddr);
	if (obj->local_ip == NULL) {
		cain_sip_get_src_addr_for(ai->ai_addr, ai->ai_addrlen, (struct sockaddr *)&laddr, &lslen, obj->local_port);
		cain_sip_address_remove_v4_mapping((struct sockaddr *)&laddr, (struct sockaddr *)&laddr, &lslen);
	}
	cain_sip_channel_set_ready(obj, (struct sockaddr *)&laddr, lslen);
	return 0;
}

static void tunnel_channel_close(cain_sip_channel_t *obj) {
	cain_sip_tunnel_channel_t *chan = reinterpret_cast<cain_sip_tunnel_channel_t *>(obj);
	chan->tunnelclient->closeSocket(chan->tunnelsocket);
	chan->tunnelsocket = NULL;
}

static void tunnel_channel_uninit(cain_sip_channel_t *obj) {
	cain_sip_tunnel_channel_t *chan = reinterpret_cast<cain_sip_tunnel_channel_t *>(obj);
	if (chan->tunnelsocket != NULL) {
		tunnel_channel_close(obj);
	}
	if (chan->pollingtimer != NULL) {
		cain_sip_main_loop_remove_source(obj->stack->ml, chan->pollingtimer);
		cain_sip_object_unref(chan->pollingtimer);
		chan->pollingtimer = NULL;
	}
}

static int tunnel_polling_timer(cain_sip_tunnel_channel_t *chan) {
	if ((chan->tunnelsocket != NULL) && chan->tunnelsocket->hasData()) {
		cain_sip_channel_process_data(reinterpret_cast<cain_sip_channel_t *>(chan), CAIN_SIP_EVENT_READ);
	}
	return CAIN_SIP_CONTINUE;
}


CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_tunnel_channel_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_tunnel_channel_t)=
{
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_tunnel_channel_t,cain_sip_channel_t,FALSE),
			(cain_sip_object_destroy_t)tunnel_channel_uninit,
			NULL,
			NULL
		},
		"UDP",
		0, /*is_reliable*/
		tunnel_channel_connect,
		tunnel_channel_send,
		tunnel_channel_recv,
		tunnel_channel_close
	}
};

cain_sip_channel_t * cain_sip_channel_new_tunnel(cain_sip_stack_t *stack, void *tunnelclient, const char *bindip, int localport, const char *dest, int port){
	cain_sip_tunnel_channel_t *obj = cain_sip_object_new(cain_sip_tunnel_channel_t);
	cain_sip_channel_init((cain_sip_channel_t*)obj, stack, bindip, localport, NULL, dest, port);
	obj->tunnelclient = static_cast<TunnelClient *>(tunnelclient);
	obj->tunnelsocket = obj->tunnelclient->createSocket(5060, 6060);
	obj->pollingtimer = cain_sip_timeout_source_new((cain_sip_source_func_t)tunnel_polling_timer, obj, TUNNEL_POLLING_DURATION);
	cain_sip_object_set_name((cain_sip_object_t*)obj->pollingtimer, "tunnel_polling_timer");
	cain_sip_main_loop_add_source(stack->ml, obj->pollingtimer);
	return (cain_sip_channel_t*)obj;
}

#endif
