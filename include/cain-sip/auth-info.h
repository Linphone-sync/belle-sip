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

#ifndef AUTH_INFO_H_
#define AUTH_INFO_H_

typedef struct cain_sip_auth_info cain_sip_auth_info_t;

const char* cain_sip_auth_info_get_username(const cain_sip_auth_info_t info);
void cain_sip_auth_info_set_username(cain_sip_auth_info_t info, const char* value);

const char* cain_sip_auth_info_get_userid(const cain_sip_auth_info_t info);
void cain_sip_auth_info_set_userid(cain_sip_auth_info_t info, const char* value);

const char* cain_sip_auth_info_get_realm(const cain_sip_auth_info_t info);
void cain_sip_auth_info_set_realm(cain_sip_auth_info_t info, const char* value);

const char* cain_sip_auth_info_get_passwd(const cain_sip_auth_info_t info);
void cain_sip_auth_info_set_passwd(cain_sip_auth_info_t info, const char* value);

const char* cain_sip_auth_info_get_ha1(const cain_sip_auth_info_t info);
void cain_sip_auth_info_set_ha1(cain_sip_auth_info_t info, const char* value);

#endif /* AUTH_INFO_H_ */
