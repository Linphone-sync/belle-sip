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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "CUnit/Basic.h"
#if HAVE_CU_CURSES
#include "CUnit/CUCurses.h"
#endif
#include <cain-sip/cain-sip.h>

extern const char *test_domain;
extern const char *auth_domain;
extern int cain_sip_uri_test_suite ();
extern int cain_sip_headers_test_suite ();
extern int cain_sip_message_test_suite ();
extern int cain_sdp_test_suite();
extern int cain_sip_authentication_helper_suite ();
extern int cain_sip_cast_test_suite();
extern int cain_sip_register_test_suite();
extern int cain_sip_dialog_test_suite();
extern int cain_sip_refresher_test_suite();
extern int cain_sip_resolver_test_suite();

#if HAVE_CU_CURSES
	static unsigned char curses = 0;
#endif

int cain_sip_tester_run_tests(char *suite_name, char *test_name) {
	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	cain_sip_uri_test_suite();

	cain_sip_headers_test_suite ();

	cain_sip_message_test_suite();

	cain_sdp_test_suite();

	cain_sip_cast_test_suite();

	cain_sip_authentication_helper_suite();

	cain_sip_register_test_suite();

	cain_sip_dialog_test_suite ();

	cain_sip_refresher_test_suite();

	cain_sip_resolver_test_suite();


#if HAVE_CU_GET_SUITE
	if (suite_name){
		CU_pSuite suite;
		CU_basic_set_mode(CU_BRM_VERBOSE);
		suite=CU_get_suite(suite_name);
		if (test_name) {
			CU_pTest test=CU_get_test_by_name(test_name, suite);
			CU_basic_run_test(suite, test);
		} else
			CU_basic_run_suite(suite);
	} else
#endif
	{
#if HAVE_CU_CURSES
		if (curses) {
			/* Run tests using the CUnit curses interface */
			CU_curses_run_tests();
		}
		else
#endif
		{
			/* Run all tests using the CUnit Basic interface */
			CU_basic_set_mode(CU_BRM_VERBOSE);
			CU_basic_run_tests();
		}
	}

	CU_cleanup_registry();
	return CU_get_error();
}


#if !WINAPI_FAMILY_APP
int main (int argc, char *argv[]) {
	int i;
	char *suite_name=NULL;
	char *test_name=NULL;
	const char *env_domain=getenv("TEST_DOMAIN");


	if (env_domain)
		test_domain=env_domain;

	for(i=1;i<argc;++i){
		if (strcmp(argv[i],"--help")==0){
				fprintf(stderr,"%s \t--help\n"
						"\t\t\t--verbose\n"
						"\t\t\t--domain <test sip domain>\n"
						"\t\t\t---auth-domain <test auth domain>\n"
#if HAVE_CU_GET_SUITE
						"\t\t\t--suite <suite name>\n"
						"\t\t\t--test <test name>\n"
#endif
#if HAVE_CU_CURSES
						"\t\t\t--curses\n"
#endif
						, argv[0]);
				return 0;
		}else if (strcmp(argv[i],"--verbose")==0){
			cain_sip_set_log_level(CAIN_SIP_LOG_DEBUG);
		}else if (strcmp(argv[i],"--domain")==0){
			i++;
			test_domain=argv[i];
		}
		else if (strcmp(argv[i],"--auth-domain")==0){
					i++;
					auth_domain=argv[i];
		}
#if HAVE_CU_GET_SUITE
		else if (strcmp(argv[i],"--test")==0){
			i++;
			test_name=argv[i];
		}else if (strcmp(argv[i],"--suite")==0){
			i++;
			suite_name=argv[i];
		}
#endif
#if HAVE_CU_CURSES
		else if (strcmp(argv[i], "--curses") == 0) {
			i++;
			curses = 1;
		}
#endif
	}

	return cain_sip_tester_run_tests(suite_name, test_name);
}
#endif
