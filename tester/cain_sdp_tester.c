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
static void test_bandwidth(void) {
	cain_sdp_bandwidth_t* l_bandwidth = cain_sdp_bandwidth_parse("b=AS:380");
	char* l_raw_bandwidth = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_bandwidth));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_bandwidth));
	l_bandwidth = cain_sdp_bandwidth_parse(l_raw_bandwidth);
	CU_ASSERT_STRING_EQUAL(cain_sdp_bandwidth_get_type(l_bandwidth), "AS");
	CU_ASSERT_EQUAL(cain_sdp_bandwidth_get_value(l_bandwidth),380);
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_bandwidth));
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
static void test_email(void) {
	cain_sdp_email_t* l_email = cain_sdp_email_parse("e= jehan <jehan@linphone.org>");
	char* l_raw_email = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_email));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_email));
	l_email = cain_sdp_email_parse(l_raw_email);
	CU_ASSERT_STRING_EQUAL(cain_sdp_email_get_value(l_email), " jehan <jehan@linphone.org>");
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_email));
}
static void test_info(void) {
	cain_sdp_info_t* l_info = cain_sdp_info_parse("i=A Seminar on the session description protocol");
	char* l_raw_info = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_info));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_info));
	l_info = cain_sdp_info_parse(l_raw_info);
	CU_ASSERT_STRING_EQUAL(cain_sdp_info_get_value(l_info), "A Seminar on the session description protocol");
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_info));
}
static void test_media(void) {
	cain_sdp_media_t* l_media = cain_sdp_media_parse("m=audio 7078 RTP/AVP 111 110 3 0 8 101");
	char* l_raw_media = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_media));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_media));
	l_media = cain_sdp_media_parse(l_raw_media);
	CU_ASSERT_STRING_EQUAL(cain_sdp_media_get_media_type(l_media), "audio");
	CU_ASSERT_EQUAL(cain_sdp_media_get_media_port(l_media), 7078);
	CU_ASSERT_STRING_EQUAL(cain_sdp_media_get_protocol(l_media), "RTP/AVP");
	cain_sip_list_t* list = cain_sdp_media_get_media_formats(l_media);
	CU_ASSERT_PTR_NOT_NULL(list);
	int fmt[] ={111,110,3,0,8,101};
	int i=0;
	for(;list!=NULL;list=list->next){
		CU_ASSERT_EQUAL((int)(list->data),fmt[i++]);
	}

	cain_sip_object_unref(CAIN_SIP_OBJECT(l_media));
}
static void test_media_description(void) {
	const char* l_src = "m=video 8078 RTP/AVP 99 97 98\r\n"\
						"c=IN IP4 192.168.0.18\r\n"\
						"b=AS:380\r\n"\
						"a=rtpmap:99 MP4V-ES/90000\r\n"\
						"a=fmtp:99 profile-level-id=3\r\n"\
						"a=rtpmap:97 theora/90000\r\n"\
						"a=rtpmap:98 H263-1998/90000\r\n"\
						"a=fmtp:98 CIF=1;QCIF=1\r\n";

	cain_sdp_media_description_t* l_media_description = cain_sdp_media_description_parse(l_src);
	char* l_raw_media_description = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_media_description));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_media_description));
	l_media_description = cain_sdp_media_description_parse(l_raw_media_description);
	/*media*/
	cain_sdp_media_t* l_media = cain_sdp_media_description_get_media(l_media_description);
	CU_ASSERT_PTR_NOT_NULL(l_media);
	CU_ASSERT_STRING_EQUAL(cain_sdp_media_get_media_type(l_media), "video");
	CU_ASSERT_EQUAL(cain_sdp_media_get_media_port(l_media), 8078);
	CU_ASSERT_STRING_EQUAL(cain_sdp_media_get_protocol(l_media), "RTP/AVP");
	cain_sip_list_t* list = cain_sdp_media_get_media_formats(l_media);
	CU_ASSERT_PTR_NOT_NULL(list);
	int fmt[] ={99,97,98};
	int i=0;
	for(;list!=NULL;list=list->next){
		CU_ASSERT_EQUAL((int)(list->data),fmt[i++]);
	}
	/*connection*/
	cain_sdp_connection_t* lConnection = cain_sdp_media_description_get_connection(l_media_description);
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_address(lConnection), "192.168.0.18");
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_address_type(lConnection), "IP4");
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_network_type(lConnection), "IN");

	/*bandwidth*/

	CU_ASSERT_EQUAL(cain_sdp_media_description_get_bandwidth(l_media_description,"AS"),380);

	/*attributes*/
	list = cain_sdp_media_description_get_attributes(l_media_description);
	CU_ASSERT_PTR_NOT_NULL(list);
	const char* attr[] ={"99 MP4V-ES/90000"
				,"99 profile-level-id=3"
				,"97 theora/90000"
				,"98 H263-1998/90000"
				,"98 CIF=1;QCIF=1"};
	i=0;
	for(;list!=NULL;list=list->next){
		CU_ASSERT_STRING_EQUAL(cain_sdp_attribute_get_value((cain_sdp_attribute_t*)(list->data)),attr[i++]);
	}
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_media_description));
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
	if (NULL == CU_add_test(pSuite, "bandwidth", test_bandwidth)) {
		return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "email", test_email)) {
			return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "info", test_info)) {
			return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "media", test_media)) {
			return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "media_description", test_media_description)) {
			return CU_get_error();
	}
	return 0;
}