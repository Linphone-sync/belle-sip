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



static int listener_ctx_compare(const void *c1, const void *c2){
	listener_ctx_t *lc1=(listener_ctx_t*)c1;
	listener_ctx_t *lc2=(listener_ctx_t*)c2;
	return !(lc1->listener==lc2->listener && lc1->data==lc2->data);
}


static void cain_sip_provider_uninit(cain_sip_provider_t *p){
	cain_sip_list_for_each (p->listeners,cain_sip_free);
	cain_sip_list_free(p->listeners);
	cain_sip_list_free(p->lps);
}

static void channel_state_changed(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, cain_sip_channel_state_t state){
}

CAIN_SIP_IMPLEMENT_INTERFACE_BEGIN(cain_sip_provider_t,cain_sip_channel_listener_t)
	channel_state_changed
CAIN_SIP_IMPLEMENT_INTERFACE_END

CAIN_SIP_DECLARE_IMPLEMENTED_INTERFACES_1(cain_sip_provider_t,cain_sip_channel_listener_t);
	
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_provider_t,cain_sip_object_t,cain_sip_provider_uninit,NULL,NULL);

cain_sip_provider_t *cain_sip_provider_new(cain_sip_stack_t *s, cain_sip_listening_point_t *lp){
	cain_sip_provider_t *p=cain_sip_object_new(cain_sip_provider_t);
	p->stack=s;
	cain_sip_provider_add_listening_point(p,lp);
	return p;
}

int cain_sip_provider_add_listening_point(cain_sip_provider_t *p, cain_sip_listening_point_t *lp){
	p->lps=cain_sip_list_append(p->lps,lp);
	return 0;
}

cain_sip_listening_point_t *cain_sip_provider_get_listening_point(cain_sip_provider_t *p, const char *transport){
	cain_sip_list_t *l;
	for(l=p->lps;l!=NULL;l=l->next){
		cain_sip_listening_point_t *lp=(cain_sip_listening_point_t*)l->data;
		if (strcasecmp(cain_sip_listening_point_get_transport(lp),transport)==0)
			return lp;
	}
	return NULL;
}

const cain_sip_list_t *cain_sip_provider_get_listening_points(cain_sip_provider_t *p){
	return p->lps;
}

void cain_sip_provider_add_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l, void *user_ctx){
	listener_ctx_t *lc=cain_sip_new(listener_ctx_t);
	lc->listener=l;
	lc->data=user_ctx;
	p->listeners=cain_sip_list_append(p->listeners,lc);
}

void cain_sip_provider_remove_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l, void *user_ctx){
	listener_ctx_t ctx={l,user_ctx};
	p->listeners=cain_sip_list_delete_custom(p->listeners,listener_ctx_compare,&ctx);
}

cain_sip_header_call_id_t * cain_sip_provider_get_new_call_id(cain_sip_provider_t *prov){
	cain_sip_header_call_id_t *cid=cain_sip_header_call_id_new();
	char tmp[32];
	snprintf(tmp,sizeof(tmp),"%u",cain_sip_random());
	cain_sip_header_call_id_set_call_id(cid,tmp);
	return cid;
}

cain_sip_client_transaction_t *cain_sip_provider_get_new_client_transaction(cain_sip_provider_t *p, cain_sip_request_t *req){
	return cain_sip_client_transaction_new(p,req);
}

cain_sip_server_transaction_t *cain_sip_provider_get_new_server_transaction(cain_sip_provider_t *p, cain_sip_request_t *req){
	return cain_sip_server_transaction_new(p,req);
}

cain_sip_stack_t *cain_sip_provider_get_sip_stack(cain_sip_provider_t *p){
	return p->stack;
}

cain_sip_channel_t * cain_sip_provider_get_channel(cain_sip_provider_t *p, const char *name,
                                                   int port, const char *transport){
	return NULL;
}


void cain_sip_provider_send_request(cain_sip_provider_t *p, cain_sip_request_t *req){
	
}

void cain_sip_provider_send_response(cain_sip_provider_t *p, cain_sip_response_t *resp){
	
}

/*private provider API*/

void cain_sip_provider_set_transaction_terminated(cain_sip_provider_t *p, cain_sip_transaction_t *t){
	cain_sip_transaction_terminated_event_t ev;
	ev.source=p;
	ev.transaction=t;
	ev.is_server_transaction=t->is_server;
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS(p,process_transaction_terminated,&ev);
}

