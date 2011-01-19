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
typedef struct _header_container {
	const char* name;
	cain_sip_list_t* header_list;
} header_container_t;
/*
static header_container_t* cain_sip_message_header_container_new(const char* name) {
	header_container_t* header_container = cain_sip_new0(header_container_t);
	header_container->name= cain_sip_strdup(name);
}
*/
struct _cain_sip_message {
	cain_sip_object_t base;
	cain_sip_list_t* header_list;
	cain_sip_list_t* headernames_list;
};


static void cain_sip_message_init(cain_sip_message_t *message){
	cain_sip_object_init_type(message,cain_sip_message_t);
}

void cain_sip_message_add_header(cain_sip_message_t *message,cain_sip_header_t* header) {

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
void cain_sip_request_set_method(cain_sip_request_t* request,const char* method) {

}
int cain_sip_message_is_request(cain_sip_message_t *msg){
	return 0;
}

int cain_sip_message_is_response(cain_sip_message_t *msg){
	return 0;
}

cain_sip_header_t *cain_sip_message_get_header_last(cain_sip_message_t *msg, const char *header_name){
	return NULL;
}

char *cain_sip_message_to_string(cain_sip_message_t *msg){
	return NULL;
}
