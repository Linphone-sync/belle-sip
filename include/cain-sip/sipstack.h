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


#ifndef cain_sip_stack_h
#define cain_sip_stack_h

struct cain_sip_hop{
	const char *host;
	const char *transport;
	int port;
};

typedef struct cain_sip_hop cain_sip_hop_t;

CAIN_SIP_BEGIN_DECLS

cain_sip_stack_t * cain_sip_stack_new(const char *properties);

cain_sip_listening_point_t *cain_sip_stack_create_listening_point(cain_sip_stack_t *s, const char *ipaddress, int port, const char *transport);

void cain_sip_stack_delete_listening_point(cain_sip_stack_t *s, cain_sip_listening_point_t *lp);

cain_sip_provider_t *cain_sip_stack_create_provider(cain_sip_stack_t *s, cain_sip_listening_point_t *lp);

void cain_sip_stack_main(cain_sip_stack_t *stack);

void cain_sip_stack_sleep(cain_sip_stack_t *stack, unsigned int milliseconds);

CAIN_SIP_END_DECLS

#endif
