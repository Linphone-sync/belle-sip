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
	cain_sdp_attribute_t* lTmp;
	cain_sdp_attribute_t* lAttribute = cain_sdp_attribute_parse("a=rtpmap:101 telephone-event/8000");
	char* l_raw_attribute = cain_sip_object_to_string(CAIN_SIP_OBJECT(lAttribute));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lAttribute));
	lTmp = cain_sdp_attribute_parse(l_raw_attribute);
	lAttribute = CAIN_SDP_ATTRIBUTE(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));
	CU_ASSERT_STRING_EQUAL(cain_sdp_attribute_get_name(lAttribute), "rtpmap");
	CU_ASSERT_STRING_EQUAL(cain_sdp_attribute_get_value(lAttribute), "101 telephone-event/8000");
	CU_ASSERT_TRUE(cain_sdp_attribute_as_value(lAttribute));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lAttribute));
}
static void test_bandwidth(void) {
	cain_sdp_bandwidth_t* lTmp;
	cain_sdp_bandwidth_t* l_bandwidth = cain_sdp_bandwidth_parse("b=AS:380");
	char* l_raw_bandwidth = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_bandwidth));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_bandwidth));
	lTmp = cain_sdp_bandwidth_parse(l_raw_bandwidth);
	l_bandwidth = CAIN_SDP_BANDWIDTH(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));
	CU_ASSERT_STRING_EQUAL(cain_sdp_bandwidth_get_type(l_bandwidth), "AS");
	CU_ASSERT_EQUAL(cain_sdp_bandwidth_get_value(l_bandwidth),380);
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_bandwidth));
}


static void test_connection(void) {
	cain_sdp_connection_t* lTmp;
	cain_sdp_connection_t* lConnection = cain_sdp_connection_parse("c=IN IP4 192.168.0.18");
	char* l_raw_connection = cain_sip_object_to_string(CAIN_SIP_OBJECT(lConnection));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lConnection));
	lTmp = cain_sdp_connection_parse(l_raw_connection);
	lConnection = CAIN_SDP_CONNECTION(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_address(lConnection), "192.168.0.18");
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_address_type(lConnection), "IP4");
	CU_ASSERT_STRING_EQUAL(cain_sdp_connection_get_network_type(lConnection), "IN");
	cain_sip_object_unref(CAIN_SIP_OBJECT(lConnection));
}
static void test_email(void) {
	cain_sdp_email_t* lTmp;
	cain_sdp_email_t* l_email = cain_sdp_email_parse("e= jehan <jehan@linphone.org>");
	char* l_raw_email = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_email));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_email));
	lTmp = cain_sdp_email_parse(l_raw_email);
	l_email = CAIN_SDP_EMAIL(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));
	CU_ASSERT_STRING_EQUAL(cain_sdp_email_get_value(l_email), " jehan <jehan@linphone.org>");
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_email));
}
static void test_info(void) {
	cain_sdp_info_t* lTmp;
	cain_sdp_info_t* l_info = cain_sdp_info_parse("i=A Seminar on the session description protocol");
	char* l_raw_info = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_info));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_info));
	lTmp = cain_sdp_info_parse(l_raw_info);
	l_info = CAIN_SDP_INFO(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));
	CU_ASSERT_STRING_EQUAL(cain_sdp_info_get_value(l_info), "A Seminar on the session description protocol");
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_info));
}
static void test_media(void) {
	cain_sdp_media_t* lTmp;
	cain_sdp_media_t* l_media = cain_sdp_media_parse("m=audio 7078 RTP/AVP 111 110 3 0 8 101");
	char* l_raw_media = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_media));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_media));
	lTmp = cain_sdp_media_parse(l_raw_media);
	l_media = CAIN_SDP_MEDIA(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));
	CU_ASSERT_STRING_EQUAL(cain_sdp_media_get_media_type(l_media), "audio");
	CU_ASSERT_EQUAL(cain_sdp_media_get_media_port(l_media), 7078);
	CU_ASSERT_STRING_EQUAL(cain_sdp_media_get_protocol(l_media), "RTP/AVP");
	cain_sip_list_t* list = cain_sdp_media_get_media_formats(l_media);
	CU_ASSERT_PTR_NOT_NULL(list);
	int fmt[] ={111,110,3,0,8,101};
	int i=0;
	for(;list!=NULL;list=list->next){
		CU_ASSERT_EQUAL(CAIN_SIP_POINTER_TO_INT(list->data),fmt[i++]);
	}

	cain_sip_object_unref(CAIN_SIP_OBJECT(l_media));
}

static void test_media_description_base(cain_sdp_media_description_t* media_description) {
	cain_sdp_media_description_t* l_media_description=media_description;
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
		CU_ASSERT_EQUAL(CAIN_SIP_POINTER_TO_INT(list->data),fmt[i++]);
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

static void test_media_description(void) {
	const char* l_src = "m=video 8078 RTP/AVP 99 97 98\r\n"\
						"c=IN IP4 192.168.0.18\r\n"\
						"b=AS:380\r\n"\
						"a=rtpmap:99 MP4V-ES/90000\r\n"\
						"a=fmtp:99 profile-level-id=3\r\n"\
						"a=rtpmap:97 theora/90000\r\n"\
						"a=rtpmap:98 H263-1998/90000\r\n"\
						"a=fmtp:98 CIF=1;QCIF=1\r\n";

	cain_sdp_media_description_t* lTmp;
	cain_sdp_media_description_t* l_media_description = cain_sdp_media_description_parse(l_src);
	char* l_raw_media_description = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_media_description));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_media_description));
	lTmp = cain_sdp_media_description_parse(l_raw_media_description);
	l_media_description = CAIN_SDP_MEDIA_DESCRIPTION(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));
	test_media_description_base(l_media_description);
	return;
}
static void test_session_description(void) {
	const char* l_src = "v=0\r\n"\
						"o=jehan-mac 1239 1239 IN IP4 192.168.0.18\r\n"\
						"s=Talk\r\n"\
						"c=IN IP4 192.168.0.18\r\n"\
						"t=0 0\r\n"\
						"m=audio 7078 RTP/AVP 111 110 3 0 8 101\r\n"\
						"a=rtpmap:111 speex/16000\r\n"\
						"a=fmtp:111 vbr=on\r\n"\
						"a=rtpmap:110 speex/8000\r\n"\
						"a=fmtp:110 vbr=on\r\n"\
						"a=rtpmap:101 telephone-event/8000\r\n"\
						"a=fmtp:101 0-11\r\n"\
						"m=video 8078 RTP/AVP 99 97 98\r\n"\
						"c=IN IP4 192.168.0.18\r\n"\
						"b=AS:380\r\n"\
						"a=rtpmap:99 MP4V-ES/90000\r\n"\
						"a=fmtp:99 profile-level-id=3\r\n"\
						"a=rtpmap:97 theora/90000\r\n"\
						"a=rtpmap:98 H263-1998/90000\r\n"\
						"a=fmtp:98 CIF=1;QCIF=1\r\n";
	cain_sdp_session_description_t* lTmp;
	cain_sdp_session_description_t* l_session_description = cain_sdp_session_description_parse(l_src);
	char* l_raw_session_description = cain_sip_object_to_string(CAIN_SIP_OBJECT(l_session_description));
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_session_description));
	lTmp = cain_sdp_session_description_parse(l_raw_session_description);
	l_session_description = CAIN_SDP_SESSION_DESCRIPTION(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));

	CU_ASSERT_PTR_NOT_NULL(cain_sdp_session_description_get_version(l_session_description));
	CU_ASSERT_EQUAL(cain_sdp_version_get_version(cain_sdp_session_description_get_version(l_session_description)),0);

	cain_sdp_origin_t* l_origin = cain_sdp_session_description_get_origin(l_session_description);
	CU_ASSERT_PTR_NOT_NULL(l_origin);
	CU_ASSERT_STRING_EQUAL(cain_sdp_origin_get_address(l_origin),"192.168.0.18")
	CU_ASSERT_STRING_EQUAL(cain_sdp_origin_get_address_type(l_origin),"IP4")
	CU_ASSERT_STRING_EQUAL(cain_sdp_origin_get_network_type(l_origin),"IN")
	CU_ASSERT_EQUAL(cain_sdp_origin_get_session_id(l_origin),1239)
	CU_ASSERT_EQUAL(cain_sdp_origin_get_session_version(l_origin),1239)

	CU_ASSERT_PTR_NOT_NULL(cain_sdp_session_description_get_session_name(l_session_description));
	CU_ASSERT_STRING_EQUAL(cain_sdp_session_name_get_value(cain_sdp_session_description_get_session_name(l_session_description)),"Talk");

	CU_ASSERT_PTR_NOT_NULL(cain_sdp_session_description_get_connection(l_session_description));
	CU_ASSERT_PTR_NOT_NULL(cain_sdp_session_description_get_time_descriptions(l_session_description));
	CU_ASSERT_EQUAL(cain_sdp_time_get_start(cain_sdp_time_description_get_time((cain_sdp_time_description_t*)(cain_sdp_session_description_get_time_descriptions(l_session_description)->data))),0);
	CU_ASSERT_EQUAL(cain_sdp_time_get_stop(cain_sdp_time_description_get_time((cain_sdp_time_description_t*)(cain_sdp_session_description_get_time_descriptions(l_session_description)->data))),0);

	cain_sip_list_t* media_descriptions = cain_sdp_session_description_get_media_descriptions(l_session_description);
	CU_ASSERT_PTR_NOT_NULL(media_descriptions);
	CU_ASSERT_STRING_EQUAL (cain_sdp_media_get_media_type(cain_sdp_media_description_get_media((cain_sdp_media_description_t*)(media_descriptions->data))),"audio");
	media_descriptions=media_descriptions->next;
	CU_ASSERT_PTR_NOT_NULL(media_descriptions);

	test_media_description_base((cain_sdp_media_description_t*)(media_descriptions->data));
	return;
}

static cain_sdp_mime_parameter_t* find_mime_parameter(cain_sip_list_t* list,const int format) {
	for(;list!=NULL;list=list->next){
		if (cain_sdp_mime_parameter_get_media_format((cain_sdp_mime_parameter_t*)list->data) == format) {
			return (cain_sdp_mime_parameter_t*)list->data;
		}
	}
	return NULL;
}
static void check_mime_param (cain_sdp_mime_parameter_t* mime_param
							,int rate
							,int channel_count
							,int ptime
							,int max_ptime
							,int media_format
							,const char* type
							,const char* parameters) {
	CU_ASSERT_PTR_NOT_NULL(mime_param);
	CU_ASSERT_EQUAL(cain_sdp_mime_parameter_get_rate(mime_param),rate);
	CU_ASSERT_EQUAL(cain_sdp_mime_parameter_get_channel_count(mime_param),channel_count);
	CU_ASSERT_EQUAL(cain_sdp_mime_parameter_get_ptime(mime_param),ptime);
	CU_ASSERT_EQUAL(cain_sdp_mime_parameter_get_max_ptime(mime_param),max_ptime);
	CU_ASSERT_EQUAL(cain_sdp_mime_parameter_get_media_format(mime_param),media_format);
	if (type) CU_ASSERT_STRING_EQUAL(cain_sdp_mime_parameter_get_type(mime_param),type);
	if (parameters) CU_ASSERT_STRING_EQUAL(cain_sdp_mime_parameter_get_parameters(mime_param),parameters);
}
static void test_mime_parameter(void) {
	const char* l_src = "m=audio 7078 RTP/AVP 111 110 0 8 9 3 101\r\n"\
						"a=rtpmap:111 speex/16000\r\n"\
						"a=fmtp:111 vbr=on\r\n"\
						"a=rtpmap:110 speex/8000\r\n"\
						"a=fmtp:110 vbr=on\r\n"\
						"a=rtpmap:8 PCMA/8000\r\n"\
						"a=rtpmap:101 telephone-event/8000\r\n"\
						"a=fmtp:101 0-11\r\n"\
						"a=ptime:40\r\n";

	cain_sdp_media_description_t* l_media_description = cain_sdp_media_description_parse(l_src);
	cain_sip_list_t* mime_parameter_list = cain_sdp_media_description_build_mime_parameters(l_media_description);
	CU_ASSERT_PTR_NOT_NULL(mime_parameter_list);
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_media_description));

	l_media_description = cain_sdp_media_description_new();
	cain_sdp_media_description_set_media(l_media_description,cain_sdp_media_parse("m=audio 7078 RTP/AVP"));
	for (;mime_parameter_list!=NULL;mime_parameter_list=mime_parameter_list->next) {
		cain_sdp_media_description_append_values_from_mime_parameter(l_media_description,(cain_sdp_mime_parameter_t*)mime_parameter_list->data);
	}
	cain_sdp_media_description_set_attribute(l_media_description,"ptime","40");

	 mime_parameter_list = cain_sdp_media_description_build_mime_parameters(l_media_description);
	cain_sdp_mime_parameter_t* l_param;
	cain_sdp_mime_parameter_t*  lTmp = find_mime_parameter(mime_parameter_list,111);
	l_param = CAIN_SDP_MIME_PARAMETER(cain_sip_object_clone(CAIN_SIP_OBJECT(lTmp)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(lTmp));

	CU_ASSERT_PTR_NOT_NULL(l_param);
	check_mime_param(l_param,16000,1,40,-1,111,"speex","vbr=on");
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_param));

	l_param = find_mime_parameter(mime_parameter_list,110);
	CU_ASSERT_PTR_NOT_NULL(l_param);
	check_mime_param(l_param,8000,1,40,-1,110,"speex","vbr=on");

	l_param = find_mime_parameter(mime_parameter_list,3);
	CU_ASSERT_PTR_NOT_NULL(l_param);
	check_mime_param(l_param,8000,1,40,-1,3,"GSM",NULL);
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_param));

	l_param = find_mime_parameter(mime_parameter_list,0);
	CU_ASSERT_PTR_NOT_NULL(l_param);
	check_mime_param(l_param,8000,1,40,-1,0,"PCMU",NULL);
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_param));

	l_param = find_mime_parameter(mime_parameter_list,8);
	CU_ASSERT_PTR_NOT_NULL(l_param);
	check_mime_param(l_param,8000,1,40,-1,8,"PCMA",NULL);
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_param));

	l_param = find_mime_parameter(mime_parameter_list,9);
	CU_ASSERT_PTR_NOT_NULL(l_param);
	check_mime_param(l_param,8000,1,40,-1,9,"G722",NULL);
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_param));

	l_param = find_mime_parameter(mime_parameter_list,101);
	CU_ASSERT_PTR_NOT_NULL(l_param);
	check_mime_param(l_param,8000,1,40,-1,101,"telephone-event","0-11");
	cain_sip_object_unref(CAIN_SIP_OBJECT(l_param));
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
	if (NULL == CU_add_test(pSuite, "mime_parameter", test_mime_parameter)) {
			return CU_get_error();
	}
	if (NULL == CU_add_test(pSuite, "session_description", test_session_description)) {
			return CU_get_error();
	}	return 0;
}
