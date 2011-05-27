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



static cain_sip_source_t * transaction_create_timer(cain_sip_transaction_t *t, cain_sip_source_func_t func, unsigned int time_ms){
	cain_sip_stack_t *stack=cain_sip_provider_get_sip_stack(t->provider);
	cain_sip_source_t *s=cain_sip_timeout_source_new (func,t,time_ms);
	cain_sip_main_loop_add_source(stack->ml,s);
	return s;
}


static void transaction_delete_timer(cain_sip_transaction_t *t, cain_sip_source_t *s){
	cain_sip_stack_t *stack=cain_sip_provider_get_sip_stack(t->provider);
	cain_sip_main_loop_cancel_source (stack->ml,s->id);
	cain_sip_object_unref(s);
}

static void cain_sip_transaction_init(cain_sip_transaction_t *t, cain_sip_provider_t *prov, cain_sip_request_t *req){
	if (req) cain_sip_object_ref(req);
	t->request=req;
	t->provider=prov;
}

static void transaction_destroy(cain_sip_transaction_t *t){
	if (t->request) cain_sip_object_unref(t->request);
	if (t->prov_response) cain_sip_object_unref(t->prov_response);
	if (t->final_response) cain_sip_object_unref(t->final_response);
	if (t->stask) cain_sip_object_unref(t->stask);
}

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_transaction_t,cain_sip_object_t,transaction_destroy,NULL);

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
	if (t->timer){
		transaction_delete_timer(t,t->timer);
		t->timer=NULL;
	}
	cain_sip_provider_set_transaction_terminated(t->provider,t);
}

cain_sip_request_t *cain_sip_transaction_get_request(cain_sip_transaction_t *t){
	return t->request;
}

/*
 *
 *
 *	Server transaction
 *
 *
*/

struct cain_sip_server_transaction{
	cain_sip_transaction_t base;
};

static void server_transaction_send_cb(cain_sip_sender_task_t *st, void *data, int retcode){
	cain_sip_server_transaction_t *t=(cain_sip_server_transaction_t *)data;
	if (retcode==0){
		t->base.is_reliable=cain_sip_sender_task_is_reliable(st);
	}else{
		/*the provider is notified of the error by the sender_task, we just need to terminate the transaction*/
		cain_sip_transaction_terminate(&t->base);
	}
}

static void server_transaction_send_response(cain_sip_server_transaction_t *t, cain_sip_response_t *resp){
	if (t->base.stask==NULL){
		t->base.stask=cain_sip_sender_task_new(t->base.provider,server_transaction_send_cb,t);
	}
	
	cain_sip_sender_task_send(t->base.stask,CAIN_SIP_MESSAGE(resp));
}

/* called when a request retransmission is received for that transaction:*/
void cain_sip_server_transaction_retransmit(cain_sip_server_transaction_t *t){
	if (t->base.final_response!=NULL){
		server_transaction_send_response (t,t->base.final_response);
	}else if (t->base.prov_response!=NULL){
		server_transaction_send_response (t,t->base.prov_response);
	}
}

void cain_sip_server_transaction_send_response(cain_sip_server_transaction_t *t, cain_sip_response_t *resp){
	int status_code=cain_sip_response_get_status_code(resp);

	server_transaction_send_response(t,resp);
	
	if (status_code<200){
		if (t->base.prov_response!=NULL){
			cain_sip_object_unref(t->base.prov_response);
		}
		t->base.prov_response=(cain_sip_response_t*)cain_sip_object_ref(resp);
		t->base.state=CAIN_SIP_TRANSACTION_PROCEEDING;
	}else if (status_code<300){
		t->base.state=CAIN_SIP_TRANSACTION_TERMINATED;
		cain_sip_transaction_terminate((cain_sip_transaction_t*)t);
	}else{
		t->base.state=CAIN_SIP_TRANSACTION_COMPLETED;
	}
	
}

static void server_transaction_destroy(cain_sip_server_transaction_t *t){
}

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_server_transaction_t,cain_sip_transaction_t,server_transaction_destroy,NULL);

cain_sip_server_transaction_t * cain_sip_server_transaction_new(cain_sip_provider_t *prov,cain_sip_request_t *req){
	cain_sip_server_transaction_t *t=cain_sip_object_new(cain_sip_server_transaction_t);
	cain_sip_transaction_init((cain_sip_transaction_t*)t,prov,req);
	return t;
}

/*
 *
 *
 *	Client transaction
 *
 *
*/

struct cain_sip_client_transaction{
	cain_sip_transaction_t base;
	uint64_t timer_F;
	uint64_t timer_E;
	uint64_t timer_K;
};

cain_sip_request_t * cain_sip_client_transaction_create_cancel(cain_sip_client_transaction_t *t){
	return NULL;
}

static int on_client_transaction_timer(void *data, unsigned int revents){
	cain_sip_client_transaction_t *t=(cain_sip_client_transaction_t*)data;
	const cain_sip_timer_config_t *tc=cain_sip_stack_get_timer_config (cain_sip_provider_get_sip_stack (t->base.provider));

	switch(t->base.state){
		case CAIN_SIP_TRANSACTION_TRYING: /*NON INVITE*/
			cain_sip_sender_task_send(t->base.stask,NULL);
			t->base.interval=MIN(t->base.interval*2,tc->T2);
			cain_sip_source_set_timeout(t->base.timer,t->base.interval);
		break;
		case CAIN_SIP_TRANSACTION_CALLING: /*INVITES*/
			cain_sip_sender_task_send(t->base.stask,NULL);
			t->base.interval=t->base.interval*2;
			cain_sip_source_set_timeout(t->base.timer,t->base.interval);
		break;
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			if (!t->base.is_invite){
				cain_sip_sender_task_send(t->base.stask,NULL);
				t->base.interval=tc->T2;
				cain_sip_source_set_timeout(t->base.timer,t->base.interval);
			}
		break;
		case CAIN_SIP_TRANSACTION_COMPLETED:
			cain_sip_transaction_terminate((cain_sip_transaction_t*)t);
			return CAIN_SIP_STOP;
		break;
		default:
			cain_sip_error("Unexpected transaction state %i while in timer callback",t->base.state);
	}
	if (cain_sip_time_ms()>=t->timer_F){
		/*report the timeout */
		cain_sip_timeout_event_t ev;
		ev.source=t->base.provider;
		ev.transaction=&t->base;
		ev.is_server_transaction=FALSE;
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS(t->base.provider,process_timeout,&ev);
		cain_sip_transaction_terminate((cain_sip_transaction_t*)t);
		return CAIN_SIP_STOP;
	}
	return CAIN_SIP_CONTINUE;
}

static void client_transaction_cb(cain_sip_sender_task_t *task, void *data, int retcode){
	cain_sip_client_transaction_t *t=(cain_sip_client_transaction_t*)data;
	const cain_sip_timer_config_t *tc=cain_sip_stack_get_timer_config (cain_sip_provider_get_sip_stack (t->base.provider));
	if (retcode==0){
		if (t->base.state==CAIN_SIP_TRANSACTION_INIT){
			t->base.is_reliable=cain_sip_sender_task_is_reliable(task);
			if (t->base.is_invite){
				t->base.state=CAIN_SIP_TRANSACTION_CALLING;
			}else{
				t->base.state=CAIN_SIP_TRANSACTION_TRYING;
			}
			t->base.start_time=cain_sip_time_ms();
			t->timer_F=t->base.start_time+(tc->T1*64);
			if (!t->base.is_reliable){
				t->base.interval=tc->T1;
				t->base.timer=transaction_create_timer(&t->base,on_client_transaction_timer,tc->T1);
			}else{
				t->base.timer=transaction_create_timer(&t->base,on_client_transaction_timer,tc->T1*64);
			}
		}
	}else{
		/* transport layer error*/
		cain_sip_transaction_terminate(&t->base);
	}
}


void cain_sip_client_transaction_send_request(cain_sip_client_transaction_t *t){
	t->base.stask=cain_sip_sender_task_new(t->base.provider,client_transaction_cb,t);
	cain_sip_sender_task_send(t->base.stask,CAIN_SIP_MESSAGE(t->base.request));
}

static void notify_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	cain_sip_provider_t *prov=t->base.provider;
	cain_sip_response_event_t ev;
	ev.source=prov;
	ev.client_transaction=t;
	ev.dialog=NULL;	/*TODO: FIND IT */
	ev.response=resp;
	CAIN_SIP_PROVIDER_INVOKE_LISTENERS(prov,process_response_event,&ev);
}

static void handle_invite_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	int code=cain_sip_response_get_status_code(resp);
	
	if (code>=100 && code<200){
		switch(t->base.state){
			case CAIN_SIP_TRANSACTION_CALLING:
				if (!t->base.is_reliable){
					/* we must stop retransmissions, then program the timer B/F only*/
					cain_sip_source_set_timeout(t->base.timer,t->timer_F-cain_sip_time_ms());
				}
				t->base.state=CAIN_SIP_TRANSACTION_PROCEEDING;
			case CAIN_SIP_TRANSACTION_PROCEEDING:
				if (t->base.prov_response!=NULL){
					cain_sip_object_unref(t->base.prov_response);
				}
				t->base.prov_response=(cain_sip_response_t*)cain_sip_object_ref(resp);
				notify_response(t,resp);
			break;
			default:
				cain_sip_warning("Unexpected provisional response while transaction in state %i",t->base.state);
		}
	}else if (code>=300){
		switch(t->base.state){
			case CAIN_SIP_TRANSACTION_CALLING:
			case CAIN_SIP_TRANSACTION_PROCEEDING:
				t->base.state=CAIN_SIP_TRANSACTION_COMPLETED;
				t->base.final_response=(cain_sip_response_t*)cain_sip_object_ref(resp);
				notify_response(t,resp);
				/*start timer D */
				cain_sip_source_set_timeout(t->base.timer,32000);
			break;
			default:
				cain_sip_warning("Unexpected final response while transaction in state %i",t->base.state);
		}
	}else if (code>=200){
		switch(t->base.state){
			case CAIN_SIP_TRANSACTION_CALLING:
			case CAIN_SIP_TRANSACTION_PROCEEDING:
				notify_response(t,resp);
				cain_sip_transaction_terminate(&t->base);
			break;
			default:
				cain_sip_warning("Unexpected final response while transaction in state %i",t->base.state);
		}
	}
}

static void handle_non_invite_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	int code=cain_sip_response_get_status_code(resp);
	const cain_sip_timer_config_t *tc=cain_sip_stack_get_timer_config (cain_sip_provider_get_sip_stack (t->base.provider));
	
	if (code>=100 && code<200){
		switch(t->base.state){
			case CAIN_SIP_TRANSACTION_CALLING:
				if (!t->base.is_reliable){
					/* we must stop retransmissions, then program the timer B/F only*/
					cain_sip_source_set_timeout(t->base.timer,t->timer_F-cain_sip_time_ms());
				}
			case CAIN_SIP_TRANSACTION_TRYING:
			case CAIN_SIP_TRANSACTION_PROCEEDING:
				t->base.state=CAIN_SIP_TRANSACTION_PROCEEDING;
				if (t->base.prov_response!=NULL){
					cain_sip_object_unref(t->base.prov_response);
				}
				t->base.prov_response=(cain_sip_response_t*)cain_sip_object_ref(resp);
				notify_response(t,resp);
			break;
			default:
				cain_sip_warning("Unexpected provisional response while transaction in state %i",t->base.state);
		}
	}else if (code>=200){
		switch(t->base.state){
			case CAIN_SIP_TRANSACTION_TRYING:
			case CAIN_SIP_TRANSACTION_PROCEEDING:
				t->base.state=CAIN_SIP_TRANSACTION_COMPLETED;
				t->base.final_response=(cain_sip_response_t*)cain_sip_object_ref(resp);
				notify_response(t,resp);
				cain_sip_source_set_timeout(t->base.timer,tc->T4);
			break;
			default:
				cain_sip_warning("Unexpected final response while transaction in state %i",t->base.state);
		}
	}
}

/*called by the transport layer when a response is received */
void cain_sip_client_transaction_add_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp){
	if (t->base.is_invite) handle_invite_response (t,resp);
	else handle_non_invite_response(t, resp);
}

static void client_transaction_destroy(cain_sip_client_transaction_t *t ){
}

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_client_transaction_t, cain_sip_transaction_t,client_transaction_destroy,NULL);

cain_sip_client_transaction_t * cain_sip_client_transaction_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_client_transaction_t *t=cain_sip_object_new(cain_sip_client_transaction_t);
	cain_sip_transaction_init((cain_sip_transaction_t*)t,prov,req);
	if (req && strcmp(cain_sip_request_get_method(req),"INVITE")==0)
		t->base.is_invite=TRUE;
	return t;
}



