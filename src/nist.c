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
 * non-INVITE server transaction implementation.
**/

#include "cain_sip_internal.h"

static void nist_destroy(cain_sip_nist_t *obj){
}


static void nist_on_terminate(cain_sip_nist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	if (obj->timer_J){
		cain_sip_transaction_stop_timer(base,obj->timer_J);
		cain_sip_object_unref(obj->timer_J);
		obj->timer_J=NULL;
	}
}

static int nist_on_timer_J(cain_sip_nist_t *obj){
	cain_sip_transaction_terminate((cain_sip_transaction_t *)obj);
	return CAIN_SIP_STOP;
}

static void nist_set_completed(cain_sip_nist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);
	int tval;
	if (!cain_sip_channel_is_reliable(base->channel))
		tval=cfg->T1*64;
	else tval=0;
	obj->timer_J=cain_sip_timeout_source_new((cain_sip_source_func_t)nist_on_timer_J,obj,tval);
	cain_sip_transaction_start_timer(base,obj->timer_J);
}

static int nist_send_new_response(cain_sip_nist_t *obj, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	int code=cain_sip_response_get_status_code(resp);
	int ret=0;
	switch(base->state){
		case CAIN_SIP_TRANSACTION_TRYING:
			if (code<200){
				base->state=CAIN_SIP_TRANSACTION_PROCEEDING;
				cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->last_response);
			}
		break;
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			if (code>=200){
				nist_set_completed(obj);
			}
			cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->last_response);
		break;
		case CAIN_SIP_TRANSACTION_COMPLETED:
			cain_sip_warning("nist_send_new_response(): not allowed to send a response while transaction is completed.");
			ret=-1; /*not allowed to send a response at this time*/
		break;
		default:
			//ignore
		break;
	}
	return ret;
}

static void nist_on_request_retransmission(cain_sip_nist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	switch(base->state){
		case CAIN_SIP_TRANSACTION_PROCEEDING:
		case CAIN_SIP_TRANSACTION_COMPLETED:
			cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->last_response);
		break;
		default:
			//ignore
		break;
	}
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_nist_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_nist_t)={
	{
		{
			{
				CAIN_SIP_VPTR_INIT(cain_sip_nist_t,cain_sip_server_transaction_t,TRUE),
				(cain_sip_object_destroy_t)nist_destroy,
				NULL,
				NULL
			},
			(void (*)(cain_sip_transaction_t *))nist_on_terminate
		},
		(int (*)(cain_sip_server_transaction_t*, cain_sip_response_t *))nist_send_new_response,
		(void (*)(cain_sip_server_transaction_t*))nist_on_request_retransmission,
	}
};


cain_sip_nist_t *cain_sip_nist_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_nist_t *obj=cain_sip_object_new(cain_sip_nist_t);
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	cain_sip_server_transaction_init((cain_sip_server_transaction_t*)obj,prov,req);
	base->state=CAIN_SIP_TRANSACTION_TRYING;
	return obj;
}
