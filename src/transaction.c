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
		case CAIN_SIP_TRANSACTION_ACCEPTED:
			return "ACCEPTED";
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
	if (t->last_response) cain_sip_object_unref(t->last_response);
	if (t->channel) cain_sip_object_unref(t->channel);
	if (t->branch_id) cain_sip_free(t->branch_id);
	if (t->dialog) cain_sip_object_unref(t->dialog);

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
	t->state=CAIN_SIP_TRANSACTION_TERMINATED;
	CAIN_SIP_OBJECT_VPTR(t,cain_sip_transaction_t)->on_terminate(t);
	cain_sip_provider_set_transaction_terminated(t->provider,t);
}

cain_sip_request_t *cain_sip_transaction_get_request(const cain_sip_transaction_t *t){
	return t->request;
}

void cain_sip_transaction_notify_timeout(cain_sip_transaction_t *t){
	cain_sip_timeout_event_t ev;
	ev.source=t->provider;
	ev.transaction=t;
	ev.is_server_transaction=CAIN_SIP_OBJECT_IS_INSTANCE_OF(t,cain_sip_server_transaction_t);
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS_FOR_TRANSACTION(t,process_timeout,&ev);
}

cain_sip_dialog_t*  cain_sip_transaction_get_dialog(const cain_sip_transaction_t *t) {
	return t->dialog;
}

void cain_sip_transaction_set_dialog(cain_sip_transaction_t *t, cain_sip_dialog_t *dialog){
	if (dialog) cain_sip_object_ref(dialog);
	if (t->dialog) cain_sip_object_unref(t->dialog); /*to avoid keeping unexpected ref*/
	t->dialog=dialog;
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

void cain_sip_server_transaction_init(cain_sip_server_transaction_t *t, cain_sip_provider_t *prov,cain_sip_request_t *req){
	cain_sip_header_via_t *via=CAIN_SIP_HEADER_VIA(cain_sip_message_get_header((cain_sip_message_t*)req,"via"));
	t->base.branch_id=cain_sip_strdup(cain_sip_header_via_get_branch(via));
	cain_sip_transaction_init((cain_sip_transaction_t*)t,prov,req);
	cain_sip_random_token(t->to_tag,sizeof(t->to_tag));
}

void cain_sip_server_transaction_send_response(cain_sip_server_transaction_t *t, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)t;
	cain_sip_header_to_t *to=(cain_sip_header_to_t*)cain_sip_message_get_header((cain_sip_message_t*)resp,"to");
	cain_sip_dialog_t *dialog=base->dialog;
	int status_code;
	
	cain_sip_object_ref(resp);
	if (!base->last_response){
		cain_sip_hop_t hop;
		cain_sip_response_get_return_hop(resp,&hop);
		base->channel=cain_sip_provider_get_channel(base->provider,hop.host, hop.port, hop.transport);
		cain_sip_object_ref(base->channel);
		cain_sip_hop_free(&hop);
	}
	status_code=cain_sip_response_get_status_code(resp);
	if (status_code!=100){
		if (cain_sip_header_to_get_tag(to)==NULL){
			//add a random to tag
			cain_sip_header_to_set_tag(to,t->to_tag);
		}
		/*12.1 Creation of a Dialog

		   Dialogs are created through the generation of non-failure responses
		   to requests with specific methods.  Within this specification, only
		   2xx and 101-199 responses with a To tag, where the request was
		   INVITE, will establish a dialog.*/
		if (dialog && status_code>100 && status_code<300){

			cain_sip_response_fill_for_dialog(resp,base->request);
		}
	}
	if (CAIN_SIP_OBJECT_VPTR(t,cain_sip_server_transaction_t)->send_new_response(t,resp)==0){
		if (base->last_response)
			cain_sip_object_unref(base->last_response);
		base->last_response=resp;
	}
	if (dialog)
		cain_sip_dialog_update(dialog,base->request,resp,TRUE);
}

static void server_transaction_notify(cain_sip_server_transaction_t *t, cain_sip_request_t *req, cain_sip_dialog_t *dialog){
	cain_sip_request_event_t event;

	event.source=t->base.provider;
	event.server_transaction=t;
	event.dialog=dialog;
	event.request=req;
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS_FOR_TRANSACTION(((cain_sip_transaction_t*) t),process_request_event,&event);
}

void cain_sip_server_transaction_on_request(cain_sip_server_transaction_t *t, cain_sip_request_t *req){
	const char *method=cain_sip_request_get_method(req);
	if (strcmp(method,"ACK")==0){
		/*this must be for an INVITE server transaction */
		if (CAIN_SIP_OBJECT_IS_INSTANCE_OF(t,cain_sip_ist_t)){
			cain_sip_ist_t *ist=(cain_sip_ist_t*)t;
			if (cain_sip_ist_process_ack(ist,(cain_sip_message_t*)req)==0){
				cain_sip_dialog_t *dialog=t->base.dialog;
				if (dialog && cain_sip_dialog_handle_ack(dialog,req)==0)
					server_transaction_notify(t,req,dialog);
				/*else nothing to do because retransmission of ACK*/

			}
		}else{
			cain_sip_warning("ACK received for non-invite server transaction ?");
		}
	}else if (strcmp(method,"CANCEL")==0){
		server_transaction_notify(t,req,t->base.dialog);
	}else
		CAIN_SIP_OBJECT_VPTR(t,cain_sip_server_transaction_t)->on_request_retransmission(t);
}

/*
 * client transaction
 */



cain_sip_request_t * cain_sip_client_transaction_create_cancel(cain_sip_client_transaction_t *t){
	cain_sip_message_t *orig=(cain_sip_message_t*)t->base.request;
	cain_sip_request_t *req;
	const char *orig_method=cain_sip_request_get_method((cain_sip_request_t*)orig);
	if (strcmp(orig_method,"ACK")==0 || strcmp(orig_method,"INVITE")!=0){
		cain_sip_error("cain_sip_client_transaction_create_cancel() cannot be used for ACK or non-INVITE transactions.");
		return NULL;
	}
	if (t->base.state!=CAIN_SIP_TRANSACTION_PROCEEDING){
		cain_sip_error("cain_sip_client_transaction_create_cancel() can only be used in state CAIN_SIP_TRANSACTION_PROCEEDING"
		               " but current transaction state is %s",cain_sip_transaction_state_to_string(t->base.state));
		return NULL;
	}
	req=cain_sip_request_new();
	cain_sip_request_set_method(req,"CANCEL");
/*	9.1 Client Behavior
	   Since requests other than INVITE are responded to immediately,
	   sending a CANCEL for a non-INVITE request would always create a
	   race condition.
	   The following procedures are used to construct a CANCEL request.  The
	   Request-URI, Call-ID, To, the numeric part of CSeq, and From header
	   fields in the CANCEL request MUST be identical to those in the
	   request being cancelled, including tags.  A CANCEL constructed by a
	   client MUST have only a single Via header field value matching the
	   top Via value in the request being cancelled.*/
	cain_sip_request_set_uri(req,(cain_sip_uri_t*)cain_sip_object_clone((cain_sip_object_t*)cain_sip_request_get_uri((cain_sip_request_t*)orig)));
	cain_sip_util_copy_headers(orig,(cain_sip_message_t*)req,"via",FALSE);
	cain_sip_util_copy_headers(orig,(cain_sip_message_t*)req,"call-id",FALSE);
	cain_sip_util_copy_headers(orig,(cain_sip_message_t*)req,"from",FALSE);
	cain_sip_util_copy_headers(orig,(cain_sip_message_t*)req,"to",FALSE);
	cain_sip_util_copy_headers(orig,(cain_sip_message_t*)req,"route",TRUE);
	cain_sip_message_add_header((cain_sip_message_t*)req,
		(cain_sip_header_t*)cain_sip_header_cseq_create(
			cain_sip_header_cseq_get_seq_number((cain_sip_header_cseq_t*)cain_sip_message_get_header(orig,"cseq")),
		    "CANCEL"));
	return req;
}


int cain_sip_client_transaction_send_request(cain_sip_client_transaction_t *t){
	return cain_sip_client_transaction_send_request_to(t,NULL);

}
int cain_sip_client_transaction_send_request_to(cain_sip_client_transaction_t *t,cain_sip_uri_t* outbound_proxy) {
	cain_sip_channel_t *chan;
	cain_sip_provider_t *prov=t->base.provider;
	int result=-1;
	
	if (t->base.state!=CAIN_SIP_TRANSACTION_INIT){
		cain_sip_error("cain_sip_client_transaction_send_request: bad state.");
		return -1;
	}

	/*store preset route for future use by refresher*/
	t->preset_route=outbound_proxy;
	if (t->preset_route) cain_sip_object_ref(t->preset_route);
	if (!t->next_hop) {
		if (outbound_proxy) {
			t->next_hop=cain_sip_hop_create(	cain_sip_uri_get_transport_param(outbound_proxy)
					,cain_sip_uri_get_host(outbound_proxy)
					,cain_sip_uri_get_listening_port(outbound_proxy));
		} else {
			t->next_hop = cain_sip_stack_create_next_hop(prov->stack,t->base.request);
		}
	} else {
		/*next hop already preset, probably in case of CANCEL*/
	}
	cain_sip_provider_add_client_transaction(t->base.provider,t); /*add it in any case*/
	chan=cain_sip_provider_get_channel(prov,t->next_hop->host, t->next_hop->port, t->next_hop->transport);
	if (chan){
		cain_sip_object_ref(chan);
		cain_sip_channel_add_listener(chan,CAIN_SIP_CHANNEL_LISTENER(t));
		t->base.channel=chan;
		if (cain_sip_channel_get_state(chan)==CAIN_SIP_CHANNEL_INIT){
			cain_sip_message("cain_sip_client_transaction_send_request(): waiting channel to be ready");
			cain_sip_channel_prepare(chan);
			/*the channel will notify us when it is ready*/
		} else {
			/*otherwise we can send immediately*/
			CAIN_SIP_OBJECT_VPTR(t,cain_sip_client_transaction_t)->send_request(t);
		}
		result=0;
	}else {
		cain_sip_error("cain_sip_client_transaction_send_request(): no channel available");
		cain_sip_transaction_terminate(CAIN_SIP_TRANSACTION(t));
		result=-1;
	}

	return result;
}

static unsigned int should_dialog_be_created(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	cain_sip_request_t* req = cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(t));
	const char* method = cain_sip_request_get_method(req);
	int status_code = cain_sip_response_get_status_code(resp);
	return status_code>=180 && status_code<300 && (strcmp(method,"INVITE")==0 || strcmp(method,"SUBSCRIBE")==0);
}

void cain_sip_client_transaction_notify_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)t;
	cain_sip_request_t* req = cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(t));
	const char* method = cain_sip_request_get_method(req);
	cain_sip_response_event_t event;
	cain_sip_dialog_t *dialog=base->dialog;
	int status_code =  cain_sip_response_get_status_code(resp);
	if (base->last_response)
		cain_sip_object_unref(base->last_response);
	base->last_response=(cain_sip_response_t*)cain_sip_object_ref(resp);

	if (dialog){
		if (status_code>=200 && status_code<300
			&& strcmp(method,"INVITE")==0
			&& (dialog->state==CAIN_SIP_DIALOG_EARLY || dialog->state==CAIN_SIP_DIALOG_CONFIRMED)){
			/*make sure this response matches the current dialog, or creates a new one*/
			if (!cain_sip_dialog_match(dialog,(cain_sip_message_t*)resp,FALSE)){
				dialog=cain_sip_provider_get_new_dialog_internal(t->base.provider,CAIN_SIP_TRANSACTION(t),FALSE);/*cain_sip_dialog_new(base);*/
				if (dialog){
					/*copy userdata to avoid application from being lost*/
					cain_sip_dialog_set_application_data(dialog,cain_sip_dialog_get_application_data(base->dialog));
					cain_sip_message("Handling response creating a new dialog !");
				}
			}
		}
	} else if (should_dialog_be_created(t,resp)) {
		dialog=cain_sip_provider_get_new_dialog_internal(t->base.provider,CAIN_SIP_TRANSACTION(t),FALSE);
	}

	if (dialog)
		cain_sip_dialog_update(dialog,base->request,resp,FALSE);

	event.source=base->provider;
	event.client_transaction=t;
	event.dialog=dialog;
	event.response=(cain_sip_response_t*)resp;
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS_FOR_TRANSACTION(((cain_sip_transaction_t*)t),process_response_event,&event);
	/*check that 200Ok for INVITEs have been acknowledged by listener*/
	if (dialog && strcmp(method,"INVITE")==0)
		cain_sip_dialog_check_ack_sent(dialog);
}


void cain_sip_client_transaction_add_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	CAIN_SIP_OBJECT_VPTR(t,cain_sip_client_transaction_t)->on_response(t,resp);
}

static void client_transaction_destroy(cain_sip_client_transaction_t *t ){
	if (t->preset_route) cain_sip_object_unref(t->preset_route);
	if (t->next_hop) cain_sip_hop_free(t->next_hop);
}

static void on_channel_state_changed(cain_sip_channel_listener_t *l, cain_sip_channel_t *chan, cain_sip_channel_state_t state){
	cain_sip_client_transaction_t *t=(cain_sip_client_transaction_t*)l;
	cain_sip_io_error_event_t ev;
	cain_sip_message("transaction [%p] channel state changed to [%s]"
						,t
						,cain_sip_channel_state_to_string(state));
	switch(state){
		case CAIN_SIP_CHANNEL_READY:
			CAIN_SIP_OBJECT_VPTR(t,cain_sip_client_transaction_t)->send_request(t);
		break;
		case CAIN_SIP_CHANNEL_DISCONNECTED:
		case CAIN_SIP_CHANNEL_ERROR:

			ev.transport=cain_sip_channel_get_transport_name(chan);
			ev.source=CAIN_SIP_OBJECT(t);
			ev.port=chan->peer_port;
			ev.host=chan->peer_name;
			if (cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(t))!=CAIN_SIP_TRANSACTION_COMPLETED
				&& cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(t))!=CAIN_SIP_TRANSACTION_CONFIRMED
				&& cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(t))!=CAIN_SIP_TRANSACTION_ACCEPTED
				&& cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(t))!=CAIN_SIP_TRANSACTION_TERMINATED) {
				CAIN_SIP_PROVIDER_INVOKE_LISTENERS_FOR_TRANSACTION(((cain_sip_transaction_t*)t),process_io_error,&ev);
			}
			if (cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(t))!=CAIN_SIP_TRANSACTION_TERMINATED) /*avoid double notification*/
				cain_sip_transaction_terminate(CAIN_SIP_TRANSACTION(t));
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
	char token[CAIN_SIP_BRANCH_ID_LENGTH];

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

cain_sip_refresher_t* cain_sip_client_transaction_create_refresher(cain_sip_client_transaction_t *t) {
	cain_sip_refresher_t* refresher = cain_sip_refresher_new(t);
	if (refresher) {
		cain_sip_refresher_start(refresher);
	}
	return refresher;
}

cain_sip_request_t* cain_sip_client_transaction_create_authenticated_request(cain_sip_client_transaction_t *t) {
	cain_sip_request_t* req=CAIN_SIP_REQUEST(cain_sip_object_clone(CAIN_SIP_OBJECT(cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(t)))));
	cain_sip_header_cseq_t* cseq=cain_sip_message_get_header_by_type(req,cain_sip_header_cseq_t);
	cain_sip_header_cseq_set_seq_number(cseq,cain_sip_header_cseq_get_seq_number(cseq)+1);
	if (cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(t)) != CAIN_SIP_TRANSACTION_COMPLETED
		&& cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(t)) != CAIN_SIP_TRANSACTION_TERMINATED) {
		cain_sip_error("Invalid state [%s] for transaction [%p], should be CAIN_SIP_TRANSACTION_COMPLETED|CAIN_SIP_TRANSACTION_TERMINATED"
					,cain_sip_transaction_state_to_string(cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(t)))
					,t);
		return NULL;
	}
	/*remove auth headers*/
	cain_sip_message_remove_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_AUTHORIZATION);
	cain_sip_message_remove_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_PROXY_AUTHORIZATION);

	/*put auth header*/
	cain_sip_provider_add_authorization(t->base.provider,req,t->base.last_response,NULL);
	return req;
}

