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



#define CAIN_SIP_MESSAGE(obj)		CAIN_SIP_CAST(obj,cain_sip_message_t)
#define CAIN_SIP_REQUEST(obj)		CAIN_SIP_CAST(obj,cain_sip_request_t)
#define CAIN_SIP_RESPONSE(obj)		CAIN_SIP_CAST(obj,cain_sip_response_t)

CAIN_SIP_BEGIN_DECLS

CAINSIP_EXPORT cain_sip_message_t* cain_sip_message_parse(const char* raw);
/**
 * Parse sip message from a raw buffer
 * @param [in] buff buffer to be parsed
 * @param [in] buff_length size of the buffer to be parsed
 * @param [out] message_length number of bytes read
 * @return parsed message
 */
CAINSIP_EXPORT cain_sip_message_t* cain_sip_message_parse_raw (const char* buff, size_t buff_length,size_t* message_length );

CAINSIP_EXPORT int cain_sip_message_is_request(cain_sip_message_t *msg);
CAINSIP_EXPORT cain_sip_request_t* cain_sip_request_new();
cain_sip_request_t* cain_sip_request_parse(const char* raw);

CAINSIP_EXPORT cain_sip_request_t* cain_sip_request_create(cain_sip_uri_t *requri, const char* method,
                                         cain_sip_header_call_id_t *callid,
                                         cain_sip_header_cseq_t *cseq,
                                         cain_sip_header_from_t *from,
                                         cain_sip_header_to_t *to,
                                         cain_sip_header_via_t *via,
                                         int max_forwards);




CAINSIP_EXPORT cain_sip_uri_t* cain_sip_request_get_uri(cain_sip_request_t* request);
void cain_sip_request_set_uri(cain_sip_request_t* request, cain_sip_uri_t* uri);
CAINSIP_EXPORT const char* cain_sip_request_get_method(const cain_sip_request_t* request);
void cain_sip_request_set_method(cain_sip_request_t* request,const char* method);
/**
 * Guess the origin of the received sip message from VIA header (thanks to received/rport)
 * @param req request to be analyzed
 * @ return a newly allocated uri
 * */
CAINSIP_EXPORT cain_sip_uri_t* cain_sip_request_extract_origin(const cain_sip_request_t* req);

int cain_sip_message_is_response(const cain_sip_message_t *msg);

CAINSIP_EXPORT cain_sip_header_t *cain_sip_message_get_header(const cain_sip_message_t *msg, const char *header_name);

CAINSIP_EXPORT cain_sip_object_t *_cain_sip_message_get_header_by_type_id(const cain_sip_message_t *message, cain_sip_type_id_t id);

#define cain_sip_message_get_header_by_type(msg,header_type)\
	(header_type*)_cain_sip_message_get_header_by_type_id(CAIN_SIP_MESSAGE(msg),CAIN_SIP_TYPE_ID(header_type))

const cain_sip_list_t* cain_sip_message_get_headers(const cain_sip_message_t *message,const char* header_name);
/**
 * Get list of all headers present in the message.
 * @param message
 * @return a newly allocated list of cain_sip_header_t
 * */
cain_sip_list_t* cain_sip_message_get_all_headers(const cain_sip_message_t *message);
/**
 * add an header to this message
 * @param msg
 * @param header to add, must be one of header type
 */
CAINSIP_EXPORT void cain_sip_message_add_header(cain_sip_message_t *msg, cain_sip_header_t* header);

void cain_sip_message_add_headers(cain_sip_message_t *message, const cain_sip_list_t *header_list);

void cain_sip_message_set_header(cain_sip_message_t *msg, cain_sip_header_t* header);

void cain_sip_message_remove_first(cain_sip_message_t *msg, const char *header_name);

void cain_sip_message_remove_last(cain_sip_message_t *msg, const char *header_name);

CAINSIP_EXPORT void cain_sip_message_remove_header(cain_sip_message_t *msg, const char *header_name);

char *cain_sip_message_to_string(cain_sip_message_t *msg);
CAINSIP_EXPORT const char* cain_sip_message_get_body(cain_sip_message_t *msg);
CAINSIP_EXPORT void cain_sip_message_set_body(cain_sip_message_t *msg,const char* body,unsigned int size);
/*message keep ownership of the null terminated body buffer
void cain_sip_message_assign_body(cain_sip_message_t *msg,char* body);*/

CAINSIP_EXPORT int cain_sip_response_get_status_code(const cain_sip_response_t *response);
void cain_sip_response_set_status_code(cain_sip_response_t *response,int status);

CAINSIP_EXPORT const char* cain_sip_response_get_reason_phrase(const cain_sip_response_t *response);
void cain_sip_response_set_reason_phrase(cain_sip_response_t *response,const char* reason_phrase);


CAINSIP_EXPORT cain_sip_response_t *cain_sip_response_new(void);

CAINSIP_EXPORT cain_sip_response_t *cain_sip_response_create_from_request(cain_sip_request_t *req, int status_code);
/**
 * This method takes  received/rport/via value of the reponse and update the contact IP/port accordingly
 * @param response use to extract via/received/rport from top most via.
 * @param contact contact to be updated
 * @returns 0 if no error
 * */
int cain_sip_response_fix_contact(const cain_sip_response_t* response,cain_sip_header_contact_t* contact);


CAIN_SIP_END_DECLS

#endif

