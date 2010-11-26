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

struct cain_sip_transaction{
	cain_sip_magic_t magic;
	char *branch_id;
	cain_sip_transaction_state_t state;
	void *appdata;
	cain_sip_request_t *request;
};

struct cain_sip_server_transaction{
	cain_sip_transaction_t base;
	cain_sip_magic_t magic;
};

struct cain_sip_client_transaction{
	cain_sip_transaction_t base;
	cain_sip_magic_t magic;
};

CAIN_SIP_IMPLEMENT_CAST(cain_sip_transaction_t);
CAIN_SIP_IMPLEMENT_CAST(cain_sip_server_transaction_t);
CAIN_SIP_IMPLEMENT_CAST(cain_sip_client_transaction_t);

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

void cain_sip_server_transaction_send_response(cain_sip_server_transaction_t *t){
}

cain_sip_request_t * cain_sip_client_transaction_create_cancel(cain_sip_client_transaction_t *t){
	return NULL;
}

void cain_sip_client_transaction_send_request(cain_sip_client_transaction_t *t){
}

static void cain_sip_transaction_init(cain_sip_transaction_t *t, cain_sip_request_t *req){
	t->magic=CAIN_SIP_MAGIC(cain_sip_transaction_t);
	t->request=req;
}

cain_sip_client_transaction_t * cain_sip_client_transaction_new(cain_sip_request_t *req){
	cain_sip_client_transaction_t *t=cain_sip_new0(cain_sip_client_transaction_t);
	cain_sip_transaction_init((cain_sip_transaction_t*)t,req);
	t->magic=CAIN_SIP_MAGIC(cain_sip_client_transaction_t);
	return t;
}

cain_sip_server_transaction_t * cain_sip_server_transaction_new(cain_sip_request_t *req){
	cain_sip_server_transaction_t *t=cain_sip_new0(cain_sip_server_transaction_t);
	cain_sip_transaction_init((cain_sip_transaction_t*)t,req);
	t->magic=CAIN_SIP_MAGIC(cain_sip_server_transaction_t);
	return t;
}


