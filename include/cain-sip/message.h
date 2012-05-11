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



#define CAIN_SIP_MESSAGE(obj)			CAIN_SIP_CAST(obj,cain_sip_message_t)
#define CAIN_SIP_REQUEST(obj)			CAIN_SIP_CAST(obj,cain_sip_request_t)
#define CAIN_SIP_RESPONSE(obj)		CAIN_SIP_CAST(obj,cain_sip_response_t)

CAIN_SIP_BEGIN_DECLS

cain_sip_message_t* cain_sip_message_parse(const char* raw);
/**
 * Parse sip message from a raw buffer
 * @param [in] buff buffer to be parsed
 * @param [in] buff_length size of the buffer to be parsed
 * @param [out] message_length number of bytes read
 * @return parsed message
 */
cain_sip_message_t* cain_sip_message_parse_raw (const char* buff, size_t buff_length,size_t* message_length );

int cain_sip_message_is_request(cain_sip_message_t *msg);
cain_sip_request_t* cain_sip_request_new();
cain_sip_request_t* cain_sip_request_parse(const char* raw);

cain_sip_request_t* cain_sip_request_create(cain_sip_uri_t *requri, const char* method,
                                         cain_sip_header_call_id_t *callid,
                                         cain_sip_header_cseq_t *cseq,
                                         cain_sip_header_from_t *from,
                                         cain_sip_header_to_t *to,
                                         cain_sip_header_via_t *via,
                                         int max_forwards);




cain_sip_uri_t* cain_sip_request_get_uri(cain_sip_request_t* request);
void cain_sip_request_set_uri(cain_sip_request_t* request, cain_sip_uri_t* uri);
const char* cain_sip_request_get_method(const cain_sip_request_t* request);
void cain_sip_request_set_method(cain_sip_request_t* request,const char* method);

int cain_sip_message_is_response(const cain_sip_message_t *msg);

cain_sip_header_t *cain_sip_message_get_header(const cain_sip_message_t *msg, const char *header_name);

const cain_sip_list_t* cain_sip_message_get_headers(const cain_sip_message_t *message,const char* header_name);

/**
 * add an header to this message
 * @param msg
 * @param header to add, must be one of header type
 */
void cain_sip_message_add_header(cain_sip_message_t *msg, cain_sip_header_t* header);

void cain_sip_message_add_headers(cain_sip_message_t *message, const cain_sip_list_t *header_list);

void cain_sip_message_set_header(cain_sip_message_t *msg, cain_sip_header_t* header);

void cain_sip_message_remove_first(cain_sip_message_t *msg, const char *header_name);

void cain_sip_message_remove_last(cain_sip_message_t *msg, const char *header_name);

void cain_sip_message_remove_header(cain_sip_message_t *msg, const char *header_name);

char *cain_sip_message_to_string(cain_sip_message_t *msg);
const char* cain_sip_message_get_body(cain_sip_message_t *msg);
void cain_sip_message_set_body(cain_sip_message_t *msg,char* body,unsigned int size);
/*message keep ownership of the null terminated body buffer
void cain_sip_message_assign_body(cain_sip_message_t *msg,char* body);*/

int cain_sip_response_get_status_code(const cain_sip_response_t *response);
void cain_sip_response_set_status_code(cain_sip_response_t *response,int status);

const char* cain_sip_response_get_reason_phrase(const cain_sip_response_t *response);
void cain_sip_response_set_reason_phrase(cain_sip_response_t *response,const char* reason_phrase);


cain_sip_response_t *cain_sip_response_new(void);

cain_sip_response_t *cain_sip_response_create_from_request(cain_sip_request_t *req, int status_code);

CAIN_SIP_END_DECLS

#endif

