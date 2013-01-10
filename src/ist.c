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
 * INVITE server transaction implementation.
**/

#include "cain_sip_internal.h"

static void ist_on_terminate(cain_sip_ist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	/*timer pointers are set to NULL because they can be released later*/
	if (obj->timer_G){
		cain_sip_transaction_stop_timer(base,obj->timer_G);
		cain_sip_object_unref(obj->timer_G);
		obj->timer_G=NULL;
	}
	if (obj->timer_H){
		cain_sip_transaction_stop_timer(base,obj->timer_H);
		cain_sip_object_unref(obj->timer_H);
		obj->timer_H=NULL;
	}
	if (obj->timer_I){
		cain_sip_transaction_stop_timer(base,obj->timer_I);
		cain_sip_object_unref(obj->timer_I);
		obj->timer_I=NULL;
	}
	if (obj->timer_L){
		cain_sip_transaction_stop_timer(base,obj->timer_L);
		cain_sip_object_unref(obj->timer_L);
		obj->timer_L=NULL;
	}
}

static void ist_destroy(cain_sip_ist_t *obj){
	ist_on_terminate(obj);
}

static int ist_on_timer_G(cain_sip_ist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	if (base->state==CAIN_SIP_TRANSACTION_COMPLETED){
		const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);
		int interval=cain_sip_source_get_timeout(obj->timer_G);
	
		cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->last_response);
		cain_sip_source_set_timeout(obj->timer_G,MIN(2*interval,cfg->T2));
		return CAIN_SIP_CONTINUE;
	}
	return CAIN_SIP_STOP;
}

static int ist_on_timer_H(cain_sip_ist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	if (base->state==CAIN_SIP_TRANSACTION_COMPLETED){
		cain_sip_transaction_terminate(base);
		/*FIXME: no ACK was received, should report the faillure */
	}
	return CAIN_SIP_STOP;
}

static int ist_on_timer_I(cain_sip_ist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	cain_sip_transaction_terminate(base);
	return CAIN_SIP_STOP;
}

static int ist_on_timer_L(cain_sip_ist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	cain_sip_transaction_terminate(base);
	return CAIN_SIP_STOP;
}

int cain_sip_ist_process_ack(cain_sip_ist_t *obj, cain_sip_message_t *ack){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	int ret=-1;
	switch(base->state){
		case CAIN_SIP_TRANSACTION_COMPLETED:
			/*clear timer G*/
			if (obj->timer_G){
				cain_sip_transaction_stop_timer(base,obj->timer_G);
				cain_sip_object_unref(obj->timer_G);
				obj->timer_G=NULL;
			}
			base->state=CAIN_SIP_TRANSACTION_CONFIRMED;
			if (!cain_sip_channel_is_reliable(base->channel)){
				const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);
				obj->timer_I=cain_sip_timeout_source_new((cain_sip_source_func_t)ist_on_timer_I,obj,cfg->T4);
				cain_sip_transaction_start_timer(base,obj->timer_I);
			}else ist_on_timer_I(obj);
		break;
		case CAIN_SIP_TRANSACTION_ACCEPTED:
			ret=0; /*let the ACK be reported to TU */
		break;
		default:
		break;
	}
	return ret;
}

static int ist_send_new_response(cain_sip_ist_t *obj, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	int code=cain_sip_response_get_status_code(resp);
	int ret=-1;
	switch(base->state){
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			{
				const cain_sip_timer_config_t *cfg=cain_sip_transaction_get_timer_config(base);
				ret=0;
				cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)resp);
				if (code>=200 && code<300){
					base->state=CAIN_SIP_TRANSACTION_ACCEPTED;
					obj->timer_L=cain_sip_timeout_source_new((cain_sip_source_func_t)ist_on_timer_L,obj,64*cfg->T1);
					cain_sip_transaction_start_timer(base,obj->timer_L);
				}else if (code>=300){
					base->state=CAIN_SIP_TRANSACTION_COMPLETED;
					if (!cain_sip_channel_is_reliable(base->channel)){
						obj->timer_G=cain_sip_timeout_source_new((cain_sip_source_func_t)ist_on_timer_G,obj,cfg->T1);
						cain_sip_transaction_start_timer(base,obj->timer_G);
					}
					obj->timer_H=cain_sip_timeout_source_new((cain_sip_source_func_t)ist_on_timer_H,obj,64*cfg->T1);
					cain_sip_transaction_start_timer(base,obj->timer_H);
				}
			}
		break;
		case CAIN_SIP_TRANSACTION_ACCEPTED:
			if (code>=200 && code<300){
				ret=0; /*let the response go to transport layer*/
			}
		default:
		break;
	}
	return ret;
}

static void ist_on_request_retransmission(cain_sip_nist_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	switch(base->state){
		case CAIN_SIP_TRANSACTION_PROCEEDING:
		case CAIN_SIP_TRANSACTION_COMPLETED:
			cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)base->last_response);
		break;
		default:
		break;
	}
}


CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_ist_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_ist_t)={
	{
		{
			{
				CAIN_SIP_VPTR_INIT(cain_sip_ist_t,cain_sip_server_transaction_t,TRUE),
				(cain_sip_object_destroy_t)ist_destroy,
				NULL,
				NULL
			},
			(void (*)(cain_sip_transaction_t *))ist_on_terminate
		},
		(int (*)(cain_sip_server_transaction_t*, cain_sip_response_t *))ist_send_new_response,
		(void (*)(cain_sip_server_transaction_t*))ist_on_request_retransmission,
	}
};


cain_sip_ist_t *cain_sip_ist_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_ist_t *obj=cain_sip_object_new(cain_sip_ist_t);
	cain_sip_server_transaction_init((cain_sip_server_transaction_t*)obj,prov,req);
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	cain_sip_response_t *resp;
	
	base->state=CAIN_SIP_TRANSACTION_PROCEEDING;
	resp=cain_sip_response_create_from_request(req,100);
	cain_sip_server_transaction_send_response((cain_sip_server_transaction_t*)obj,resp);
	return obj;
}
