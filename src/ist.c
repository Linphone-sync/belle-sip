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

static void ist_destroy(cain_sip_ist_t *obj){
}


static void ist_on_terminate(cain_sip_ist_t *obj){
//	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
}

static int ist_send_new_response(cain_sip_ist_t *obj, cain_sip_response_t *resp){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	int code=cain_sip_response_get_status_code(resp);
	int ret=0;
	switch(base->state){
		case CAIN_SIP_TRANSACTION_PROCEEDING:
			if (code==100)
				cain_sip_channel_queue_message(base->channel,(cain_sip_message_t*)resp);
		break;
		default:
		break;
	}
	return ret;
}

static void ist_on_request_retransmission(cain_sip_nist_t *obj){
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
	cain_sip_object_unref(resp);
	return obj;
}
