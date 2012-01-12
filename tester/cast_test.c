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
	cain_sip_stack_t *stack=cain_sip_stack_new(NULL);
	cain_sip_listening_point_t *lp=cain_sip_stack_create_listening_point(stack,"0.0.0.0",5060,"UDP");
	cain_sip_provider_t *provider=cain_sip_stack_create_provider(stack,lp);
	cain_sip_server_transaction_t *st=cain_sip_provider_get_new_server_transaction(provider,NULL);
	cain_sip_client_transaction_t *ct=cain_sip_provider_get_new_client_transaction(provider,NULL);
	cain_sip_transaction_t *t;
	
	printf("Casting cain_sip_server_transaction_t to cain_sip_transaction_t\n");
	t=CAIN_SIP_TRANSACTION(st);
	printf("Ok.\n");
	printf("Casting cain_sip_client_transaction_t to cain_sip_transaction_t\n");
	t=CAIN_SIP_TRANSACTION(ct);
	printf("Ok.\n");
	printf("Casting cain_sip_server_transaction_t to cain_sip_client_transaction_t\n");
	ct=CAIN_SIP_CLIENT_TRANSACTION(st);
	printf("Ooops we should have aborted before, t=%p\n",t);
	return 0;
}
