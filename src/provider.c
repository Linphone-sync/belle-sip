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
#include "md5.h"

cain_sip_dialog_t *cain_sip_provider_find_dialog_from_msg(cain_sip_provider_t *prov, cain_sip_request_t *msg, int as_uas);

typedef struct authorization_context {
	cain_sip_header_call_id_t* callid;
	const char* scheme;
	const char* realm;
	const char* nonce;
	const char* qop;
	const char* opaque;
	int nonce_count;
	int is_proxy;
}authorization_context_t;

GET_SET_STRING(authorization_context,realm)
GET_SET_STRING(authorization_context,nonce)
GET_SET_STRING(authorization_context,qop)
GET_SET_STRING(authorization_context,scheme)
GET_SET_STRING(authorization_context,opaque)
GET_SET_INT(authorization_context,nonce_count,int)
static authorization_context_t* cain_sip_authorization_create(cain_sip_header_call_id_t* call_id) {
	authorization_context_t* result = malloc(sizeof(authorization_context_t));
	memset(result,0,sizeof(authorization_context_t));
	result->callid=call_id;
	cain_sip_object_ref(result->callid);
	return result;
}
static void cain_sip_authorization_destroy(authorization_context_t* object) {
	DESTROY_STRING(object,scheme);
	DESTROY_STRING(object,realm);
	DESTROY_STRING(object,nonce);
	DESTROY_STRING(object,qop);
	DESTROY_STRING(object,opaque);
	cain_sip_object_unref(object->callid);
	cain_sip_free(object);
}

static void cain_sip_provider_uninit(cain_sip_provider_t *p){
	p->listeners=cain_sip_list_free(p->listeners);
	p->internal_listeners=cain_sip_list_free(p->internal_listeners);
	p->client_transactions=cain_sip_list_free_with_data(p->client_transactions,cain_sip_object_unref);
	p->server_transactions=cain_sip_list_free_with_data(p->server_transactions,cain_sip_object_unref);
	p->auth_contexts=cain_sip_list_free_with_data(p->auth_contexts,(void(*)(void*))cain_sip_authorization_destroy);
	p->dialogs=cain_sip_list_free_with_data(p->dialogs,cain_sip_object_unref);
	p->lps=cain_sip_list_free_with_data(p->lps,cain_sip_object_unref);
}

static void channel_state_changed(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, cain_sip_channel_state_t state){
	cain_sip_io_error_event_t ev;
	cain_sip_provider_t* prov=CAIN_SIP_PROVIDER(obj);
	if (state == CAIN_SIP_CHANNEL_ERROR || state == CAIN_SIP_CHANNEL_DISCONNECTED) {
		ev.transport=cain_sip_channel_get_transport_name(chan);
		ev.port=chan->peer_port;
		ev.host=chan->peer_name;
		ev.source=CAIN_SIP_OBJECT(prov);
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov->listeners,process_io_error,&ev);
		/*IO error is also relevant for internal listener like refreshers*/
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov->internal_listeners,process_io_error,&ev);
		if (!chan->force_close) cain_sip_provider_release_channel(prov,chan);
	}
}

static void cain_sip_provider_dispatch_request(cain_sip_provider_t* prov, cain_sip_request_t *req){
	cain_sip_server_transaction_t *t;
	cain_sip_request_event_t ev;
	t=cain_sip_provider_find_matching_server_transaction(prov,req);
	if (t){
		cain_sip_object_ref(t);
		cain_sip_server_transaction_on_request(t,req);
		cain_sip_object_unref(t);
	}else{
		ev.dialog=NULL;
		/* Should we limit to ACK ?  */
		/*Search for a dialog if exist */

		ev.dialog=cain_sip_provider_find_dialog_from_msg(prov,req,1/*request=uas*/);
		if (strcmp("ACK",cain_sip_request_get_method(req))==0){
			if (!ev.dialog) {
				cain_sip_warning("Provider [%p] received an unexpected stateless ACK",prov);
			} else 	if (cain_sip_dialog_handle_ack(ev.dialog,req)==-1){
				/*absorbed ACK retransmission, ignore */
				return;
			}
		}
		ev.source=prov;
		ev.server_transaction=NULL;
		ev.request=req;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov->listeners,process_request_event,&ev);
	}
}

static void cain_sip_provider_dispatch_response(cain_sip_provider_t* prov, cain_sip_response_t *msg){
	cain_sip_client_transaction_t *t;
	t=cain_sip_provider_find_matching_client_transaction(prov,msg);
	/*
	 * If a transaction is found, pass it to the transaction and let it decide what to do.
	 * Else notifies directly.
	 */
	if (t){
		/*since the add_response may indirectly terminate the transaction, we need to guarantee the transaction is not freed
		 * until full completion*/
		cain_sip_object_ref(t);
		cain_sip_client_transaction_add_response(t,msg);
		cain_sip_object_unref(t);
	}else{
		cain_sip_response_event_t event;
		event.source=prov;
		event.client_transaction=NULL;
		event.dialog=NULL;
		event.response=msg;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov->listeners,process_response_event,&event);
	}
}

static void cain_sip_provider_dispatch_message(cain_sip_provider_t *prov, cain_sip_message_t *msg){
	if (cain_sip_message_is_request(msg)){
		cain_sip_provider_dispatch_request(prov,(cain_sip_request_t*)msg);
	}else{
		cain_sip_provider_dispatch_response(prov,(cain_sip_response_t*)msg);
	}
	cain_sip_object_unref(msg);
}

/*
 * takes example on 16.11 of RFC3261
 */
static void compute_hash_from_invariants(cain_sip_message_t *msg, char *branchid, size_t branchid_size, const char *initial){
	md5_state_t ctx;
	unsigned int cseq=cain_sip_header_cseq_get_seq_number(cain_sip_message_get_header_by_type(msg,cain_sip_header_cseq_t));
	char tmp[256]={0};
	uint8_t digest[16];
	const char*callid=cain_sip_header_call_id_get_call_id(cain_sip_message_get_header_by_type(msg,cain_sip_header_call_id_t));
	const char *from_tag=cain_sip_header_from_get_tag(cain_sip_message_get_header_by_type(msg,cain_sip_header_from_t));
	const char *to_tag=cain_sip_header_to_get_tag(cain_sip_message_get_header_by_type(msg,cain_sip_header_to_t));
	cain_sip_uri_t *requri=NULL;
	cain_sip_header_via_t *via=NULL;
	cain_sip_header_via_t *prev_via=NULL;
	const cain_sip_list_t *vias=cain_sip_message_get_headers(msg,"via");
	int is_request=cain_sip_message_is_request(msg);
	
	if (vias){
		via=(cain_sip_header_via_t*)vias->data;
		if (vias->next){
			prev_via=(cain_sip_header_via_t*)vias->next->data;
		}
	}
	
	if (is_request){
		requri=cain_sip_request_get_uri(CAIN_SIP_REQUEST(msg));
	}
	
	cain_sip_md5_init(&ctx);
	if (initial)
		cain_sip_md5_append(&ctx,(uint8_t*)initial,strlen(initial));
	if (requri){
		cain_sip_object_marshal((cain_sip_object_t*)requri,tmp,0,sizeof(tmp)-1);
		cain_sip_md5_append(&ctx,(uint8_t*)tmp,strlen(tmp));
	}
	if (from_tag)
		cain_sip_md5_append(&ctx,(uint8_t*)from_tag,strlen(from_tag));
	if (to_tag)
		cain_sip_md5_append(&ctx,(uint8_t*)to_tag,strlen(to_tag));
	cain_sip_md5_append(&ctx,(uint8_t*)callid,strlen(callid));
	cain_sip_md5_append(&ctx,(uint8_t*)&cseq,sizeof(cseq));
	if (is_request){
		if (prev_via){
			cain_sip_object_marshal((cain_sip_object_t*)prev_via,tmp,0,sizeof(tmp)-1);
			cain_sip_md5_append(&ctx,(uint8_t*)tmp,strlen(tmp));
		}
	}else{
		if (via){
			cain_sip_object_marshal((cain_sip_object_t*)via,tmp,0,sizeof(tmp)-1);
			cain_sip_md5_append(&ctx,(uint8_t*)tmp,strlen(tmp));
		}
	}
	cain_sip_md5_finish(&ctx,digest);
	cain_sip_octets_to_text(digest,sizeof(digest),branchid,branchid_size);
}

static void fix_outgoing_via(cain_sip_provider_t *p, cain_sip_channel_t *chan, cain_sip_message_t *msg){
	cain_sip_header_via_t *via=CAIN_SIP_HEADER_VIA(cain_sip_message_get_header(msg,"via"));
	if (p->rport_enabled) cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(via),"rport",NULL);

	if (cain_sip_header_via_get_host(via)==NULL){
		const char *local_ip;
		int local_port;
		local_ip=cain_sip_channel_get_local_address(chan,&local_port);
		cain_sip_header_via_set_host(via,local_ip);
		cain_sip_header_via_set_port(via,local_port);
		cain_sip_header_via_set_protocol(via,"SIP/2.0");
		cain_sip_header_via_set_transport(via,cain_sip_channel_get_transport_name(chan));
	}
	if (cain_sip_header_via_get_branch(via)==NULL){
		/*branch id should not be set random here (stateless forwarding): but rather a hash of message invariants*/
		char branchid[24];
		char token[CAIN_SIP_BRANCH_ID_LENGTH];
		compute_hash_from_invariants(msg,token,sizeof(token),NULL);
		snprintf(branchid,sizeof(branchid)-1,CAIN_SIP_BRANCH_MAGIC_COOKIE ".%s",token);
		cain_sip_header_via_set_branch(via,branchid);
		cain_sip_message("Computing branch id %s for message sent statelessly", branchid);
	}
}

static int channel_on_event(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, unsigned int revents){
	if (revents & CAIN_SIP_EVENT_READ){
		cain_sip_message_t *msg=cain_sip_channel_pick_message(chan);
		cain_sip_provider_dispatch_message(CAIN_SIP_PROVIDER(obj),msg);
	}
	return 0;
}

static void channel_on_sending(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, cain_sip_message_t *msg){
	cain_sip_header_contact_t* contact = (cain_sip_header_contact_t*)cain_sip_message_get_header(msg,"Contact");
	cain_sip_header_content_length_t* content_lenght = (cain_sip_header_content_length_t*)cain_sip_message_get_header(msg,"Content-Length");
	cain_sip_uri_t* contact_uri;

	if (cain_sip_message_is_request(msg)){
		/*probably better to be in channel*/
		fix_outgoing_via((cain_sip_provider_t*)obj,chan,msg);
	}

	if (contact){
		/* fix the contact if empty*/
		if (!(contact_uri =cain_sip_header_address_get_uri((cain_sip_header_address_t*)contact))) {
			contact_uri = cain_sip_uri_new();
			cain_sip_header_address_set_uri((cain_sip_header_address_t*)contact,contact_uri);
		}
		if (!cain_sip_uri_get_host(contact_uri)) {
			cain_sip_uri_set_host(contact_uri,chan->local_ip);
		}
		if (cain_sip_uri_get_transport_param(contact_uri) == NULL && strcasecmp("udp",cain_sip_channel_get_transport_name(chan))!=0) {
			cain_sip_uri_set_transport_param(contact_uri,cain_sip_channel_get_transport_name_lower_case(chan));
		}
		if (cain_sip_uri_get_port(contact_uri) <= 0 && chan->local_port!=5060) {
			cain_sip_uri_set_port(contact_uri,chan->local_port);
		}
		cain_sip_uri_fix(contact_uri);
	}
	if (!content_lenght && strcasecmp("udp",cain_sip_channel_get_transport_name(chan))!=0) {
		content_lenght = cain_sip_header_content_length_create(0);
		cain_sip_message_add_header(msg,(cain_sip_header_t*)content_lenght);
	}
}

CAIN_SIP_IMPLEMENT_INTERFACE_BEGIN(cain_sip_provider_t,cain_sip_channel_listener_t)
	channel_state_changed,
	channel_on_event,
	channel_on_sending
CAIN_SIP_IMPLEMENT_INTERFACE_END

CAIN_SIP_DECLARE_IMPLEMENTED_INTERFACES_1(cain_sip_provider_t,cain_sip_channel_listener_t);
	
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_provider_t,cain_sip_object_t,cain_sip_provider_uninit,NULL,NULL,FALSE);

cain_sip_provider_t *cain_sip_provider_new(cain_sip_stack_t *s, cain_sip_listening_point_t *lp){
	cain_sip_provider_t *p=cain_sip_object_new(cain_sip_provider_t);
	p->stack=s;
	p->rport_enabled=1;
	if (lp) cain_sip_provider_add_listening_point(p,lp);
	return p;
}

int cain_sip_provider_add_listening_point(cain_sip_provider_t *p, cain_sip_listening_point_t *lp){
	if (lp == NULL) {
		cain_sip_error("Cannot add NULL lp to provider [%p]",p);
		return -1;
	}
	cain_sip_listening_point_set_channel_listener(lp,CAIN_SIP_CHANNEL_LISTENER(p));
	p->lps=cain_sip_list_append(p->lps,cain_sip_object_ref(lp));
	return 0;
}

void cain_sip_provider_remove_listening_point(cain_sip_provider_t *p, cain_sip_listening_point_t *lp) {
	p->lps=cain_sip_list_remove(p->lps,lp);
	cain_sip_object_unref(lp);
	return;
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

void cain_sip_provider_add_internal_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l){
	p->internal_listeners=cain_sip_list_append(p->internal_listeners,l);
}

void cain_sip_provider_remove_internal_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l){
	p->internal_listeners=cain_sip_list_remove(p->internal_listeners,l);
}

void cain_sip_provider_add_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l){
	p->listeners=cain_sip_list_append(p->listeners,l);
}

void cain_sip_provider_remove_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l){
	p->listeners=cain_sip_list_remove(p->listeners,l);
}

cain_sip_header_call_id_t * cain_sip_provider_get_new_call_id(const cain_sip_provider_t *prov){
	cain_sip_header_call_id_t *cid=cain_sip_header_call_id_new();
	char tmp[11];
	cain_sip_header_call_id_set_call_id(cid,cain_sip_random_token(tmp,sizeof(tmp)));
	return cid;
}

cain_sip_dialog_t * cain_sip_provider_get_new_dialog(cain_sip_provider_t *prov, cain_sip_transaction_t *t) {
	return cain_sip_provider_get_new_dialog_internal(prov,t,TRUE);
}

cain_sip_dialog_t * cain_sip_provider_get_new_dialog_internal(cain_sip_provider_t *prov, cain_sip_transaction_t *t,unsigned int check_last_resp){
	cain_sip_dialog_t *dialog=NULL;
	
	if (check_last_resp && t->last_response){
		int code=cain_sip_response_get_status_code(t->last_response);
		if (code>=200 && code<300){
			cain_sip_fatal("You must not create dialog after sending the response that establish the dialog.");
			return NULL;
		}
	}
	dialog=cain_sip_dialog_new(t);
	if (dialog) {
		cain_sip_transaction_set_dialog(t,dialog);
		cain_sip_provider_add_dialog(prov,dialog);
	}
	return dialog;
}

/*finds an existing dialog for an outgoing or incoming request */
cain_sip_dialog_t *cain_sip_provider_find_dialog_from_msg(cain_sip_provider_t *prov, cain_sip_request_t *msg, int as_uas){
	cain_sip_list_t *elem;
	cain_sip_dialog_t *dialog;
	cain_sip_dialog_t *returned_dialog=NULL;
	cain_sip_header_call_id_t *call_id;
	cain_sip_header_from_t *from;
	cain_sip_header_to_t *to=cain_sip_message_get_header_by_type(msg,cain_sip_header_to_t);
	const char *from_tag;
	const char *to_tag;
	const char *call_id_value;
	const char *local_tag,*remote_tag;
	
	if (to==NULL || (to_tag=cain_sip_header_to_get_tag(to))==NULL){
		/* a request without to tag cannot be part of a dialog */
		return NULL;
	}
	
	call_id=cain_sip_message_get_header_by_type(msg,cain_sip_header_call_id_t);
	from=cain_sip_message_get_header_by_type(msg,cain_sip_header_from_t);

	if (call_id==NULL || from==NULL) return NULL;

	call_id_value=cain_sip_header_call_id_get_call_id(call_id);
	from_tag=cain_sip_header_from_get_tag(from);
	
	local_tag=as_uas ? to_tag : from_tag;
	remote_tag=as_uas ? from_tag : to_tag;
	
	for (elem=prov->dialogs;elem!=NULL;elem=elem->next){
		dialog=(cain_sip_dialog_t*)elem->data;
		/*ignore dialog in state CAIN_SIP_DIALOG_NULL, is it really the correct things to do*/
		if (cain_sip_dialog_get_state(dialog) != CAIN_SIP_DIALOG_NULL && _cain_sip_dialog_match(dialog,call_id_value,local_tag,remote_tag)) {
			if (!returned_dialog)
				returned_dialog=dialog;
			else
				cain_sip_fatal("More than 1 dialog is matching, check your app");
		}
	}
	return returned_dialog;
}

void cain_sip_provider_add_dialog(cain_sip_provider_t *prov, cain_sip_dialog_t *dialog){
	prov->dialogs=cain_sip_list_prepend(prov->dialogs,cain_sip_object_ref(dialog));
}

void cain_sip_provider_remove_dialog(cain_sip_provider_t *prov, cain_sip_dialog_t *dialog){
	cain_sip_dialog_terminated_event_t ev;
	ev.source=prov;
	ev.dialog=dialog;
	prov->dialogs=cain_sip_list_remove(prov->dialogs,dialog);
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov->listeners,process_dialog_terminated,&ev);
	cain_sip_object_unref(dialog);
}

cain_sip_client_transaction_t *cain_sip_provider_get_new_client_transaction(cain_sip_provider_t *prov, cain_sip_request_t *req){
	const char *method=cain_sip_request_get_method(req);
	cain_sip_client_transaction_t *t;
	cain_sip_client_transaction_t *inv_transaction;
	if (strcmp(method,"INVITE")==0)
		t=(cain_sip_client_transaction_t*)cain_sip_ict_new(prov,req);
	else if (strcmp(method,"ACK")==0){
		cain_sip_error("cain_sip_provider_get_new_client_transaction() cannot be used for ACK requests.");
		return NULL;
	} else {
		t=(cain_sip_client_transaction_t*)cain_sip_nict_new(prov,req);
		if (strcmp(method,"CANCEL")==0){
			/*force next hop*/
			inv_transaction=cain_sip_provider_find_matching_client_transaction_from_req(prov,req);
			if (inv_transaction && inv_transaction->next_hop) {
				/*found corresponding ict, taking next hop*/
				/*9.1 Client Behavior
				 * The destination address,
				   port, and transport for the CANCEL MUST be identical to those used to
				   send the original request.*/
				t->next_hop=(cain_sip_hop_t*)cain_sip_object_ref(inv_transaction->next_hop);
			} else {
				cain_sip_error (" No corresponding ict nor dest found for cancel request attached to transaction [%p]",t);
			}
		}
	}
	cain_sip_transaction_set_dialog((cain_sip_transaction_t*)t,cain_sip_provider_find_dialog_from_msg(prov,req,FALSE));
	return t;
}

cain_sip_server_transaction_t *cain_sip_provider_get_new_server_transaction(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_server_transaction_t* t;
	if (strcmp(cain_sip_request_get_method(req),"INVITE")==0){
		t=(cain_sip_server_transaction_t*)cain_sip_ist_new(prov,req);
	}else if (strcmp(cain_sip_request_get_method(req),"ACK")==0){
		cain_sip_error("Creating a server transaction for an ACK is not a good idea, probably");
		return NULL;
	}else 
		t=(cain_sip_server_transaction_t*)cain_sip_nist_new(prov,req);
	cain_sip_transaction_set_dialog((cain_sip_transaction_t*)t,cain_sip_provider_find_dialog_from_msg(prov,req,TRUE));
	cain_sip_provider_add_server_transaction(prov,t);
	return t;
}

cain_sip_stack_t *cain_sip_provider_get_sip_stack(cain_sip_provider_t *p){
	return p->stack;
}

cain_sip_channel_t * cain_sip_provider_get_channel(cain_sip_provider_t *p, const cain_sip_hop_t *hop){
	cain_sip_list_t *l;
	cain_sip_listening_point_t *candidate=NULL,*lp;
	cain_sip_channel_t *chan;
	const char *transport=hop->transport;

	if (transport==NULL) transport="UDP";
	
	for(l=p->lps;l!=NULL;l=l->next){
		lp=(cain_sip_listening_point_t*)l->data;
		if (strcasecmp(cain_sip_listening_point_get_transport(lp),transport)==0){
			chan=cain_sip_listening_point_get_channel(lp,hop);
			if (chan) return chan;
			candidate=lp;
		}
	}
	if (candidate){
		chan=cain_sip_listening_point_create_channel(candidate,hop);
		if (!chan) cain_sip_error("Could not create channel to %s://%s:%i",transport,hop->host,hop->port);
		return chan;
	}
	cain_sip_error("No listening point matching for transport %s",transport);
	return NULL;
}

void cain_sip_provider_release_channel(cain_sip_provider_t *p, cain_sip_channel_t *chan){
	cain_sip_listening_point_remove_channel(chan->lp,chan);
}

void cain_sip_provider_clean_channels(cain_sip_provider_t *p){
	cain_sip_list_t *l;
	cain_sip_listening_point_t *lp;
	
	for(l=p->lps;l!=NULL;l=l->next){
		lp=(cain_sip_listening_point_t*)l->data;
		cain_sip_listening_point_clean_channels(lp);
	}
}

void cain_sip_provider_send_request(cain_sip_provider_t *p, cain_sip_request_t *req){
	cain_sip_hop_t* hop;
	cain_sip_channel_t *chan;
	hop=cain_sip_stack_get_next_hop(p->stack,req);
	chan=cain_sip_provider_get_channel(p,hop);
	if (chan) {
		cain_sip_channel_queue_message(chan,CAIN_SIP_MESSAGE(req));
	}
}

void cain_sip_provider_send_response(cain_sip_provider_t *p, cain_sip_response_t *resp){
	cain_sip_hop_t* hop;
	cain_sip_channel_t *chan;
	cain_sip_header_to_t *to=(cain_sip_header_to_t*)cain_sip_message_get_header((cain_sip_message_t*)resp,"to");

	if (cain_sip_response_get_status_code(resp)!=100 && cain_sip_header_to_get_tag(to)==NULL){
		char token[CAIN_SIP_TAG_LENGTH];
		compute_hash_from_invariants((cain_sip_message_t*)resp,token,sizeof(token),"tag");
		cain_sip_header_to_set_tag(to,token);
	}
	hop=cain_sip_response_get_return_hop(resp);
	chan=cain_sip_provider_get_channel(p,hop);
	if (chan) cain_sip_channel_queue_message(chan,CAIN_SIP_MESSAGE(resp));
	cain_sip_object_unref(hop);
}


/*private provider API*/

void cain_sip_provider_set_transaction_terminated(cain_sip_provider_t *p, cain_sip_transaction_t *t){
	cain_sip_transaction_terminated_event_t ev;
	
	CAIN_SIP_OBJECT_VPTR(t,cain_sip_transaction_t)->on_terminate(t);
	ev.source=t->provider;
	ev.transaction=t;
	ev.is_server_transaction=CAIN_SIP_IS_INSTANCE_OF(t,cain_sip_server_transaction_t);
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS_FOR_TRANSACTION(t,process_transaction_terminated,&ev);
	if (!ev.is_server_transaction){
		cain_sip_provider_remove_client_transaction(p,(cain_sip_client_transaction_t*)t);
	}else{
		cain_sip_provider_remove_server_transaction(p,(cain_sip_server_transaction_t*)t);
	}
}

void cain_sip_provider_add_client_transaction(cain_sip_provider_t *prov, cain_sip_client_transaction_t *t){
	prov->client_transactions=cain_sip_list_prepend(prov->client_transactions,cain_sip_object_ref(t));
}

struct client_transaction_matcher{
	const char *branchid;
	const char *method;
};

static int client_transaction_match(const void *p_tr, const void *p_matcher){
	cain_sip_client_transaction_t *tr=(cain_sip_client_transaction_t*)p_tr;
	struct client_transaction_matcher *matcher=(struct client_transaction_matcher*)p_matcher;
	const char *req_method=cain_sip_request_get_method(tr->base.request);
	if (strcmp(matcher->branchid,tr->base.branch_id)==0 && strcmp(matcher->method,req_method)==0) return 0;
	return -1;
}

cain_sip_client_transaction_t * cain_sip_provider_find_matching_client_transaction(cain_sip_provider_t *prov, 
                                                                                   cain_sip_response_t *resp){
	struct client_transaction_matcher matcher;
	cain_sip_header_via_t *via=(cain_sip_header_via_t*)cain_sip_message_get_header((cain_sip_message_t*)resp,"via");
	cain_sip_header_cseq_t *cseq=(cain_sip_header_cseq_t*)cain_sip_message_get_header((cain_sip_message_t*)resp,"cseq");
	cain_sip_client_transaction_t *ret=NULL;
	cain_sip_list_t *elem;
	if (via==NULL){
		cain_sip_warning("Response has no via.");
		return NULL;
	}
	if (cseq==NULL){
		cain_sip_warning("Response has no cseq.");
		return NULL;
	}
	matcher.branchid=cain_sip_header_via_get_branch(via);
	matcher.method=cain_sip_header_cseq_get_method(cseq);
	elem=cain_sip_list_find_custom(prov->client_transactions,client_transaction_match,&matcher);
	if (elem){
		ret=(cain_sip_client_transaction_t*)elem->data;
		cain_sip_message("Found transaction matching response.");
	}
	return ret;
}

void cain_sip_provider_remove_client_transaction(cain_sip_provider_t *prov, cain_sip_client_transaction_t *t){	
	cain_sip_list_t* elem=cain_sip_list_find(prov->client_transactions,t);
	if (elem) {
		prov->client_transactions=cain_sip_list_delete_link(prov->client_transactions,elem);
		cain_sip_object_unref(t);
	} else {
		cain_sip_error("trying to remove transaction [%p] not part of provider [%p]",t,prov);
	}

}

void cain_sip_provider_add_server_transaction(cain_sip_provider_t *prov, cain_sip_server_transaction_t *t){
	prov->server_transactions=cain_sip_list_prepend(prov->server_transactions,cain_sip_object_ref(t));
}

struct transaction_matcher{
	const char *branchid;
	const char *method;
	const char *sentby;
	int is_ack_or_cancel;
};

static int rfc3261_transaction_match(const void *p_tr, const void *p_matcher){
	cain_sip_transaction_t *tr=(cain_sip_transaction_t*)p_tr;
	struct transaction_matcher *matcher=(struct transaction_matcher*)p_matcher;
	const char *req_method=cain_sip_request_get_method(tr->request);
	if (strcmp(matcher->branchid,tr->branch_id)==0){
		if (strcmp(matcher->method,req_method)==0) return 0;
		if (matcher->is_ack_or_cancel && strcmp(req_method,"INVITE")==0) return 0;
	}
	return -1;
}

cain_sip_transaction_t * cain_sip_provider_find_matching_transaction(cain_sip_list_t *transactions, cain_sip_request_t *req){
	struct transaction_matcher matcher;
	cain_sip_header_via_t *via=(cain_sip_header_via_t*)cain_sip_message_get_header((cain_sip_message_t*)req,"via");
	cain_sip_transaction_t *ret=NULL;
	cain_sip_list_t *elem=NULL;
	if (via==NULL){
		cain_sip_warning("Request has no via.");
		return NULL;
	}
	matcher.branchid=cain_sip_header_via_get_branch(via);
	matcher.method=cain_sip_request_get_method(req);
	matcher.is_ack_or_cancel=(strcmp(matcher.method,"ACK")==0 || strcmp(matcher.method,"CANCEL")==0);
	if (strncmp(matcher.branchid,CAIN_SIP_BRANCH_MAGIC_COOKIE,strlen(CAIN_SIP_BRANCH_MAGIC_COOKIE))==0){
		/*compliant to RFC3261*/
		elem=cain_sip_list_find_custom(transactions,rfc3261_transaction_match,&matcher);
	}else{
		//FIXME
	}
	
	if (elem){
		ret=(cain_sip_transaction_t*)elem->data;
		cain_sip_message("Found transaction [%p] matching request.",ret);
	}
	return ret;
}
cain_sip_server_transaction_t * cain_sip_provider_find_matching_server_transaction(cain_sip_provider_t *prov, cain_sip_request_t *req) {
	cain_sip_transaction_t *ret=cain_sip_provider_find_matching_transaction(prov->server_transactions,req);
	return ret?CAIN_SIP_SERVER_TRANSACTION(ret):NULL;
}
cain_sip_client_transaction_t * cain_sip_provider_find_matching_client_transaction_from_req(cain_sip_provider_t *prov, cain_sip_request_t *req) {
	cain_sip_transaction_t *ret=cain_sip_provider_find_matching_transaction(prov->client_transactions,req);
	return ret?CAIN_SIP_CLIENT_TRANSACTION(ret):NULL;
}

void cain_sip_provider_remove_server_transaction(cain_sip_provider_t *prov, cain_sip_server_transaction_t *t){	
	prov->server_transactions=cain_sip_list_remove(prov->server_transactions,t);
	cain_sip_object_unref(t);
}


static void authorization_context_fill_from_auth(authorization_context_t* auth_context,cain_sip_header_www_authenticate_t* authenticate) {
	authorization_context_set_realm(auth_context,cain_sip_header_www_authenticate_get_realm(authenticate));
	if (auth_context->nonce && strcmp(cain_sip_header_www_authenticate_get_nonce(authenticate),auth_context->nonce)!=0) {
		/*new nonce, resetting nounce_count*/
		auth_context->nonce_count=0;
	}
	authorization_context_set_nonce(auth_context,cain_sip_header_www_authenticate_get_nonce(authenticate));
	authorization_context_set_qop(auth_context,cain_sip_header_www_authenticate_get_qop_first(authenticate));
	authorization_context_set_scheme(auth_context,cain_sip_header_www_authenticate_get_scheme(authenticate));
	authorization_context_set_opaque(auth_context,cain_sip_header_www_authenticate_get_opaque(authenticate));
	if (cain_sip_object_is_instance_of(CAIN_SIP_OBJECT(authenticate),CAIN_SIP_TYPE_ID(cain_sip_header_proxy_authenticate_t))) {
		auth_context->is_proxy=1;
	}
}

static cain_sip_list_t*  cain_sip_provider_get_auth_context_by_call_id(cain_sip_provider_t *p,cain_sip_header_call_id_t* call_id) {
	cain_sip_list_t* auth_context_lst=NULL;
	cain_sip_list_t* result=NULL;
	authorization_context_t* auth_context;
	for (auth_context_lst=p->auth_contexts;auth_context_lst!=NULL;auth_context_lst=auth_context_lst->next) {
		auth_context=(authorization_context_t*)auth_context_lst->data;
		if (cain_sip_header_call_id_equals(auth_context->callid,call_id) ) {
			result=cain_sip_list_append(result,auth_context_lst->data);
		}
	}
	return result;
}

static void  cain_sip_provider_update_or_create_auth_context(cain_sip_provider_t *p,cain_sip_header_call_id_t* call_id,cain_sip_header_www_authenticate_t* authenticate) {
	 cain_sip_list_t* auth_context_lst =  cain_sip_provider_get_auth_context_by_call_id(p,call_id);
	 authorization_context_t* auth_context;
	 for (;auth_context_lst!=NULL;auth_context_lst=auth_context_lst->next) {
		 auth_context= (authorization_context_t*)auth_context_lst->data;
		 if (strcmp(auth_context->realm,cain_sip_header_www_authenticate_get_realm(authenticate))==0) {
			 authorization_context_fill_from_auth(auth_context,authenticate);
			 if (auth_context_lst) cain_sip_free(auth_context_lst);
			 return; /*only one realm is supposed to be found for now*/
		 }
	 }
	 /*no auth context found, creating one*/
	 auth_context=cain_sip_authorization_create(call_id);
	 authorization_context_fill_from_auth(auth_context,authenticate);
	 p->auth_contexts=cain_sip_list_append(p->auth_contexts,auth_context);
	 if (auth_context_lst) cain_sip_free(auth_context_lst);
	 return;
}

int cain_sip_provider_add_authorization(cain_sip_provider_t *p, cain_sip_request_t* request,cain_sip_response_t *resp,cain_sip_list_t** auth_infos) {
	cain_sip_header_call_id_t* call_id;
	cain_sip_list_t* auth_context_lst;
	cain_sip_list_t* authenticate_lst;
	cain_sip_list_t* head;
	cain_sip_header_www_authenticate_t* authenticate;
	cain_sip_header_authorization_t* authorization;

	cain_sip_header_address_t* from;
	cain_sip_auth_event_t* auth_event;
	authorization_context_t* auth_context;
	cain_sip_uri_t* from_uri;
	const char* ha1;
	char computed_ha1[33];
	int result=0;
	const char* request_method;
	/*check params*/
	if (!p || !request) {
		cain_sip_error("cain_sip_provider_add_authorization bad parameters");
		return-1;
	}
	request_method=cain_sip_request_get_method(request);
/*22 Usage of HTTP Authentication
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

   Although the CANCEL method does take a response (a 2xx), servers MUST
   NOT attempt to challenge CANCEL requests since these requests cannot
   be resubmitted.  Generally, a CANCEL request SHOULD be accepted by a
   server if it comes from the same hop that sent the request being
   canceled (provided that some sort of transport or network layer
   security association, as described in Section 26.2.1, is in place).
   */

	if (strcmp("CANCEL",request_method)==0 || strcmp("ACK",request_method)==0) {
		cain_sip_debug("no authorization header needed for method [%s]",request_method);
		return 0;
	}
	/*get authenticates value from response*/
	if (resp) {
		cain_sip_list_t *it;
		call_id = cain_sip_message_get_header_by_type(CAIN_SIP_MESSAGE(resp),cain_sip_header_call_id_t);
		/*searching for authentication headers*/
		authenticate_lst = cain_sip_list_copy(cain_sip_message_get_headers(CAIN_SIP_MESSAGE(resp),CAIN_SIP_WWW_AUTHENTICATE));
		/*search for proxy authenticate*/
		authenticate_lst=cain_sip_list_concat(authenticate_lst,cain_sip_list_copy(cain_sip_message_get_headers(CAIN_SIP_MESSAGE(resp),CAIN_SIP_PROXY_AUTHENTICATE)));
		/*update auth contexts with authenticate headers from response*/
		for (it=authenticate_lst;it!=NULL;it=it->next) {
			authenticate=CAIN_SIP_HEADER_WWW_AUTHENTICATE(it->data);
			cain_sip_provider_update_or_create_auth_context(p,call_id,authenticate);
		}
		cain_sip_list_free(authenticate_lst);
	}

	/*put authorization header if passwd found*/
	call_id = cain_sip_message_get_header_by_type(CAIN_SIP_MESSAGE(request),cain_sip_header_call_id_t);
	from = CAIN_SIP_HEADER_ADDRESS(cain_sip_message_get_header(CAIN_SIP_MESSAGE(request),CAIN_SIP_FROM));
	from_uri = cain_sip_header_address_get_uri(from);
	if ((head=auth_context_lst = cain_sip_provider_get_auth_context_by_call_id(p,call_id))) {
		/*we assume there no existing auth headers*/
		for (;auth_context_lst!=NULL;auth_context_lst=auth_context_lst->next) {
			/*clear auth info*/
			auth_context=(authorization_context_t*)auth_context_lst->data;
			auth_event = cain_sip_auth_event_create(auth_context->realm,cain_sip_uri_get_user(from_uri));
			/*put data*/
			/*call listener*/
			CAIN_SIP_PROVIDER_INVOKE_LISTENERS(p->listeners,process_auth_requested,auth_event);
			if (auth_event->passwd || auth_event->ha1) {
				if (!auth_event->userid) {
					/*if no userid, username = userid*/

					cain_sip_auth_event_set_userid(auth_event,(const char*)auth_event->username);
				}
				cain_sip_message("Auth info found for [%s] realm [%s]",auth_event->userid,auth_event->realm);
				if (auth_context->is_proxy) {
					authorization=CAIN_SIP_HEADER_AUTHORIZATION(cain_sip_header_proxy_authorization_new());
				} else {
					authorization=cain_sip_header_authorization_new();
				}
				cain_sip_header_authorization_set_scheme(authorization,auth_context->scheme);
				cain_sip_header_authorization_set_realm(authorization,auth_context->realm);
				cain_sip_header_authorization_set_username(authorization,auth_event->userid);
				cain_sip_header_authorization_set_nonce(authorization,auth_context->nonce);
				cain_sip_header_authorization_set_qop(authorization,auth_context->qop);
				cain_sip_header_authorization_set_opaque(authorization,auth_context->opaque);
				cain_sip_header_authorization_set_uri(authorization,(cain_sip_uri_t*)cain_sip_request_get_uri(request));
				if (auth_context->qop)
					cain_sip_header_authorization_set_nonce_count(authorization,++auth_context->nonce_count);
				if (auth_event->ha1) {
					ha1=auth_event->ha1;
				} else {
					cain_sip_auth_helper_compute_ha1(auth_event->userid,auth_context->realm,auth_event->passwd, computed_ha1);
					ha1=computed_ha1;
				}
				if (cain_sip_auth_helper_fill_authorization(authorization
															,cain_sip_request_get_method(request)
															,ha1)) {
					cain_sip_object_unref(authorization);
				} else
					cain_sip_message_add_header(CAIN_SIP_MESSAGE(request),CAIN_SIP_HEADER(authorization));
				result=1;
				cain_sip_auth_event_destroy(auth_event);
		} else {
			cain_sip_message("No auth info found for call id [%s]",cain_sip_header_call_id_get_call_id(call_id));
			if (auth_infos) {
				/*stored to give user information on realm/username which requires authentications*/
				*auth_infos=cain_sip_list_append(*auth_infos,auth_event);
			} else {
				cain_sip_auth_event_destroy(auth_event);
			}
		}

		}
		cain_sip_list_free(head);
	} else {
		/*nothing to do*/
	}
	return result;
}
void cain_sip_provider_set_recv_error(cain_sip_provider_t *prov, int recv_error) {
	cain_sip_list_t *lps;
	cain_sip_list_t *channels;
	for(lps=prov->lps;lps!=NULL;lps=lps->next){
		for(channels=((cain_sip_listening_point_t*)lps->data)->channels;channels!=NULL;channels=channels->next){
			((cain_sip_channel_t*)channels->data)->recv_error=recv_error;
			((cain_sip_source_t*)channels->data)->notify_required=(recv_error<=0);
		}
	}
}
void cain_sip_provider_enable_rport(cain_sip_provider_t *prov, int enable) {
	prov->rport_enabled=enable;
}

int cain_sip_provider_is_rport_enabled(cain_sip_provider_t *prov) {
	return prov->rport_enabled;
}
