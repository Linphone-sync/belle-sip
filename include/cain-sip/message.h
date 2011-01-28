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
#ifndef CAIN_SIP_MESSAGE_H
#define CAIN_SIP_MESSAGE_H

typedef struct _cain_sip_message cain_sip_message_t;
typedef struct _cain_sip_request cain_sip_request_t;
typedef struct _cain_sip_response cain_sip_response_t;

#define CAIN_SIP_MESSAGE(obj)			CAIN_SIP_CAST(obj,cain_sip_message_t)
#define CAIN_SIP_REQUEST(obj)			CAIN_SIP_CAST(obj,cain_sip_request_t)
#define CAIN_SIP_RESPONSE(obj)		CAIN_SIP_CAST(obj,cain_sip_response_t)

cain_sip_message_t* cain_sip_message_parse(const char* raw);
int cain_sip_message_is_request(cain_sip_message_t *msg);
cain_sip_request_t* cain_sip_request_new();
cain_sip_request_t* cain_sip_request_parse(const char* raw);






cain_sip_uri_t* cain_sip_request_get_uri(cain_sip_request_t* request);
void cain_sip_request_set_uri(cain_sip_request_t* request,cain_sip_uri_t* uri);
const char* cain_sip_request_get_method(cain_sip_request_t* request);
void cain_sip_request_set_method(cain_sip_request_t* request,const char* method);

int cain_sip_message_is_response(cain_sip_message_t *msg);

cain_sip_header_t *cain_sip_message_get_header_last(cain_sip_message_t *msg, const char *header_name);
/**
 * add an header to this message
 * @param msg
 * @param header to add, must be one of header type
 */
void cain_sip_message_add_header(cain_sip_message_t *msg, cain_sip_object_t* header);

char *cain_sip_message_to_string(cain_sip_message_t *msg);

#endif

