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

/*
 Listening points: base, udp
*/

struct cain_sip_listening_point{
	cain_sip_object_t base;
	cain_sip_stack_t *stack;
	cain_sip_list_t *channels;
	char *transport;
	char *addr;
	int port;
	int is_reliable;
};

static void cain_sip_listening_point_init(cain_sip_listening_point_t *lp, cain_sip_stack_t *s, const char *transport, const char *address, int port){
	lp->transport=cain_sip_strdup(transport);
	lp->port=port;
	lp->addr=cain_sip_strdup(address);
	lp->stack=s;
}

static void cain_sip_listening_point_uninit(cain_sip_listening_point_t *lp){
	cain_sip_list_free_with_data(lp->channels,(void (*)(void*))cain_sip_object_unref);
	cain_sip_free(lp->addr);
	cain_sip_free(lp->transport);
}

#if 0
static void cain_sip_listening_point_add_channel(cain_sip_listening_point_t *lp, cain_sip_channel_t *chan){
	lp->channels=cain_sip_list_append(lp->channels,chan);
}

static void cain_sip_listening_point_remove_channel(cain_sip_listening_point_t *lp, cain_sip_channel_t *chan){
	lp->channels=cain_sip_list_remove(lp->channels,chan);
}
#endif

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_listening_point_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_listening_point_t,cain_sip_object_t,cain_sip_listening_point_uninit,NULL,NULL);

const char *cain_sip_listening_point_get_ip_address(const cain_sip_listening_point_t *lp){
	return lp->addr;
}

int cain_sip_listening_point_get_port(const cain_sip_listening_point_t *lp){
	return lp->port;
}

const char *cain_sip_listening_point_get_transport(const cain_sip_listening_point_t *lp){
	return lp->transport;
}

int cain_sip_listening_point_is_reliable(const cain_sip_listening_point_t *lp){
	return lp->is_reliable;
}

int cain_sip_listening_point_get_well_known_port(const char *transport){
	if (strcasecmp(transport,"UDP")==0 || strcasecmp(transport,"TCP")==0 ) return 5060;
	if (strcasecmp(transport,"DTLS")==0 || strcasecmp(transport,"TLS")==0 ) return 5061;
	cain_sip_error("No well known port for transport %s", transport);
	return -1;
}

cain_sip_channel_t *cain_sip_listening_point_find_channel (cain_sip_listening_point_t *lp,const char *peer_name, int peer_port){
	cain_sip_list_t *elem;
	for(elem=lp->channels;elem!=NULL;elem=elem->next){
		cain_sip_channel_t *chan=(cain_sip_channel_t*)elem->data;
		if (cain_sip_channel_matches(chan,peer_name,peer_port))
			return chan;
	}
	return NULL;
}

struct cain_sip_udp_listening_point{
	cain_sip_listening_point_t base;
	cain_sip_channel_t *channel;
	int sock;
};


static void cain_sip_udp_listening_point_uninit(cain_sip_udp_listening_point_t *lp){
	cain_sip_object_unref(lp->channel);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_udp_listening_point_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_udp_listening_point_t,cain_sip_listening_point_t,cain_sip_udp_listening_point_uninit,NULL,NULL);


cain_sip_listening_point_t * cain_sip_udp_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port){
	cain_sip_udp_listening_point_t *lp=cain_sip_object_new(cain_sip_udp_listening_point_t);
	cain_sip_listening_point_init((cain_sip_listening_point_t*)lp,s,"UDP",ipaddress,port);
	lp->base.is_reliable=FALSE;
	//cain_sip_listening_point_add_channel(lp,cain_sip_channel_new_udp_master(s->provider
	return CAIN_SIP_LISTENING_POINT(lp);
}


