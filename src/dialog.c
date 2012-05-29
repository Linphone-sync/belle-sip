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
		CAIN_SIP_VPTR_INIT(cain_sip_dialog_t, cain_sip_object_t,FALSE),
		(cain_sip_object_destroy_t)cain_sip_dialog_uninit,
		NULL,
		NULL
};

static void set_to_tag(cain_sip_dialog_t *obj, cain_sip_header_to_t *to){
	const char *to_tag=cain_sip_header_to_get_tag(to);
	if (obj->is_server){
		if (to_tag)
			obj->local_tag=cain_sip_strdup(to_tag);
	}else{
		if (to_tag)
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
	cain_sip_header_call_id_t *call_id=cain_sip_message_get_header_by_type(req,cain_sip_header_call_id_t);
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
	if (!call_id){
		cain_sip_error("No call_id in request.");
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
	for(elem=cain_sip_message_get_headers((cain_sip_message_t*)req,CAIN_SIP_RECORD_ROUTE);elem!=NULL;elem=elem->next){
		obj->route_set=cain_sip_list_append(obj->route_set,cain_sip_header_route_create(
		                                     (cain_sip_header_address_t*)elem->data));
	}
	check_route_set(obj->route_set);
	obj->remote_target=(cain_sip_header_address_t*)cain_sip_object_ref(ct);
	obj->remote_cseq=cain_sip_header_cseq_get_seq_number(cseq);
	obj->call_id=(cain_sip_header_call_id_t*)cain_sip_object_ref(call_id);
	/*remote party already set */
	obj->local_party=(cain_sip_header_address_t*)cain_sip_object_ref(to);
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
	cain_sip_header_call_id_t *call_id=cain_sip_message_get_header_by_type(req,cain_sip_header_call_id_t);
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
	if (!call_id){
		cain_sip_error("No call_id in request.");
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
	for(elem=cain_sip_message_get_headers((cain_sip_message_t*)resp,CAIN_SIP_RECORD_ROUTE);elem!=NULL;elem=elem->next){
		obj->route_set=cain_sip_list_prepend(obj->route_set,cain_sip_header_route_create(
		                                     (cain_sip_header_address_t*)elem->data));
	}
	check_route_set(obj->route_set);
	obj->remote_target=(cain_sip_header_address_t*)cain_sip_object_ref(ct);
	obj->local_cseq=cain_sip_header_cseq_get_seq_number(cseq);
	obj->call_id=(cain_sip_header_call_id_t*)cain_sip_object_ref(call_id);
	/*local_tag is already set*/
	obj->remote_party=(cain_sip_header_address_t*)cain_sip_object_ref(to);
	/*local party is already set*/
	if (strcmp(cain_sip_request_get_method(req),"INVITE")==0){
		set_last_out_invite(obj,req);
	}
	return 0;
}

int cain_sip_dialog_establish_full(cain_sip_dialog_t *obj, cain_sip_request_t *req, cain_sip_response_t *resp){
	if (obj->is_server)
		return cain_sip_dialog_init_as_uas(obj,req,resp);
	else
		return cain_sip_dialog_init_as_uac(obj,req,resp);
}

int cain_sip_dialog_establish(cain_sip_dialog_t *obj, cain_sip_request_t *req, cain_sip_response_t *resp){
	int code=cain_sip_response_get_status_code(resp);
	cain_sip_header_to_t *to=cain_sip_message_get_header_by_type(resp,cain_sip_header_to_t);

	if (!to){
		cain_sip_error("No to in response.");
		return -1;
	}
	if (code>100 && code<200){
		if (obj->state==CAIN_SIP_DIALOG_NULL)
			set_to_tag(obj,to);
		obj->state=CAIN_SIP_DIALOG_EARLY;
		return -1;
	}else if (code>=200 && code<300){
		if (obj->state==CAIN_SIP_DIALOG_NULL)
			set_to_tag(obj,to);
		if (cain_sip_dialog_establish_full(obj,req,resp)==0){
			obj->state=CAIN_SIP_DIALOG_CONFIRMED;
			obj->needs_ack=TRUE;
		}else return -1;
	}
	return 0;
}

int cain_sip_dialog_update(cain_sip_dialog_t *obj,cain_sip_request_t *req, cain_sip_response_t *resp, int as_uas){
	int code;
	switch (obj->state){
		case CAIN_SIP_DIALOG_NULL:
		case CAIN_SIP_DIALOG_EARLY:
			return cain_sip_dialog_establish(obj,req,resp);
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
				obj->needs_ack=TRUE;
			}else if (strcmp(cain_sip_request_get_method(req),"INVITE")==0 && code>=200 && code<300){
				if (obj->terminate_on_bye) cain_sip_dialog_delete(obj);
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
	obj=cain_sip_object_new(cain_sip_dialog_t);
	obj->terminate_on_bye=1;
	obj->provider=t->provider;
	
	if (CAIN_SIP_OBJECT_IS_INSTANCE_OF(t,cain_sip_server_transaction_t)){
		obj->remote_tag=cain_sip_strdup(from_tag);
		obj->remote_party=(cain_sip_header_address_t*)cain_sip_object_ref(from);
		obj->is_server=TRUE;
	}else{
		obj->local_tag=cain_sip_strdup(from_tag);
		obj->local_party=(cain_sip_header_address_t*)cain_sip_object_ref(from);
		obj->is_server=FALSE;
	}
	obj->state=CAIN_SIP_DIALOG_NULL;
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
	if (obj->local_cseq==0) obj->local_cseq=110;
	cain_sip_request_t *req=cain_sip_request_create(cain_sip_header_address_get_uri(obj->remote_target),
	                                                method,
	                                                obj->call_id,
	                                                cain_sip_header_cseq_create(obj->local_cseq,method),
	                                                cain_sip_header_from_create(obj->local_party,NULL),
	                                                cain_sip_header_to_create(obj->remote_party,NULL),
	                                                cain_sip_header_via_new(),
	                                                0);
	cain_sip_message_add_headers((cain_sip_message_t*)req,obj->route_set);
	if (strcmp(method,"ACK")!=0) obj->local_cseq++;
	return req;
}

void cain_sip_dialog_delete(cain_sip_dialog_t *obj){
	cain_sip_dialog_state_t prevstate=obj->state;
	obj->state=CAIN_SIP_DIALOG_TERMINATED;
	if (prevstate!=CAIN_SIP_DIALOG_NULL)
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

const cain_sip_header_address_t *cain_sip_get_local_party(const cain_sip_dialog_t *dialog){
	return dialog->local_party;
}

const cain_sip_header_address_t *cain_sip_get_remote_party(const cain_sip_dialog_t *dialog){
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
	const char *dcid=cain_sip_header_call_id_get_call_id(obj->call_id);
	if (obj->state==CAIN_SIP_DIALOG_NULL) cain_sip_fatal("_cain_sip_dialog_match() must not be used for dialog in null state.");
	return strcmp(dcid,call_id)==0 && strcmp(obj->local_tag,local_tag)==0 && strcmp(obj->remote_tag,remote_tag)==0;
}

void cain_sip_dialog_check_ack_sent(cain_sip_dialog_t*obj){
	if (obj->needs_ack){
		cain_sip_request_t *req;
		cain_sip_error("Your listener did not ACK'd the 200Ok for your INVITE request. The dialog will be terminated.");
		req=cain_sip_dialog_create_request(obj,"BYE");
		cain_sip_client_transaction_send_request(
			cain_sip_provider_get_new_client_transaction(obj->provider,req));
	}
}

void cain_sip_dialog_handle_200Ok(cain_sip_dialog_t *obj, cain_sip_message_t *msg){
	if (obj->last_out_ack){
		cain_sip_header_cseq_t *cseq=cain_sip_message_get_header_by_type(msg,cain_sip_header_cseq_t);
		if (cseq){
			cain_sip_header_cseq_t *ack_cseq=cain_sip_message_get_header_by_type(msg,cain_sip_header_cseq_t);
			if (cain_sip_header_cseq_get_seq_number(cseq)==cain_sip_header_cseq_get_seq_number(ack_cseq)){
				/*pass for retransmission*/
				cain_sip_message("Dialog retransmitting last ack automatically");
				cain_sip_provider_send_request(obj->provider,obj->last_out_ack);
			}else cain_sip_warning("No ACK to retransmit matching 200Ok");
		}
	}
}
