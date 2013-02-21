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
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#define USERNAME "toto"
#define SIPDOMAIN "sip.linphone.org"
#define PASSWD "secret"

typedef enum auth_mode {
	none
	,digest
	,digest_auth
}auth_mode_t;

typedef struct _stat {
	int twoHundredOk;
	int fourHundredOne;
	int refreshOk;
}stat_t;
typedef struct endpoint {
	cain_sip_stack_t* stack;
	cain_sip_listener_callbacks_t* listener_callbacks;
	cain_sip_provider_t* provider;
	cain_sip_listening_point_t *lp;
	cain_sip_listener_t *listener;
	auth_mode_t auth;
	stat_t stat;
	unsigned char expire_in_contact;
	char nonce[32];
	unsigned int nonce_count;
	const char* received;
	int rport;
	unsigned char unreconizable_contact;
} endpoint_t;

static unsigned int  wait_for(cain_sip_stack_t*s1, cain_sip_stack_t*s2,int* counter,int value,int timeout) {
	int retry=0;
#define ITER 100
	while (*counter<value && retry++ <(timeout/ITER)) {
		if (s1) cain_sip_stack_sleep(s1,ITER/2);
		if (s2) cain_sip_stack_sleep(s2,ITER/2);
	}
	if(*counter<value) return FALSE;
	else return TRUE;
}

//static void process_dialog_terminated(void *obj, const cain_sip_dialog_terminated_event_t *event){
//	cain_sip_message("process_dialog_terminated called");
//}
//static void process_io_error(void *obj, const cain_sip_io_error_event_t *event){
//	cain_sip_warning("process_io_error");
//	/*cain_sip_main_loop_quit(cain_sip_stack_get_main_loop(stack));*/
//	/*CU_ASSERT(CU_FALSE);*/
//}
static void compute_response(const char* username
									,const char* realm
									,const char* passwd
									,const char* nonce
									,const char* method
									,const char* uri
									,char response[33] ) {
	char ha1[33],ha2[33];
	cain_sip_auth_helper_compute_ha1(username,realm,passwd,ha1);
	cain_sip_auth_helper_compute_ha2(method,uri,ha2);
	cain_sip_auth_helper_compute_response(ha1,nonce,ha2,response);
}
static void compute_response_auth_qop(const char* username
										,const char* realm
										,const char* passwd
										,const char* nonce
										,unsigned int nonce_count
										,const char* cnonce
										,const char* qop
										,const char* method
										,const char* uri
										,char response[33] ) {
	char ha1[33],ha2[33];
	cain_sip_auth_helper_compute_ha1(username,realm,passwd,ha1);
	cain_sip_auth_helper_compute_ha2(method,uri,ha2);
	cain_sip_auth_helper_compute_response_qop_auth(ha1, nonce,nonce_count, cnonce,qop,ha2,response);
}

static void server_process_request_event(void *obj, const cain_sip_request_event_t *event){
	endpoint_t* endpoint = (endpoint_t*)obj;
	cain_sip_server_transaction_t* server_transaction =cain_sip_provider_get_new_server_transaction(endpoint->provider,cain_sip_request_event_get_request(event));
	cain_sip_request_t* req = cain_sip_transaction_get_request(CAIN_SIP_TRANSACTION(server_transaction));
	cain_sip_response_t* resp;
	cain_sip_header_contact_t* contact;
	cain_sip_header_expires_t* expires;
	cain_sip_header_authorization_t* authorization;
	cain_sip_header_via_t* via;
	const char* raw_authenticate_digest = "WWW-Authenticate: Digest "
			"algorithm=MD5, realm=\"" SIPDOMAIN "\", opaque=\"1bc7f9097684320\"";

	cain_sip_header_www_authenticate_t* www_authenticate=NULL;
	const char* auth_uri;
	const char* qop;
	unsigned char auth_ok=0;
	char local_resp[33];

	cain_sip_message("caller_process_request_event received [%s] message",cain_sip_request_get_method(cain_sip_request_event_get_request(event)));

	switch (endpoint->auth) {
	case none: {
		auth_ok=1;
		break;
	}
	case digest_auth:
	case digest: {
		if ((authorization=cain_sip_message_get_header_by_type(req,cain_sip_header_authorization_t)) != NULL){
			qop=cain_sip_header_authorization_get_qop(authorization);

			if (qop && strcmp(qop,"auth")==0) {
				compute_response_auth_qop(	cain_sip_header_authorization_get_username(authorization)
											,cain_sip_header_authorization_get_realm(authorization)
											,PASSWD
											,endpoint->nonce
											,endpoint->nonce_count
											,cain_sip_header_authorization_get_cnonce(authorization)
											,cain_sip_header_authorization_get_qop(authorization)
											,cain_sip_request_get_method(req)
											,auth_uri=cain_sip_uri_to_string(cain_sip_header_authorization_get_uri(authorization))
											,local_resp);
			} else {
				/*digest*/
				compute_response(cain_sip_header_authorization_get_username(authorization)
						,cain_sip_header_authorization_get_realm(authorization)
						,PASSWD
						,endpoint->nonce
						,cain_sip_request_get_method(req)
						,auth_uri=cain_sip_uri_to_string(cain_sip_header_authorization_get_uri(authorization))
						,local_resp);

			}
			cain_sip_free((void*)auth_uri);
			auth_ok=strcmp(cain_sip_header_authorization_get_response(authorization),local_resp)==0;
		}
		if (auth_ok) {
			if (endpoint->auth == digest) {
			sprintf(endpoint->nonce,"%p",authorization); //*change the nonce for next auth*/
			} else {
				endpoint->nonce_count++;
			}
		} else {
			www_authenticate=cain_sip_header_www_authenticate_parse(raw_authenticate_digest);
			cain_sip_header_www_authenticate_set_nonce(www_authenticate,endpoint->nonce);
			if (endpoint->auth == digest_auth) {
				cain_sip_header_www_authenticate_add_qop(www_authenticate,"auth");
			}
		}
	}
	break;
	default:
		break;
	}
	if (auth_ok) {
		resp=cain_sip_response_create_from_request(cain_sip_request_event_get_request(event),200);
		if (!endpoint->expire_in_contact) {
			cain_sip_message_add_header(CAIN_SIP_MESSAGE(resp),CAIN_SIP_HEADER(expires=cain_sip_message_get_header_by_type(req,cain_sip_header_expires_t)));
		}
		if (strcmp(cain_sip_request_get_method(req),"REGISTER")==0) {
			contact=cain_sip_message_get_header_by_type(req,cain_sip_header_contact_t);
		} else {
			contact=cain_sip_header_contact_new();
		}
		if(endpoint->unreconizable_contact) {
			/*put an unexopected address*/
			cain_sip_uri_set_host(cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(contact)),"nimportequoi.com");
		}
		cain_sip_message_add_header(CAIN_SIP_MESSAGE(resp),CAIN_SIP_HEADER(contact));

	} else {
		resp=cain_sip_response_create_from_request(cain_sip_request_event_get_request(event),401);
		if (www_authenticate)
			cain_sip_message_add_header(CAIN_SIP_MESSAGE(resp),CAIN_SIP_HEADER(www_authenticate));
	}
	if (endpoint->received) {
		via=cain_sip_message_get_header_by_type(req,cain_sip_header_via_t);
		cain_sip_header_via_set_received(via,endpoint->received);
	}
	cain_sip_server_transaction_send_response(server_transaction,resp);
}
static void client_process_response_event(void *obj, const cain_sip_response_event_t *event){
	//cain_sip_client_transaction_t* client_transaction = cain_sip_response_event_get_client_transaction(event);
	endpoint_t* endpoint = (endpoint_t*)obj;
	int status = cain_sip_response_get_status_code(cain_sip_response_event_get_response(event));
	cain_sip_message("caller_process_response_event [%i]",status);
	switch (status) {
	case 200:endpoint->stat.twoHundredOk++; break;
	case 401:endpoint->stat.fourHundredOne++; break;
	default: break;
	}


}
//static void process_timeout(void *obj, const cain_sip_timeout_event_t *event){
//	cain_sip_message("process_timeout");
//}
//static void process_transaction_terminated(void *obj, const cain_sip_transaction_terminated_event_t *event){
//	cain_sip_message("process_transaction_terminated");
//}
static void client_process_auth_requested(void *obj, cain_sip_auth_event_t *event){
	CAINSIP_UNUSED(obj);
	cain_sip_message("process_auth_requested requested for [%s@%s]"
			,cain_sip_auth_event_get_username(event)
			,cain_sip_auth_event_get_realm(event));
	cain_sip_auth_event_set_passwd(event,PASSWD);
}

static void cain_sip_refresher_listener ( const cain_sip_refresher_t* refresher
		,void* user_pointer
		,unsigned int status_code
		,const char* reason_phrase) {
	endpoint_t* endpoint = (endpoint_t*)user_pointer;
	CAINSIP_UNUSED(refresher);
	cain_sip_message("cain_sip_refresher_listener [%i] reason [%s]",status_code,reason_phrase);
	switch (status_code) {
		case 200:endpoint->stat.refreshOk++; break;
		default: break;
	}
}

static endpoint_t* create_endpoint(int port,const char* transport,cain_sip_listener_callbacks_t* listener_callbacks) {
	endpoint_t* endpoint = cain_sip_new0(endpoint_t);
	endpoint->stack=cain_sip_stack_new(NULL);
	endpoint->listener_callbacks=listener_callbacks;
	endpoint->lp=cain_sip_stack_create_listening_point(endpoint->stack,"0.0.0.0",port,transport);
	CU_ASSERT_PTR_NOT_NULL_FATAL(endpoint->lp);
	cain_sip_object_ref(endpoint->lp);
	endpoint->provider=cain_sip_stack_create_provider(endpoint->stack,endpoint->lp);
	cain_sip_provider_add_sip_listener(endpoint->provider,(endpoint->listener=cain_sip_listener_create_from_callbacks(endpoint->listener_callbacks,endpoint)));
	sprintf(endpoint->nonce,"%p",endpoint); /*initial nonce*/
	endpoint->nonce_count=1;
	return endpoint;
}

static void destroy_endpoint(endpoint_t* endpoint) {
	cain_sip_object_unref(endpoint->lp);
	cain_sip_object_unref(endpoint->provider);
	cain_sip_object_unref(endpoint->stack);
	cain_sip_object_unref(endpoint->listener);
	cain_sip_free(endpoint);
}

static endpoint_t* create_udp_endpoint(int port,cain_sip_listener_callbacks_t* listener_callbacks) {
	return create_endpoint(port,"udp",listener_callbacks);
}
static void register_base(endpoint_t* client,endpoint_t *server) {
	cain_sip_request_t* req;
	cain_sip_client_transaction_t* trans;
	cain_sip_header_route_t* destination_route;
	cain_sip_refresher_t* refresher;
	const char* identity = "sip:" USERNAME "@" SIPDOMAIN ;
	const char* domain="sip:" SIPDOMAIN ;
	cain_sip_header_contact_t* contact=cain_sip_header_contact_new();
	cain_sip_uri_t *dest_uri;
	uint64_t begin;
	uint64_t end;
	if (client->expire_in_contact) cain_sip_header_contact_set_expires(contact,1);

	
	dest_uri=(cain_sip_uri_t*)cain_sip_object_clone((cain_sip_object_t*)cain_sip_listening_point_get_uri(server->lp));
	cain_sip_uri_set_host(dest_uri,"127.0.0.1");
	destination_route=cain_sip_header_route_create(cain_sip_header_address_create(NULL,dest_uri));


	req=cain_sip_request_create(
		                    cain_sip_uri_parse(domain),
		                    "REGISTER",
		                    cain_sip_provider_get_new_call_id(client->provider),
		                    cain_sip_header_cseq_create(20,"REGISTER"),
		                    cain_sip_header_from_create2(identity,CAIN_SIP_RANDOM_TAG),
		                    cain_sip_header_to_create2(identity,NULL),
		                    cain_sip_header_via_new(),
		                    70);
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(contact));
	if (!client->expire_in_contact)
		cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(cain_sip_header_expires_create(1)));

	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(destination_route));
	trans=cain_sip_provider_get_new_client_transaction(client->provider,req);
	cain_sip_object_ref(trans);/*to avoid trans from being deleted before refresher can use it*/
	cain_sip_client_transaction_send_request(trans);

	if (server->auth == none) {
		CU_ASSERT_TRUE(wait_for(server->stack,client->stack,&client->stat.twoHundredOk,1,1000));
	} else {
		CU_ASSERT_TRUE(wait_for(server->stack,client->stack,&client->stat.fourHundredOne,1,1000));
		/*update cseq*/
		req=cain_sip_client_transaction_create_authenticated_request(trans);
		cain_sip_object_unref(trans);
		trans=cain_sip_provider_get_new_client_transaction(client->provider,req);
		cain_sip_object_ref(trans);
		cain_sip_client_transaction_send_request(trans);
		CU_ASSERT_TRUE_FATAL(wait_for(server->stack,client->stack,&client->stat.twoHundredOk,1,1000));
	}
	refresher = cain_sip_client_transaction_create_refresher(trans);
	CU_ASSERT_TRUE_FATAL(refresher!=NULL);
	cain_sip_object_unref(trans);
	cain_sip_refresher_set_listener(refresher,cain_sip_refresher_listener,client);

	begin = cain_sip_time_ms();
	CU_ASSERT_TRUE(wait_for(server->stack,client->stack,&client->stat.refreshOk,3,4000));
	end = cain_sip_time_ms();
	CU_ASSERT_TRUE(end-begin>=3000);
	CU_ASSERT_TRUE(end-begin<5000);
	/*unregister*/
	cain_sip_refresher_refresh(refresher,0);
	CU_ASSERT_TRUE(wait_for(server->stack,client->stack,&client->stat.refreshOk,4,1000));
	cain_sip_refresher_stop(refresher);
	cain_sip_object_unref(refresher);
}
static void register_test_with_param(unsigned char expire_in_contact,auth_mode_t auth_mode) {
	cain_sip_listener_callbacks_t client_callbacks;
	cain_sip_listener_callbacks_t server_callbacks;
	endpoint_t* client,*server;
	memset(&client_callbacks,0,sizeof(cain_sip_listener_callbacks_t));
	memset(&server_callbacks,0,sizeof(cain_sip_listener_callbacks_t));
	client_callbacks.process_response_event=client_process_response_event;
	client_callbacks.process_auth_requested=client_process_auth_requested;
	server_callbacks.process_request_event=server_process_request_event;
	client = create_udp_endpoint(3452,&client_callbacks);
	server = create_udp_endpoint(6788,&server_callbacks);
	server->expire_in_contact=client->expire_in_contact=expire_in_contact;
	server->auth=auth_mode;
	register_base(client,server);
	destroy_endpoint(client);
	destroy_endpoint(server);
}

static void subscribe_test(void) {
	cain_sip_listener_callbacks_t client_callbacks;
	cain_sip_listener_callbacks_t server_callbacks;
	cain_sip_request_t* req;
	cain_sip_client_transaction_t* trans;
	cain_sip_header_route_t* destination_route;
	const char* identity = "sip:" USERNAME "@" SIPDOMAIN ;
	const char* domain="sip:" SIPDOMAIN ;
	endpoint_t* client,*server;
	cain_sip_uri_t *dest_uri;
	cain_sip_refresher_t* refresher;
	cain_sip_header_contact_t* contact=cain_sip_header_contact_new();
	uint64_t begin;
	uint64_t end;
	memset(&client_callbacks,0,sizeof(cain_sip_listener_callbacks_t));
	memset(&server_callbacks,0,sizeof(cain_sip_listener_callbacks_t));

	client_callbacks.process_response_event=client_process_response_event;
	client_callbacks.process_auth_requested=client_process_auth_requested;
	server_callbacks.process_request_event=server_process_request_event;

	client = create_udp_endpoint(3452,&client_callbacks);
	server = create_udp_endpoint(6788,&server_callbacks);
	server->expire_in_contact=0;
	server->auth=digest_auth;
	
	dest_uri=(cain_sip_uri_t*)cain_sip_object_clone((cain_sip_object_t*)cain_sip_listening_point_get_uri(server->lp));
	cain_sip_uri_set_host(dest_uri,"127.0.0.1");
	destination_route=cain_sip_header_route_create(cain_sip_header_address_create(NULL,dest_uri));


	req=cain_sip_request_create(
		                    cain_sip_uri_parse(domain),
		                    "SUBSCRIBE",
		                    cain_sip_provider_get_new_call_id(client->provider),
		                    cain_sip_header_cseq_create(20,"SUBSCRIBE"),
		                    cain_sip_header_from_create2(identity,CAIN_SIP_RANDOM_TAG),
		                    cain_sip_header_to_create2(identity,NULL),
		                    cain_sip_header_via_new(),
		                    70);
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(contact));
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(cain_sip_header_expires_create(1)));
	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(cain_sip_header_create("Event","Presence")));

	cain_sip_message_add_header(CAIN_SIP_MESSAGE(req),CAIN_SIP_HEADER(destination_route));
	trans=cain_sip_provider_get_new_client_transaction(client->provider,req);
	cain_sip_object_ref(trans);/*to avoid trans from being deleted before refresher can use it*/
	cain_sip_client_transaction_send_request(trans);

	CU_ASSERT_TRUE(wait_for(server->stack,client->stack,&client->stat.fourHundredOne,1,1000));

	req=cain_sip_client_transaction_create_authenticated_request(trans);
	cain_sip_object_unref(trans);
	trans=cain_sip_provider_get_new_client_transaction(client->provider,req);
	cain_sip_object_ref(trans);
	cain_sip_client_transaction_send_request(trans);
	CU_ASSERT_TRUE_FATAL(wait_for(server->stack,client->stack,&client->stat.twoHundredOk,1,1000));
	 /*maybe dialog should be automatically created*/
	CU_ASSERT_PTR_NOT_NULL_FATAL(cain_sip_transaction_get_dialog(CAIN_SIP_TRANSACTION(trans)))

	refresher = cain_sip_client_transaction_create_refresher(trans);
	cain_sip_object_unref(trans);
	cain_sip_refresher_set_listener(refresher,cain_sip_refresher_listener,client);

	begin = cain_sip_time_ms();
	CU_ASSERT_TRUE(wait_for(server->stack,client->stack,&client->stat.refreshOk,3,4000));
	end = cain_sip_time_ms();
	CU_ASSERT_TRUE(end-begin>=3000);
	CU_ASSERT_TRUE(end-begin<5000);
	cain_sip_refresher_stop(refresher);
	cain_sip_object_unref(refresher);
	destroy_endpoint(client);
	destroy_endpoint(server);
}

static void register_expires_header(void) {
	register_test_with_param(0,none);
}
static void register_expires_in_contact(void) {
	register_test_with_param(1,none);
}
static void register_expires_header_digest(void) {
	register_test_with_param(0,digest);
}

static void register_expires_in_contact_header_digest_auth(void) {
	register_test_with_param(1,digest_auth);
}

static void register_with_unreconizable_contact(void) {
	cain_sip_listener_callbacks_t client_callbacks;
	cain_sip_listener_callbacks_t server_callbacks;
	endpoint_t* client,*server;
	memset(&client_callbacks,0,sizeof(cain_sip_listener_callbacks_t));
	memset(&server_callbacks,0,sizeof(cain_sip_listener_callbacks_t));
	client_callbacks.process_response_event=client_process_response_event;
	client_callbacks.process_auth_requested=client_process_auth_requested;
	server_callbacks.process_request_event=server_process_request_event;
	client = create_udp_endpoint(3452,&client_callbacks);
	server = create_udp_endpoint(6788,&server_callbacks);
	server->expire_in_contact=1;
	server->unreconizable_contact=1;
	server->auth=digest;
	register_base(client,server);
	destroy_endpoint(client);
	destroy_endpoint(server);
}


int cain_sip_refresher_test_suite(){
	CU_pSuite pSuite = CU_add_suite("Refresher", NULL, NULL);

	if (NULL == CU_add_test(pSuite, "register_expires_header", register_expires_header)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "register_expires_in_contact", register_expires_in_contact)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "register_expires_header_digest", register_expires_header_digest)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "register_expires_in_contact_header_digest_auth", register_expires_in_contact_header_digest_auth)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "subscribe_test", subscribe_test)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "register_with_unreconizable_contact", register_with_unreconizable_contact)) {
		return CU_get_error();
	}
	return 0;
}

