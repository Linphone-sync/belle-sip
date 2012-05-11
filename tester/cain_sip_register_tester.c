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

const char *test_domain="test.linphone.org";
static int is_register_ok;
static int using_transaction;
static cain_sip_stack_t * stack;
static cain_sip_provider_t *prov;

static void process_dialog_terminated(cain_sip_listener_t *obj, const cain_sip_dialog_terminated_event_t *event){
	cain_sip_message("process_dialog_terminated called");
}
static void process_io_error(cain_sip_listener_t *obj, const cain_sip_io_error_event_t *event){
	cain_sip_warning("process_io_error");
	cain_sip_main_loop_quit(cain_sip_stack_get_main_loop(stack));
	/*CU_ASSERT(CU_FALSE);*/
}
static void process_request_event(cain_sip_listener_t *obj, const cain_sip_request_event_t *event){
	cain_sip_message("process_request_event");
}
static void process_response_event(cain_sip_listener_t *obj, const cain_sip_response_event_t *event){
	cain_sip_message("process_response_event");
	CU_ASSERT_PTR_NOT_NULL_FATAL(cain_sip_response_event_get_response(event));
	CU_ASSERT_EQUAL(cain_sip_response_get_status_code(cain_sip_response_event_get_response(event)),200);
	is_register_ok=1;
	using_transaction=cain_sip_response_event_get_client_transaction(event)!=NULL;
	cain_sip_main_loop_quit(cain_sip_stack_get_main_loop(stack));
}
static void process_timeout(cain_sip_listener_t *obj, const cain_sip_timeout_event_t *event){
	cain_sip_message("process_timeout");
}
static void process_transaction_terminated(cain_sip_listener_t *obj, const cain_sip_transaction_terminated_event_t *event){
	cain_sip_message("process_transaction_terminated");
}

/*this would normally go to a .h file*/
struct test_listener{
	cain_sip_object_t base;
	void *some_context;
};

typedef struct test_listener test_listener_t;

CAIN_SIP_DECLARE_TYPES_BEGIN(test,0x1000)
	CAIN_SIP_TYPE_ID(test_listener_t)
CAIN_SIP_DECLARE_TYPES_END

CAIN_SIP_DECLARE_VPTR(test_listener_t);

/*the following would go to .c file */

CAIN_SIP_IMPLEMENT_INTERFACE_BEGIN(test_listener_t,cain_sip_listener_t)
	process_dialog_terminated,
	process_io_error,
	process_request_event,
	process_response_event,
	process_timeout,
	process_transaction_terminated
CAIN_SIP_IMPLEMENT_INTERFACE_END

CAIN_SIP_DECLARE_IMPLEMENTED_INTERFACES_1(test_listener_t,cain_sip_listener_t);

CAIN_SIP_INSTANCIATE_VPTR(test_listener_t,cain_sip_object_t,NULL,NULL,NULL,FALSE);

static test_listener_t *listener;

static int init(void) {
	stack=cain_sip_stack_new(NULL);
	cain_sip_listening_point_t *lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",7060,"UDP");
	prov=cain_sip_stack_create_provider(stack,lp);
	cain_sip_object_unref(lp);
	lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",7060,"TCP");
	cain_sip_provider_add_listening_point(prov,lp);
	cain_sip_object_unref(lp);
	lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",7061,"TLS");
	if (lp) {
		cain_sip_provider_add_listening_point(prov,lp);
		cain_sip_object_unref(lp);
	}
	listener=cain_sip_object_new(test_listener_t);
	cain_sip_provider_add_sip_listener(prov,CAIN_SIP_LISTENER(listener));
	return 0;
}
static int uninit(void) {
	cain_sip_object_unref(prov);
	cain_sip_object_unref(stack);
	cain_sip_object_unref(listener);
	return 0;
}
static void register_test(const char *transport, int use_transaction) {
	cain_sip_request_t *req;
	char identity[256];
	char uri[256];

	if (transport)
		snprintf(uri,sizeof(uri),"sip:%s;transport=%s",test_domain,transport);
	else snprintf(uri,sizeof(uri),"sip:%s",test_domain);

	if (transport && strcasecmp("tls",transport)==0 && cain_sip_provider_get_listening_point(prov,"tls")==NULL){
		cain_sip_error("No TLS support, test skipped.");
		return;
	}

	snprintf(identity,sizeof(identity),"Tester <sip:tester@%s>",test_domain);
	req=cain_sip_request_create(
	                    cain_sip_uri_parse(uri),
	                    "REGISTER",
	                    cain_sip_provider_get_new_call_id(prov),
	                    cain_sip_header_cseq_create(20,"REGISTER"),
	                    cain_sip_header_from_create2(identity,CAIN_SIP_RANDOM_TAG),
	                    cain_sip_header_to_create2(identity,NULL),
	                    cain_sip_header_via_new(),
	                    70);

	is_register_ok=0;
	using_transaction=0;
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(cain_sip_header_expires_create(600)));
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(cain_sip_header_contact_new()));
	if (use_transaction){
		cain_sip_client_transaction_t *t=cain_sip_provider_get_new_client_transaction(prov,req);
		cain_sip_client_transaction_send_request(t);
	}else cain_sip_provider_send_request(prov,req);
	cain_sip_stack_sleep(stack,33000);
	CU_ASSERT_EQUAL(is_register_ok,1);
	CU_ASSERT_EQUAL(using_transaction,use_transaction);
	/*unregister*/
	is_register_ok=0;
	using_transaction=0;
	req=(cain_sip_request_t*)cain_sip_object_clone((cain_sip_object_t*)req);
	cain_sip_header_cseq_t* cseq=(cain_sip_header_cseq_t*)cain_sip_message_get_header((cain_sip_message_t*)req,CAIN_SIP_CSEQ);
	cain_sip_header_cseq_set_seq_number(cseq,cain_sip_header_cseq_get_seq_number(cseq)+1);
	cain_sip_header_expires_t* expires_header=(cain_sip_header_expires_t*)cain_sip_message_get_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_EXPIRES);
	cain_sip_header_expires_set_expires(expires_header,0);
	if (use_transaction){
		cain_sip_client_transaction_t *t=cain_sip_provider_get_new_client_transaction(prov,req);
		cain_sip_client_transaction_send_request(t);
	}else cain_sip_provider_send_request(prov,req);
	cain_sip_stack_sleep(stack,33000);
	CU_ASSERT_EQUAL(is_register_ok,1);
	CU_ASSERT_EQUAL(using_transaction,use_transaction);

	return;
}


static void stateless_register_udp(void){
	register_test(NULL,0);
}

static void stateless_register_tls(void){
	register_test("tls",0);
}

static void stateless_register_tcp(void){
	register_test("tcp",0);
}

static void stateful_register_udp(void){
	register_test(NULL,1);
}

static void stateful_register_udp_delayed(void){
	cain_sip_stack_set_tx_delay(stack,3000);
	register_test(NULL,1);
	cain_sip_stack_set_tx_delay(stack,0);
}

static void stateful_register_tcp(void){
	register_test("tcp",1);
}

static void stateful_register_tls(void){
	register_test("tls",1);
}

int cain_sip_register_test_suite(){
	CU_pSuite pSuite = CU_add_suite("Register test suite", init, uninit);

	if (NULL == CU_add_test(pSuite, "stateful udp register", stateful_register_udp)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "stateful udp register with network delay", stateful_register_udp_delayed)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "stateful tcp register", stateful_register_tcp)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "stateful tls register", stateful_register_tls)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "stateless udp register", stateless_register_udp)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "stateless tcp register", stateless_register_tcp)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "stateless tls register", stateless_register_tls)) {
			return CU_get_error();
	}
	return 0;
}

