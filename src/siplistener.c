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


cain_sip_response_t* cain_sip_response_event_get_response(const cain_sip_response_event_t* event) {
	return event->response;
}

cain_sip_client_transaction_t *cain_sip_response_event_get_client_transaction(const cain_sip_response_event_t* event){
	return event->client_transaction;
}

cain_sip_dialog_t *cain_sip_response_event_get_dialog(const cain_sip_response_event_t* event){
	return event->dialog;
}

cain_sip_request_t* cain_sip_request_event_get_request(const cain_sip_request_event_t* event){
	return event->request;
}

cain_sip_server_transaction_t *cain_sip_request_event_get_server_transaction(const cain_sip_request_event_t* event){
	return event->server_transaction;
}

cain_sip_dialog_t *cain_sip_request_event_get_dialog(const cain_sip_request_event_t* event){
	return event->dialog;
}



typedef struct cain_sip_callbacks cain_sip_callbacks_t;

struct cain_sip_callbacks{
	cain_sip_object_t base;
	cain_sip_listener_callbacks_t cbs;
	void *user_ctx;
};


static void process_dialog_terminated(cain_sip_listener_t *l, const cain_sip_dialog_terminated_event_t *event){
	cain_sip_callbacks_t *obj=(cain_sip_callbacks_t*)l;
	if (obj->cbs.process_dialog_terminated)
		obj->cbs.process_dialog_terminated(obj->user_ctx,event);
}

static void process_io_error(cain_sip_listener_t *l, const cain_sip_io_error_event_t *event){
	cain_sip_callbacks_t *obj=(cain_sip_callbacks_t*)l;
	if (obj->cbs.process_io_error)
		obj->cbs.process_io_error(obj->user_ctx,event);
}

static void process_request_event(cain_sip_listener_t *l, const cain_sip_request_event_t *event){
	cain_sip_callbacks_t *obj=(cain_sip_callbacks_t*)l;
	if (obj->cbs.process_request_event)
		obj->cbs.process_request_event(obj->user_ctx,event);
}

static void process_response_event(cain_sip_listener_t *l, const cain_sip_response_event_t *event){
	cain_sip_callbacks_t *obj=(cain_sip_callbacks_t*)l;
	if (obj->cbs.process_response_event)
		obj->cbs.process_response_event(obj->user_ctx,event);
}

static void process_timeout(cain_sip_listener_t *l, const cain_sip_timeout_event_t *event){
	cain_sip_callbacks_t *obj=(cain_sip_callbacks_t*)l;
	if (obj->cbs.process_timeout)
		obj->cbs.process_timeout(obj->user_ctx,event);
}

static void process_transaction_terminated(cain_sip_listener_t *l, const cain_sip_transaction_terminated_event_t *event){
	cain_sip_callbacks_t *obj=(cain_sip_callbacks_t*)l;
	if (obj->cbs.process_transaction_terminated)
		obj->cbs.process_transaction_terminated(obj->user_ctx,event);
}

CAIN_SIP_DECLARE_VPTR(cain_sip_callbacks_t);

CAIN_SIP_IMPLEMENT_INTERFACE_BEGIN(cain_sip_callbacks_t,cain_sip_listener_t)
	process_dialog_terminated,
	process_io_error,
	process_request_event,
	process_response_event,
	process_timeout,
	process_transaction_terminated
CAIN_SIP_IMPLEMENT_INTERFACE_END

CAIN_SIP_DECLARE_IMPLEMENTED_INTERFACES_1(cain_sip_callbacks_t,cain_sip_listener_t);

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_callbacks_t,cain_sip_object_t,NULL,NULL,NULL,TRUE);


cain_sip_listener_t *cain_sip_listener_create_from_callbacks(const cain_sip_listener_callbacks_t *callbacks, void *user_ctx){
	cain_sip_callbacks_t *obj=cain_sip_object_new(cain_sip_callbacks_t);
	memcpy(&obj->cbs,callbacks,sizeof(cain_sip_listener_callbacks_t));
	obj->user_ctx=user_ctx;
	return CAIN_SIP_LISTENER(obj);
}

