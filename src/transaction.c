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
#include "sender_task.h"

struct cain_sip_transaction{
	cain_sip_object_t base;
	cain_sip_provider_t *provider; /*the provider that created this transaction */
	cain_sip_request_t *request;
	char *branch_id;
	cain_sip_transaction_state_t state;
	cain_sip_sender_task_t *stask;
	void *appdata;
};


static unsigned long transaction_add_timeout(cain_sip_transaction_t *t, cain_sip_source_func_t func, unsigned int time_ms){
	cain_sip_stack_t *stack=cain_sip_provider_get_sip_stack(t->provider);
	return cain_sip_main_loop_add_timeout (stack->ml,func,t,time_ms);
}

/*
static void transaction_remove_timeout(cain_sip_transaction_t *t, unsigned long id){
	cain_sip_stack_t *stack=cain_sip_provider_get_sip_stack(t->provider);
	cain_sip_main_loop_cancel_source (stack->ml,id);
}
*/

static void cain_sip_transaction_init(cain_sip_transaction_t *t, cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_object_init_type(t,cain_sip_transaction_t);
	if (req) cain_sip_object_ref(req);
	t->request=req;
	t->provider=prov;
}

static void transaction_destroy(cain_sip_transaction_t *t){
	if (t->request) cain_sip_object_unref(t->request);
	if (t->stask) cain_sip_object_unref(t->stask);
}

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
	
}

cain_sip_request_t *cain_sip_transaction_get_request(cain_sip_transaction_t *t){
	return t->request;
}

/*
 Server transaction
*/

struct cain_sip_server_transaction{
	cain_sip_transaction_t base;
};

void cain_sip_server_transaction_send_response(cain_sip_server_transaction_t *t){
}

static void server_transaction_destroy(cain_sip_server_transaction_t *t){
	transaction_destroy((cain_sip_transaction_t*)t);
}

cain_sip_server_transaction_t * cain_sip_server_transaction_new(cain_sip_provider_t *prov,cain_sip_request_t *req){
	cain_sip_server_transaction_t *t=cain_sip_object_new(cain_sip_server_transaction_t,(cain_sip_object_destroy_t)server_transaction_destroy);
	cain_sip_transaction_init((cain_sip_transaction_t*)t,prov,req);
	return t;
}

/*
 Client transaction
*/

struct cain_sip_client_transaction{
	cain_sip_transaction_t base;
	unsigned long timer_id;
	uint64_t start_time;
	uint64_t time_F;
	uint64_t time_E;
};

cain_sip_request_t * cain_sip_client_transaction_create_cancel(cain_sip_client_transaction_t *t){
	return NULL;
}

static int on_client_transaction_timer(void *data, unsigned int revents){
	return CAIN_SIP_CONTINUE;
}

static void client_transaction_cb(cain_sip_sender_task_t *task, void *data, int retcode){
	cain_sip_client_transaction_t *t=(cain_sip_client_transaction_t*)data;
	const cain_sip_timer_config_t *tc=cain_sip_stack_get_timer_config (cain_sip_provider_get_sip_stack (t->base.provider));
	if (retcode==0){
		t->base.state=CAIN_SIP_TRANSACTION_TRYING;
		t->timer_id=transaction_add_timeout(&t->base,on_client_transaction_timer,tc->T1);
	}else{
		cain_sip_transaction_terminated_event_t ev;
		ev.source=t->base.provider;
		ev.transaction=(cain_sip_transaction_t*)t;
		ev.is_server_transaction=FALSE;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(t->base.provider,process_transaction_terminated,&ev);
	}
}


void cain_sip_client_transaction_send_request(cain_sip_client_transaction_t *t){
	t->base.stask=cain_sip_sender_task_new(t->base.provider,CAIN_SIP_MESSAGE(t->base.request),client_transaction_cb,t);
	cain_sip_sender_task_send(t->base.stask);
}

static void client_transaction_destroy(cain_sip_client_transaction_t *t ){
	transaction_destroy((cain_sip_transaction_t*)t);
}


cain_sip_client_transaction_t * cain_sip_client_transaction_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_client_transaction_t *t=cain_sip_object_new(cain_sip_client_transaction_t,(cain_sip_object_destroy_t)client_transaction_destroy);
	cain_sip_transaction_init((cain_sip_transaction_t*)t,prov,req);
	return t;
}



