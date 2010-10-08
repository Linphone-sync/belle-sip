//============================================================================
// Name        : parser-antlr.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "cain_sip_uri.h"
#include <stdio.h>
#include "CUnit/Basic.h"

int init_suite1(void) {
      return 0;
}

int clean_suite1(void) {
      return 0;
}


void testSIMPLEURI(void) {
	cain_sip_uri* L_uri = cain_sip_uri_parse("sip:titi.com");
	CU_ASSERT_PTR_NULL(cain_sip_uri_get_user(L_uri));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "titi.com");
	CU_ASSERT_PTR_NULL(cain_sip_uri_get_transport_param(L_uri));
	cain_sip_uri_delete(L_uri);
}

void testCOMPLEXURI(void) {
	cain_sip_uri* L_uri = cain_sip_uri_parse("sip:toto@titi.com:5060;transport=tcp");
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_user(L_uri), "toto");
	CU_ASSERT_EQUAL(cain_sip_uri_get_port(L_uri), 5060);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "titi.com");
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_transport_param(L_uri), "tcp");
	cain_sip_uri_delete(L_uri);
}
void testSIPSURI(void) {
	cain_sip_uri* L_uri = cain_sip_uri_parse("sips:linphone.org");
	CU_ASSERT_EQUAL(cain_sip_uri_is_secure(L_uri), 1);
	cain_sip_uri_delete(L_uri);
	L_uri = cain_sip_uri_parse("sip:linphone.org");
	CU_ASSERT_EQUAL(cain_sip_uri_is_secure(L_uri), 0);
	cain_sip_uri_delete(L_uri);
}
void test_ip_host(void) {
	cain_sip_uri* L_uri = cain_sip_uri_parse("sip:192.168.0.1");
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "192.168.0.1");
	cain_sip_uri_delete(L_uri);
}
void test_lr(void) {
	cain_sip_uri* L_uri = cain_sip_uri_parse("sip:192.168.0.1;lr");
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "192.168.0.1");
	CU_ASSERT_EQUAL(cain_sip_uri_has_lr_param(L_uri), 1);
	cain_sip_uri_delete(L_uri);

}
void test_maddr(void) {
	cain_sip_uri* L_uri = cain_sip_uri_parse("sip:192.168.0.1;lr;maddr=linphone.org");
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_maddr_param(L_uri), "linphone.org");
	cain_sip_uri_delete(L_uri);

}
void test_headers(void) {
	cain_sip_uri* L_uri = cain_sip_uri_parse("sip:192.168.0.1?toto=titi");
	CU_ASSERT_PTR_NOT_NULL_FATAL(cain_sip_uri_get_header(L_uri,"toto"));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_header(L_uri,"toto"), "titi");

	CU_ASSERT_PTR_NULL(cain_sip_uri_get_header(L_uri,"bla"));
	cain_sip_uri_delete(L_uri);
	L_uri = cain_sip_uri_parse("sip:192.168.0.1?toto=titi&header2=popo");

	CU_ASSERT_PTR_NOT_NULL_FATAL(cain_sip_uri_get_header(L_uri,"toto"));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_header(L_uri,"header2"), "popo");

}



int main (int argc, char *argv[]) {

	   CU_pSuite pSuite = NULL;

	   /* initialize the CUnit test registry */
	   if (CUE_SUCCESS != CU_initialize_registry())
	      return CU_get_error();

	   /* add a suite to the registry */
	   pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
	   if (NULL == pSuite) {
	      CU_cleanup_registry();
	      return CU_get_error();
	   }

	   /* add the tests to the suite */
	   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
	   if ((NULL == CU_add_test(pSuite, "test of simple uri", testSIMPLEURI))
	     ||  (NULL == CU_add_test(pSuite, "test of complex uri", testCOMPLEXURI))
		   || (NULL == CU_add_test(pSuite, "test of ip uri", test_ip_host))
		   || (NULL == CU_add_test(pSuite, "test of lr uri", test_lr))
		   || (NULL == CU_add_test(pSuite, "test of maddr uri", test_maddr))
		   || (NULL == CU_add_test(pSuite, "test of headers", test_headers))

	       || (NULL == CU_add_test(pSuite, "test of sips uri", testSIPSURI)))
	   {
	      CU_cleanup_registry();
	      return CU_get_error();
	   }

	   /* Run all tests using the CUnit Basic interface */
	   CU_basic_set_mode(CU_BRM_VERBOSE);
	   CU_basic_run_tests();
	   CU_cleanup_registry();
	   return CU_get_error();
}
