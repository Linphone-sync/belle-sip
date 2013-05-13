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

#define DEFAULT_RETRY_AFTER 60000
typedef enum cain_sip_refresher_state {
	started,
	stopped
}cain_sip_refresher_state_t;

struct cain_sip_refresher {
	cain_sip_object_t obj;
	cain_sip_refresher_listener_t listener;
	cain_sip_source_t* timer;
	cain_sip_client_transaction_t* transaction;
	int expires;
	cain_sip_refresher_state_t state;
	cain_sip_listener_callbacks_t listener_callbacks;
	cain_sip_listener_t *sip_listener;
	void* user_data;
	int retry_after;
	cain_sip_list_t* auth_events;
	cain_sip_header_contact_t* nated_contact;
	int enable_nat_helper;
	int auth_failures;
};
static int set_expires_from_trans(cain_sip_refresher_t* refresher);

static int timer_cb(void *user_data, unsigned int events) ;
static int cain_sip_refresher_refresh_internal(cain_sip_refresher_t* refresher,int expires,int auth_mandatory, cain_sip_list_t** auth_infos);


static void schedule_timer_at(cain_sip_refresher_t* refresher,int delay) {
	cain_sip_message("Refresher: scheduling next timer in %i ms",delay);
	if (refresher->timer){
		cain_sip_main_loop_remove_source(cain_sip_stack_get_main_loop(refresher->transaction->base.provider->stack),refresher->timer);
		cain_sip_object_unref(refresher->timer);
	}
	refresher->timer=cain_sip_timeout_source_new(timer_cb,refresher,delay);
	cain_sip_object_set_name((cain_sip_object_t*)refresher->timer,"Refresher timeout");
	cain_sip_main_loop_add_source(cain_sip_stack_get_main_loop(refresher->transaction->base.provider->stack),refresher->timer);

}
static void retry_later(cain_sip_refresher_t* refresher) {
	schedule_timer_at(refresher,refresher->retry_after);
}

static void schedule_timer(cain_sip_refresher_t* refresher) {
	schedule_timer_at(refresher,refresher->expires*900);
}

static void process_dialog_terminated(void *user_ctx, const cain_sip_dialog_terminated_event_t *event){
	/*nop*/
}
static void process_io_error(void *user_ctx, const cain_sip_io_error_event_t *event){
	cain_sip_refresher_t* refresher=(cain_sip_refresher_t*)user_ctx;
	cain_sip_client_transaction_t*client_transaction;

	if (cain_sip_object_is_instance_of(CAIN_SIP_OBJECT(cain_sip_io_error_event_get_source(event)),CAIN_SIP_TYPE_ID(cain_sip_client_transaction_t))) {
		client_transaction=CAIN_SIP_CLIENT_TRANSACTION(cain_sip_io_error_event_get_source(event));
		if (!refresher || (refresher && ((refresher->state==stopped
											&& cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(refresher->transaction)) != CAIN_SIP_TRANSACTION_TRYING
											&& cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(refresher->transaction)) != CAIN_SIP_TRANSACTION_INIT /*to cover dns or certificate error*/)
										|| client_transaction !=refresher->transaction )))
				return; /*not for me or no longuer involved*/

		if (refresher->state==started) retry_later(refresher);
		if (refresher->listener) refresher->listener(refresher,refresher->user_data,503, "io error");
		return;
	} else if (cain_sip_object_is_instance_of(CAIN_SIP_OBJECT(cain_sip_io_error_event_get_source(event)),CAIN_SIP_TYPE_ID(cain_sip_provider_t))) {
		/*something went wrong on this provider, checking if my channel is still up*/
		if (refresher->state==started  /*refresher started or trying to refresh */
				&& cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(refresher->transaction)) == CAIN_SIP_TRANSACTION_TERMINATED /*else we are notified by transaction error*/
				&&	(cain_sip_channel_get_state(refresher->transaction->base.channel) == CAIN_SIP_CHANNEL_DISCONNECTED
								||cain_sip_channel_get_state(refresher->transaction->base.channel) == CAIN_SIP_CHANNEL_ERROR)) {
			cain_sip_message("refresher [%p] has channel [%p] in state [%s], reporting error"
								,refresher
								,refresher->transaction->base.channel
								,cain_sip_channel_state_to_string(cain_sip_channel_get_state(refresher->transaction->base.channel)));
			if (refresher->state==started) retry_later(refresher);
			if (refresher->listener) refresher->listener(refresher,refresher->user_data,503, "io error");
		}
		return;
	}else {
		cain_sip_error("Refresher process_io_error not implemented yet for non transaction/provider source");
	}
}



static void process_response_event(void *user_ctx, const cain_sip_response_event_t *event){
	cain_sip_client_transaction_t* client_transaction = cain_sip_response_event_get_client_transaction(event);
	cain_sip_response_t* response = cain_sip_response_event_get_response(event);
	cain_sip_request_t* request=cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(client_transaction));
	int response_code = cain_sip_response_get_status_code(response);
	cain_sip_refresher_t* refresher=(cain_sip_refresher_t*)user_ctx;
	cain_sip_header_contact_t* contact_header;

	if (refresher && (client_transaction !=refresher->transaction))
		return; /*not for me*/

	if ((contact_header=cain_sip_message_get_header_by_type(request,cain_sip_header_contact_t))) {
		if (refresher->nated_contact) cain_sip_object_unref(refresher->nated_contact);
		refresher->nated_contact=CAIN_SIP_HEADER_CONTACT(cain_sip_object_clone(CAIN_SIP_OBJECT(contact_header)));
		cain_sip_object_ref(refresher->nated_contact);
		cain_sip_response_fix_contact(response,refresher->nated_contact);
	}
	/*handle authorization*/
	switch (response_code) {
	case 200:
		refresher->auth_failures=0;
		/*great, success*/
		if (strcmp(cain_sip_request_get_method(request),"PUBLISH")==0) {
			/*search for etag*/
			cain_sip_header_t* etag=cain_sip_message_get_header(CAIN_SIP_MESSAGE(response),"SIP-ETag");
			if (etag) {
				cain_sip_header_t* sip_if_match = cain_sip_header_create("SIP-If-Match",cain_sip_header_extension_get_value(CAIN_SIP_HEADER_EXTENSION(etag)));
				/*update request for next refresh*/
				cain_sip_message_remove_header(CAIN_SIP_MESSAGE(request),"SIP-If-Match");
				cain_sip_message_add_header(CAIN_SIP_MESSAGE(request),sip_if_match);
			} else {
				cain_sip_warning("Refresher [%p] receive 200ok to a publish without etag");
			}
		}
		/*update expire if needed*/
		set_expires_from_trans(refresher);
		if (refresher->state==started) schedule_timer(refresher); /*re-arm timer*/
		else cain_sip_message("Refresher [%p] not scheduling next refresh, because it was stopped");
		break;
	case 401:
	case 407:
		refresher->auth_failures++;
		if (refresher->auth_failures>3){
			/*avoid looping with 407 or 401 */
			cain_sip_warning("Authentication is failling constantly, giving up.");
			if (refresher->expires>0) retry_later(refresher);
			break;
		}
		if (refresher->auth_events) {
			refresher->auth_events=cain_sip_list_free_with_data(refresher->auth_events,(void (*)(void*))cain_sip_auth_event_destroy);
		}
		if (cain_sip_refresher_refresh_internal(refresher,refresher->expires,TRUE,&refresher->auth_events))
			break; /*Notify user of registration failure*/
		else
			return; /*ok, keep 401 internal*/
	case 423:{
		cain_sip_header_extension_t *min_expires=CAIN_SIP_HEADER_EXTENSION(cain_sip_message_get_header((cain_sip_message_t*)response,"Min-Expires"));
		if (min_expires){
			const char *value=cain_sip_header_extension_get_value(min_expires);
			if (value){
				int new_expires=atoi(value);
				if (new_expires>0 && refresher->state==started){
					refresher->expires=new_expires;
					cain_sip_refresher_refresh(refresher,refresher->expires);
					return;
				}
			}
		}else cain_sip_warning("Receiving 423 but no min-expires header.");
		break;
	}
	case 408:
	case 480:
	case 503:
	case 504:
		if (refresher->expires>0) retry_later(refresher);
		break;
	default:
		break;
	}
	if (refresher->listener) refresher->listener(refresher,refresher->user_data,response_code, cain_sip_response_get_reason_phrase(response));

}
static void process_timeout(void *user_ctx, const cain_sip_timeout_event_t *event) {
	cain_sip_refresher_t* refresher=(cain_sip_refresher_t*)user_ctx;
	cain_sip_client_transaction_t*client_transaction =cain_sip_timeout_event_get_client_transaction(event);

	if (refresher && (client_transaction !=refresher->transaction))
		return; /*not for me*/

	if (refresher->state==started) {
		/*retry in 2 seconds but not immediately to let the current transaction be cleaned*/
		schedule_timer_at(refresher,2000);
	}
	if (refresher->listener) refresher->listener(refresher,refresher->user_data,408, "timeout");
}

static void process_transaction_terminated(void *user_ctx, const cain_sip_transaction_terminated_event_t *event) {
	/*cain_sip_message("process_transaction_terminated Transaction terminated [%p]",event);*/
}

static void destroy(cain_sip_refresher_t *refresher){
	cain_sip_refresher_stop(refresher);
	cain_sip_provider_remove_internal_sip_listener(refresher->transaction->base.provider,refresher->sip_listener);
	cain_sip_object_unref(refresher->transaction);
	refresher->transaction=NULL;
	cain_sip_object_unref(refresher->sip_listener);
	refresher->sip_listener=NULL;
	if (refresher->auth_events) refresher->auth_events=cain_sip_list_free_with_data(refresher->auth_events,(void (*)(void*))cain_sip_auth_event_destroy);
	if (refresher->nated_contact) cain_sip_object_unref(refresher->nated_contact);

}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_refresher_t);

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_refresher_t, cain_sip_object_t,destroy, NULL, NULL,FALSE);

void cain_sip_refresher_set_listener(cain_sip_refresher_t* refresher, cain_sip_refresher_listener_t listener,void* user_pointer) {
	refresher->listener=listener;
	refresher->user_data=user_pointer;
}

int cain_sip_refresher_refresh(cain_sip_refresher_t* refresher,int expires) {
	return cain_sip_refresher_refresh_internal(refresher,expires,FALSE,NULL);
}

static int cain_sip_refresher_refresh_internal(cain_sip_refresher_t* refresher,int expires,int auth_mandatory, cain_sip_list_t** auth_infos) {
	cain_sip_request_t*old_request=cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(refresher->transaction));
	cain_sip_response_t*old_response=cain_sip_transaction_get_response(CAIN_SIP_TRANSACTION(refresher->transaction));
	cain_sip_dialog_t* dialog = cain_sip_transaction_get_dialog(CAIN_SIP_TRANSACTION(refresher->transaction));
	cain_sip_client_transaction_t* client_transaction;
	cain_sip_request_t* request;
	cain_sip_header_expires_t* expires_header;
	cain_sip_uri_t* preset_route=refresher->transaction->preset_route;
	cain_sip_provider_t* prov=refresher->transaction->base.provider;
	cain_sip_header_contact_t* contact;
	/*first remove timer if any*/
	if (expires >=0) {
		refresher->expires=expires;
	} else {
		/*-1 keep last value*/
	}
	if (!dialog) {
		const cain_sip_transaction_state_t state=cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(refresher->transaction));
		/*create new request*/
		if (cain_sip_transaction_state_is_transient(state)) {
			/*operation pending, cannot update authorization headers*/
			cain_sip_header_cseq_t* cseq;
			cain_sip_message("Refresher [%p] already have transaction [%p] in state [%s]"	,refresher
																							,refresher->transaction
																							,cain_sip_transaction_state_to_string(state));
			request=cain_sip_request_clone_with_body(cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(refresher->transaction)));
			cseq=cain_sip_message_get_header_by_type(request,cain_sip_header_cseq_t);
			cain_sip_header_cseq_set_seq_number(cseq,cain_sip_header_cseq_get_seq_number(cseq)+1);
		} else {
			request=cain_sip_client_transaction_create_authenticated_request(refresher->transaction,auth_infos);
		}
	} else if (dialog && cain_sip_dialog_get_state(dialog)==CAIN_SIP_DIALOG_CONFIRMED) {
		request=cain_sip_dialog_create_request_from(dialog,old_request);
		if (strcmp(cain_sip_request_get_method(request),"SUBSCRIBE")==0) {
			/*put expire header*/
			if (!(expires_header = cain_sip_message_get_header_by_type(request,cain_sip_header_expires_t))) {
				expires_header = cain_sip_header_expires_new();
				cain_sip_message_add_header(CAIN_SIP_MESSAGE(request),CAIN_SIP_HEADER(expires_header));
			}
		}
		cain_sip_provider_add_authorization(prov,request,old_response,auth_infos);
	} else {
		cain_sip_error("Unexpected dialog state [%s] for dialog [%p], cannot refresh [%s]"
				,cain_sip_dialog_state_to_string(cain_sip_dialog_get_state(dialog))
				,dialog
				,cain_sip_request_get_method(old_request));
		return -1;
	}

	if (auth_mandatory && auth_infos && (*auth_infos!=NULL || cain_sip_list_size(*auth_infos)) >0) {
		cain_sip_message("Auth info not found for this refresh operation on [%p]",refresher);
		if (request) cain_sip_object_unref(request);
		return -1;
	}
	if (refresher->enable_nat_helper && refresher->nated_contact) {
		/*update contact with fixed contact*/
		cain_sip_message_remove_header(CAIN_SIP_MESSAGE(request),CAIN_SIP_CONTACT);
		cain_sip_message_add_header(CAIN_SIP_MESSAGE(request),CAIN_SIP_HEADER(refresher->nated_contact));
	}
	/*update expires in any cases*/
	expires_header = cain_sip_message_get_header_by_type(request,cain_sip_header_expires_t);
	if (expires_header)
		cain_sip_header_expires_set_expires(expires_header,refresher->expires);
	contact=cain_sip_message_get_header_by_type(request,cain_sip_header_contact_t);
	if (contact && cain_sip_header_contact_get_expires(contact)>=0)
		cain_sip_header_contact_set_expires(contact,refresher->expires);

	/*update the Date header if it exists*/
	{
		cain_sip_header_date_t *date=cain_sip_message_get_header_by_type(request,cain_sip_header_date_t);
		if (date){
			time_t curtime=time(NULL);
			cain_sip_header_date_set_time(date,&curtime);
		}
	}
	
	client_transaction = cain_sip_provider_get_new_client_transaction(prov,request);
	client_transaction->base.is_internal=1;
	cain_sip_transaction_set_application_data(CAIN_SIP_TRANSACTION(client_transaction),refresher);
	/*update reference transaction for next refresh*/
	cain_sip_object_unref(refresher->transaction);
	refresher->transaction=client_transaction;
	cain_sip_object_ref(refresher->transaction);

	if (cain_sip_client_transaction_send_request_to(client_transaction,preset_route)) {
		cain_sip_error("Cannot send refresh method [%s] for refresher [%p]"
				,cain_sip_request_get_method(request)
				,refresher);
		return -1;
	}
	return 0;
}


static int timer_cb(void *user_data, unsigned int events) {
	cain_sip_refresher_t* refresher = (cain_sip_refresher_t*)user_data;
	refresher->auth_failures=0;/*reset the auth_failures to get a chance to authenticate again*/
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
	if (!response)
		return NULL;
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
			cain_sip_message("No matching contact neither for [%s] nor [%s]", tmp_string, tmp_string2);
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
	cain_sip_header_expires_t*  expires_header=cain_sip_message_get_header_by_type(request,cain_sip_header_expires_t);
	cain_sip_header_contact_t* contact_header;

	refresher->expires=-1;
	
	if (strcmp("REGISTER",cain_sip_request_get_method(request))==0
			|| expires_header /*if request has an expire header, refresher can always work*/) {

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
			if (response && (expires_header=(cain_sip_header_expires_t*)cain_sip_message_get_header(CAIN_SIP_MESSAGE(response),CAIN_SIP_EXPIRES))) {
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
	} else if (strcmp("INVITE",cain_sip_request_get_method(request))==0) {
		cain_sip_error("Refresher does not support INVITE yet");
		return -1;
	} else {
		cain_sip_error("Refresher does not support [%s] yet",cain_sip_request_get_method(request));
		return -1;
	}
	return 0;
}


int cain_sip_refresher_start(cain_sip_refresher_t* refresher) {
	if(refresher->state==started) {
		cain_sip_warning("Refresher [%p] already started",refresher);
	} else {
		if (refresher->expires>0) {
			refresher->state=started;
			schedule_timer(refresher);
			cain_sip_message("Refresher [%p] started, next refresh in [%i] s",refresher,refresher->expires);
		}else{
			cain_sip_message("Refresher [%p] stopped, expires=%i",refresher,refresher->expires);
			refresher->state=stopped;
		}
	}
	return 0;
}

void cain_sip_refresher_stop(cain_sip_refresher_t* refresher) {
	cain_sip_message("Refresher [%p] stopped.",refresher);
	if (refresher->timer){
		cain_sip_main_loop_remove_source(cain_sip_stack_get_main_loop(refresher->transaction->base.provider->stack), refresher->timer);
		cain_sip_object_unref(refresher->timer);
		refresher->timer=NULL;
	}
	refresher->state=stopped;
}

cain_sip_refresher_t* cain_sip_refresher_new(cain_sip_client_transaction_t* transaction) {
	cain_sip_refresher_t* refresher;
	cain_sip_transaction_state_t state=cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(transaction));
	cain_sip_request_t* request = cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(transaction));
	if ( strcmp("REGISTER",cain_sip_request_get_method(request))!=0
			&& strcmp("PUBLISH",cain_sip_request_get_method(request))!=0
			&& state!=CAIN_SIP_TRANSACTION_TERMINATED
			&& state != CAIN_SIP_TRANSACTION_COMPLETED) {
		cain_sip_error("Invalid state [%s] for %s transaction [%p], should be CAIN_SIP_TRANSACTION_COMPLETED/CAIN_SIP_TRANSACTION_TERMINATED"
					,cain_sip_transaction_state_to_string(cain_sip_transaction_get_state(CAIN_SIP_TRANSACTION(transaction)))
					,cain_sip_request_get_method(request)
					,transaction);
		return NULL;
	}
	refresher = (cain_sip_refresher_t*)cain_sip_object_new(cain_sip_refresher_t);
	refresher->transaction=transaction;
	refresher->state=stopped;
	refresher->enable_nat_helper=1;
	cain_sip_object_ref(transaction);
	refresher->retry_after=DEFAULT_RETRY_AFTER;
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
	if (cain_sip_transaction_state_is_transient(state)) {
		cain_sip_message(" refresher [%p] takes ownership of transaction [%p]",refresher,transaction);
		transaction->base.is_internal=1;
		refresher->state=started;
	}
	return refresher;
}

int cain_sip_refresher_get_expires(const cain_sip_refresher_t* refresher) {
	return refresher->expires;
}

int cain_sip_refresher_get_retry_after(const cain_sip_refresher_t* refresher){
	return refresher->retry_after;
}

void cain_sip_refresher_set_retry_after(cain_sip_refresher_t* refresher, int delay_ms) {
	refresher->retry_after=delay_ms;
}
const cain_sip_client_transaction_t* cain_sip_refresher_get_transaction(const cain_sip_refresher_t* refresher) {
	return refresher->transaction;
}
const cain_sip_list_t* cain_sip_refresher_get_auth_events(const cain_sip_refresher_t* refresher) {
	return refresher->auth_events;
}
const cain_sip_header_contact_t* cain_sip_refresher_get_nated_contact(const cain_sip_refresher_t* refresher) {
	return refresher->nated_contact;
}
void cain_sip_refresher_enable_nat_helper(cain_sip_refresher_t* refresher,int enable) {
	refresher->enable_nat_helper=enable;
}
int cain_sip_refresher_is_nat_helper_enabled(const cain_sip_refresher_t* refresher) {
	return refresher->enable_nat_helper;
}
