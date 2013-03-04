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

static void cain_sip_dialog_init_200Ok_retrans(cain_sip_dialog_t *obj, cain_sip_response_t *resp);
static void cain_sip_dialog_stop_200Ok_retrans(cain_sip_dialog_t *obj);
static int cain_sip_dialog_handle_200Ok(cain_sip_dialog_t *obj, cain_sip_response_t *msg);

static void cain_sip_dialog_uninit(cain_sip_dialog_t *obj){
	if (obj->route_set)
		cain_sip_list_free_with_data(obj->route_set,cain_sip_object_unref);
	if (obj->remote_target)
		cain_sip_object_unref(obj->remote_target);
	if (obj->call_id)
		cain_sip_object_unref(obj->call_id);
	if (obj->local_party)
		cain_sip_object_unref(obj->local_party);
	if (obj->remote_party)
		cain_sip_object_unref(obj->remote_party);
	if (obj->local_tag)
		cain_sip_free(obj->local_tag);
	if (obj->remote_tag)
		cain_sip_free(obj->remote_tag);
	if (obj->last_out_invite)
		cain_sip_object_unref(obj->last_out_invite);
	if (obj->last_out_ack)
		cain_sip_object_unref(obj->last_out_ack);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_dialog_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_dialog_t)={ 
		CAIN_SIP_VPTR_INIT(cain_sip_dialog_t, cain_sip_object_t,TRUE),
		(cain_sip_object_destroy_t)cain_sip_dialog_uninit,
		NULL,
		NULL
};
const char* cain_sip_dialog_state_to_string(const cain_sip_dialog_state_t state) {
	switch(state) {
	case CAIN_SIP_DIALOG_NULL: return "CAIN_SIP_DIALOG_NULL";
	case CAIN_SIP_DIALOG_EARLY: return "CAIN_SIP_DIALOG_EARLY";
	case CAIN_SIP_DIALOG_CONFIRMED: return "CAIN_SIP_DIALOG_CONFIRMED";
	case CAIN_SIP_DIALOG_TERMINATED: return "CAIN_SIP_DIALOG_TERMINATED";
	default: return "Unknown state";
	}
}

static void set_state(cain_sip_dialog_t *obj,cain_sip_dialog_state_t state) {
	obj->previous_state=obj->state;
	obj->state=state;
}

static void set_to_tag(cain_sip_dialog_t *obj, cain_sip_header_to_t *to){
	const char *to_tag=cain_sip_header_to_get_tag(to);
	if (obj->is_server){
		if (to_tag && !obj->local_tag)
			obj->local_tag=cain_sip_strdup(to_tag);
	}else{
		if (to_tag && !obj->remote_tag)
			obj->remote_tag=cain_sip_strdup(to_tag);
	}
}

static void check_route_set(cain_sip_list_t *rs){
	if (rs){
		cain_sip_header_route_t *r=(cain_sip_header_route_t*)rs->data;
		if (!cain_sip_uri_has_lr_param(cain_sip_header_address_get_uri((cain_sip_header_address_t*)r))){
			cain_sip_warning("top uri of route set does not contain 'lr', not really supported.");
		}
	}
}

static int cain_sip_dialog_init_as_uas(cain_sip_dialog_t *obj, cain_sip_request_t *req, cain_sip_response_t *resp){
	const cain_sip_list_t *elem;
	cain_sip_header_contact_t *ct=cain_sip_message_get_header_by_type(req,cain_sip_header_contact_t);
	cain_sip_header_cseq_t *cseq=cain_sip_message_get_header_by_type(req,cain_sip_header_cseq_t);
	cain_sip_header_to_t *to=cain_sip_message_get_header_by_type(resp,cain_sip_header_to_t);
	cain_sip_header_via_t *via=cain_sip_message_get_header_by_type(req,cain_sip_header_via_t);
	cain_sip_uri_t *requri=cain_sip_request_get_uri(req);

	if (!ct){
		cain_sip_error("No contact in request.");
		return -1;
	}
	if (!to){
		cain_sip_error("No to in response.");
		return -1;
	}
	if (!cseq){
		cain_sip_error("No cseq in request.");
		return -1;
	}
	if (!via){
		cain_sip_error("No via in request.");
		return -1;
	}
	if (strcasecmp(cain_sip_header_via_get_protocol(via),"TLS")==0
	    && cain_sip_uri_is_secure(requri)){
		obj->is_secure=TRUE;
	}
	/* 12.1.1
	*The route set MUST be set to the list of URIs in the Record-Route
	* header field from the request, taken in order and preserving all URI
	* parameters.  If no Record-Route header field is present in the
	*request, the route set MUST be set to the empty set.
	*/
	obj->route_set=cain_sip_list_free_with_data(obj->route_set,cain_sip_object_unref);
	for(elem=cain_sip_message_get_headers((cain_sip_message_t*)req,CAIN_SIP_RECORD_ROUTE);elem!=NULL;elem=elem->next){
		obj->route_set=cain_sip_list_append(obj->route_set,cain_sip_object_ref(cain_sip_header_route_create(
		                                     (cain_sip_header_address_t*)elem->data)));
	}
	check_route_set(obj->route_set);
	obj->remote_target=(cain_sip_header_address_t*)cain_sip_object_ref(ct);
	obj->remote_cseq=cain_sip_header_cseq_get_seq_number(cseq);
	/*call id already set */
	/*remote party already set */
	obj->local_party=(cain_sip_header_address_t*)cain_sip_object_ref(to);
	if (strcmp(cain_sip_request_get_method(req),"INVITE")==0){
		cain_sip_dialog_init_200Ok_retrans(obj,resp);
		obj->needs_ack=TRUE;
	}
	return 0;
}

static void set_last_out_invite(cain_sip_dialog_t *obj, cain_sip_request_t *req){
	if (obj->last_out_invite)
		cain_sip_object_unref(obj->last_out_invite);
	obj->last_out_invite=(cain_sip_request_t*)cain_sip_object_ref(req);
}

static int cain_sip_dialog_init_as_uac(cain_sip_dialog_t *obj, cain_sip_request_t *req, cain_sip_response_t *resp){
	const cain_sip_list_t *elem;
	cain_sip_header_contact_t *ct=cain_sip_message_get_header_by_type(resp,cain_sip_header_contact_t);
	cain_sip_header_cseq_t *cseq=cain_sip_message_get_header_by_type(req,cain_sip_header_cseq_t);
	cain_sip_header_to_t *to=cain_sip_message_get_header_by_type(resp,cain_sip_header_to_t);
	cain_sip_header_via_t *via=cain_sip_message_get_header_by_type(req,cain_sip_header_via_t);
	cain_sip_uri_t *requri=cain_sip_request_get_uri(req);

	if (!ct){
		cain_sip_error("No contact in response.");
		return -1;
	}
	if (!to){
		cain_sip_error("No to in response.");
		return -1;
	}
	if (!cseq){
		cain_sip_error("No cseq in request.");
		return -1;
	}
	if (!via){
		cain_sip_error("No via in request.");
		return -1;
	}
	if (strcasecmp(cain_sip_header_via_get_protocol(via),"TLS")==0
	    && cain_sip_uri_is_secure(requri)){
		obj->is_secure=TRUE;
	}
	/**12.1.2
	 *  The route set MUST be set to the list of URIs in the Record-Route
   	 *header field from the response, taken in reverse order and preserving
   	 *all URI parameters.  If no Record-Route header field is present in
   	 *the response, the route set MUST be set to the empty set.
   	 **/
	obj->route_set=cain_sip_list_free_with_data(obj->route_set,cain_sip_object_unref);
	for(elem=cain_sip_message_get_headers((cain_sip_message_t*)resp,CAIN_SIP_RECORD_ROUTE);elem!=NULL;elem=elem->next){
		obj->route_set=cain_sip_list_prepend(obj->route_set,cain_sip_object_ref(cain_sip_header_route_create(
		                                     (cain_sip_header_address_t*)elem->data)));
	}

	check_route_set(obj->route_set);
	obj->remote_target=(cain_sip_header_address_t*)cain_sip_object_ref(ct);
	obj->local_cseq=cain_sip_header_cseq_get_seq_number(cseq);
	/*call id is already set */
	/*local_tag is already set*/
	obj->remote_party=(cain_sip_header_address_t*)cain_sip_object_ref(to);
	/*local party is already set*/
	if (strcmp(cain_sip_request_get_method(req),"INVITE")==0){
		set_last_out_invite(obj,req);
		obj->needs_ack=TRUE;
	}
	return 0;
}

int cain_sip_dialog_establish_full(cain_sip_dialog_t *obj, cain_sip_request_t *req, cain_sip_response_t *resp){
	int err;
	if (obj->is_server)
		err= cain_sip_dialog_init_as_uas(obj,req,resp);
	else
		err= cain_sip_dialog_init_as_uac(obj,req,resp);
	if (err==0) set_state(obj,CAIN_SIP_DIALOG_CONFIRMED);
	return err;
}

int cain_sip_dialog_establish(cain_sip_dialog_t *obj, cain_sip_request_t *req, cain_sip_response_t *resp){
	int code=cain_sip_response_get_status_code(resp);
	cain_sip_header_to_t *to=cain_sip_message_get_header_by_type(resp,cain_sip_header_to_t);
	cain_sip_header_call_id_t *call_id=cain_sip_message_get_header_by_type(req,cain_sip_header_call_id_t);

	if (!to){
		cain_sip_error("No to in response.");
		return -1;
	}
	if (!call_id){
		cain_sip_error("No call-id in response.");
		return -1;
	}
	
	if (code>100 && code<200){
		if (obj->state==CAIN_SIP_DIALOG_NULL){
			set_to_tag(obj,to);
			obj->call_id=(cain_sip_header_call_id_t*)cain_sip_object_ref(call_id);
			set_state(obj,CAIN_SIP_DIALOG_EARLY);
		}
		return -1;
	}else if (code>=200 && code<300){
		if (obj->state==CAIN_SIP_DIALOG_NULL){
			set_to_tag(obj,to);
			obj->call_id=(cain_sip_header_call_id_t*)cain_sip_object_ref(call_id);
		}
		if (cain_sip_dialog_establish_full(obj,req,resp)==-1){
			return -1;
		}
	} else if (code>=300 && obj->state!=CAIN_SIP_DIALOG_CONFIRMED) {
		/*12.3 Termination of a Dialog
   	   	   Independent of the method, if a request outside of a dialog generates
   	   	   a non-2xx final response, any early dialogs created through
   	   	   provisional responses to that request are terminated.  The mechanism
   	   	   for terminating confirmed dialogs is method specific.*/
		cain_sip_dialog_delete(obj);
	}
	return 0;
}

int cain_sip_dialog_check_incoming_request_ordering(cain_sip_dialog_t *obj, cain_sip_request_t *req){
	cain_sip_header_cseq_t *cseqh=cain_sip_message_get_header_by_type(req,cain_sip_header_cseq_t);
	unsigned int cseq=cain_sip_header_cseq_get_seq_number(cseqh);
	if (obj->remote_cseq==0){
		obj->remote_cseq=cseq;
	}else if (cseq>obj->remote_cseq){
			return 0;
	}
	cain_sip_warning("Ignoring request because cseq is inconsistent.");
	return -1;
}

static int dialog_on_200Ok_timer(cain_sip_dialog_t *dialog){
	/*reset the timer */
	const cain_sip_timer_config_t *cfg=cain_sip_stack_get_timer_config(dialog->provider->stack);
	unsigned int prev_timeout=cain_sip_source_get_timeout(dialog->timer_200Ok);
	cain_sip_source_set_timeout(dialog->timer_200Ok,MIN(2*prev_timeout,(unsigned int)cfg->T2));
	cain_sip_message("Dialog sending retransmission of 200Ok");
	cain_sip_provider_send_response(dialog->provider,dialog->last_200Ok);
	return CAIN_SIP_CONTINUE;
}

static int dialog_on_200Ok_end(cain_sip_dialog_t *dialog){
	cain_sip_request_t *bye;
	cain_sip_client_transaction_t *trn;
	cain_sip_dialog_stop_200Ok_retrans(dialog);
	cain_sip_error("Dialog [%p] was not ACK'd within T1*64 seconds, it is going to be terminated.",dialog);
	dialog->state=CAIN_SIP_DIALOG_CONFIRMED;
	bye=cain_sip_dialog_create_request(dialog,"BYE");
	trn=cain_sip_provider_get_new_client_transaction(dialog->provider,bye);
	CAIN_SIP_TRANSACTION(trn)->is_internal=1; /*don't bother user with this transaction*/
	cain_sip_client_transaction_send_request(trn);
	return CAIN_SIP_STOP;
}

static void cain_sip_dialog_init_200Ok_retrans(cain_sip_dialog_t *obj, cain_sip_response_t *resp){
	const cain_sip_timer_config_t *cfg=cain_sip_stack_get_timer_config(obj->provider->stack);
	obj->timer_200Ok=cain_sip_timeout_source_new((cain_sip_source_func_t)dialog_on_200Ok_timer,obj,cfg->T1);
	cain_sip_object_set_name((cain_sip_object_t*)obj->timer_200Ok,"dialog_200Ok_timer");
	cain_sip_main_loop_add_source(obj->provider->stack->ml,obj->timer_200Ok);
	
	obj->timer_200Ok_end=cain_sip_timeout_source_new((cain_sip_source_func_t)dialog_on_200Ok_end,obj,cfg->T1*64);
	cain_sip_object_set_name((cain_sip_object_t*)obj->timer_200Ok_end,"dialog_200Ok_timer_end");
	cain_sip_main_loop_add_source(obj->provider->stack->ml,obj->timer_200Ok_end);
	
	obj->last_200Ok=(cain_sip_response_t*)cain_sip_object_ref(resp);
}

static void cain_sip_dialog_stop_200Ok_retrans(cain_sip_dialog_t *obj){
	cain_sip_main_loop_t *ml=obj->provider->stack->ml;
	if (obj->timer_200Ok){
		cain_sip_main_loop_remove_source(ml,obj->timer_200Ok);
		cain_sip_object_unref(obj->timer_200Ok);
		obj->timer_200Ok=NULL;
	}
	if (obj->timer_200Ok_end){
		cain_sip_main_loop_remove_source(ml,obj->timer_200Ok_end);
		cain_sip_object_unref(obj->timer_200Ok_end);
		obj->timer_200Ok_end=NULL;
	}
	if (obj->last_200Ok){
		cain_sip_object_unref(obj->last_200Ok);
		obj->last_200Ok=NULL;
	}
}
/*
 * return 0 if message should be delivered to the next listener, otherwise, its a retransmision, just keep it
 * */
int cain_sip_dialog_update(cain_sip_dialog_t *obj,cain_sip_request_t *req, cain_sip_response_t *resp, int as_uas){
	int code;
	int is_retransmition=FALSE;
	/*first update local/remote cseq*/
	if (as_uas) {
		cain_sip_header_cseq_t* cseq=cain_sip_message_get_header_by_type(CAIN_SIP_MESSAGE(req),cain_sip_header_cseq_t);
		obj->remote_cseq=cain_sip_header_cseq_get_seq_number(cseq);
	}
	switch (obj->state){
		case CAIN_SIP_DIALOG_NULL:
		case CAIN_SIP_DIALOG_EARLY:
			cain_sip_dialog_establish(obj,req,resp);
			break;
		case CAIN_SIP_DIALOG_CONFIRMED:
			code=cain_sip_response_get_status_code(resp);
			if (strcmp(cain_sip_request_get_method(req),"INVITE")==0 && code>=200 && code<300){
				/*refresh the remote_target*/
				cain_sip_header_contact_t *ct;
				if (as_uas){
					ct=cain_sip_message_get_header_by_type(req,cain_sip_header_contact_t);

				}else{
					set_last_out_invite(obj,req);
					ct=cain_sip_message_get_header_by_type(resp,cain_sip_header_contact_t);
				}
				if (ct){
					cain_sip_object_unref(obj->remote_target);
					obj->remote_target=(cain_sip_header_address_t*)cain_sip_object_ref(ct);
				}
				/*handle possible retransmission of 200Ok */
				if (!as_uas && (is_retransmition=(cain_sip_dialog_handle_200Ok(obj,resp)==0))) {
					return is_retransmition;
				} else {
					obj->needs_ack=TRUE; /*REINVITE case, ack needed by both uas and uac*/
				}

			}if (strcmp(cain_sip_request_get_method(req),"INVITE")==0 && code >=300) {
				/*final response, ack will be automatically sent by transaction layer*/
				obj->needs_ack=FALSE;
			} else if (strcmp(cain_sip_request_get_method(req),"BYE")==0 && (/*(*/code>=200 /*&& code<300) || code==481 || code==408*/)){
				/*15.1.1 UAC Behavior

				   A BYE request is constructed as would any other request within a
				   dialog, as described in Section 12.

				   Once the BYE is constructed, the UAC core creates a new non-INVITE
				   client transaction, and passes it the BYE request.  The UAC MUST
				   consider the session terminated (and therefore stop sending or
				   listening for media) as soon as the BYE request is passed to the
				   client transaction.  If the response for the BYE is a 481
				   (Call/Transaction Does Not Exist) or a 408 (Request Timeout) or no
				   response at all is received for the BYE (that is, a timeout is
				   returned by the client transaction), the UAC MUST consider the
				   session and the dialog terminated. */
				/*what should we do with other reponse >300 ?? */
				if (obj->terminate_on_bye) cain_sip_dialog_delete(obj);
				obj->needs_ack=FALSE; /*no longuer need ACK*/
			}
		break;
		case CAIN_SIP_DIALOG_TERMINATED:
			/*ignore*/
		break;
	}
	return 0;
}

cain_sip_dialog_t *cain_sip_dialog_new(cain_sip_transaction_t *t){
	cain_sip_dialog_t *obj;
	cain_sip_header_from_t *from;
	const char *from_tag;
	cain_sip_header_to_t *to;
	const char *to_tag=NULL;

	from=cain_sip_message_get_header_by_type(t->request,cain_sip_header_from_t);
	if (from==NULL){
		cain_sip_error("cain_sip_dialog_new(): no from!");
		return NULL;
	}
	from_tag=cain_sip_header_from_get_tag(from);
	if (from_tag==NULL){
		cain_sip_error("cain_sip_dialog_new(): no from tag!");
		return NULL;
	}

	if (t->last_response) {
		to=cain_sip_message_get_header_by_type(t->last_response,cain_sip_header_to_t);
		if (to==NULL){
			cain_sip_error("cain_sip_dialog_new(): no to!");
			return NULL;
		}
		to_tag=cain_sip_header_to_get_tag(to);
	}
	obj=cain_sip_object_new(cain_sip_dialog_t);
	obj->terminate_on_bye=1;
	obj->provider=t->provider;
	
	if (CAIN_SIP_OBJECT_IS_INSTANCE_OF(t,cain_sip_server_transaction_t)){
		obj->remote_tag=cain_sip_strdup(from_tag);
		obj->local_tag=to_tag?cain_sip_strdup(to_tag):NULL; /*might be null at dialog creation*/
		obj->remote_party=(cain_sip_header_address_t*)cain_sip_object_ref(from);
		obj->is_server=TRUE;
	}else{
		const cain_sip_list_t *predefined_routes=NULL;
		obj->local_tag=cain_sip_strdup(from_tag);
		obj->remote_tag=to_tag?cain_sip_strdup(to_tag):NULL; /*might be null at dialog creation*/
		obj->local_party=(cain_sip_header_address_t*)cain_sip_object_ref(from);
		obj->is_server=FALSE;
		for(predefined_routes=cain_sip_message_get_headers((cain_sip_message_t*)t->request,CAIN_SIP_ROUTE);
			predefined_routes!=NULL;predefined_routes=predefined_routes->next){
			obj->route_set=cain_sip_list_append(obj->route_set,cain_sip_object_ref(predefined_routes->data));	
		}
	}
	cain_sip_message("New %s dialog [%x] , local tag [%s], remote tag [%s]"
			,obj->is_server?"server":"client"
			,obj
			,obj->local_tag
			,obj->remote_tag);
	set_state(obj,CAIN_SIP_DIALOG_NULL);
	return obj;
}

cain_sip_request_t *cain_sip_dialog_create_ack(cain_sip_dialog_t *obj, unsigned int cseq){
	cain_sip_header_cseq_t *cseqh;
	cain_sip_request_t *invite=obj->last_out_invite;
	cain_sip_request_t *ack;
	if (!invite){
		cain_sip_error("No INVITE to ACK.");
		return NULL;
	}
	cseqh=cain_sip_message_get_header_by_type(invite,cain_sip_header_cseq_t);
	if (cain_sip_header_cseq_get_seq_number(cseqh)!=cseq){
		cain_sip_error("No INVITE with cseq %i to create ack for.",cseq);
		return NULL;
	}
	ack=cain_sip_dialog_create_request(obj,"ACK");
/*
22 Usage of HTTP Authentication
22.1 Framework
   While a server can legitimately challenge most SIP requests, there
   are two requests defined by this document that require special
   handling for authentication: ACK and CANCEL.
   Under an authentication scheme that uses responses to carry values
   used to compute nonces (such as Digest), some problems come up for
   any requests that take no response, including ACK.  For this reason,
   any credentials in the INVITE that were accepted by a server MUST be
   accepted by that server for the ACK.  UACs creating an ACK message
   will duplicate all of the Authorization and Proxy-Authorization
   header field values that appeared in the INVITE to which the ACK
   corresponds.  Servers MUST NOT attempt to challenge an ACK.
  */
	if (ack){
		const cain_sip_list_t *aut=cain_sip_message_get_headers((cain_sip_message_t*)obj->last_out_invite,"Authorization");
		const cain_sip_list_t *prx_aut=cain_sip_message_get_headers((cain_sip_message_t*)obj->last_out_invite,"Proxy-Authorization");
		if (aut)
			cain_sip_message_add_headers((cain_sip_message_t*)ack,aut);
		if (prx_aut)
			cain_sip_message_add_headers((cain_sip_message_t*)ack,prx_aut);
	}
	return ack;
}

cain_sip_request_t *cain_sip_dialog_create_request(cain_sip_dialog_t *obj, const char *method){
	cain_sip_request_t *req;

	if (obj->state != CAIN_SIP_DIALOG_CONFIRMED && obj->state != CAIN_SIP_DIALOG_EARLY) {
		cain_sip_error("Cannot create method [%s] from dialog [%p] in state [%s]",method,obj,cain_sip_dialog_state_to_string(obj->state));
		return NULL;
	}
	if (obj->local_cseq==0) obj->local_cseq=110;
	if (strcmp(method,"ACK")!=0) obj->local_cseq++;
	req=cain_sip_request_create(cain_sip_header_address_get_uri(obj->remote_target),
	                                                method,
	                                                obj->call_id,
	                                                cain_sip_header_cseq_create(obj->local_cseq,method),
	                                                cain_sip_header_from_create(obj->local_party,NULL),
	                                                cain_sip_header_to_create(obj->remote_party,NULL),
	                                                cain_sip_header_via_new(),
	                                                0);
	if (obj->route_set) {
		cain_sip_message_add_headers((cain_sip_message_t*)req,obj->route_set);
	}
	return req;
}
static unsigned int is_system_header(cain_sip_header_t* header) {
	const char* name=cain_sip_header_get_name(header);
	return strcasecmp(CAIN_SIP_VIA,name) ==0
			|| strcasecmp(CAIN_SIP_FROM,name) ==0
			|| strcasecmp(CAIN_SIP_TO,name) ==0
			|| strcasecmp(CAIN_SIP_CSEQ,name) ==0
			|| strcasecmp(CAIN_SIP_CALL_ID,name) ==0
			|| strcasecmp(CAIN_SIP_PROXY_AUTHORIZATION,name) == 0
			|| strcasecmp(CAIN_SIP_AUTHORIZATION,name) == 0
			|| strcasecmp(CAIN_SIP_MAX_FORWARDS,name) == 0
			|| strcasecmp(CAIN_SIP_ALLOW,name) ==0
			|| strcasecmp(CAIN_SIP_ROUTE,name) ==0;
}
static void copy_non_system_headers(cain_sip_header_t* header,cain_sip_request_t* req ) {
	if (!is_system_header(header)) {
		cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),header);
	}
}

cain_sip_request_t *cain_sip_dialog_create_request_from(cain_sip_dialog_t *obj, const cain_sip_request_t *initial_req){
	cain_sip_request_t* req = cain_sip_dialog_create_request(obj, cain_sip_request_get_method(initial_req));
	cain_sip_header_content_length_t* content_lenth = cain_sip_message_get_header_by_type(initial_req,cain_sip_header_content_length_t);
	/*first copy non system headers*/
	cain_sip_list_t* headers = cain_sip_message_get_all_headers(CAIN_SIP_MESSAGE(initial_req));
	cain_sip_list_for_each2(headers,(void (*)(void *, void *))copy_non_system_headers,req);
	cain_sip_list_free(headers);
	/*copy body*/
	if (content_lenth && cain_sip_header_content_length_get_content_length(content_lenth)>0) {
		cain_sip_message_set_body(CAIN_SIP_MESSAGE(req),cain_sip_message_get_body(CAIN_SIP_MESSAGE(initial_req)),cain_sip_header_content_length_get_content_length(content_lenth));
	}
	return req;
}

void cain_sip_dialog_delete(cain_sip_dialog_t *obj){
	cain_sip_dialog_stop_200Ok_retrans(obj); /*if any*/
	set_state(obj,CAIN_SIP_DIALOG_TERMINATED);
	cain_sip_provider_remove_dialog(obj->provider,obj);
}

void *cain_sip_dialog_get_application_data(const cain_sip_dialog_t *dialog){
	return dialog->appdata;
}

void cain_sip_dialog_set_application_data(cain_sip_dialog_t *dialog, void *data){
	dialog->appdata=data;
}

const cain_sip_header_call_id_t *cain_sip_dialog_get_call_id(const cain_sip_dialog_t *dialog){
	return dialog->call_id;
}

const char *cain_sip_dialog_get_dialog_id(const cain_sip_dialog_t *dialog){
	return NULL;
}

const cain_sip_header_address_t *cain_sip_dialog_get_local_party(const cain_sip_dialog_t *dialog){
	return dialog->local_party;
}

const cain_sip_header_address_t *cain_sip_dialog_get_remote_party(const cain_sip_dialog_t *dialog){
	return dialog->remote_party;
}

unsigned int cain_sip_dialog_get_local_seq_number(const cain_sip_dialog_t *dialog){
	return dialog->local_cseq;
}

unsigned int cain_sip_dialog_get_remote_seq_number(const cain_sip_dialog_t *dialog){
	return dialog->remote_cseq;
}

const char *cain_sip_dialog_get_local_tag(const cain_sip_dialog_t *dialog){
	return dialog->local_tag;
}

const char *cain_sip_dialog_get_remote_tag(const cain_sip_dialog_t *dialog){
	return dialog->remote_tag;
}

const cain_sip_header_address_t *cain_sip_dialog_get_remote_target(cain_sip_dialog_t *dialog){
	return dialog->remote_target;
}

const cain_sip_list_t* cain_sip_dialog_get_route_set(cain_sip_dialog_t *dialog){
	return dialog->route_set;
}

cain_sip_dialog_state_t cain_sip_dialog_get_state(const cain_sip_dialog_t *dialog){
	return dialog->state;
}

cain_sip_dialog_state_t cain_sip_dialog_get_previous_state(const cain_sip_dialog_t *dialog) {
	return dialog->previous_state;
}
int cain_sip_dialog_is_server(const cain_sip_dialog_t *dialog){
	return dialog->is_server;
}

int cain_sip_dialog_is_secure(const cain_sip_dialog_t *dialog){
	return dialog->is_secure;
}

void cain_sip_dialog_send_ack(cain_sip_dialog_t *obj, cain_sip_request_t *request){
	if (obj->needs_ack){
		obj->needs_ack=FALSE;
		if (obj->last_out_ack)
			cain_sip_object_unref(obj->last_out_ack);
		obj->last_out_ack=(cain_sip_request_t*)cain_sip_object_ref(request);
		cain_sip_provider_send_request(obj->provider,request);
	}else{
		cain_sip_error("Why do you want to send an ACK ?");
	}
}

void cain_sip_dialog_terminate_on_bye(cain_sip_dialog_t *obj, int val){
	obj->terminate_on_bye=val;
}

/*returns 1 if message belongs to the dialog, 0 otherwise */
int cain_sip_dialog_match(cain_sip_dialog_t *obj, cain_sip_message_t *msg, int as_uas){
	cain_sip_header_call_id_t *call_id=cain_sip_message_get_header_by_type(msg,cain_sip_header_call_id_t);
	cain_sip_header_from_t *from=cain_sip_message_get_header_by_type(msg,cain_sip_header_from_t);
	cain_sip_header_to_t *to=cain_sip_message_get_header_by_type(msg,cain_sip_header_to_t);
	const char *from_tag;
	const char *to_tag;
	const char *call_id_value;

	if (call_id==NULL || from==NULL || to==NULL) return 0;

	call_id_value=cain_sip_header_call_id_get_call_id(call_id);
	from_tag=cain_sip_header_from_get_tag(from);
	to_tag=cain_sip_header_to_get_tag(to);
	
	return _cain_sip_dialog_match(obj,call_id_value,as_uas ? to_tag : from_tag, as_uas ? from_tag : to_tag);
}

int _cain_sip_dialog_match(cain_sip_dialog_t *obj, const char *call_id, const char *local_tag, const char *remote_tag){
	const char *dcid;
	if (obj->state==CAIN_SIP_DIALOG_NULL) cain_sip_fatal("_cain_sip_dialog_match() must not be used for dialog in null state.");
	dcid=cain_sip_header_call_id_get_call_id(obj->call_id);
	return strcmp(dcid,call_id)==0 && strcmp(obj->local_tag,local_tag)==0 && strcmp(obj->remote_tag,remote_tag)==0;
}

void cain_sip_dialog_check_ack_sent(cain_sip_dialog_t*obj){
	cain_sip_client_transaction_t* client_trans;
	if (obj->needs_ack){
		cain_sip_request_t *req;
		cain_sip_error("Your listener did not ACK'd the 200Ok for your INVITE request. The dialog will be terminated.");
		req=cain_sip_dialog_create_request(obj,"BYE");
		client_trans=cain_sip_provider_get_new_client_transaction(obj->provider,req);
		CAIN_SIP_TRANSACTION(client_trans)->is_internal=TRUE; /*internal transaction, don't bother user with 200ok*/
		cain_sip_client_transaction_send_request(client_trans);
		/*call dialog terminated*/
	}
}
/*
 * return 0 if dialog handle the 200ok
 * */
static int cain_sip_dialog_handle_200Ok(cain_sip_dialog_t *obj, cain_sip_response_t *msg){
	if (obj->last_out_ack){
		cain_sip_header_cseq_t *cseq=cain_sip_message_get_header_by_type(msg,cain_sip_header_cseq_t);
		if (cseq){
			cain_sip_header_cseq_t *ack_cseq=cain_sip_message_get_header_by_type(obj->last_out_ack,cain_sip_header_cseq_t);
			if (cain_sip_header_cseq_get_seq_number(cseq)==cain_sip_header_cseq_get_seq_number(ack_cseq)){
				/*pass for retransmission*/
				cain_sip_message("Dialog retransmitting last ack automatically");
				cain_sip_provider_send_request(obj->provider,obj->last_out_ack);
				return 0;
			}else cain_sip_message("No ACK matching 200Ok for dialog [%p]",obj);

		}
	}
	return -1;
}

int cain_sip_dialog_handle_ack(cain_sip_dialog_t *obj, cain_sip_request_t *ack){
	cain_sip_header_cseq_t *cseq=cain_sip_message_get_header_by_type(ack,cain_sip_header_cseq_t);
	if (obj->needs_ack && cain_sip_header_cseq_get_seq_number(cseq)==obj->remote_cseq){
		cain_sip_message("Incoming INVITE has ACK, dialog is happy");
		obj->needs_ack=FALSE;
		cain_sip_dialog_stop_200Ok_retrans(obj);
		return 0;
	}
	cain_sip_message("Dialog ignoring incoming ACK (surely a retransmission)");
	return -1;
}

cain_sip_dialog_t* cain_sip_provider_find_dialog(const cain_sip_provider_t *prov, const char* call_id,const char* from_tag,const char* to_tag) {
	cain_sip_list_t* iterator;

	for(iterator=prov->dialogs;iterator!=NULL;iterator=iterator->next) {
		cain_sip_dialog_t* dialog=(cain_sip_dialog_t*)iterator->data;
		if (strcmp(cain_sip_header_call_id_get_call_id(cain_sip_dialog_get_call_id(dialog)),call_id)==0) {
			const char* target_from;
			const char*target_to;
			if (cain_sip_dialog_is_server(dialog)) {
				target_to=cain_sip_dialog_get_local_tag(dialog);
				target_from=cain_sip_dialog_get_remote_tag(dialog);
			} else {
				target_from=cain_sip_dialog_get_local_tag(dialog);
				target_to=cain_sip_dialog_get_remote_tag(dialog);
			}
			if (strcmp(from_tag,target_from)==0 && strcmp(to_tag,target_to)==0) {
				return dialog;
			}
		}
	}
	return NULL;
}
