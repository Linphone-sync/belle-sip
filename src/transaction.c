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

const char *cain_sip_transaction_state_to_string(cain_sip_transaction_state_t state){
	switch(state){
		case CAIN_SIP_TRANSACTION_INIT:
			return "INIT";
		case CAIN_SIP_TRANSACTION_TRYING:
			return "TRYING";
		case CAIN_SIP_TRANSACTION_CALLING:
			return "CALLING";
		case CAIN_SIP_TRANSACTION_COMPLETED:
			return "COMPLETED";
		case CAIN_SIP_TRANSACTION_CONFIRMED:
			return "CONFIRMED";
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			return "PROCEEDING";
		case CAIN_SIP_TRANSACTION_TERMINATED:
			return "TERMINATED";
	}
	cain_sip_fatal("Invalid transaction state.");
	return "INVALID";
}

static void cain_sip_transaction_init(cain_sip_transaction_t *t, cain_sip_provider_t *prov, cain_sip_request_t *req){
	t->request=(cain_sip_request_t*)cain_sip_object_ref(req);
	t->provider=prov;
}

static void transaction_destroy(cain_sip_transaction_t *t){
	if (t->request) cain_sip_object_unref(t->request);
	if (t->prov_response) cain_sip_object_unref(t->prov_response);
	if (t->final_response) cain_sip_object_unref(t->final_response);
	if (t->channel) cain_sip_object_unref(t->channel);
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

/*
 * client transaction
 */


static void clone_headers(cain_sip_message_t *orig, cain_sip_message_t *dest, const char*header, int multiple){
	const cain_sip_list_t *elem;
	elem=cain_sip_message_get_headers(orig,header);
	for (;elem!=NULL;elem=elem->next){
		cain_sip_header_t *ref_header=(cain_sip_header_t*)elem->data;
		if (ref_header){
			cain_sip_message_add_header(dest,
	                           (cain_sip_header_t*)cain_sip_object_clone((cain_sip_object_t*)ref_header));
		}
		if (!multiple) break; /*just one*/
	}
}

cain_sip_request_t * cain_sip_client_transaction_create_cancel(cain_sip_client_transaction_t *t){
	cain_sip_message_t *orig=(cain_sip_message_t*)t->base.request;
	cain_sip_request_t *req;
	const char *orig_method=cain_sip_request_get_method((cain_sip_request_t*)orig);
	if (strcmp(orig_method,"ACK")==0 || strcmp(orig_method,"INVITE")!=0){
		cain_sip_error("cain_sip_client_transaction_create_cancel() cannot be used for ACK or non-INVITE transactions.");
		return NULL;
	}
	if (t->base.state==CAIN_SIP_TRANSACTION_PROCEEDING){
		cain_sip_error("cain_sip_client_transaction_create_cancel() can only be used in state CAIN_SIP_TRANSACTION_PROCEEDING"
		               " but current transaction state is %s",cain_sip_transaction_state_to_string(t->base.state));
	}
	req=cain_sip_request_new();
	cain_sip_request_set_method(req,"CANCEL");
	cain_sip_request_set_uri(req,(cain_sip_uri_t*)cain_sip_object_clone((cain_sip_object_t*)cain_sip_request_get_uri((cain_sip_request_t*)orig)));
	clone_headers(orig,(cain_sip_message_t*)req,"via",FALSE);
	clone_headers(orig,(cain_sip_message_t*)req,"call-id",FALSE);
	clone_headers(orig,(cain_sip_message_t*)req,"from",FALSE);
	clone_headers(orig,(cain_sip_message_t*)req,"to",FALSE);
	clone_headers(orig,(cain_sip_message_t*)req,"route",TRUE);
	cain_sip_message_add_header((cain_sip_message_t*)req,
		(cain_sip_header_t*)cain_sip_header_cseq_create(
			cain_sip_header_cseq_get_seq_number((cain_sip_header_cseq_t*)cain_sip_message_get_header(orig,"cseq")),
		    "CANCEL"));
	return req;
}


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
		cain_sip_object_ref(chan);
		cain_sip_channel_add_listener(chan,CAIN_SIP_CHANNEL_LISTENER(t));
		t->base.channel=chan;
		if (cain_sip_channel_get_state(chan)==CAIN_SIP_CHANNEL_INIT)
			cain_sip_channel_prepare(chan);
		if (cain_sip_channel_get_state(chan)!=CAIN_SIP_CHANNEL_READY){
			cain_sip_message("cain_sip_client_transaction_send_request(): waiting channel to be ready");
		}
	}else cain_sip_error("cain_sip_client_transaction_send_request(): no channel available");
}

int cain_sip_client_transaction_add_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)t;
	int pass=CAIN_SIP_OBJECT_VPTR(t,cain_sip_client_transaction_t)->on_response(t,resp);
	if (pass){
		if (base->prov_response)
			cain_sip_object_unref(base->prov_response);
		base->prov_response=(cain_sip_response_t*)cain_sip_object_ref(resp);
	}
	return pass;
}

static void client_transaction_destroy(cain_sip_client_transaction_t *t ){
}

static void on_channel_state_changed(cain_sip_channel_listener_t *l, cain_sip_channel_t *chan, cain_sip_channel_state_t state){
	cain_sip_client_transaction_t *t=(cain_sip_client_transaction_t*)l;
	cain_sip_message("transaction on_channel_state_changed");
	switch(state){
		case CAIN_SIP_CHANNEL_READY:
			cain_sip_provider_add_client_transaction(t->base.provider,t);
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
	cain_sip_header_via_t *via=CAIN_SIP_HEADER_VIA(cain_sip_message_get_header((cain_sip_message_t*)req,"via"));
	char token[10];

	if (!via){
		cain_sip_fatal("cain_sip_client_transaction_init(): No via in request.");
	}
	
	if (strcmp(cain_sip_request_get_method(req),"CANCEL")!=0){
		obj->base.branch_id=cain_sip_strdup_printf(CAIN_SIP_BRANCH_MAGIC_COOKIE ".%s",cain_sip_random_token(token,sizeof(token)));
		cain_sip_header_via_set_branch(via,obj->base.branch_id);
	}else{
		obj->base.branch_id=cain_sip_strdup(cain_sip_header_via_get_branch(via));
	}
	cain_sip_transaction_init((cain_sip_transaction_t*)obj, prov,req);
}

cain_sip_ist_t *cain_sip_ist_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	return NULL;
}

cain_sip_nist_t *cain_sip_nist_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	return NULL;
}

