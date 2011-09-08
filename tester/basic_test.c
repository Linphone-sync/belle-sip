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

#include <stdio.h>
#include "cain-sip/cain-sip.h"

int main(int argc, char *argv[]){
	cain_sip_stack_t * stack=cain_sip_stack_new(NULL);
	cain_sip_listening_point_t *lp;
	cain_sip_provider_t *prov;
	cain_sip_request_t *req;

	cain_sip_set_log_level_mask(CAIN_SIP_LOG_ALL);
	
	lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",7060,"UDP");
	prov=cain_sip_stack_create_provider(stack,lp);
	req=cain_sip_request_create(
	                    cain_sip_uri_parse("sip:test.linphone.org"),
	                    "REGISTER",
	                    cain_sip_provider_get_new_call_id(prov),
	                    cain_sip_header_cseq_create(20,"REGISTER"),
	                    cain_sip_header_from_create("Tester <sip:tester@test.linphone.org>","a0dke45"),
	                    cain_sip_header_to_create("Tester <sip:tester@test.linphone.org>",NULL),
	                    cain_sip_header_via_create(cain_sip_listening_point_get_ip_address(lp),
	                                               cain_sip_listening_point_get_port(lp),
	                                               cain_sip_listening_point_get_transport(lp),"67687djeij"),
	                    70);
	char *tmp=cain_sip_object_to_string(CAIN_SIP_OBJECT(req));

	
	printf("Message to send:\n%s\n",tmp);
	cain_sip_free(tmp);
	cain_sip_provider_send_request(prov,req);
	cain_sip_main_loop_add_timeout(cain_sip_stack_get_main_loop(stack),(cain_sip_source_func_t)cain_sip_main_loop_quit,
	                               cain_sip_stack_get_main_loop(stack),5000);
	cain_sip_stack_main(stack);
	printf("Exiting\n");
	cain_sip_object_unref(CAIN_SIP_OBJECT(prov));
	cain_sip_object_unref(CAIN_SIP_OBJECT(stack));
	return 0;
}
