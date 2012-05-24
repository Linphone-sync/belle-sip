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


static void cain_sip_provider_uninit(cain_sip_provider_t *p){
	cain_sip_list_free(p->listeners);
	cain_sip_list_free_with_data(p->lps,cain_sip_object_unref);
}

static void channel_state_changed(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, cain_sip_channel_state_t state){
	cain_sip_io_error_event_t ev;
	if (state == CAIN_SIP_CHANNEL_ERROR || state == CAIN_SIP_CHANNEL_DISCONNECTED) {
		ev.transport=cain_sip_channel_get_transport_name(chan);
		ev.source=(cain_sip_provider_t*)obj;
		ev.port=chan->peer_port;
		ev.host=chan->peer_name;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(ev.source,process_io_error,&ev);
	}
}

static void cain_sip_provider_dispatch_request(cain_sip_provider_t* prov, cain_sip_request_t *req){
	cain_sip_server_transaction_t *t;
	t=cain_sip_provider_find_matching_server_transaction(prov,req);
	if (t){
		cain_sip_object_ref(t);
		cain_sip_server_transaction_on_request(t,req);
		cain_sip_object_unref(t);
	}else{
		cain_sip_request_event_t ev;
		ev.source=prov;
		ev.server_transaction=NULL;
		ev.dialog=NULL;
		ev.request=req;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov,process_request_event,&ev);
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
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov,process_response_event,&event);
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

static void fix_outgoing_via(cain_sip_provider_t *p, cain_sip_channel_t *chan, cain_sip_message_t *msg){
	cain_sip_header_via_t *via=CAIN_SIP_HEADER_VIA(cain_sip_message_get_header(msg,"via"));
	cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(via),"rport",NULL);
	char token[7]="fixme";
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
		/*FIXME: should not be set random here: but rather a hash of message invariants*/
		char *branchid=cain_sip_strdup_printf(CAIN_SIP_BRANCH_MAGIC_COOKIE ".%s",token);
		cain_sip_header_via_set_branch(via,branchid);
		cain_sip_free(branchid);
	}
}
/*
static void cain_sip_provider_read_message(cain_sip_provider_t *prov, cain_sip_channel_t *chan){
	char buffer[cain_sip_network_buffer_size];
	int err;
	err=cain_sip_channel_recv(chan,buffer,sizeof(buffer));
	if (err>0){
		cain_sip_message_t *msg;
		buffer[err]='\0';
		cain_sip_message("provider %p read message from %s:%i\n%s",prov,chan->peer_name,chan->peer_port,buffer);
		msg=cain_sip_message_parse(buffer);
		if (msg){
			if (cain_sip_message_is_request(msg)) fix_incoming_via(CAIN_SIP_REQUEST(msg),chan->peer);
			cain_sip_provider_dispatch_message(prov,msg);
		}else{
			cain_sip_error("Could not parse this message.");
		}
	}
}
*/
static int channel_on_event(cain_sip_channel_listener_t *obj, cain_sip_channel_t *chan, unsigned int revents){
	if (revents & CAIN_SIP_EVENT_READ){
		cain_sip_provider_dispatch_message(CAIN_SIP_PROVIDER(obj),cain_sip_channel_pick_message(chan));
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
		if (cain_sip_uri_get_port(contact_uri) == 0 && chan->local_port!=5060) {
			cain_sip_uri_set_port(contact_uri,chan->local_port);
		}
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
	if (lp) cain_sip_provider_add_listening_point(p,lp);
	return p;
}

int cain_sip_provider_add_listening_point(cain_sip_provider_t *p, cain_sip_listening_point_t *lp){
	if (lp == NULL) {
		cain_sip_error("Cannot add NULL lp to provider [%p]",p);
		return -1;
	}
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

cain_sip_dialog_t * cain_sip_provider_get_new_dialog(cain_sip_provider_t *prov, cain_sip_transaction_t *t){
	cain_sip_dialog_t *dialog=NULL;
	
	if (t->last_response){
		int code=cain_sip_response_get_status_code(t->last_response);
		if (code>=200 && code<300){
			cain_sip_fatal("You must not create dialog after sending the response that establish the dialog.");
			return NULL;
		}
	}
	dialog=cain_sip_dialog_new(t);
	if (dialog)
		t->dialog=(cain_sip_dialog_t*)cain_sip_object_ref(dialog);
	return dialog;
}

/*finds an existing dialog for an outgoing or incoming request */
cain_sip_dialog_t *cain_sip_provider_find_dialog(cain_sip_provider_t *prov, cain_sip_request_t *msg, int as_uas){
	cain_sip_list_t *elem;
	cain_sip_dialog_t *dialog;
	cain_sip_header_call_id_t *call_id=cain_sip_message_get_header_by_type(msg,cain_sip_header_call_id_t);
	cain_sip_header_from_t *from=cain_sip_message_get_header_by_type(msg,cain_sip_header_from_t);
	cain_sip_header_to_t *to=cain_sip_message_get_header_by_type(msg,cain_sip_header_to_t);
	const char *from_tag;
	const char *to_tag;
	const char *call_id_value;
	const char *local_tag,*remote_tag;

	if (call_id==NULL || from==NULL || to==NULL) return NULL;

	call_id_value=cain_sip_header_call_id_get_call_id(call_id);
	from_tag=cain_sip_header_from_get_tag(from);
	to_tag=cain_sip_header_to_get_tag(to);
	local_tag=as_uas ? to_tag : from_tag;
	remote_tag=as_uas ? from_tag : to_tag;
	
	for (elem=prov->dialogs;elem!=NULL;elem=elem->next){
		dialog=(cain_sip_dialog_t*)elem->data;
		if (_cain_sip_dialog_match(dialog,call_id_value,local_tag,remote_tag))
			return dialog;
	}
	return NULL;
}

void cain_sip_provider_add_dialog(cain_sip_provider_t *prov, cain_sip_dialog_t *dialog){
	prov->dialogs=cain_sip_list_prepend(prov->dialogs,cain_sip_object_ref(dialog));
}

void cain_sip_provider_remove_dialog(cain_sip_provider_t *prov, cain_sip_dialog_t *dialog){
	prov->dialogs=cain_sip_list_remove(prov->dialogs,dialog);
	cain_sip_object_unref(dialog);
}

cain_sip_client_transaction_t *cain_sip_provider_get_new_client_transaction(cain_sip_provider_t *prov, cain_sip_request_t *req){
	const char *method=cain_sip_request_get_method(req);
	if (strcmp(method,"INVITE")==0)
		return (cain_sip_client_transaction_t*)cain_sip_ict_new(prov,req);
	else if (strcmp(method,"ACK")==0){
		cain_sip_error("cain_sip_provider_get_new_client_transaction() cannot be used for ACK requests.");
		return NULL;
	}
	else return (cain_sip_client_transaction_t*)cain_sip_nict_new(prov,req);
}

cain_sip_server_transaction_t *cain_sip_provider_get_new_server_transaction(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_server_transaction_t* t;
	if (strcmp(cain_sip_request_get_method(req),"INVITE")==0)
		t=(cain_sip_server_transaction_t*)cain_sip_ist_new(prov,req);
	else 
		t=(cain_sip_server_transaction_t*)cain_sip_nist_new(prov,req);
	cain_sip_provider_add_server_transaction(prov,t);
	return t;
}

cain_sip_stack_t *cain_sip_provider_get_sip_stack(cain_sip_provider_t *p){
	return p->stack;
}

cain_sip_channel_t * cain_sip_provider_get_channel(cain_sip_provider_t *p, const char *name, int port, const char *transport){
	cain_sip_list_t *l;
	cain_sip_listening_point_t *candidate=NULL,*lp;
	cain_sip_channel_t *chan;

	if (transport==NULL) transport="UDP";
	
	for(l=p->lps;l!=NULL;l=l->next){
		lp=(cain_sip_listening_point_t*)l->data;
		if (strcasecmp(cain_sip_listening_point_get_transport(lp),transport)==0){
			chan=cain_sip_listening_point_get_channel(lp,name,port);
			if (chan) return chan;
			candidate=lp;
		}
	}
	if (candidate){
		chan=cain_sip_listening_point_create_channel(candidate,name,port);
		if (chan==NULL) cain_sip_error("Could not create channel to %s:%s:%i",transport,name,port);
		else cain_sip_channel_add_listener(chan,CAIN_SIP_CHANNEL_LISTENER(p));
		return chan;
	}
	cain_sip_error("No listening point matching for transport %s",transport);
	return NULL;
}

void cain_sip_provider_send_request(cain_sip_provider_t *p, cain_sip_request_t *req){
	cain_sip_hop_t hop={0};
	cain_sip_channel_t *chan;
	cain_sip_stack_get_next_hop(p->stack,req,&hop);
	chan=cain_sip_provider_get_channel(p,hop.host, hop.port, hop.transport);
	if (chan) {
		cain_sip_channel_queue_message(chan,CAIN_SIP_MESSAGE(req));
	}
	cain_sip_hop_free(&hop);
}

void cain_sip_provider_send_response(cain_sip_provider_t *p, cain_sip_response_t *resp){
	cain_sip_hop_t hop;
	cain_sip_channel_t *chan;
	cain_sip_header_to_t *to=(cain_sip_header_to_t*)cain_sip_message_get_header((cain_sip_message_t*)resp,"to");

	if (cain_sip_response_get_status_code(resp)!=100 && cain_sip_header_to_get_tag(to)==NULL){
		cain_sip_fatal("Generation of unique to tags for stateless responses is not implemented.");
	}
	cain_sip_response_get_return_hop(resp,&hop);
	chan=cain_sip_provider_get_channel(p,hop.host, hop.port, hop.transport);
	if (chan) cain_sip_channel_queue_message(chan,CAIN_SIP_MESSAGE(resp));
	cain_sip_hop_free(&hop);
}


/*private provider API*/

void cain_sip_provider_set_transaction_terminated(cain_sip_provider_t *p, cain_sip_transaction_t *t){
	cain_sip_transaction_terminated_event_t ev;
	
	CAIN_SIP_OBJECT_VPTR(t,cain_sip_transaction_t)->on_terminate(t);
	ev.source=t->provider;
	ev.transaction=t;
	ev.is_server_transaction=CAIN_SIP_IS_INSTANCE_OF(t,cain_sip_server_transaction_t);
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS(t->provider,process_transaction_terminated,&ev);
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
	prov->client_transactions=cain_sip_list_remove(prov->client_transactions,t);
	cain_sip_object_unref(t);
}

void cain_sip_provider_add_server_transaction(cain_sip_provider_t *prov, cain_sip_server_transaction_t *t){
	prov->server_transactions=cain_sip_list_prepend(prov->server_transactions,cain_sip_object_ref(t));
}

struct server_transaction_matcher{
	const char *branchid;
	const char *method;
	const char *sentby;
	int is_ack;
};

static int rfc3261_server_transaction_match(const void *p_tr, const void *p_matcher){
	cain_sip_server_transaction_t *tr=(cain_sip_server_transaction_t*)p_tr;
	struct server_transaction_matcher *matcher=(struct server_transaction_matcher*)p_matcher;
	const char *req_method=cain_sip_request_get_method(tr->base.request);
	if (strcmp(matcher->branchid,tr->base.branch_id)==0){
		if (strcmp(matcher->method,req_method)==0) return 0;
		if (matcher->is_ack && strcmp(req_method,"INVITE")==0) return 0;
	}
	return -1;
}

cain_sip_server_transaction_t * cain_sip_provider_find_matching_server_transaction(cain_sip_provider_t *prov, cain_sip_request_t *req){
	struct server_transaction_matcher matcher;
	cain_sip_header_via_t *via=(cain_sip_header_via_t*)cain_sip_message_get_header((cain_sip_message_t*)req,"via");
	cain_sip_server_transaction_t *ret=NULL;
	cain_sip_list_t *elem;
	if (via==NULL){
		cain_sip_warning("Request has no via.");
		return NULL;
	}
	matcher.branchid=cain_sip_header_via_get_branch(via);
	matcher.method=cain_sip_request_get_method(req);
	matcher.is_ack=(strcmp(matcher.method,"ACK")==0);
	if (strncmp(matcher.branchid,CAIN_SIP_BRANCH_MAGIC_COOKIE,strlen(CAIN_SIP_BRANCH_MAGIC_COOKIE))==0){
		/*compliant to RFC3261*/
		elem=cain_sip_list_find_custom(prov->client_transactions,rfc3261_server_transaction_match,&matcher);
	}else{
		//FIXME
	}
	
	if (elem){
		ret=(cain_sip_server_transaction_t*)elem->data;
		cain_sip_message("Found transaction matching request.");
	}
	return ret;
}

void cain_sip_provider_remove_server_transaction(cain_sip_provider_t *prov, cain_sip_server_transaction_t *t){	
	prov->server_transactions=cain_sip_list_remove(prov->server_transactions,t);
	cain_sip_object_unref(t);
}
