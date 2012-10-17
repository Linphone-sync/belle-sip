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
#ifndef CAIN_SIP_TRANSACTION_H
#define CAIN_SIP_TRANSACTION_H


typedef enum cain_sip_transaction_state{
	CAIN_SIP_TRANSACTION_INIT,
	CAIN_SIP_TRANSACTION_CALLING,
	CAIN_SIP_TRANSACTION_COMPLETED,
	CAIN_SIP_TRANSACTION_CONFIRMED,
	CAIN_SIP_TRANSACTION_ACCEPTED, /*<for Invite transaction, introduced by RFC6026, fixing bugs in RFC3261*/
	CAIN_SIP_TRANSACTION_PROCEEDING,
	CAIN_SIP_TRANSACTION_TRYING,
	CAIN_SIP_TRANSACTION_TERMINATED
}cain_sip_transaction_state_t;

CAIN_SIP_BEGIN_DECLS

void *cain_sip_transaction_get_application_data(const cain_sip_transaction_t *t);
void cain_sip_transaction_set_application_data(cain_sip_transaction_t *t, void *data);
const char *cain_sip_transaction_get_branch_id(const cain_sip_transaction_t *t);
cain_sip_transaction_state_t cain_sip_transaction_get_state(const cain_sip_transaction_t *t);
void cain_sip_transaction_terminate(cain_sip_transaction_t *t);
cain_sip_request_t *cain_sip_transaction_get_request(const cain_sip_transaction_t *t);
cain_sip_dialog_t*  cain_sip_transaction_get_dialog(const cain_sip_transaction_t *t);

void cain_sip_server_transaction_send_response(cain_sip_server_transaction_t *t, cain_sip_response_t *resp);

cain_sip_request_t * cain_sip_client_transaction_create_cancel(cain_sip_client_transaction_t *t);
int cain_sip_client_transaction_send_request(cain_sip_client_transaction_t *t);
/**
 * Create an authenticated request based on an existing terminated transaction
 * */
cain_sip_request_t* cain_sip_client_create_authenticated_request(cain_sip_client_transaction_t *t);

#define CAIN_SIP_TRANSACTION(t) CAIN_SIP_CAST(t,cain_sip_transaction_t)
#define CAIN_SIP_SERVER_TRANSACTION(t) CAIN_SIP_CAST(t,cain_sip_server_transaction_t)
#define CAIN_SIP_CLIENT_TRANSACTION(t) CAIN_SIP_CAST(t,cain_sip_client_transaction_t)


CAIN_SIP_END_DECLS

#endif

