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

#include <stdio.h>
#include "CUnit/Basic.h"
#include "cain-sip/cain-sip.h"
#include "pthread.h"

extern cain_sip_stack_t * stack;
extern cain_sip_provider_t *prov;
extern const char *test_domain;
extern int register_init(void);
extern int register_uninit(void);
extern cain_sip_request_t* register_user(cain_sip_stack_t * stack
		,cain_sip_provider_t *prov
		,const char *transport
		,int use_transaction
		,const char* username) ;
extern void unregister_user(cain_sip_stack_t * stack
					,cain_sip_provider_t *prov
					,cain_sip_request_t* initial_request
					,int use_transaction);



static int init(void) {
	register_init();
	return 0;
}
static int uninit(void) {
	register_uninit();

	return 0;
}

cain_sip_request_t* build_request(cain_sip_stack_t * stack
									, cain_sip_provider_t *prov
									,cain_sip_header_address_t* from
									,cain_sip_header_address_t* to
									,cain_sip_header_address_t* route
									,const char* method) {
	cain_sip_header_from_t* from_header;
	cain_sip_header_to_t* to_header;
	cain_sip_request_t *req;
	cain_sip_uri_t* req_uri;
	cain_sip_header_contact_t* contact_header;


	from_header = cain_sip_header_from_create(from,CAIN_SIP_RANDOM_TAG);
	to_header = cain_sip_header_to_create(to,NULL);
	req_uri = (cain_sip_uri_t*)cain_sip_object_clone((cain_sip_object_t*)cain_sip_header_address_get_uri((cain_sip_header_address_t*)to_header));

	contact_header= cain_sip_header_contact_new();
	cain_sip_header_address_set_uri((cain_sip_header_address_t*)contact_header,cain_sip_uri_new());
	cain_sip_uri_set_user(cain_sip_header_address_get_uri((cain_sip_header_address_t*)contact_header),cain_sip_uri_get_user(req_uri));
	req=cain_sip_request_create(
							req_uri,
							method,
		                    cain_sip_provider_get_new_call_id(prov),
		                    cain_sip_header_cseq_create(20,method),
		                    from_header,
		                    to_header,
		                    cain_sip_header_via_new(),
		                    70);
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(contact_header));
	if (route) {
		cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(cain_sip_header_route_create(route)));
	}
	return req;
}

static void process_dialog_terminated(void *user_ctx, const cain_sip_dialog_terminated_event_t *event){
	cain_sip_message("process_dialog_terminated not implemented yet");
}
static void process_io_error(void *user_ctx, const cain_sip_io_error_event_t *event){
	cain_sip_message("process_io_error not implemented yet");
}
static void process_request_event(void *user_ctx, const cain_sip_request_event_t *event) {
	/*cain_sip_server_transaction_t* server_transaction = cain_sip_request_event_get_server_transaction(event);
	SalOp* op = (SalOp*)cain_sip_transaction_get_application_data(CAIN_SIP_TRANSACTION(server_transaction));*/
	cain_sip_message("process_request_event not implemented yet");
}

static void process_response_event(void *user_ctx, const cain_sip_response_event_t *event){
	cain_sip_message("process_response_event not implemented yet");
}
static void process_timeout(void *user_ctx, const cain_sip_timeout_event_t *event) {
/*	cain_sip_client_transaction_t* client_transaction = cain_sip_timeout_event_get_client_transaction(event);
	SalOp* op = (SalOp*)cain_sip_transaction_get_application_data(CAIN_SIP_TRANSACTION(client_transaction));
	if (op->callbacks.process_timeout) {
		op->callbacks.process_timeout(op,event);
	} else*/ {
		cain_sip_message("Unhandled event timeout [%p]",event);
	}
}
static void process_transaction_terminated(void *user_ctx, const cain_sip_transaction_terminated_event_t *event) {
/*	cain_sip_client_transaction_t* client_transaction = cain_sip_transaction_terminated_event_get_client_transaction(event);
	SalOp* op = (SalOp*)cain_sip_transaction_get_application_data(client_transaction);
	if (op->calbacks.process_transaction_terminated) {
		op->calbacks.process_transaction_terminated(op,event);
	} else */{
		cain_sip_message("Unhandled transaction terminated [%p]",event);
	}
}

static const char* sdp = 		"v=0\r\n"\
								"o=jehan-mac 1239 1239 IN IP4 192.168.0.18\r\n"\
								"s=Talk\r\n"\
								"c=IN IP4 192.168.0.18\r\n"\
								"t=0 0\r\n"\
								"m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"\
								"a=rtpmap:111 speex/16000\r\n"\
								"a=fmtp:111 vbr=on\r\n"\
								"a=rtpmap:110 speex/8000\r\n"\
								"a=fmtp:110 vbr=on\r\n"\
								"a=rtpmap:101 telephone-event/8000\r\n"\
								"a=fmtp:101 0-11\r\n"\
								"m=video 8078 RTP/AVP 99 97 98\r\n"\
								"c=IN IP4 192.168.0.18\r\n"\
								"b=AS:380\r\n"\
								"a=rtpmap:99 MP4V-ES/90000\r\n"\
								"a=fmtp:99 profile-level-id=3\r\n"\
								"a=rtpmap:97 theora/90000\r\n"\
								"a=rtpmap:98 H263-1998/90000\r\n"\
								"a=fmtp:98 CIF=1;QCIF=1\r\n";

static void simple_call(void) {
#define CALLER "marie"
#define CALLEE "pauline"
	cain_sip_request_t *pauline_register_req;
	cain_sip_request_t *marie_register_req;
	cain_sip_listener_callbacks_t caller_listener_callbacks;
	cain_sip_listener_callbacks_t callee_listener_callbacks;
	cain_sip_request_t* req;
	cain_sip_header_address_t* from;
	cain_sip_header_address_t* to;
	cain_sip_header_address_t* route;
	cain_sip_header_allow_t* header_allow;
	cain_sip_header_content_type_t* content_type ;
	cain_sip_header_content_length_t* content_length;
	cain_sip_client_transaction_t* client_transaction;

	caller_listener_callbacks.process_dialog_terminated=process_dialog_terminated;
	caller_listener_callbacks.process_io_error=process_io_error;
	caller_listener_callbacks.process_request_event=process_request_event;
	caller_listener_callbacks.process_response_event=process_response_event;
	caller_listener_callbacks.process_timeout=process_timeout;
	caller_listener_callbacks.process_transaction_terminated=process_transaction_terminated;

	callee_listener_callbacks.process_dialog_terminated=process_dialog_terminated;
	callee_listener_callbacks.process_io_error=process_io_error;
	callee_listener_callbacks.process_request_event=process_request_event;
	callee_listener_callbacks.process_response_event=process_response_event;
	callee_listener_callbacks.process_timeout=process_timeout;
	callee_listener_callbacks.process_transaction_terminated=process_transaction_terminated;

	pauline_register_req=register_user(stack, prov, "TCP" ,1 ,CALLER);
	marie_register_req=register_user(stack, prov, "TLS" ,1 ,CALLEE);

	cain_sip_provider_add_sip_listener(prov,cain_sip_listener_create_from_callbacks(&caller_listener_callbacks,NULL));
	cain_sip_provider_add_sip_listener(prov,cain_sip_listener_create_from_callbacks(&callee_listener_callbacks,NULL));

	from=cain_sip_header_address_create(NULL,cain_sip_uri_create(CALLER,test_domain));
	to=cain_sip_header_address_create(NULL,cain_sip_uri_create(CALLEE,test_domain));
	route = cain_sip_header_address_create(NULL,cain_sip_uri_create(NULL,test_domain));
	cain_sip_uri_set_transport_param(cain_sip_header_address_get_uri(route),"tcp");

	req=build_request(stack,prov,from,to,route,"INVITE");
	header_allow = cain_sip_header_allow_create("INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(header_allow));

	content_length= cain_sip_header_content_length_create(strlen(sdp));
	content_type = cain_sip_header_content_type_create("application","sdp");
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(content_type));
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(content_length));
	cain_sip_message_set_body(CAIN_SIP_MESSAGE(req),sdp,strlen(sdp));


	client_transaction = cain_sip_provider_get_new_client_transaction(prov,req);
	//cain_sip_transaction_set_application_data(CAIN_SIP_TRANSACTION(client_transaction),op);
	cain_sip_client_transaction_send_request(client_transaction);
	cain_sip_stack_sleep(stack,3000);

	unregister_user(stack, prov, pauline_register_req ,1);
	unregister_user(stack, prov, marie_register_req ,1);
}
int cain_sip_dialog_test_suite(){
	CU_pSuite pSuite = CU_add_suite("Dialog", init, uninit);

	if (NULL == CU_add_test(pSuite, "simple call", simple_call)) {
		return CU_get_error();
	}
	return 0;
}

