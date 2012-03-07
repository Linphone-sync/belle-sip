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


struct cain_sip_stream_listening_point{
	cain_sip_listening_point_t base;
};


static void cain_sip_stream_listening_point_uninit(cain_sip_stream_listening_point_t *lp){
}

static cain_sip_channel_t *stream_create_channel(cain_sip_listening_point_t *lp, const char *dest_ip, int port){
	cain_sip_channel_t *chan=cain_sip_channel_new_tcp(lp->stack,lp->addr,lp->port,dest_ip,port);
	return chan;
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_stream_listening_point_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_stream_listening_point_t)={
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_stream_listening_point_t, cain_sip_listening_point_t,FALSE),
			(cain_sip_object_destroy_t)cain_sip_stream_listening_point_uninit,
			NULL,
			NULL
		},
		"TCP",
		stream_create_channel
	}
};


cain_sip_listening_point_t * cain_sip_stream_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port){
	cain_sip_stream_listening_point_t *lp=cain_sip_object_new(cain_sip_stream_listening_point_t);
	cain_sip_listening_point_init((cain_sip_listening_point_t*)lp,s,ipaddress,port);
	return CAIN_SIP_LISTENING_POINT(lp);
}
