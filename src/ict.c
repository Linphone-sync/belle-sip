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

/** 
 * INVITE client transaction implementation.
**/

#include "cain_sip_internal.h"


static void on_ict_terminate(cain_sip_ict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	if (obj->timer_A){
		cain_sip_transaction_stop_timer(base,obj->timer_A);
		cain_sip_object_unref(obj->timer_A);
		obj->timer_A=NULL;
	}
	if (obj->timer_B){
		cain_sip_transaction_stop_timer(base,obj->timer_B);
		cain_sip_object_unref(obj->timer_B);
		obj->timer_B=NULL;
	}
	if (obj->timer_D){
		cain_sip_transaction_stop_timer(base,obj->timer_D);
		cain_sip_object_unref(obj->timer_D);
		obj->timer_D=NULL;
	}
	if (obj->timer_M){
		cain_sip_transaction_stop_timer(base,obj->timer_M);
		cain_sip_object_unref(obj->timer_M);
		obj->timer_M=NULL;
	}
	if (obj->ack){
		cain_sip_object_unref(obj->ack);
		obj->ack=NULL;
	}
}

static void ict_destroy(cain_sip_ict_t *obj){
	on_ict_terminate(obj);
}

static cain_sip_request_t *make_ack(cain_sip_ict_t *obj, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	if (obj->ack==NULL){
		obj->ack=cain_sip_request_new();
		cain_sip_object_ref(obj->ack);
		cain_sip_request_set_method(obj->ack,"ACK");
		cain_sip_request_set_uri(obj->ack,cain_sip_request_get_uri(base->request));
		cain_sip_util_copy_headers((cain_sip_message_t*)base->request,(cain_sip_message_t*)obj->ack,CAIN_SIP_VIA,FALSE);
		cain_sip_util_copy_headers((cain_sip_message_t*)base->request,(cain_sip_message_t*)obj->ack,CAIN_SIP_CALL_ID,FALSE);
		cain_sip_util_copy_headers((cain_sip_message_t*)base->request,(cain_sip_message_t*)obj->ack,CAIN_SIP_FROM,FALSE);
		cain_sip_util_copy_headers((cain_sip_message_t*)resp,(cain_sip_message_t*)obj->ack,CAIN_SIP_TO,FALSE);
		cain_sip_util_copy_headers((cain_sip_message_t*)base->request,(cain_sip_message_t*)obj->ack,CAIN_SIP_CONTACT,TRUE);
		cain_sip_util_copy_headers((cain_sip_message_t*)base->request,(cain_sip_message_t*)obj->ack,CAIN_SIP_ROUTE,TRUE);
		cain_sip_message_add_header((cain_sip_message_t*)obj->ack,
		(cain_sip_header_t*)cain_sip_header_cseq_create(
			cain_sip_header_cseq_get_seq_number((cain_sip_header_cseq_t*)cain_sip_message_get_header((cain_sip_message_t*)base->request,CAIN_SIP_CSEQ)),
		    "ACK"));
	}

	return obj->ack;
}

static int ict_on_timer_D(cain_sip_ict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	if (base->state==CAIN_SIP_TRANSACTION_COMPLETED){
		cain_sip_transaction_terminate(base);
	}
	return CAIN_SIP_STOP;
}

static int ict_on_timer_M(cain_sip_ict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	if (base->state==CAIN_SIP_TRANSACTION_ACCEPTED){
		cain_sip_transaction_terminate(base);
	}
	return CAIN_SIP_STOP;
}

static void ict_on_response(cain_sip_ict_t *obj, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	int code=cain_sip_response_get_status_code(resp);
	const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);

	switch (base->state){
		case CAIN_SIP_TRANSACTION_CALLING:
			base->state=CAIN_SIP_TRANSACTION_PROCEEDING;
			/* no break*/
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			if (code>=300){
				base->state=CAIN_SIP_TRANSACTION_COMPLETED;
				cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)make_ack(obj,resp));
				cain_sip_client_transaction_notify_response((cain_sip_client_transaction_t*)obj,resp);
				obj->timer_D=cain_sip_timeout_source_new((cain_sip_source_func_t)ict_on_timer_D,obj,cfg->T1*64);
				cain_sip_transaction_start_timer(base,obj->timer_D);
			}else if (code>=200){
				obj->timer_M=cain_sip_timeout_source_new((cain_sip_source_func_t)ict_on_timer_M,obj,cfg->T1*64);
				cain_sip_transaction_start_timer(base,obj->timer_M);
				base->state=CAIN_SIP_TRANSACTION_ACCEPTED;
				cain_sip_client_transaction_notify_response((cain_sip_client_transaction_t*)obj,resp);
			}else if (code>=100){
				cain_sip_client_transaction_notify_response((cain_sip_client_transaction_t*)obj,resp);
			}
		break;
		case CAIN_SIP_TRANSACTION_ACCEPTED:
			if (code>=200 && code<300){
				cain_sip_client_transaction_notify_response((cain_sip_client_transaction_t*)obj,resp);
			}
		break;
		case CAIN_SIP_TRANSACTION_COMPLETED:
			if (code>=300 && obj->ack){
				cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)obj->ack);
			}
		break;
		default:
		break;
	}
}

static int ict_on_timer_A(cain_sip_ict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;

	switch(base->state){
		case CAIN_SIP_TRANSACTION_CALLING:
		{
			/*reset the timer to twice the previous value, and retransmit */
			unsigned int prev_timeout=cain_sip_source_get_timeout(obj->timer_A);
			cain_sip_source_set_timeout(obj->timer_A,2*prev_timeout);
			cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->request);
		}
		break;
		default:
		break;
	}
	
	return CAIN_SIP_CONTINUE;
}

static int ict_on_timer_B(cain_sip_ict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	switch (base->state){
		case CAIN_SIP_TRANSACTION_CALLING:
			cain_sip_transaction_notify_timeout(base);
			cain_sip_transaction_terminate(base);
		break;
		default:
		break;
	}
	return CAIN_SIP_STOP;
}


static void ict_send_request(cain_sip_ict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);

	base->state=CAIN_SIP_TRANSACTION_CALLING;
	
	if (!cain_sip_channel_is_reliable(base->channel)){
		obj->timer_A=cain_sip_timeout_source_new((cain_sip_source_func_t)ict_on_timer_A,obj,cfg->T1);
		cain_sip_transaction_start_timer(base,obj->timer_A);
	}

	obj->timer_B=cain_sip_timeout_source_new((cain_sip_source_func_t)ict_on_timer_B,obj,cfg->T1*64);
	cain_sip_transaction_start_timer(base,obj->timer_B);
	
	cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->request);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_ict_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_ict_t)={
	{
		{
			{
				CAIN_SIP_VPTR_INIT(cain_sip_ict_t,cain_sip_client_transaction_t,TRUE),
				(cain_sip_object_destroy_t)ict_destroy,
				NULL,
				NULL
			},
			(void (*)(cain_sip_transaction_t*))on_ict_terminate
		},
		(void (*)(cain_sip_client_transaction_t*))ict_send_request,
		(void (*)(cain_sip_client_transaction_t*,cain_sip_response_t*))ict_on_response
	}
};


cain_sip_ict_t *cain_sip_ict_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_ict_t *obj=cain_sip_object_new(cain_sip_ict_t);
	cain_sip_client_transaction_init((cain_sip_client_transaction_t*)obj,prov,req);
	return obj;
}

