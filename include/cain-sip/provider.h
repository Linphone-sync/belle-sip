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


#ifndef cain_sip_provider_h
#define cain_sip_provider_h

#define CAIN_SIP_BRANCH_MAGIC_COOKIE "z9hG4bK"

CAIN_SIP_BEGIN_DECLS

int cain_sip_provider_add_listening_point(cain_sip_provider_t *p, cain_sip_listening_point_t *lp);

void cain_sip_provider_remove_listening_point(cain_sip_provider_t *p, cain_sip_listening_point_t *lp);

cain_sip_listening_point_t *cain_sip_provider_get_listening_point(cain_sip_provider_t *p, const char *transport);

const cain_sip_list_t *cain_sip_provider_get_listening_points(cain_sip_provider_t *p);

void cain_sip_provider_add_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l);

void cain_sip_provider_remove_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l);

cain_sip_header_call_id_t * cain_sip_provider_get_new_call_id(const cain_sip_provider_t *prov);

cain_sip_dialog_t *cain_sip_provider_get_new_dialog(cain_sip_provider_t *prov, cain_sip_transaction_t *t);

cain_sip_client_transaction_t *cain_sip_provider_get_new_client_transaction(cain_sip_provider_t *p, cain_sip_request_t *req);

cain_sip_server_transaction_t *cain_sip_provider_get_new_server_transaction(cain_sip_provider_t *p, cain_sip_request_t *req);

cain_sip_stack_t *cain_sip_provider_get_sip_stack(cain_sip_provider_t *p);

void cain_sip_provider_send_request(cain_sip_provider_t *p, cain_sip_request_t *req);

void cain_sip_provider_send_response(cain_sip_provider_t *p, cain_sip_response_t *resp);

void cain_sip_provider_clean_channels(cain_sip_provider_t *p);

/**
 * Add auth info to the request if found
 * @param p object
 * @param request to be updated
 * @param resp response to take authentication values from, might be NULL
 * @param auth_infos  if auth infos cannot be added for an authenticate header, (I.E case where this method return TRUE),
 * a newly allocated cain_sip_auth_info_t object is added to this list. These object contains useful information like realm and username.
 * @returns 0 in case of success,
 *
 **/
int cain_sip_provider_add_authorization(cain_sip_provider_t *p, cain_sip_request_t* request,cain_sip_response_t *resp,cain_sip_list_t** auth_infos);

CAIN_SIP_END_DECLS

#define CAIN_SIP_PROVIDER(obj) CAIN_SIP_CAST(obj,cain_sip_provider_t)

#endif
