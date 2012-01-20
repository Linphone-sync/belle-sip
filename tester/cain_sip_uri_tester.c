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

#include "cain-sip/cain-sip.h"
#include <stdio.h>
#include "CUnit/Basic.h"

static int init_suite_uri(void) {
      return 0;
}

static int clean_suite_uri(void) {
      return 0;
}


static void testSIMPLEURI(void) {
	cain_sip_uri_t* L_tmp;
	cain_sip_uri_t* L_uri = cain_sip_uri_parse("sip:titi.com");
	char* l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_tmp = cain_sip_uri_parse(l_raw_uri);
	L_uri = CAIN_SIP_URI(cain_sip_object_clone(CAIN_SIP_OBJECT(L_tmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_tmp));
	cain_sip_free(l_raw_uri);

	CU_ASSERT_PTR_NULL(cain_sip_uri_get_user(L_uri));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "titi.com");
	CU_ASSERT_PTR_NULL(cain_sip_uri_get_transport_param(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
}

static void testCOMPLEXURI(void) {
	cain_sip_uri_t* L_tmp;
	cain_sip_uri_t *  L_uri = cain_sip_uri_parse("sip:toto@titi.com:5060;transport=tcp");
	char* l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_tmp = cain_sip_uri_parse(l_raw_uri);
	L_uri = CAIN_SIP_URI(cain_sip_object_clone(CAIN_SIP_OBJECT(L_tmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_tmp));
	cain_sip_free(l_raw_uri);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_user(L_uri), "toto");
	CU_ASSERT_EQUAL(cain_sip_uri_get_port(L_uri), 5060);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "titi.com");
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_transport_param(L_uri), "tcp");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
}
static void testSIPSURI(void) {

	cain_sip_uri_t *  L_uri = cain_sip_uri_parse("sips:linphone.org");
	char* l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_uri = cain_sip_uri_parse(l_raw_uri);
	cain_sip_free(l_raw_uri);
	CU_ASSERT_EQUAL(cain_sip_uri_is_secure(L_uri), 1);
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_uri = cain_sip_uri_parse("sip:linphone.org");
	CU_ASSERT_EQUAL(cain_sip_uri_is_secure(L_uri), 0);
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
}
static void test_ip_host(void) {
	cain_sip_uri_t *  L_uri = cain_sip_uri_parse("sip:192.168.0.1");
	char* l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_uri = cain_sip_uri_parse(l_raw_uri);
	cain_sip_free(l_raw_uri);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "192.168.0.1");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
}
static void test_lr(void) {
	cain_sip_uri_t *  L_uri = cain_sip_uri_parse("sip:192.168.0.1;lr");
	char* l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_uri = cain_sip_uri_parse(l_raw_uri);
	cain_sip_free(l_raw_uri);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "192.168.0.1");
	CU_ASSERT_EQUAL(cain_sip_uri_has_lr_param(L_uri), 1);
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));

}
static void test_maddr(void) {
	cain_sip_uri_t *  L_uri = cain_sip_uri_parse("sip:192.168.0.1;lr;maddr=linphone.org");
	char* l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_uri = cain_sip_uri_parse(l_raw_uri);
	cain_sip_free(l_raw_uri);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_maddr_param(L_uri), "linphone.org");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));

}
static void test_uri_parameters () {
	cain_sip_uri_t* L_tmp;
	cain_sip_uri_t *  L_uri = cain_sip_uri_parse("sip:192.168.0.1;ttl=12");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));

	L_uri = cain_sip_uri_parse("sip:maddr=@192.168.0.1;lr;maddr=192.168.0.1;user=ip;ttl=140;transport=sctp;method=INVITE;rport=5060");
	char* l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));

	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_tmp = cain_sip_uri_parse(l_raw_uri);
	L_uri = CAIN_SIP_URI(cain_sip_object_clone(CAIN_SIP_OBJECT(L_tmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_tmp));

	cain_sip_free(l_raw_uri);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_maddr_param(L_uri), "192.168.0.1");
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_user_param(L_uri), "ip");
	CU_ASSERT_EQUAL(cain_sip_uri_get_ttl_param(L_uri),140);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_transport_param(L_uri), "sctp");
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_method_param(L_uri), "INVITE");

	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
}
static void test_headers(void) {
	cain_sip_uri_t *  L_uri = cain_sip_uri_parse("sip:192.168.0.1?toto=titi");
	char* l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_uri = cain_sip_uri_parse(l_raw_uri);
	cain_sip_free(l_raw_uri);
	CU_ASSERT_PTR_NOT_NULL_FATAL(cain_sip_uri_get_header(L_uri,"toto"));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_header(L_uri,"toto"), "titi");

	CU_ASSERT_PTR_NULL(cain_sip_uri_get_header(L_uri,"bla"));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_uri = cain_sip_uri_parse("sip:192.168.0.1?toto=titi&header2=popo");
	l_raw_uri = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_uri));
	L_uri = cain_sip_uri_parse(l_raw_uri);
	cain_sip_free(l_raw_uri);

	CU_ASSERT_PTR_NOT_NULL_FATAL(cain_sip_uri_get_header(L_uri,"toto"));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_header(L_uri,"header2"), "popo");

}



int cain_sip_uri_test_suite () {

	   CU_pSuite pSuite = NULL;


	   /* add a suite to the registry */
	   pSuite = CU_add_suite("uri suite", init_suite_uri, clean_suite_uri);
	   if (NULL == pSuite) {
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
		   || (NULL == CU_add_test(pSuite, "test of uri parameters", test_uri_parameters))
	       || (NULL == CU_add_test(pSuite, "test of sips uri", testSIPSURI)))
	   {
	      return CU_get_error();
	   }

	   return CU_get_error();
}
