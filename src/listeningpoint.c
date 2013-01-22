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


void cain_sip_listening_point_init(cain_sip_listening_point_t *lp, cain_sip_stack_t *s, const char *address, int port){
	cain_sip_init_sockets();
	lp->stack=s;
	lp->listening_uri=cain_sip_uri_create(NULL,address);
	cain_sip_object_ref(lp->listening_uri);
	cain_sip_uri_set_port(lp->listening_uri,port);
	cain_sip_uri_set_transport_param(lp->listening_uri,CAIN_SIP_OBJECT_VPTR(lp,cain_sip_listening_point_t)->transport);
}

static void cain_sip_listening_point_uninit(cain_sip_listening_point_t *lp){
	
	cain_sip_listening_point_clean_channels(lp);
	cain_sip_message("Listening [%p] on [%s://%s:%i] destroyed"	,lp
															,cain_sip_uri_get_transport_param(CAIN_SIP_LISTENING_POINT(lp)->listening_uri)
															,cain_sip_uri_get_host(CAIN_SIP_LISTENING_POINT(lp)->listening_uri)
															,cain_sip_uri_get_port(CAIN_SIP_LISTENING_POINT(lp)->listening_uri));
	cain_sip_object_unref(lp->listening_uri);
	lp->channel_listener=NULL; /*does not unref provider*/
	cain_sip_uninit_sockets();
}


void cain_sip_listening_point_add_channel(cain_sip_listening_point_t *lp, cain_sip_channel_t *chan){
	lp->channels=cain_sip_list_append(lp->channels,chan);/*channel is already owned*/
}

cain_sip_channel_t *cain_sip_listening_point_create_channel(cain_sip_listening_point_t *obj, const char *dest, int port){
	cain_sip_channel_t *chan=CAIN_SIP_OBJECT_VPTR(obj,cain_sip_listening_point_t)->create_channel(obj,dest,port);
	if (chan){
		chan->lp=obj;
		cain_sip_channel_add_listener(chan,obj->channel_listener);
		cain_sip_listening_point_add_channel(obj,chan);
	}
	return chan;
}


void cain_sip_listening_point_remove_channel(cain_sip_listening_point_t *lp, cain_sip_channel_t *chan){
	lp->channels=cain_sip_list_remove(lp->channels,chan);
	cain_sip_object_unref(chan);
}


void cain_sip_listening_point_clean_channels(cain_sip_listening_point_t *lp){
	int existing_channels;
	cain_sip_list_t* iterator;
	if ((existing_channels=cain_sip_list_size(lp->channels)) > 0) {
		cain_sip_warning("Listening point destroying [%i] channels",existing_channels);
	}
	for (iterator=lp->channels;iterator!=NULL;iterator=iterator->next) {
		cain_sip_main_loop_remove_source(lp->stack->ml,(cain_sip_source_t*)(iterator->data));
	}
	lp->channels=cain_sip_list_free_with_data(lp->channels,(void (*)(void*))cain_sip_object_unref);
}


CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_listening_point_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_listening_point_t)={
	{ 
		CAIN_SIP_VPTR_INIT(cain_sip_listening_point_t, cain_sip_object_t,FALSE),
		(cain_sip_object_destroy_t)cain_sip_listening_point_uninit,
		NULL,
		NULL
	},
	NULL,
	NULL
};

const char *cain_sip_listening_point_get_ip_address(const cain_sip_listening_point_t *lp){
	return cain_sip_uri_get_host(lp->listening_uri);
}

int cain_sip_listening_point_get_port(const cain_sip_listening_point_t *lp){
	return cain_sip_uri_get_listening_port(lp->listening_uri);
}

const char *cain_sip_listening_point_get_transport(const cain_sip_listening_point_t *lp){
	return cain_sip_uri_get_transport_param(lp->listening_uri);
}

const cain_sip_uri_t* cain_sip_listening_point_get_uri(const  cain_sip_listening_point_t *lp) {
	return lp->listening_uri;
}
int cain_sip_listening_point_get_well_known_port(const char *transport){
	if (strcasecmp(transport,"UDP")==0 || strcasecmp(transport,"TCP")==0 ) return 5060;
	if (strcasecmp(transport,"DTLS")==0 || strcasecmp(transport,"TLS")==0 ) return 5061;
	cain_sip_error("No well known port for transport %s", transport);
	return -1;
}

cain_sip_channel_t *_cain_sip_listening_point_get_channel(cain_sip_listening_point_t *lp,const char *peer_name, int peer_port, const struct addrinfo *addr){
	cain_sip_list_t *elem;
	cain_sip_channel_t *chan;
	
	for(elem=lp->channels;elem!=NULL;elem=elem->next){
		chan=(cain_sip_channel_t*)elem->data;
		if (cain_sip_channel_matches(chan,peer_name,peer_port,addr)){
			return chan;
		}
	}
	return NULL;
}

cain_sip_channel_t *cain_sip_listening_point_get_channel(cain_sip_listening_point_t *lp,const char *peer_name, int peer_port){
	struct addrinfo *res=NULL;
	struct addrinfo hints={0};
	char portstr[20];
	cain_sip_channel_t *chan;

	hints.ai_flags=AI_NUMERICHOST|AI_NUMERICSERV;
	snprintf(portstr,sizeof(portstr),"%i",peer_port);
	getaddrinfo(peer_name,portstr,&hints,&res);
	chan=_cain_sip_listening_point_get_channel(lp,peer_name,peer_port,res);
	if (res) freeaddrinfo(res);
	return chan;
}

void cain_sip_listener_set_channel_listener(cain_sip_listening_point_t *lp,cain_sip_channel_listener_t* channel_listener) {
	lp->channel_listener=channel_listener;
}


