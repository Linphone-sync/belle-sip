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
#include "cain_sip_internal.h"
#include <stdio.h>
#include "CUnit/Basic.h"

int init_suite1(void) {
      return 0;
}

int clean_suite1(void) {
      return 0;
}


void test_simple_header_contact(void) {

	cain_sip_header_contact_t* L_contact = cain_sip_header_contact_parse("Contact:sip:titi.com");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_contact));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_contact));
	L_contact = cain_sip_header_contact_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	cain_sip_uri_t* L_uri = cain_sip_header_address_get_uri((cain_sip_header_address_t*)L_contact);

	CU_ASSERT_PTR_NULL(cain_sip_uri_get_user(L_uri));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "titi.com");
	CU_ASSERT_PTR_NULL(cain_sip_uri_get_transport_param(L_uri));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_contact));
}

void test_complex_header_contact(void) {

	cain_sip_header_contact_t* L_contact = cain_sip_header_contact_parse("Contact: \"j�remis\" <sip:sip.linphone.org>;expires=3600;q=0.7, sip:titi.com");


	cain_sip_uri_t* L_uri = cain_sip_header_address_get_uri((cain_sip_header_address_t*)L_contact);

	CU_ASSERT_PTR_NOT_NULL(L_uri);
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "sip.linphone.org");

	CU_ASSERT_STRING_EQUAL(cain_sip_header_address_get_displayname((cain_sip_header_address_t*)L_contact), "j�remis");

	CU_ASSERT_EQUAL(cain_sip_header_contact_get_expires(L_contact),3600);
	float l_qvalue = cain_sip_header_contact_get_qvalue(L_contact);
	CU_ASSERT_EQUAL(l_qvalue,0.7);

	cain_sip_header_t* l_next = cain_sip_header_get_next(CAIN_SIP_HEADER(L_contact));
	cain_sip_header_contact_t* L_next_contact = CAIN_SIP_HEADER_CONTACT(l_next);
	CU_ASSERT_PTR_NOT_NULL(L_next_contact);
	CU_ASSERT_PTR_NOT_NULL( cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(L_contact)));

	cain_sip_object_unref(CAIN_SIP_OBJECT(L_contact));

	L_contact = cain_sip_header_contact_parse("Contact: toto <sip:titi.com>;expires=3600; q=0.7");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_contact));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_contact));
	L_contact = cain_sip_header_contact_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	CU_ASSERT_STRING_EQUAL(cain_sip_header_address_get_displayname((cain_sip_header_address_t*)L_contact), "toto");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_contact));

}

void test_simple_header_from(void) {

	cain_sip_header_from_t* L_from = cain_sip_header_from_parse("From:<sip:titi.com;transport=tcp>;tag=dlfjklcn6545614XX");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_from));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_from));
	L_from = cain_sip_header_from_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	cain_sip_uri_t* L_uri = cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(L_from));

	CU_ASSERT_PTR_NULL(cain_sip_uri_get_user(L_uri));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "titi.com");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_from_get_tag(L_from),"dlfjklcn6545614XX");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_from));
}

void test_simple_header_to(void) {

	cain_sip_header_to_t* L_to = cain_sip_header_to_parse("To : < sip:titi.com;transport=tcp> ; tag = dlfjklcn6545614XX");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_to));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_to));
	L_to = cain_sip_header_to_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	cain_sip_uri_t* L_uri = cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(L_to));

	CU_ASSERT_PTR_NULL(cain_sip_uri_get_user(L_uri));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "titi.com");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_to_get_tag(L_to),"dlfjklcn6545614XX");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_to));
}
void test_header_via(void) {

	cain_sip_header_via_t* L_via = cain_sip_header_via_parse("Via: SIP/2.0/UDP 192.168.0.19:5062;rport;received=192.169.0.4;branch=z9hG4bK368560724");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_via));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_via));
	L_via = cain_sip_header_via_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	CU_ASSERT_STRING_EQUAL(cain_sip_header_via_get_protocol(L_via), "SIP/2.0");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_via_get_transport(L_via), "UDP");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_via_get_host(L_via), "192.168.0.19");
	CU_ASSERT_EQUAL(cain_sip_header_via_get_port(L_via),5062);

	CU_ASSERT_TRUE(cain_sip_parameters_is_parameter(CAIN_SIP_PARAMETERS(L_via),"rport"));
	CU_ASSERT_STRING_EQUAL(cain_sip_header_via_get_received(L_via),"192.169.0.4");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_via_get_branch(L_via),"z9hG4bK368560724");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_via));

	L_via = cain_sip_header_via_parse("Via: SIP/2.0/UDP 192.168.0.19:5062;rport;received=192.169.0.4;branch=z9hG4bK368560724, SIP/2.0/UDP 192.168.0.19:5062");

	cain_sip_header_t* l_next = cain_sip_header_get_next(CAIN_SIP_HEADER(L_via));
	cain_sip_header_via_t* L_next_via = CAIN_SIP_HEADER_VIA(l_next);
	CU_ASSERT_PTR_NOT_NULL(L_next_via);
	CU_ASSERT_STRING_EQUAL(cain_sip_header_via_get_host(L_next_via),"192.168.0.19");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_via));

}
void test_header_call_id(void) {

	cain_sip_header_call_id_t* L_call_id = cain_sip_header_call_id_parse("Call-ID: 1665237789@titi.com");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_call_id));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_call_id));
	L_call_id = cain_sip_header_call_id_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	CU_ASSERT_STRING_EQUAL(cain_sip_header_call_id_get_call_id(L_call_id), "1665237789@titi.com");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_call_id));
}
void test_header_cseq(void) {

	cain_sip_header_cseq_t* L_cseq = cain_sip_header_cseq_parse("CSeq: 21 INVITE");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_cseq));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_cseq));
	L_cseq = cain_sip_header_cseq_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	CU_ASSERT_EQUAL(cain_sip_header_cseq_get_seq_number(L_cseq),21);
	CU_ASSERT_STRING_EQUAL(cain_sip_header_cseq_get_method(L_cseq),"INVITE");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_cseq));
}
void test_header_content_type(void) {

	cain_sip_header_content_type_t* L_content_type = cain_sip_header_content_type_parse("Content-Type: text/html; charset=ISO-8859-4");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_content_type));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_content_type));
	L_content_type = cain_sip_header_content_type_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	CU_ASSERT_STRING_EQUAL(cain_sip_header_content_type_get_type(L_content_type),"text");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_content_type_get_subtype(L_content_type),"html");
	CU_ASSERT_STRING_EQUAL(cain_sip_parameters_get_parameter(CAIN_SIP_PARAMETERS(L_content_type),"charset"),"ISO-8859-4");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_content_type));

	L_content_type = cain_sip_header_content_type_parse("Content-Type: application/sdp");
	l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_content_type));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_content_type));
	L_content_type = cain_sip_header_content_type_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	CU_ASSERT_STRING_EQUAL(cain_sip_header_content_type_get_type(L_content_type),"application");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_content_type_get_subtype(L_content_type),"sdp");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_content_type));

	L_content_type = cain_sip_header_content_type_parse("Content-Type: application/pkcs7-mime; smime-type=enveloped-data; \r\n name=smime.p7m");
	l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_content_type));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_content_type));
	L_content_type = cain_sip_header_content_type_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_content_type));

}
void test_header_record_route(void) {

	cain_sip_header_record_route_t* L_record_route = cain_sip_header_record_route_parse("Record-Route: <sip:212.27.52.5:5060;transport=udp;lr>;charset=ISO-8859-4");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_record_route));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_record_route));
	L_record_route = cain_sip_header_record_route_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	cain_sip_uri_t* L_uri = cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(L_record_route));
	CU_ASSERT_PTR_NULL(cain_sip_uri_get_user(L_uri));
	CU_ASSERT_STRING_EQUAL(cain_sip_uri_get_host(L_uri), "212.27.52.5");
	CU_ASSERT_STRING_EQUAL(cain_sip_parameters_get_parameter(CAIN_SIP_PARAMETERS(L_record_route),"charset"),"ISO-8859-4");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_record_route));

	L_record_route = cain_sip_header_record_route_parse("Record-Route: <sip:212.27.52.5:5060;transport=udp;lr>;charset=ISO-8859-4, <sip:212.27.52.5:5060;transport=udp;lr>");
	cain_sip_header_t* l_next = cain_sip_header_get_next(CAIN_SIP_HEADER(L_record_route));
	cain_sip_header_record_route_t* L_next_route = CAIN_SIP_HEADER_RECORD_ROUTE(l_next);
	CU_ASSERT_PTR_NOT_NULL(L_next_route);
	CU_ASSERT_PTR_NOT_NULL( cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(L_next_route)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_record_route));
}
void test_header_route(void) {

	cain_sip_header_route_t* L_route = cain_sip_header_route_parse("Route: <sip:212.27.52.5:5060;transport=udp;lr>;charset=ISO-8859-4");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_route));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_route));
	L_route = cain_sip_header_route_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	cain_sip_uri_t* L_uri = cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(L_route));
	CU_ASSERT_PTR_NULL(cain_sip_uri_get_user(L_uri));
	CU_ASSERT_EQUAL(cain_sip_uri_get_port(L_uri), 5060);
	CU_ASSERT_STRING_EQUAL(cain_sip_parameters_get_parameter(CAIN_SIP_PARAMETERS(L_route),"charset"),"ISO-8859-4");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_route));

	L_route = cain_sip_header_route_parse("Route: <sip:212.27.52.5:5060;transport=udp;lr>;charset=ISO-8859-4, <sip:titi.com>");
	cain_sip_header_t* l_next = cain_sip_header_get_next(CAIN_SIP_HEADER(L_route));
	cain_sip_header_route_t* L_next_route = CAIN_SIP_HEADER_ROUTE(l_next);
	CU_ASSERT_PTR_NOT_NULL(L_next_route);
	CU_ASSERT_PTR_NOT_NULL( cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(L_next_route)));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_route));
}
void test_header_content_length(void) {

	cain_sip_header_content_length_t* L_content_length = cain_sip_header_content_length_parse("Content-Length: 3495");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_content_length));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_content_length));
	L_content_length = cain_sip_header_content_length_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	CU_ASSERT_EQUAL(cain_sip_header_content_length_get_content_length(L_content_length), 3495);
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_content_length));
}
void test_header_extention(void) {
	cain_sip_header_extension_t* L_extension = cain_sip_header_extension_parse("toto: titi");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_extension));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_extension));
	L_extension = cain_sip_header_extension_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	CU_ASSERT_STRING_EQUAL(cain_sip_header_extension_get_value(L_extension), "titi");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_extension));
}
void test_header_authorization(void) {
	const char* l_header = "Authorization: Digest username=\"0033482532176\", "\
			"realm=\"sip.ovh.net\", nonce=\"1bcdcb194b30df5f43973d4c69bdf54f\", uri=\"sip:sip.ovh.net\", response=\"eb36c8d5c8642c1c5f44ec3404613c81\","\
			"algorithm=MD5, opaque=\"1bc7f9097684320\","
			"\r\n qop=auth, nc=00000001,cnonce=\"0a4f113b\", blabla=\"toto\"";
	cain_sip_header_authorization_t* L_authorization = cain_sip_header_authorization_parse(l_header);
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_authorization));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_authorization));
	L_authorization = cain_sip_header_authorization_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	CU_ASSERT_PTR_NOT_NULL(L_authorization);
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_scheme(L_authorization), "Digest");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_username(L_authorization), "0033482532176");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_realm(L_authorization), "sip.ovh.net");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_nonce(L_authorization), "1bcdcb194b30df5f43973d4c69bdf54f");
	CU_ASSERT_PTR_NOT_NULL(cain_sip_header_authorization_get_uri(L_authorization));
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_response(L_authorization), "eb36c8d5c8642c1c5f44ec3404613c81");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_algorithm(L_authorization), "MD5");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_opaque(L_authorization), "1bc7f9097684320");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_qop(L_authorization), "auth");
	CU_ASSERT_EQUAL(cain_sip_header_authorization_get_nonce_count(L_authorization), 1);
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_cnonce(L_authorization), "0a4f113b");
	CU_ASSERT_STRING_EQUAL(cain_sip_parameters_get_parameter(CAIN_SIP_PARAMETERS(L_authorization), "blabla"),"\"toto\"");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_authorization));
}
void test_header_proxy_authorization(void) {
	const char* l_header = "Proxy-Authorization: Digest username=\"Alice\""
			", realm=\"atlanta.com\", nonce=\"c60f3082ee1212b402a21831ae\""
			", response=\"245f23415f11432b3434341c022\"";
	cain_sip_header_proxy_authorization_t* L_authorization = cain_sip_header_proxy_authorization_parse(l_header);
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_authorization));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_authorization));
	L_authorization = cain_sip_header_proxy_authorization_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	CU_ASSERT_PTR_NOT_NULL(L_authorization);
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_username(CAIN_SIP_HEADER_AUTHORIZATION(L_authorization)), "Alice");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_realm(CAIN_SIP_HEADER_AUTHORIZATION(L_authorization)), "atlanta.com");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_authorization_get_nonce(CAIN_SIP_HEADER_AUTHORIZATION(L_authorization)), "c60f3082ee1212b402a21831ae");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_authorization));

}
void test_header_www_authenticate(void) {
	const char* l_header = "WWW-Authenticate: Digest "
			"algorithm=MD5, realm=\"atlanta.com\", opaque=\"1bc7f9097684320\","
			" qop=\"auth\", nonce=\"c60f3082ee1212b402a21831ae\", stale=true, domain=\"sip:boxesbybob.com\"";
	cain_sip_header_www_authenticate_t* L_authorization = cain_sip_header_www_authenticate_parse(l_header);
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_authorization));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_authorization));
	L_authorization = cain_sip_header_www_authenticate_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	CU_ASSERT_PTR_NOT_NULL(L_authorization);
	CU_ASSERT_STRING_EQUAL(cain_sip_header_www_authenticate_get_realm(L_authorization), "atlanta.com");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_www_authenticate_get_domain(L_authorization), "sip:boxesbybob.com");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_www_authenticate_get_nonce(L_authorization), "c60f3082ee1212b402a21831ae");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_www_authenticate_get_algorithm(L_authorization), "MD5");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_www_authenticate_get_opaque(L_authorization), "1bc7f9097684320");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_www_authenticate_get_qop(L_authorization), "auth");
	CU_ASSERT_STRING_EQUAL(cain_sip_header_www_authenticate_get_scheme(L_authorization), "Digest");
	CU_ASSERT_EQUAL(cain_sip_header_www_authenticate_is_stale(L_authorization),1);
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_authorization));

}
void test_header_max_forwards(void) {
	const char* l_header = "Max-Forwards: 6";
	cain_sip_header_max_forwards_t* L_max_forwards = cain_sip_header_max_forwards_parse(l_header);
	cain_sip_header_max_forwards_decrement_max_forwards(L_max_forwards);
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_max_forwards));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_max_forwards));
	L_max_forwards = cain_sip_header_max_forwards_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	CU_ASSERT_PTR_NOT_NULL(L_max_forwards);
	CU_ASSERT_EQUAL(cain_sip_header_max_forwards_get_max_forwards(L_max_forwards), 5);

	cain_sip_object_unref(CAIN_SIP_OBJECT(L_max_forwards));

}
void test_header_user_agent(void) {
	const char* l_header = "User-Agent: Linphone/3.4.99.1 (eXosip2/3.3.0)";
	cain_sip_header_user_agent_t* L_user_agent = cain_sip_header_user_agent_parse(l_header);
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_user_agent));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_user_agent));
	L_user_agent = cain_sip_header_user_agent_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	const char* values[] ={"Linphone/3.4.99.1"
				,"(eXosip2/3.3.0)"};
	int i=0;
	cain_sip_list_t* products = cain_sip_header_user_agent_get_products(L_user_agent);

	for(i=0;i<2;i++){
		CU_ASSERT_PTR_NOT_NULL(products);
		CU_ASSERT_STRING_EQUAL((const char *)(products->data),values[i]);
		products=products->next;
	}

	cain_sip_object_unref(CAIN_SIP_OBJECT(L_user_agent));

}
void test_header_expires(void) {

	cain_sip_header_expires_t* L_expires = cain_sip_header_expires_parse("Expires: 3600");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_expires));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_expires));
	L_expires = cain_sip_header_expires_parse(l_raw_header);
	cain_sip_free(l_raw_header);
	CU_ASSERT_EQUAL(cain_sip_header_expires_get_expires(L_expires), 3600);
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_expires));
}
void test_header_allow(void) {
	cain_sip_header_allow_t* L_allow = cain_sip_header_allow_parse("Allow:INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
	char* l_raw_header = cain_sip_object_to_string(CAIN_SIP_OBJECT(L_allow));
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_allow));
	L_allow = cain_sip_header_allow_parse(l_raw_header);
	cain_sip_free(l_raw_header);

	CU_ASSERT_STRING_EQUAL(cain_sip_header_allow_get_method(L_allow), "INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
	cain_sip_object_unref(CAIN_SIP_OBJECT(L_allow));
}

int cain_sip_headers_test_suite() {
	
	   CU_pSuite pSuite = NULL;
	   /* add a suite to the registry */
	   pSuite = CU_add_suite("header_suite", init_suite1, clean_suite1);

	   /* add the tests to the suite */
	   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
	   if (NULL == CU_add_test(pSuite, "test of simple contact header", test_simple_header_contact)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of complex contact header", test_complex_header_contact)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of from header", test_simple_header_from)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of to header", test_simple_header_to)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of via header", test_header_via)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of call_id header", test_header_call_id)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of cseq header", test_header_cseq)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of content type header", test_header_content_type)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of record route header", test_header_record_route)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of route header", test_header_route)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of content length", test_header_content_length)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of extension", test_header_extention)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of authorization", test_header_authorization)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of proxy authorization", test_header_proxy_authorization)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of www authenticate", test_header_www_authenticate)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of max forwards", test_header_max_forwards)) {
	      return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of  user agent", test_header_user_agent)) {
	   	  return CU_get_error();
	   }
	   if (NULL == CU_add_test(pSuite, "test of  expires", test_header_expires)) {
	   	  return CU_get_error();
	   	}
	   if (NULL == CU_add_test(pSuite, "test of  allow", test_header_allow)) {
	   	  return CU_get_error();
	   	}
	   return 0;
}
