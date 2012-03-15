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

#include "cain-sip/auth-helper.h"
#include <stdio.h>
#include "CUnit/Basic.h"

static void test_authentication(void) {
	const char* l_raw_header = "WWW-Authenticate: Digest "
				"algorithm=MD5, realm=\"sip.linphone.org\", opaque=\"1bc7f9097684320\","
				" qop=\"auth,auth-int\", nonce=\"cz3h0gAAAAC06TKKAABmTz1V9OcAAAAA\"";

	cain_sip_header_www_authenticate_t* www_authenticate=cain_sip_header_www_authenticate_parse(l_raw_header);
	cain_sip_header_authorization_t* authorization = cain_sip_auth_helper_create_authorization(www_authenticate);
	cain_sip_header_authorization_set_uri(authorization,cain_sip_uri_parse("sip:sip.linphone.org"));
	cain_sip_header_authorization_set_qop(authorization,"auth");
	CU_ASSERT_EQUAL_FATAL(0,cain_sip_auth_helper_fill_authorization(authorization,"REGISTER","jehan-mac","toto"));
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_response(authorization),"77ebf3de72e41934d806175586086508");
	cain_sip_object_unref(www_authenticate);
	cain_sip_object_unref(authorization);
}
static void test_proxy_authentication(void) {
	const char* l_raw_header = "Proxy-Authenticate: Digest "
				"algorithm=MD5, realm=\"sip.linphone.org\", opaque=\"1bc7f9097684320\","
				" qop=\"auth,auth-int\", nonce=\"cz3h0gAAAAC06TKKAABmTz1V9OcAAAAA\"";

	cain_sip_header_proxy_authenticate_t* proxy_authenticate=cain_sip_header_proxy_authenticate_parse(l_raw_header);
	cain_sip_header_proxy_authorization_t* proxy_authorization = cain_sip_auth_helper_create_proxy_authorization(proxy_authenticate);
	cain_sip_header_authorization_set_uri(CAIN_SIP_HEADER_AUTHORIZATION(proxy_authorization),cain_sip_uri_parse("sip:sip.linphone.org"));
	cain_sip_header_authorization_set_qop(CAIN_SIP_HEADER_AUTHORIZATION(proxy_authorization),"auth");
	CU_ASSERT_EQUAL_FATAL(0,cain_sip_auth_helper_fill_proxy_authorization(proxy_authorization,"REGISTER","jehan-mac","toto"));
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_response(CAIN_SIP_HEADER_AUTHORIZATION(proxy_authorization))
							,"77ebf3de72e41934d806175586086508");
	cain_sip_object_unref(proxy_authenticate);
	cain_sip_object_unref(proxy_authorization);

}
int cain_sip_authentication_helper_suite () {

	   CU_pSuite pSuite = NULL;


	   /* add a suite to the registry */
	   pSuite = CU_add_suite("authentication helper suite", NULL,NULL);
	   if (NULL == pSuite) {
	      return CU_get_error();
	   }

	   if (NULL == CU_add_test(pSuite, "test of simple digest authentication uri", test_authentication)) {
	   	   return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of simple digest proxy authentication", test_proxy_authentication)) {
		   return CU_get_error();
	   }

	   return CU_get_error();
}
