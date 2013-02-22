/*
	cain-sip - SIP (RFC3261) library.
    Copyright (C) 2012  Belledonne Communications SARL

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
#include "cain-sip/refresher-helper.h"

struct cain_sip_refresher {
	cain_sip_object_t obj;
	cain_sip_refresher_listener_t listener;
	cain_sip_source_t* timer;
	cain_sip_client_transaction_t* transaction;
	int expires;
	unsigned int started;
	cain_sip_listener_callbacks_t listener_callbacks;
	cain_sip_listener_t *sip_listener;
	void* user_data;
};

static void process_dialog_terminated(void *user_ctx, const cain_sip_dialog_terminated_event_t *event){
	/*nop*/
}
static void process_io_error(void *user_ctx, const cain_sip_io_error_event_t *event){
	cain_sip_refresher_t* refresher=(cain_sip_refresher_t*)user_ctx;
	cain_sip_client_transaction_t*client_transaction;

	if (cain_sip_object_is_instance_of(CAIN_SIP_OBJECT(cain_sip_io_error_event_get_source(event)),CAIN_SIP_TYPE_ID(cain_sip_client_transaction_t))) {
		client_transaction=CAIN_SIP_CLIENT_TRANSACTION(cain_sip_io_error_event_get_source(event));
		if (refresher && (client_transaction !=refresher->transaction))
				return; /*not for me*/

		/*first stop timer if any*/
		cain_sip_refresher_stop(refresher);
		refresher->listener(refresher,refresher->user_data,503, "io error");
		return;
	} else if (cain_sip_object_is_instance_of(CAIN_SIP_OBJECT(cain_sip_io_error_event_get_source(event)),CAIN_SIP_TYPE_ID(cain_sip_provider_t))) {
		/*something went wrong on this provider, checking if my channel is still up*/
		if ((refresher->started  /*refresher started or trying to refresh */
				|| cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(refresher->transaction)) == CAIN_SIP_TRANSACTION_TRYING
				|| cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(refresher->transaction)) == CAIN_SIP_TRANSACTION_TRYING)
			&&	(cain_sip_channel_get_state(refresher->transaction->base.channel) == CAIN_SIP_CHANNEL_DISCONNECTED
								||cain_sip_channel_get_state(refresher->transaction->base.channel) == CAIN_SIP_CHANNEL_ERROR)) {
			cain_sip_message("refresher [%p] has channel [%p] in state [%s], reporting error"
								,refresher
								,refresher->transaction->base.channel
								,cain_sip_channel_state_to_string(cain_sip_channel_get_state(refresher->transaction->base.channel)));
			cain_sip_refresher_stop(refresher);
			refresher->listener(refresher,refresher->user_data,503, "io error");
		}
		return;
	}else {
		cain_sip_error("Refresher process_io_error not implemented yet for non transaction/provider source");
	}
}

static int set_expires_from_trans(cain_sip_refresher_t* refresher);

static int timer_cb(void *user_data, unsigned int events) ;

static void schedule_timer(cain_sip_refresher_t* refresher) {
	if (refresher->expires>0) {
		if (refresher->timer){
			cain_sip_main_loop_remove_source(cain_sip_stack_get_main_loop(refresher->transaction->base.provider->stack),refresher->timer);
			cain_sip_object_unref(refresher->timer);
		}
		refresher->timer=cain_sip_timeout_source_new(timer_cb,refresher,refresher->expires*1000);
		cain_sip_object_set_name((cain_sip_object_t*)refresher->timer,"Refresher timeout");
		cain_sip_main_loop_add_source(cain_sip_stack_get_main_loop(refresher->transaction->base.provider->stack),refresher->timer);
	}
}

static void process_response_event(void *user_ctx, const cain_sip_response_event_t *event){
	cain_sip_client_transaction_t* client_transaction = cain_sip_response_event_get_client_transaction(event);
	cain_sip_response_t* response = cain_sip_response_event_get_response(event);
	int response_code = cain_sip_response_get_status_code(response);
	cain_sip_refresher_t* refresher=(cain_sip_refresher_t*)user_ctx;
	if (refresher && (client_transaction !=refresher->transaction))
		return; /*not for me*/

		/*handle authorization*/
		switch (response_code) {
		case 200: {
			/*great, success*/
			/*update expire if needed*/
			set_expires_from_trans(refresher);
			schedule_timer(refresher); /*re-arm timer*/
			break;
		}
		case 401:
		case 407:{
			cain_sip_refresher_refresh(refresher,refresher->expires); /*authorization is supposed to be available immediately*/
			return;
		}
		default:
			break;
		}
		refresher->listener(refresher,refresher->user_data,response_code, cain_sip_response_get_reason_phrase(response));

}
static void process_timeout(void *user_ctx, const cain_sip_timeout_event_t *event) {
	cain_sip_refresher_t* refresher=(cain_sip_refresher_t*)user_ctx;
	cain_sip_client_transaction_t*client_transaction =cain_sip_timeout_event_get_client_transaction(event);

	if (refresher && (client_transaction !=refresher->transaction))
				return; /*not for me*/

		/*first stop timer if any*/
	cain_sip_refresher_stop(refresher);
	refresher->listener(refresher,refresher->user_data,408, "timeout");
	return;
}
static void process_transaction_terminated(void *user_ctx, const cain_sip_transaction_terminated_event_t *event) {
	/*cain_sip_message("process_transaction_terminated Transaction terminated [%p]",event);*/
}

static void destroy(cain_sip_refresher_t *refresher){
	cain_sip_provider_remove_internal_sip_listener(refresher->transaction->base.provider,refresher->sip_listener);
	cain_sip_object_unref(refresher->transaction);
	cain_sip_object_unref(refresher->sip_listener);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_refresher_t);

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_refresher_t, cain_sip_object_t,destroy, NULL, NULL,FALSE);

void cain_sip_refresher_set_listener(cain_sip_refresher_t* refresher, cain_sip_refresher_listener_t listener,void* user_pointer) {
	refresher->listener=listener;
	refresher->user_data=user_pointer;
}

int cain_sip_refresher_refresh(cain_sip_refresher_t* refresher,int expires) {
	cain_sip_request_t*old_request=cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(refresher->transaction));
	cain_sip_dialog_t* dialog = cain_sip_transaction_get_dialog(CAIN_SIP_TRANSACTION(refresher->transaction));
	cain_sip_client_transaction_t* client_transaction;
	cain_sip_request_t* request;
	cain_sip_header_expires_t* expires_header;
	cain_sip_uri_t* preset_route=refresher->transaction->preset_route;
	cain_sip_provider_t* prov=refresher->transaction->base.provider;
	cain_sip_header_contact_t* contact;
	/*first remove timer if any*/
	cain_sip_refresher_stop(refresher);
	refresher->expires=expires;
	if (!dialog) {
		/*create new request*/
		request=cain_sip_client_transaction_create_authenticated_request(refresher->transaction);
	} else if (dialog && cain_sip_dialog_get_state(dialog)==CAIN_SIP_DIALOG_CONFIRMED) {
		request=cain_sip_dialog_create_request_from(dialog,old_request);
		if (strcmp(cain_sip_request_get_method(request),"SUBSCRIBE")==0) {
			/*put expire header*/
			if (!(expires_header = cain_sip_message_get_header_by_type(request,cain_sip_header_expires_t))) {
				expires_header = cain_sip_header_expires_new();
				cain_sip_message_add_header(CAIN_SIP_MESSAGE(request),CAIN_SIP_HEADER(expires_header));
			}


		}
		cain_sip_provider_add_authorization(prov,request,NULL,NULL);
	} else {
		cain_sip_error("Unexpected dialog state [%s] for dialog [%p], cannot refresh [%s]"
				,cain_sip_dialog_state_to_string(cain_sip_dialog_get_state(dialog))
				,dialog
				,cain_sip_request_get_method(old_request));
		return -1;
	}
	/*update expires in any cases*/
	expires_header = cain_sip_message_get_header_by_type(request,cain_sip_header_expires_t);
	if (expires_header)
		cain_sip_header_expires_set_expires(expires_header,refresher->expires);
	contact=cain_sip_message_get_header_by_type(request,cain_sip_header_contact_t);
	if (cain_sip_header_contact_get_expires(contact)>=0)
		cain_sip_header_contact_set_expires(contact,refresher->expires);

	client_transaction = cain_sip_provider_get_new_client_transaction(prov,request);
	client_transaction->base.is_internal=1;
	cain_sip_transaction_set_application_data(CAIN_SIP_TRANSACTION(client_transaction),refresher);
	/*update reference transaction for next refresh*/
	cain_sip_object_unref(refresher->transaction);
	refresher->transaction=client_transaction;
	cain_sip_object_ref(refresher->transaction);

	if (cain_sip_client_transaction_send_request_to(client_transaction,preset_route)) {
		cain_sip_error("Cannot send refresh method [%s] for refresher [%p]"
				,cain_sip_request_get_method(old_request)
				,refresher);
		return -1;
	}
	return 0;
}


static int timer_cb(void *user_data, unsigned int events) {
	cain_sip_refresher_t* refresher = (cain_sip_refresher_t*)user_data;
	cain_sip_refresher_refresh(refresher,refresher->expires);
	return CAIN_SIP_STOP;
}

static cain_sip_header_contact_t* get_matching_contact(const cain_sip_transaction_t* transaction) {
	cain_sip_request_t*request=cain_sip_transaction_get_request(transaction);
	cain_sip_response_t*response=transaction->last_response;
	const cain_sip_list_t* contact_header_list;
	cain_sip_header_contact_t* unfixed_local_contact;
	cain_sip_header_contact_t* fixed_local_contact;
	char* tmp_string;
	char* tmp_string2;
	/*we assume, there is only one contact in request*/
	unfixed_local_contact= cain_sip_message_get_header_by_type(CAIN_SIP_MESSAGE(request),cain_sip_header_contact_t);
	fixed_local_contact= CAIN_SIP_HEADER_CONTACT(cain_sip_object_clone(CAIN_SIP_OBJECT(unfixed_local_contact)));

	/*first fix contact using received/rport*/
	cain_sip_response_fix_contact(response,fixed_local_contact);
	contact_header_list = cain_sip_message_get_headers(CAIN_SIP_MESSAGE(response),CAIN_SIP_CONTACT);

	if (contact_header_list) {
			contact_header_list = cain_sip_list_find_custom((cain_sip_list_t*)contact_header_list
																,(cain_sip_compare_func)cain_sip_header_contact_not_equals
																, (const void*)fixed_local_contact);
			if (!contact_header_list) {
				/*reset header list*/
				contact_header_list = cain_sip_message_get_headers(CAIN_SIP_MESSAGE(response),CAIN_SIP_CONTACT);
				contact_header_list = cain_sip_list_find_custom((cain_sip_list_t*)contact_header_list
																,(cain_sip_compare_func)cain_sip_header_contact_not_equals
																,unfixed_local_contact);
			}
			if (!contact_header_list) {
				tmp_string=cain_sip_object_to_string(CAIN_SIP_OBJECT(fixed_local_contact));
				tmp_string2=cain_sip_object_to_string(CAIN_SIP_OBJECT(unfixed_local_contact));
				cain_sip_error("No matching contact neither for [%s] nor [%s]", tmp_string, tmp_string2);
				cain_sip_free(tmp_string);
				cain_sip_free(tmp_string2);
				return NULL;
			} else {
				return CAIN_SIP_HEADER_CONTACT(contact_header_list->data);
			}
		} else {
		return NULL;
	}

}
static int set_expires_from_trans(cain_sip_refresher_t* refresher) {
	cain_sip_transaction_t* transaction = CAIN_SIP_TRANSACTION(refresher->transaction);
	cain_sip_response_t*response=transaction->last_response;
	cain_sip_request_t*request=cain_sip_transaction_get_request(transaction);
	cain_sip_header_expires_t* expires_header;
	cain_sip_header_contact_t* contact_header;

	refresher->expires=-1;
	
	if (strcmp("REGISTER",cain_sip_request_get_method(request))==0
			|| strcmp("SUBSCRIBE",cain_sip_request_get_method(request))==0) {

		/*An "expires" parameter on the "Contact" header has no semantics for
		*   SUBSCRIBE and is explicitly not equivalent to an "Expires" header in
		*  a SUBSCRIBE request or response.
		*/
		if (strcmp("REGISTER",cain_sip_request_get_method(request))==0
				&& (contact_header=get_matching_contact(transaction))!=NULL){
			refresher->expires=cain_sip_header_contact_get_expires(CAIN_SIP_HEADER_CONTACT(contact_header));
			/*great, we have an expire param from contact header*/
		}
		if (refresher->expires==-1){
			/*no contact with expire or not relevant, looking for Expires header*/
			if ((expires_header=(cain_sip_header_expires_t*)cain_sip_message_get_header(CAIN_SIP_MESSAGE(response),CAIN_SIP_EXPIRES))) {
				refresher->expires = cain_sip_header_expires_get_expires(expires_header);
			}
		}
		if (refresher->expires<0) {
			cain_sip_message("Neither Expires header nor corresponding Contact header found, checking from original request");
			if ((expires_header=(cain_sip_header_expires_t*)cain_sip_message_get_header(CAIN_SIP_MESSAGE(request),CAIN_SIP_EXPIRES))) {
				refresher->expires = cain_sip_header_expires_get_expires(expires_header);
			} else {
				cain_sip_message("Not possible to guess expire value, giving up");
				refresher->expires=0;
				return 1;
			}
		}

	} 	else if (strcmp("INVITE",cain_sip_request_get_method(request))==0) {
		cain_sip_error("Refresher does not support ERROR yet");
		return -1;
	} else {
		cain_sip_error("Refresher does not support [%s] yet",cain_sip_request_get_method(request));
		return -1;
	}
	return 0;
}


int cain_sip_refresher_start(cain_sip_refresher_t* refresher) {
	if(refresher->started) {
		cain_sip_warning("Refresher[%p] already started",refresher);
	} else {
		if (refresher->expires>0) {
			schedule_timer(refresher);
			cain_sip_message("Refresher [%p] started, next refresh in [%i] s",refresher,refresher->expires);
		}
	}
	refresher->started=1;
	return 0;
}

void cain_sip_refresher_stop(cain_sip_refresher_t* refresher) {
	if (refresher->timer){
		cain_sip_main_loop_remove_source(cain_sip_stack_get_main_loop(refresher->transaction->base.provider->stack), refresher->timer);
		cain_sip_object_unref(refresher->timer);
		refresher->timer=NULL;
	}
	refresher->started=0;
}

cain_sip_refresher_t* cain_sip_refresher_new(cain_sip_client_transaction_t* transaction) {
	cain_sip_refresher_t* refresher;
	if (cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(transaction)) != CAIN_SIP_TRANSACTION_COMPLETED) {
		cain_sip_error("Invalid state [%s] for transaction [%p], should be CAIN_SIP_TRANSACTION_COMPLETED"
					,cain_sip_transaction_state_to_string(cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(transaction)))
					,transaction);
		return NULL;
	}
	refresher = (cain_sip_refresher_t*)cain_sip_object_new(cain_sip_refresher_t);
	refresher->transaction=transaction;
	cain_sip_object_ref(transaction);
	refresher->listener_callbacks.process_response_event=process_response_event;
	refresher->listener_callbacks.process_timeout=process_timeout;
	refresher->listener_callbacks.process_io_error=process_io_error;
	refresher->listener_callbacks.process_dialog_terminated=process_dialog_terminated;
	refresher->listener_callbacks.process_transaction_terminated=process_transaction_terminated;;
	refresher->sip_listener=cain_sip_listener_create_from_callbacks(&(refresher->listener_callbacks),refresher);
	cain_sip_provider_add_internal_sip_listener(transaction->base.provider,refresher->sip_listener);
	if (set_expires_from_trans(refresher)){
		cain_sip_error("Unable to extract refresh value from transaction [%p]",transaction);
	}
	return refresher;
}

int cain_sip_refresher_get_expires(const cain_sip_refresher_t* refresher) {
	return refresher->expires;
}
