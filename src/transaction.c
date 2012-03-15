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

static void cain_sip_transaction_init(cain_sip_transaction_t *t, cain_sip_provider_t *prov, cain_sip_request_t *req){
	if (req) cain_sip_object_ref(req);
	t->request=(cain_sip_request_t*)cain_sip_object_ref(req);
	t->provider=prov;
}

static void transaction_destroy(cain_sip_transaction_t *t){
	if (t->request) cain_sip_object_unref(t->request);
	if (t->prov_response) cain_sip_object_unref(t->prov_response);
	if (t->final_response) cain_sip_object_unref(t->final_response);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_transaction_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_transaction_t)={
	{
		CAIN_SIP_VPTR_INIT(cain_sip_transaction_t,cain_sip_object_t,FALSE),
		(cain_sip_object_destroy_t) transaction_destroy,
		NULL,/*no clone*/
		NULL,/*no marshall*/
	},
	NULL /*on_terminate*/
};

void *cain_sip_transaction_get_application_data(const cain_sip_transaction_t *t){
	return t->appdata;
}

void cain_sip_transaction_set_application_data(cain_sip_transaction_t *t, void *data){
	t->appdata=data;
}

const char *cain_sip_transaction_get_branch_id(const cain_sip_transaction_t *t){
	return t->branch_id;
}

cain_sip_transaction_state_t cain_sip_transaction_get_state(const cain_sip_transaction_t *t){
	return t->state;
}

void cain_sip_transaction_terminate(cain_sip_transaction_t *t){
	cain_sip_transaction_terminated_event_t ev;
	
	t->state=CAIN_SIP_TRANSACTION_TERMINATED;
	cain_sip_provider_set_transaction_terminated(t->provider,t);
	CAIN_SIP_OBJECT_VPTR(t,cain_sip_transaction_t)->on_terminate(t);
	
	ev.source=t->provider;
	ev.transaction=t;
	ev.is_server_transaction=CAIN_SIP_IS_INSTANCE_OF(t,cain_sip_server_transaction_t);
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS(t->provider,process_transaction_terminated,&ev);
}

cain_sip_request_t *cain_sip_transaction_get_request(cain_sip_transaction_t *t){
	return t->request;
}

void cain_sip_transaction_notify_timeout(cain_sip_transaction_t *t){
	cain_sip_timeout_event_t ev;
	ev.source=t->provider;
	ev.transaction=t;
	ev.is_server_transaction=CAIN_SIP_OBJECT_IS_INSTANCE_OF(t,cain_sip_server_transaction_t);
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS(t->provider,process_timeout,&ev);
}

/*
 * Server transaction
 */

static void server_transaction_destroy(cain_sip_server_transaction_t *t){
}


CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_server_transaction_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_server_transaction_t)={
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_server_transaction_t,cain_sip_transaction_t,FALSE),
			(cain_sip_object_destroy_t) server_transaction_destroy,
			NULL,
			NULL
		},
		NULL
	}
};

cain_sip_server_transaction_t * cain_sip_server_transaction_new(cain_sip_provider_t *prov,cain_sip_request_t *req){
	cain_sip_server_transaction_t *t=cain_sip_object_new(cain_sip_server_transaction_t);
	cain_sip_transaction_init((cain_sip_transaction_t*)t,prov,req);
	return t;
}

void cain_sip_server_transaction_send_response(cain_sip_server_transaction_t *t, cain_sip_response_t *resp){
	//CAIN_SIP_OBJECT_VPTR(t,cain_sip_transaction_t)->on_response((cain_sip_transaction_t*)t,resp);
}


cain_sip_request_t * cain_sip_client_transaction_create_cancel(cain_sip_client_transaction_t *t){
	return NULL;
}

/*
 * client transaction
 */

void cain_sip_client_transaction_send_request(cain_sip_client_transaction_t *t){
	cain_sip_hop_t hop={0};
	cain_sip_channel_t *chan;
	cain_sip_provider_t *prov=t->base.provider;
	
	if (t->base.state!=CAIN_SIP_TRANSACTION_INIT){
		cain_sip_error("cain_sip_client_transaction_send_request: bad state.");
		return;
	}
	
	cain_sip_stack_get_next_hop(prov->stack,t->base.request,&hop);
	chan=cain_sip_provider_get_channel(prov,hop.host, hop.port, hop.transport);
	if (chan){
		cain_sip_channel_add_listener(chan,CAIN_SIP_CHANNEL_LISTENER(t));
		t->base.channel=chan;
		if (cain_sip_channel_get_state(chan)==CAIN_SIP_CHANNEL_READY){
			CAIN_SIP_OBJECT_VPTR(t,cain_sip_client_transaction_t)->send_request(t);
		}
	}
}

void cain_sip_client_transaction_add_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)t;
	int pass=CAIN_SIP_OBJECT_VPTR(t,cain_sip_client_transaction_t)->on_response(t,resp);
	if (pass){
		cain_sip_response_event_t ev;

		if (base->prov_response)
			cain_sip_object_unref(base->prov_response);
		base->prov_response=(cain_sip_response_t*)cain_sip_object_ref(resp);
		
		ev.source=base->provider;
		ev.response=resp;
		ev.client_transaction=t;
		ev.dialog=NULL;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(base->provider,process_response_event,&ev);
	}
}

static void client_transaction_destroy(cain_sip_client_transaction_t *t ){
}

static void on_channel_state_changed(cain_sip_channel_listener_t *l, cain_sip_channel_t *chan, cain_sip_channel_state_t state){
	cain_sip_client_transaction_t *t=(cain_sip_client_transaction_t*)l;
	switch(state){
		case CAIN_SIP_CHANNEL_READY:
			CAIN_SIP_OBJECT_VPTR(t,cain_sip_client_transaction_t)->send_request(t);
		break;
		default:
			/*ignored*/
		break;
	}
}

CAIN_SIP_IMPLEMENT_INTERFACE_BEGIN(cain_sip_client_transaction_t,cain_sip_channel_listener_t)
on_channel_state_changed,
NULL,
NULL
CAIN_SIP_IMPLEMENT_INTERFACE_END

CAIN_SIP_DECLARE_IMPLEMENTED_INTERFACES_1(cain_sip_client_transaction_t, cain_sip_channel_listener_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_client_transaction_t)={
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_client_transaction_t,cain_sip_transaction_t,FALSE),
			(cain_sip_object_destroy_t)client_transaction_destroy,
			NULL,
			NULL
		},
		NULL
	},
	NULL,
	NULL
};

void cain_sip_client_transaction_init(cain_sip_client_transaction_t *obj, cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_transaction_init((cain_sip_transaction_t*)obj, prov,req);
}

cain_sip_ist_t *cain_sip_ist_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	return NULL;
}

cain_sip_nist_t *cain_sip_nist_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	return NULL;
}

