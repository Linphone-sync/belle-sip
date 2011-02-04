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
#include "cain_sip_messageParser.h"
#include "cain_sip_messageLexer.h"
#include "cain_sip_internal.h"
typedef struct _headers_container {
	const char* name;
	cain_sip_list_t* header_list;
} headers_container_t;

static headers_container_t* cain_sip_message_headers_container_new(const char* name) {
	headers_container_t* headers_container = cain_sip_new0(headers_container_t);
	headers_container->name= cain_sip_strdup(name);
	return  NULL; /*FIXME*/
}

struct _cain_sip_message {
	cain_sip_object_t base;
	cain_sip_list_t* header_list;
	cain_sip_list_t* headernames_list;
};

CAIN_SIP_PARSE(message)

static int cain_sip_headers_container_comp_func(const headers_container_t *a, const char*b) {
	return strcmp(a->name,b);
}
static void cain_sip_message_init(cain_sip_message_t *message){
	cain_sip_object_init_type(message,cain_sip_message_t);
}

headers_container_t* cain_sip_headers_container_get(cain_sip_message_t* message,const char* header_name) {
	cain_sip_list_t *  result = cain_sip_list_find_custom(	message->header_list
															, (cain_sip_compare_func)cain_sip_headers_container_comp_func
															, header_name);
	return result?(headers_container_t*)(result->data):NULL;
}
void cain_sip_message_add_header(cain_sip_message_t *message,cain_sip_header_t* header) {
	// first check if already exist
	headers_container_t* headers_container = cain_sip_headers_container_get(message,cain_sip_header_get_name(header));
	if (headers_container == NULL) {
		headers_container = cain_sip_message_headers_container_new(cain_sip_header_get_name(header));
		cain_sip_list_append(message->header_list,headers_container);
	}
	cain_sip_list_append(headers_container->header_list,header);

}
const cain_sip_list_t* cain_sip_message_get_headers(cain_sip_message_t *message,const char* header_name) {
	headers_container_t* headers_container = cain_sip_headers_container_get(message,header_name);
	return headers_container ? headers_container->header_list:NULL;
}
struct _cain_sip_request {
	cain_sip_message_t message;
};

static void cain_sip_request_destroy(cain_sip_request_t* request) {

}
CAIN_SIP_NEW(request,message)
CAIN_SIP_PARSE(request)

void cain_sip_request_set_uri(cain_sip_request_t* request,cain_sip_uri_t* uri) {

}

cain_sip_uri_t * cain_sip_request_get_uri(cain_sip_request_t *request){
	return NULL;
}

void cain_sip_request_set_method(cain_sip_request_t* request,const char* method) {

}
int cain_sip_message_is_request(cain_sip_message_t *msg){
	return 0;
}

int cain_sip_message_is_response(cain_sip_message_t *msg){
	return 0;
}

cain_sip_header_t *cain_sip_message_get_header(cain_sip_message_t *msg, const char *header_name){
	const cain_sip_list_t *l=cain_sip_message_get_headers(msg,header_name);
	if (l!=NULL)
		return (cain_sip_header_t*)l->data;
	return NULL;
}


char *cain_sip_message_to_string(cain_sip_message_t *msg){
	return NULL;
}

void cain_sip_response_get_return_hop(cain_sip_response_t *msg, cain_sip_hop_t *hop){
	cain_sip_header_via_t *via=CAIN_SIP_HEADER_VIA(cain_sip_message_get_header(CAIN_SIP_MESSAGE(msg),"via"));
	hop->transport=cain_sip_header_via_get_protocol(via);
	hop->host=cain_sip_header_via_get_received(via);
	if (hop->host==NULL)
		hop->host=cain_sip_header_via_get_host(via);
	hop->port=cain_sip_header_via_get_rport(via);
	if (hop->port==-1)
		hop->port=cain_sip_header_via_get_listening_port(via);
}

int cain_sip_response_get_status_code(const cain_sip_response_t *response){
	return 0;
}

