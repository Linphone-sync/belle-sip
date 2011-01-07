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

#include "cain_sip_internal.h"

int cain_sip_message_is_request(cain_sip_message_t *msg){
	return 0;
}

int cain_sip_message_is_response(cain_sip_message_t *msg){
	return 0;
}

cain_sip_header_t *cain_sip_message_get_header_last(cain_sip_message_t *msg, const char *header_name){
	return NULL;
}

char *cain_sip_message_to_string(cain_sip_message_t *msg){
	return NULL;
}
