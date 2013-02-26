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
#include "cain_sip_internal.h"
#include "cain_sip_tester.h"

#include "register_tester.h"


const char *test_domain="test.linphone.org";
const char *auth_domain="sip.linphone.org";
static int is_register_ok;
static int number_of_challenge;
static int using_transaction;
cain_sip_stack_t * stack;
cain_sip_provider_t *prov;
static cain_sip_listener_t* l;
cain_sip_request_t* authorized_request;
cain_sip_listener_callbacks_t listener_callbacks;
cain_sip_listener_t *listener;


static void process_dialog_terminated(void *user_ctx, const cain_sip_dialog_terminated_event_t *event){
	CAINSIP_UNUSED(user_ctx);
	CAINSIP_UNUSED(event);
	cain_sip_message("process_dialog_terminated called");
}

static void process_io_error(void *user_ctx, const cain_sip_io_error_event_t *event){
	CAINSIP_UNUSED(user_ctx);
	CAINSIP_UNUSED(event);
	cain_sip_warning("process_io_error");
	cain_sip_main_loop_quit(cain_sip_stack_get_main_loop(stack));
	/*CU_ASSERT(CU_FALSE);*/
}

static void process_request_event(void *user_ctx, const cain_sip_request_event_t *event){
	CAINSIP_UNUSED(user_ctx);
	CAINSIP_UNUSED(event);
	cain_sip_message("process_request_event");
}

static void process_response_event(void *user_ctx, const cain_sip_response_event_t *event){
	int status;
	cain_sip_request_t* request;
	CAINSIP_UNUSED(user_ctx);
	CU_ASSERT_PTR_NOT_NULL_FATAL(cain_sip_response_event_get_response(event));
	cain_sip_message("process_response_event [%i] [%s]"
					,status=cain_sip_response_get_status_code(cain_sip_response_event_get_response(event))
					,cain_sip_response_get_reason_phrase(cain_sip_response_event_get_response(event)));
	if (status==401) {
		cain_sip_header_cseq_t* cseq;
		cain_sip_client_transaction_t *t;
		CU_ASSERT_NOT_EQUAL_FATAL(number_of_challenge,2);
		CU_ASSERT_PTR_NOT_NULL_FATAL(cain_sip_response_event_get_client_transaction(event)); /*require transaction mode*/
		request=cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(cain_sip_response_event_get_client_transaction(event)));
		cseq=(cain_sip_header_cseq_t*)cain_sip_message_get_header(CAIN_SIP_MESSAGE(request),CAIN_SIP_CSEQ);
		cain_sip_header_cseq_set_seq_number(cseq,cain_sip_header_cseq_get_seq_number(cseq)+1);
		CU_ASSERT_TRUE_FATAL(cain_sip_provider_add_authorization(prov,request,cain_sip_response_event_get_response(event),NULL));
		t=cain_sip_provider_get_new_client_transaction(prov,request);
		cain_sip_client_transaction_send_request(t);
		number_of_challenge++;
		authorized_request=request;
		cain_sip_object_ref(authorized_request);
	}  else {
		CU_ASSERT_EQUAL(status,200);
		is_register_ok=1;
		using_transaction=cain_sip_response_event_get_client_transaction(event)!=NULL;
		cain_sip_main_loop_quit(cain_sip_stack_get_main_loop(stack));
	}
}

static void process_timeout(void *user_ctx, const cain_sip_timeout_event_t *event){
	CAINSIP_UNUSED(user_ctx);
	CAINSIP_UNUSED(event);
	cain_sip_message("process_timeout");
}

static void process_transaction_terminated(void *user_ctx, const cain_sip_transaction_terminated_event_t *event){
	CAINSIP_UNUSED(user_ctx);
	CAINSIP_UNUSED(event);
	cain_sip_message("process_transaction_terminated");
}

static void process_auth_requested(void *user_ctx, cain_sip_auth_event_t *event){
	CAINSIP_UNUSED(user_ctx);
	cain_sip_message("process_auth_requested requested for [%s@%s]"
			,cain_sip_auth_event_get_username(event)
			,cain_sip_auth_event_get_realm(event));
	cain_sip_auth_event_set_passwd(event,"secret");
}

int register_init(void) {
	cain_sip_listening_point_t *lp;
	stack=cain_sip_stack_new(NULL);
	lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",7060,"UDP");
	prov=cain_sip_stack_create_provider(stack,lp);
	
	lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",7060,"TCP");
	cain_sip_provider_add_listening_point(prov,lp);
	lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",7061,"TLS");
	if (lp) {
		cain_sip_provider_add_listening_point(prov,lp);
	}

	listener_callbacks.process_dialog_terminated=process_dialog_terminated;
	listener_callbacks.process_io_error=process_io_error;
	listener_callbacks.process_request_event=process_request_event;
	listener_callbacks.process_response_event=process_response_event;
	listener_callbacks.process_timeout=process_timeout;
	listener_callbacks.process_transaction_terminated=process_transaction_terminated;
	listener_callbacks.process_auth_requested=process_auth_requested;
	listener_callbacks.listener_destroyed=NULL;
	listener=cain_sip_listener_create_from_callbacks(&listener_callbacks,NULL);
	return 0;
}

int register_uninit(void) {
	cain_sip_object_unref(prov);
	cain_sip_object_unref(stack);
	cain_sip_object_unref(listener);
	return 0;
}

void unregister_user(cain_sip_stack_t * stack
					,cain_sip_provider_t *prov
					,cain_sip_request_t* initial_request
					,int use_transaction) {
	cain_sip_request_t *req;
	cain_sip_header_cseq_t* cseq;
	cain_sip_header_expires_t* expires_header;
	int i;
	cain_sip_provider_add_sip_listener(prov,l);
	is_register_ok=0;
	using_transaction=0;
	req=(cain_sip_request_t*)cain_sip_object_clone((cain_sip_object_t*)initial_request);
	cseq=(cain_sip_header_cseq_t*)cain_sip_message_get_header((cain_sip_message_t*)req,CAIN_SIP_CSEQ);
	cain_sip_header_cseq_set_seq_number(cseq,cain_sip_header_cseq_get_seq_number(cseq)+2); /*+2 if initial reg was challenged*/
	expires_header=(cain_sip_header_expires_t*)cain_sip_message_get_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_EXPIRES);
	cain_sip_header_expires_set_expires(expires_header,0);
	if (use_transaction){
		cain_sip_client_transaction_t *t;
		cain_sip_message_remove_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_AUTHORIZATION);
		cain_sip_message_remove_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_PROXY_AUTHORIZATION);
		cain_sip_provider_add_authorization(prov,req,NULL,NULL); /*just in case*/
		t=cain_sip_provider_get_new_client_transaction(prov,req);
		cain_sip_client_transaction_send_request(t);
	}else cain_sip_provider_send_request(prov,req);
	for(i=0;!is_register_ok && i<2 ;i++)
		cain_sip_stack_sleep(stack,5000);
	CU_ASSERT_EQUAL(is_register_ok,1);
	CU_ASSERT_EQUAL(using_transaction,use_transaction);
	cain_sip_provider_remove_sip_listener(prov,l);
}

cain_sip_request_t* try_register_user_at_domain(cain_sip_stack_t * stack
					,cain_sip_provider_t *prov
					,const char *transport
					,int use_transaction
					,const char* username
					,const char* domain
					,const char* outbound_proxy
					,int success_expected) {
	cain_sip_request_t *req,*copy;
	char identity[256];
	char uri[256];
	int i;
	char *outbound=NULL;
	
	number_of_challenge=0;
	if (transport)
		snprintf(uri,sizeof(uri),"sip:%s;transport=%s",domain,transport);
	else snprintf(uri,sizeof(uri),"sip:%s",domain);

	if (transport && strcasecmp("tls",transport)==0 && cain_sip_provider_get_listening_point(prov,"tls")==NULL){
		cain_sip_error("No TLS support, test skipped.");
		return NULL;
	}
	
	if (outbound_proxy){
		if (strstr(outbound_proxy,"sip:")==NULL){
			outbound=cain_sip_strdup_printf("sip:%s",outbound_proxy);
		}else outbound=cain_sip_strdup(outbound_proxy);
	}

	snprintf(identity,sizeof(identity),"Tester <sip:%s@%s>",username,domain);
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
	copy=(cain_sip_request_t*)cain_sip_object_ref(cain_sip_object_clone((cain_sip_object_t*)req));
	cain_sip_provider_add_sip_listener(prov,l=CAIN_SIP_LISTENER(listener));
	if (use_transaction){
		cain_sip_client_transaction_t *t=cain_sip_provider_get_new_client_transaction(prov,req);
		cain_sip_client_transaction_send_request_to(t,outbound?cain_sip_uri_parse(outbound):NULL);
	}else cain_sip_provider_send_request(prov,req);
	for(i=0;!is_register_ok && i<2 ;i++)
		cain_sip_stack_sleep(stack,5000);
	CU_ASSERT_EQUAL(is_register_ok,success_expected);
	if (success_expected) CU_ASSERT_EQUAL(using_transaction,use_transaction);

	cain_sip_provider_remove_sip_listener(prov,l);
	if (outbound) cain_sip_free(outbound);
	return copy;
}

cain_sip_request_t* register_user_at_domain(cain_sip_stack_t * stack
					,cain_sip_provider_t *prov
					,const char *transport
					,int use_transaction
					,const char* username
					,const char* domain
					,const char* outband) {
	return try_register_user_at_domain(stack,prov,transport,use_transaction,username,domain,outband,1);

}

cain_sip_request_t* register_user(cain_sip_stack_t * stack
					,cain_sip_provider_t *prov
					,const char *transport
					,int use_transaction
					,const char* username
					,const char* outbound) {
	return register_user_at_domain(stack,prov,transport,use_transaction,username,test_domain,outbound);
}

static void register_with_outbound(const char *transport, int use_transaction,const char* outbound ) {
	cain_sip_request_t *req;
	
	req=register_user(stack, prov, transport,use_transaction,"tester",outbound);
	if (req) {
		unregister_user(stack,prov,req,use_transaction);
		cain_sip_object_unref(req);
	}
}

static void register_test(const char *transport, int use_transaction) {
	register_with_outbound(transport,use_transaction,NULL);
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

static void stateful_register_udp_with_keep_alive(void) {
	cain_sip_listening_point_set_keep_alive(cain_sip_provider_get_listening_point(prov,"udp"),200);
	register_test(NULL,1);
	cain_sip_main_loop_sleep(cain_sip_stack_get_main_loop(stack),500);
	cain_sip_listening_point_set_keep_alive(cain_sip_provider_get_listening_point(prov,"udp"),-1);
}

static void stateful_register_udp_with_outbound_proxy(void){
	register_with_outbound("udp",1,test_domain);
}

static void stateful_register_udp_delayed(void){
	cain_sip_stack_set_tx_delay(stack,3000);
	register_test(NULL,1);
	cain_sip_stack_set_tx_delay(stack,0);
}

static void stateful_register_udp_with_send_error(void){
	cain_sip_stack_set_send_error(stack,-1);
	try_register_user_at_domain(stack, prov, NULL,1,"tester",test_domain,NULL,0);
	cain_sip_stack_set_send_error(stack,0);
}

static void stateful_register_tcp(void){
	register_test("tcp",1);
}

static void stateful_register_tls(void){
	register_test("tls",1);
}

static void bad_req_process_io_error(void *user_ctx, const cain_sip_io_error_event_t *event){
	CAINSIP_UNUSED(user_ctx);
	CAINSIP_UNUSED(event);
	cain_sip_message("bad_req_process_io_error not implemented yet");
}

static void bad_req_process_response_event(void *user_ctx, const cain_sip_response_event_t *event){
	CAINSIP_UNUSED(user_ctx);
	CAINSIP_UNUSED(event);
	cain_sip_message("bad_req_process_response_event not implemented yet");
}

static void test_bad_request(void) {
	cain_sip_request_t *req;
	cain_sip_listener_t *bad_req_listener;
	cain_sip_client_transaction_t *t;
	cain_sip_header_address_t* route_address=cain_sip_header_address_create(NULL,cain_sip_uri_create(NULL,test_domain));
	cain_sip_header_route_t* route;
	cain_sip_header_to_t* to = cain_sip_header_to_create2("sip:toto@titi.com",NULL);
	cain_sip_listener_callbacks_t cbs;
	memset(&cbs,0,sizeof(cbs));

	bad_req_listener = cain_sip_listener_create_from_callbacks(&cbs,NULL);
	cbs.process_io_error=bad_req_process_io_error;
	cbs.process_response_event=bad_req_process_response_event;

	req=cain_sip_request_create(
	                    CAIN_SIP_URI(cain_sip_object_clone(CAIN_SIP_OBJECT(cain_sip_header_address_get_uri(route_address)))),
	                    "REGISTER",
	                    cain_sip_provider_get_new_call_id(prov),
	                    cain_sip_header_cseq_create(20,"REGISTER"),
	                    cain_sip_header_from_create2("sip:toto@titi.com",CAIN_SIP_RANDOM_TAG),
	                    to,
	                    cain_sip_header_via_new(),
	                    70);

	cain_sip_uri_set_transport_param(cain_sip_header_address_get_uri(route_address),"tcp");
	route = cain_sip_header_route_create(route_address);
	cain_sip_header_set_name(CAIN_SIP_HEADER(to),"BrokenHeader");

	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(cain_sip_header_expires_create(600)));
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(route));
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(cain_sip_header_contact_new()));
	cain_sip_provider_add_sip_listener(prov,bad_req_listener);
	t=cain_sip_provider_get_new_client_transaction(prov,req);
	cain_sip_client_transaction_send_request(t);
	cain_sip_stack_sleep(stack,100);
	cain_sip_provider_remove_sip_listener(prov,bad_req_listener);
	cain_sip_object_unref(bad_req_listener);
}

static void test_register_authenticate(void) {
	cain_sip_request_t *reg;
	number_of_challenge=0;
	authorized_request=NULL;
	reg=register_user_at_domain(stack, prov, "udp",1,"cainsip",auth_domain,NULL);
	if (authorized_request) {
		unregister_user(stack,prov,authorized_request,1);
		cain_sip_object_unref(authorized_request);
	}
	cain_sip_object_unref(reg);
}


test_t register_tests[] = {
	{ "Stateful UDP", stateful_register_udp },
	{ "Stateful UDP with keep-alive", stateful_register_udp_with_keep_alive },
	{ "Stateful UDP with network delay", stateful_register_udp_delayed },
	{ "Stateful UDP with send error", stateful_register_udp_with_send_error },
	{ "Stateful UDP with outbound proxy", stateful_register_udp_with_outbound_proxy },
	{ "Stateful TCP", stateful_register_tcp },
	{ "Stateful TLS", stateful_register_tls },
	{ "Stateless UDP", stateless_register_udp },
	{ "Stateless TCP", stateless_register_tcp },
	{ "Stateless TLS", stateless_register_tls },
	{ "Bad TCP request", test_bad_request },
	{ "Authenticate", test_register_authenticate }
};

test_suite_t register_test_suite = {
	"REGISTER",
	register_init,
	register_uninit,
	sizeof(register_tests) / sizeof(register_tests[0]),
	register_tests
};

