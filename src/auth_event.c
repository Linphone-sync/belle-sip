/*
	auth_info.c cain-sip - SIP (RFC3261) library.
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


GET_SET_STRING(cain_sip_auth_event,username)

GET_SET_STRING(cain_sip_auth_event,userid)
GET_SET_STRING(cain_sip_auth_event,realm)
GET_SET_STRING(cain_sip_auth_event,passwd)
GET_SET_STRING(cain_sip_auth_event,ha1)
cain_sip_auth_event_t* cain_sip_auth_event_create(const char* realm,const char* username) {
	cain_sip_auth_event_t* result = cain_sip_new(cain_sip_auth_event_t);
	memset(result,0,sizeof(cain_sip_auth_event_t));
	cain_sip_auth_event_set_realm(result,realm);
	cain_sip_auth_event_set_username(result,username);
	return result;
}
void cain_sip_auth_event_destroy(cain_sip_auth_event_t* event) {
	DESTROY_STRING(event,username);
	DESTROY_STRING(event,userid);
	DESTROY_STRING(event,realm);
	DESTROY_STRING(event,passwd);
	DESTROY_STRING(event,ha1);
	cain_sip_free(event);
}
