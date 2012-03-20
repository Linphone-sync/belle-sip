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


#ifndef cain_sip_dialog_h
#define cain_sip_dialog_h

enum cain_sip_dialog_state{
	CAIN_SIP_DIALOG_NULL,
	CAIN_SIP_DIALOG_EARLY,
	CAIN_SIP_DIALOG_CONFIRMED,
	CAIN_SIP_DIALOG_TERMINATED
};

typedef enum cain_sip_dialog_state cain_sip_dialog_state_t;

CAIN_SIP_BEGIN_DECLS

cain_sip_request_t *cain_sip_dialog_create_ack(cain_sip_dialog_t *dialog, unsigned int cseq);

cain_sip_request_t *cain_sip_dialog_create_request(cain_sip_dialog_t *dialog, const char *method);

void cain_sip_dialog_delete(cain_sip_dialog_t *dialog);

void *cain_sip_get_application_data(cain_sip_dialog_t *dialog);

void cain_sip_set_application_data(cain_sip_dialog_t *dialog, void *data);

const char *cain_sip_dialog_get_dialog_id(cain_sip_dialog_t *dialog);

const cain_sip_header_address_t *cain_sip_get_local_party(cain_sip_dialog_t *dialog);

const cain_sip_header_address_t *cain_sip_get_remote_party(cain_sip_dialog_t *dialog);

unsigned int cain_sip_dialog_get_local_seq_number(cain_sip_dialog_t *dialog);

unsigned int cain_sip_dialog_get_remote_seq_number(cain_sip_dialog_t *dialog);

const char *cain_sip_dialog_get_local_tag(cain_sip_dialog_t *dialog);

const char *cain_sip_dialog_get_remote_tag(cain_sip_dialog_t *dialog);

const cain_sip_header_address_t *cain_sip_dialog_get_remote_target(cain_sip_dialog_t *dialog);

const cain_sip_list_t* cain_sip_dialog_get_route_set(cain_sip_dialog_t *dialog);

cain_sip_dialog_state_t cain_sip_dialog_get_state(cain_sip_dialog_t *dialog);

int cain_sip_dialog_is_server(cain_sip_dialog_t *dialog);

void cain_sip_dialog_send_ack(cain_sip_dialog_t *dialog, cain_sip_request_t *request);

void cain_sip_dialog_terminate_on_bye(cain_sip_dialog_t *dialog, int val);

CAIN_SIP_END_DECLS

#endif
