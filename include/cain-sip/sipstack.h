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


struct cain_sip_timer_config{
	int T1;
	int T2;
	int T3;
	int T4;
};

typedef struct cain_sip_timer_config cain_sip_timer_config_t;

CAIN_SIP_BEGIN_DECLS

CAINSIP_EXPORT cain_sip_stack_t * cain_sip_stack_new(const char *properties);

CAINSIP_EXPORT cain_sip_listening_point_t *cain_sip_stack_create_listening_point(cain_sip_stack_t *s, const char *ipaddress, int port, const char *transport);

CAINSIP_EXPORT void cain_sip_stack_delete_listening_point(cain_sip_stack_t *s, cain_sip_listening_point_t *lp);

CAINSIP_EXPORT cain_sip_provider_t *cain_sip_stack_create_provider(cain_sip_stack_t *s, cain_sip_listening_point_t *lp);

CAINSIP_EXPORT cain_sip_main_loop_t* cain_sip_stack_get_main_loop(cain_sip_stack_t *stack);

CAINSIP_EXPORT void cain_sip_stack_main(cain_sip_stack_t *stack);

CAINSIP_EXPORT void cain_sip_stack_sleep(cain_sip_stack_t *stack, unsigned int milliseconds);

CAINSIP_EXPORT int cain_sip_stack_get_transport_timeout(const cain_sip_stack_t *stack);

CAINSIP_EXPORT int cain_sip_stack_get_dns_timeout(const cain_sip_stack_t *stack);

CAINSIP_EXPORT void cain_sip_stack_set_dns_timeout(cain_sip_stack_t *stack, int timeout);

/**
 * Can be used to simulate network transmission delays, for tests.
**/
CAINSIP_EXPORT void cain_sip_stack_set_tx_delay(cain_sip_stack_t *stack, int delay_ms);
/**
 * Can be used to simulate network sending error, for tests.
 * @param stack
 * @param send_error if <0, will cause channel error to be reported
**/

CAINSIP_EXPORT void cain_sip_stack_set_send_error(cain_sip_stack_t *stack, int send_error);

/**
 * Can be used to simulate network transmission delays, for tests.
**/
CAINSIP_EXPORT void cain_sip_stack_set_resolver_tx_delay(cain_sip_stack_t *stack, int delay_ms);

/**
 * Can be used to simulate network sending error, for tests.
 * @param stack
 * @param send_error if <0, will cause the resolver to fail with this error code.
**/
CAINSIP_EXPORT void cain_sip_stack_set_resolver_send_error(cain_sip_stack_t *stack, int send_error);

/**
 * Returns the time interval in seconds after which a connection must be closed when inactive.
**/
CAINSIP_EXPORT int cain_sip_stack_get_inactive_transport_timeout(const cain_sip_stack_t *stack);

/**
 * Sets the time interval in seconds after which a connection must be closed when inactive.
**/
CAINSIP_EXPORT void cain_sip_stack_set_inactive_transport_timeout(cain_sip_stack_t *stack, int seconds);


/**
 * Set the default dscp value to be used for all SIP sockets created and used in the stack.
**/
CAINSIP_EXPORT void cain_sip_stack_set_default_dscp(cain_sip_stack_t *stack, int dscp);

/**
 * Get the default dscp value to be used for all SIP sockets created and used in the stack.
**/
CAINSIP_EXPORT int cain_sip_stack_get_default_dscp(cain_sip_stack_t *stack);

CAINSIP_EXPORT void cain_sip_stack_push_pool(cain_sip_stack_t *stack);

CAINSIP_EXPORT void cain_sip_stack_pop_pool(cain_sip_stack_t *stack);

CAIN_SIP_END_DECLS

#endif

