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

#include "CUnit/Basic.h"

#include "cain-sip/cain-sip.h"

static int init_cast_suite(void){
	return 0;
}

static int cleanup_cast_suite(void){
	return 0;
}

static void cast_test(void){
	cain_sip_stack_t *stack=cain_sip_stack_new(NULL);
	cain_sip_listening_point_t *lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",7060,"UDP");
	cain_sip_provider_t *provider;
	cain_sip_request_t *req=cain_sip_request_new();
	cain_sip_response_t *resp=cain_sip_response_new();
	cain_sip_message_t *msg;
	int tmp;
	
	CU_ASSERT_PTR_NOT_NULL_FATAL(stack);
	CU_ASSERT_PTR_NOT_NULL_FATAL(lp);
	provider=cain_sip_stack_create_provider(stack,lp);
	CU_ASSERT_PTR_NOT_NULL_FATAL(provider);
	CU_ASSERT_PTR_NOT_NULL_FATAL(req);
	CU_ASSERT_PTR_NOT_NULL_FATAL(resp);
	
	cain_sip_message("Casting cain_sip_request_t to cain_sip_message_t");
	msg=CAIN_SIP_MESSAGE(req);
	CU_ASSERT_PTR_NOT_NULL(msg);
	cain_sip_message("Ok.");
	cain_sip_message("Casting cain_sip_response_t to cain_sip_message_t");
	msg=CAIN_SIP_MESSAGE(resp);
	CU_ASSERT_PTR_NOT_NULL(msg);
	cain_sip_message("Ok.");
	tmp=CAIN_SIP_IS_INSTANCE_OF(req,cain_sip_response_t);
	cain_sip_message("Casting cain_sip_request_t to cain_sip_response_t: %s",tmp ? "yes" : "no");
	CU_ASSERT_EQUAL(tmp,0);
	cain_sip_object_unref(req);
	cain_sip_object_unref(resp);
	cain_sip_object_unref(provider);
	cain_sip_object_unref(stack);
}


int cain_sip_cast_test_suite(){
	CU_pSuite pSuite = CU_add_suite("Object inheritence", init_cast_suite, cleanup_cast_suite);

	if (NULL == CU_add_test(pSuite, "casting requests and responses", cast_test)) {
		return CU_get_error();
	}
	return 0;
}

