/*
	auth_helper.c cain-sip - SIP (RFC3261) library.
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
#include "cain_sip_internal.h"
#include "md5.h"
#include <string.h>

#define CHECK_IS_PRESENT(obj,header_name,name) \
	if (!cain_sip_header_##header_name##_get_##name(obj)) {\
		 cain_sip_error("parameter ["#name"]not found for header ["#header_name);\
		 return-1;\
	}

static void cain_sip_auth_helper_clone_authorization(cain_sip_header_authorization_t* authorization, const cain_sip_header_www_authenticate_t* authentication) {
	CLONE_STRING_GENERIC(cain_sip_header_www_authenticate,cain_sip_header_authorization,scheme,authorization,authentication)
	CLONE_STRING_GENERIC(cain_sip_header_www_authenticate,cain_sip_header_authorization,realm,authorization,authentication)
	CLONE_STRING_GENERIC(cain_sip_header_www_authenticate,cain_sip_header_authorization,nonce,authorization,authentication)
	CLONE_STRING_GENERIC(cain_sip_header_www_authenticate,cain_sip_header_authorization,algorithm,authorization,authentication)
	CLONE_STRING_GENERIC(cain_sip_header_www_authenticate,cain_sip_header_authorization,opaque,authorization,authentication)
}
cain_sip_header_authorization_t* cain_sip_auth_helper_create_authorization(const cain_sip_header_www_authenticate_t* authentication) {
	cain_sip_header_authorization_t* authorization = cain_sip_header_authorization_new();
	cain_sip_auth_helper_clone_authorization(authorization,authentication);
	return authorization;
}
cain_sip_header_proxy_authorization_t* cain_sip_auth_helper_create_proxy_authorization(const cain_sip_header_proxy_authenticate_t* proxy_authentication){
	cain_sip_header_proxy_authorization_t* authorization = cain_sip_header_proxy_authorization_new();
	cain_sip_auth_helper_clone_authorization(CAIN_SIP_HEADER_AUTHORIZATION(authorization),CAIN_SIP_HEADER_WWW_AUTHENTICATE(proxy_authentication));
	return authorization;
}
int cain_sip_auth_helper_fill_authorization(cain_sip_header_authorization_t* authorization
										,const char* method
										,const char* username
										,const char* password) {

	md5_byte_t out[16];
	md5_state_t state;
	char* uri;
	char ha1[16*2 + 1];
	char ha2[16*2 + 1];
	char response[16*2 + 1];
	int di;
	if (cain_sip_header_authorization_get_scheme(authorization) != NULL &&
		strcmp("Digest",cain_sip_header_authorization_get_scheme(authorization))!=0) {
		cain_sip_error("cain_sip_fill_authorization_header, unsupported schema [%s]"
						,cain_sip_header_authorization_get_scheme(authorization));
		return -1;
	}
	if (cain_sip_header_authorization_get_qop(authorization)
		&& strcmp("auth",cain_sip_header_authorization_get_qop(authorization))!=0) {
		cain_sip_error("cain_sip_fill_authorization_header, unsupported qop [%s], use auth instead"
								,cain_sip_header_authorization_get_qop(authorization));
		return -1;
	}
	CHECK_IS_PRESENT(authorization,authorization,realm)
	CHECK_IS_PRESENT(authorization,authorization,nonce)
	CHECK_IS_PRESENT(authorization,authorization,uri)
	if (!method) {
		 cain_sip_error("cain_sip_fill_authorization_header, method not found ");
		 return -1;
	}
	if (!username) {
		 cain_sip_error("cain_sip_fill_authorization_header, username not found ");
		 return -1;
	}
	if (!password) {
		 cain_sip_error("cain_sip_fill_authorization_header, password not found ");
		 return -1;
	}
	/*HA1=MD5(username:realm:passwd)*/

	md5_init(&state);
	md5_append(&state,(const md5_byte_t *)username,strlen(username));
	md5_append(&state,(const md5_byte_t *)":",1);
	md5_append(&state
			,(const md5_byte_t *)cain_sip_header_authorization_get_realm(authorization)
			,strlen(cain_sip_header_authorization_get_realm(authorization)));
	md5_append(&state,(const md5_byte_t *)":",1);
	md5_append(&state,(const md5_byte_t *)password,strlen(password));
	md5_finish(&state,out);
	for (di = 0; di < 16; ++di)
			    sprintf(ha1 + di * 2, "%02x", out[di]);

	/*HA2=MD5(method:uri)*/
	md5_init(&state);
	md5_append(&state,(const md5_byte_t *)method,strlen(method));
	md5_append(&state,(const md5_byte_t *)":",1);
	uri=cain_sip_uri_to_string(cain_sip_header_authorization_get_uri(authorization));
	md5_append(&state,(const md5_byte_t *)uri,strlen(uri));
	md5_finish(&state,out);
	cain_sip_free(uri);
	for (di = 0; di < 16; ++di)
				    sprintf(ha2 + di * 2, "%02x", out[di]);
	/*response=MD5(HA1:nonce:HA2)*/
	md5_init(&state);
	md5_append(&state,(const md5_byte_t *)ha1,strlen(ha1));
	md5_append(&state,(const md5_byte_t *)":",1);
	md5_append(&state
			,(const md5_byte_t *)cain_sip_header_authorization_get_nonce(authorization)
			,strlen(cain_sip_header_authorization_get_nonce(authorization)));
	md5_append(&state,(const md5_byte_t *)":",1);
	md5_append(&state,(const md5_byte_t *)ha2,strlen(ha2));
	md5_finish(&state,out);
	/*copy values*/
	for (di = 0; di < 16; ++di)
		    sprintf(response + di * 2, "%02x", out[di]);

	cain_sip_header_authorization_set_response(authorization,(const char*)response);
	cain_sip_header_authorization_set_qop(authorization,"auth");
	return 0;
}

int cain_sip_auth_helper_fill_proxy_authorization(cain_sip_header_proxy_authorization_t* proxy_authorization
												,const char* method
												,const char* username
												,const char* password) {
	return cain_sip_auth_helper_fill_authorization(CAIN_SIP_HEADER_AUTHORIZATION(proxy_authorization)
													,method
													,username
													,password);


}

