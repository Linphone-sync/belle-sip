/*
	cain-sdp - SDP (RFC) library.
    Copyright (C) 2011  Belledonne Communications SARL

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

#include "cain-sip/cain-sdp.h"
#include <stdio.h>
#include "CUnit/Basic.h"

static int init_suite_sdp(void) {
      return 0;
}

static int clean_suite_sdp(void) {
      return 0;
}
//v=0
//o=jehan-mac 1239 1239 IN IP4 192.168.0.18
//s=Talk
//c=IN IP4 192.168.0.18
//t=0 0
//m=audio 7078 RTP/AVP 111 110 3 0 8 101
//a=rtpmap:111 speex/16000
//a=fmtp:111 vbr=on
//a=rtpmap:110 speex/8000
//a=fmtp:110 vbr=on
//a=rtpmap:101 telephone-event/8000
//a=fmtp:101 0-11
//m=video 8078 RTP/AVP 99 97 98
//a=rtpmap:99 MP4V-ES/90000
//a=fmtp:99 profile-level-id=3
//a=rtpmap:97 theora/90000
//a=rtpmap:98 H263-1998/90000
//a=fmtp:98 CIF=1;QCIF=1

static void test_attribute(void) {
	cain_sdp_attribute_t* lAttribute = cain_sdp_attribute_parse("a=rtpmap:101 telephone-event/8000");
	char* l_raw_attribute = cain_sip_object_to_string(CAIN_SIP_OBJECT(lAttribute));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lAttribute));
	lAttribute = cain_sdp_attribute_parse(l_raw_attribute);
	CU_ASSERT_STRING_EQUAL(cain_sdp_attribute_get_name(lAttribute), "rtpmap");
	CU_ASSERT_STRING_EQUAL(cain_sdp_attribute_get_value(lAttribute), "101 telephone-event/8000");
	CU_ASSERT_TRUE(cain_sdp_attribute_as_value(lAttribute));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lAttribute));
}

static void test_connection(void) {
	cain_sdp_connection_t* lConnection = cain_sdp_connection_parse("c=IN IP4 192.168.0.18");
	char* l_raw_connection = cain_sip_object_to_string(CAIN_SIP_OBJECT(lConnection));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lConnection));
	lConnection = cain_sdp_connection_parse(l_raw_connection);
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_address(lConnection), "192.168.0.18");
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_address_type(lConnection), "IP4");
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_network_type(lConnection), "IN");
	cain_sip_object_unref(CAIN_SIP_OBJECT(lConnection));
}



int cain_sdp_test_suite () {

	CU_pSuite pSuite = NULL;
	/* add a suite to the registry */
	pSuite = CU_add_suite("sdp_suite", init_suite_sdp, clean_suite_sdp);

	/* add the tests to the suite */
	/* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
	if (NULL == CU_add_test(pSuite, "connection", test_connection)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "attribute", test_attribute)) {
			return CU_get_error();
		}
	return 0;
}
