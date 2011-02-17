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

static int init_suite_message(void) {
      return 0;
}

static int clean_suite_message(void) {
      return 0;
}



static void testMessage(void) {
	const char* raw_message = "REGISTER sip:192.168.0.20 SIP/2.0\n\r"\
							"Via: SIP/2.0/UDP 192.168.1.8:5062;rport;branch=z9hG4bK1439638806\n\r"\
							"From: <sip:jehan-mac@sip.linphone.org>;tag=465687829\n\r"\
							"To: <sip:jehan-mac@sip.linphone.org>\n\rCall-ID: 1053183492\n\r"\
							"CSeq: 1 REGISTER\n\r"\
							"Contact: <sip:jehan-mac@192.168.1.8:5062>\n\r"\
							"Max-Forwards: 70\n\r"\
							"User-Agent: Linphone/3.3.99.10 (eXosip2/3.3.0)\n\r"
							"Expires: 3600\n\r"\
							"Content-Length: 0\n\r\n\r";
	cain_sip_message_t* message = cain_sip_message_parse(raw_message);
	cain_sip_request_t* request = CAIN_SIP_REQUEST(message);
	CU_ASSERT_STRING_EQUAL(cain_sip_request_get_method(request),"REGISTER");
}



int cain_sip_message_test_suite () {

	   CU_pSuite pSuite = NULL;


	   /* add a suite to the registry */
	   pSuite = CU_add_suite("message suite", init_suite_message, clean_suite_message);
	   if (NULL == pSuite) {
	      return CU_get_error();
	   }

	   /* add the tests to the suite */
	   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
	   if ((NULL == CU_add_test(pSuite, "test of simple messgae", testMessage))
	   	   )
	   {
	      return CU_get_error();
	   }

	   return CU_get_error();
}
