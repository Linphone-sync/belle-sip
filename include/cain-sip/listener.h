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


#ifndef cain_sip_listener_h
#define cain_sip_listener_h

typedef struct cain_sip_dialog_terminated_event cain_sip_dialog_terminated_event_t;
typedef struct cain_sip_io_error_event cain_sip_io_error_event_t;
typedef struct cain_sip_request_event cain_sip_request_event_t;
typedef struct cain_sip_response_event cain_sip_response_event_t;
typedef struct cain_sip_timeout_event cain_sip_timeout_event_t;
typedef struct cain_sip_transaction_terminated_event cain_sip_transaction_terminated_event_t;
typedef struct cain_sip_auth_event cain_sip_auth_event_t;


CAIN_SIP_DECLARE_INTERFACE_BEGIN(cain_sip_listener_t)
	void (*process_dialog_terminated)(cain_sip_listener_t *user_ctx, const cain_sip_dialog_terminated_event_t *event);
	void (*process_io_error)(cain_sip_listener_t *user_ctx, const cain_sip_io_error_event_t *event);
	void (*process_request_event)(cain_sip_listener_t *user_ctx, const cain_sip_request_event_t *event);
	void (*process_response_event)(cain_sip_listener_t *user_ctx, const cain_sip_response_event_t *event);
	void (*process_timeout)(cain_sip_listener_t *user_ctx, const cain_sip_timeout_event_t *event);
	void (*process_transaction_terminated)(cain_sip_listener_t *user_ctx, const cain_sip_transaction_terminated_event_t *event);
	void (*process_auth_requested)(cain_sip_listener_t *user_ctx, cain_sip_auth_event_t *event);
CAIN_SIP_DECLARE_INTERFACE_END

#define CAIN_SIP_LISTENER(obj) CAIN_SIP_INTERFACE_CAST(obj,cain_sip_listener_t)

/*Response event*/
cain_sip_response_t* cain_sip_response_event_get_response(const cain_sip_response_event_t* event);
cain_sip_client_transaction_t *cain_sip_response_event_get_client_transaction(const cain_sip_response_event_t* event);
cain_sip_dialog_t *cain_sip_response_event_get_dialog(const cain_sip_response_event_t* event);

/*Request event*/
cain_sip_request_t* cain_sip_request_event_get_request(const cain_sip_request_event_t* event);
cain_sip_server_transaction_t *cain_sip_request_event_get_server_transaction(const cain_sip_request_event_t* event);
cain_sip_dialog_t *cain_sip_request_event_get_dialog(const cain_sip_request_event_t* event);

/*Dialog terminated event*/
cain_sip_dialog_t* cain_sip_dialog_terminated_get_dialog(const cain_sip_dialog_terminated_event_t *event);

/*auth event*/
void cain_sip_auth_event_destroy(cain_sip_auth_event_t* event);
const char* cain_sip_auth_event_get_username(const cain_sip_auth_event_t* event);
void cain_sip_auth_event_set_username(cain_sip_auth_event_t* event, const char* value);

const char* cain_sip_auth_event_get_userid(const cain_sip_auth_event_t* event);
void cain_sip_auth_event_set_userid(cain_sip_auth_event_t* event, const char* value);

const char* cain_sip_auth_event_get_realm(const cain_sip_auth_event_t* event);
void cain_sip_auth_event_set_realm(cain_sip_auth_event_t* event, const char* value);

const char* cain_sip_auth_event_get_passwd(const cain_sip_auth_event_t* event);
void cain_sip_auth_event_set_passwd(cain_sip_auth_event_t* event, const char* value);

const char* cain_sip_auth_event_get_ha1(const cain_sip_auth_event_t* event);
void cain_sip_auth_event_set_ha1(cain_sip_auth_event_t* event, const char* value);

/*Io error event*/
/*
 * Give access to the remote host
 * @param event object
 * @return host value the socket is pointing to
 * */
const char* cain_sip_io_error_event_get_host(const cain_sip_io_error_event_t* event);
/*
 * Give access to the used transport
 * @param event object
 * @return host value the socket is pointing to
 * */
const char* cain_sip_io_error_event_get_transport(const cain_sip_io_error_event_t* event);
/*
 * Give access to the remote port
 * @param event object
 * @return port value the socket is pointing to
 * */
unsigned int cain_sip_io_error_event_port(const cain_sip_io_error_event_t* event);

/*
 * Get access to the object involved in this error, can be either cain_sip_dialog_t or cain_sip_transaction_t or cain_sip_provider_t
 * @param event
 * @return cain_sip_object_t source, use cain_sip_object_is_instance_of to check returns type
 * */

cain_sip_object_t* cain_sip_io_error_event_get_source(const cain_sip_io_error_event_t* event);


struct cain_sip_listener_callbacks{
	void (*process_dialog_terminated)(void *user_ctx, const cain_sip_dialog_terminated_event_t *event);
	void (*process_io_error)(void *user_ctx, const cain_sip_io_error_event_t *event);
	void (*process_request_event)(void *user_ctx, const cain_sip_request_event_t *event);
	void (*process_response_event)(void *user_ctx, const cain_sip_response_event_t *event);
	void (*process_timeout)(void *user_ctx, const cain_sip_timeout_event_t *event);
	void (*process_transaction_terminated)(void *user_ctx, const cain_sip_transaction_terminated_event_t *event);
	void (*process_auth_requested)(void *user_ctx, cain_sip_auth_event_t *auth_event);
	void (*listener_destroyed)(void *user_ctx);
};

typedef struct cain_sip_listener_callbacks cain_sip_listener_callbacks_t;

/**
 * Creates an object implementing the cain_sip_listener_t interface.
 * This object passes the events to the callbacks, providing also the user context.
**/
cain_sip_listener_t *cain_sip_listener_create_from_callbacks(const cain_sip_listener_callbacks_t *callbacks, void *user_ctx);

#endif

