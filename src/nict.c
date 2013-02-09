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
 * non-INVITE client transaction implementation.
**/

#include "cain_sip_internal.h"

static int nict_on_timer_K(cain_sip_nict_t *obj){
	cain_sip_transaction_terminate((cain_sip_transaction_t*)obj);
	return CAIN_SIP_STOP;
}

static void nict_set_completed(cain_sip_nict_t *obj, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);
	base->state=CAIN_SIP_TRANSACTION_COMPLETED;
	if (obj->timer_K) cain_sip_fatal("Should never happen.");

	cain_sip_client_transaction_notify_response((cain_sip_client_transaction_t*)obj,resp);

	if (!cain_sip_channel_is_reliable(base->channel)){
		obj->timer_K=cain_sip_timeout_source_new((cain_sip_source_func_t)nict_on_timer_K,obj,cfg->T4);
		cain_sip_object_set_name((cain_sip_object_t*)obj->timer_K,"timer_K");
		cain_sip_transaction_start_timer(base,obj->timer_K);
	}else cain_sip_transaction_terminate(base);
}

static void nict_on_response(cain_sip_nict_t *obj, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	int code=cain_sip_response_get_status_code(resp);
	
	switch(base->state){
		case CAIN_SIP_TRANSACTION_TRYING:
			if (code<200){
				base->state=CAIN_SIP_TRANSACTION_PROCEEDING;
				cain_sip_client_transaction_notify_response((cain_sip_client_transaction_t*)obj,resp);
			}
			else {
				nict_set_completed(obj,resp);
			}
		break;
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			if (code>=200){
				nict_set_completed(obj,resp);
			}
		break;
		default:
		break;
	}
}

static void nict_on_terminate(cain_sip_nict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	if (obj->timer_F){
		cain_sip_transaction_stop_timer(base,obj->timer_F);
		cain_sip_object_unref(obj->timer_F);
		obj->timer_F=NULL;
	}
	if (obj->timer_E){
		cain_sip_transaction_stop_timer(base,obj->timer_E);
		cain_sip_object_unref(obj->timer_E);
		obj->timer_E=NULL;
	}
	if (obj->timer_K){
		cain_sip_transaction_stop_timer(base,obj->timer_K);
		cain_sip_object_unref(obj->timer_K);
		obj->timer_K=NULL;
	}
}

static int nict_on_timer_F(cain_sip_nict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	switch (base->state){
		case CAIN_SIP_TRANSACTION_TRYING:
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			cain_sip_transaction_notify_timeout(base);
			cain_sip_transaction_terminate(base);
		break;
		default:
		break;
	}
	return CAIN_SIP_STOP;
}

static int nict_on_timer_E(cain_sip_nict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);

	switch(base->state){
		case CAIN_SIP_TRANSACTION_TRYING:
		{
			/*reset the timer */
			unsigned int prev_timeout=cain_sip_source_get_timeout(obj->timer_E);
			cain_sip_source_set_timeout(obj->timer_E,MIN(2*prev_timeout,(unsigned int)cfg->T2));
			cain_sip_message("nict_on_timer_E: sending retransmission");
			cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->request);
		}
		break;
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			cain_sip_source_set_timeout(obj->timer_E,cfg->T2);
			cain_sip_message("nict_on_timer_E: sending retransmission");
			cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->request);
		break;
		default:
			/*if we are not in these cases, timer_E does nothing, so remove it*/
			return CAIN_SIP_STOP;
		break;
	}
	return CAIN_SIP_CONTINUE;
}

static void nict_send_request(cain_sip_nict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);
	
	base->state=CAIN_SIP_TRANSACTION_TRYING;
	obj->timer_F=cain_sip_timeout_source_new((cain_sip_source_func_t)nict_on_timer_F,obj,cfg->T1*64);
	cain_sip_object_set_name((cain_sip_object_t*)obj->timer_F,"timer_F");
	cain_sip_transaction_start_timer(base,obj->timer_F);
	
	if (!cain_sip_channel_is_reliable(base->channel)){
		obj->timer_E=cain_sip_timeout_source_new((cain_sip_source_func_t)nict_on_timer_E,obj,cfg->T1);
		cain_sip_object_set_name((cain_sip_object_t*)obj->timer_E,"timer_E");
		cain_sip_transaction_start_timer(base,obj->timer_E);
	}
	
	cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->request);
}

static void nict_destroy(cain_sip_nict_t *obj){
	nict_on_terminate(obj);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_nict_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_nict_t)={
	{
		{
			{
				CAIN_SIP_VPTR_INIT(cain_sip_nict_t,cain_sip_client_transaction_t,TRUE),
				(cain_sip_object_destroy_t)nict_destroy,
				NULL,
				NULL
			},
			(void (*)(cain_sip_transaction_t *))nict_on_terminate
		},
		(void (*)(cain_sip_client_transaction_t*))nict_send_request,
		(void (*)(cain_sip_client_transaction_t*,cain_sip_response_t*))nict_on_response
	}
};


cain_sip_nict_t *cain_sip_nict_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_nict_t *obj=cain_sip_object_new(cain_sip_nict_t);
	cain_sip_client_transaction_init((cain_sip_client_transaction_t*)obj,prov,req);
	return obj;
}

