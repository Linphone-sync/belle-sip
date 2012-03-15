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

static void ict_destroy(cain_sip_ict_t *obj){
}

static void on_ict_terminate(cain_sip_ict_t *obj){
}

static int ict_on_response(cain_sip_ict_t *obj, cain_sip_response_t *resp){
	return 0;
}

static void ict_send_request(cain_sip_ict_t *obj){
	cain_sip_transaction_t *base=(cain_sip_transaction_t*)obj;
	base->state=CAIN_SIP_TRANSACTION_CALLING;
	
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_ict_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_ict_t)={
	{
		{
			{
				CAIN_SIP_VPTR_INIT(cain_sip_ict_t,cain_sip_client_transaction_t,FALSE),
				(cain_sip_object_destroy_t)ict_destroy,
				NULL,
				NULL
			},
			(void (*)(cain_sip_transaction_t*))on_ict_terminate
		},
		(void (*)(cain_sip_client_transaction_t*))ict_send_request,
		(int (*)(cain_sip_client_transaction_t*,cain_sip_response_t*))ict_on_response
	}
};


cain_sip_ict_t *cain_sip_ict_new(cain_sip_provider_t *prov, cain_sip_request_t *req){
	cain_sip_ict_t *obj=cain_sip_object_new(cain_sip_ict_t);
	cain_sip_client_transaction_init((cain_sip_client_transaction_t*)obj,prov,req);
	return obj;
}

