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

CAIN_SIP_DECLARE_INTERFACE_BEGIN(cain_sip_listener_t)
	void (*process_dialog_terminated)(cain_sip_listener_t *user_ctx, const cain_sip_dialog_terminated_event_t *event);
	void (*process_io_error)(cain_sip_listener_t *user_ctx, const cain_sip_io_error_event_t *event);
	void (*process_request_event)(cain_sip_listener_t *user_ctx, const cain_sip_request_event_t *event);
	void (*process_response_event)(cain_sip_listener_t *user_ctx, const cain_sip_response_event_t *event);
	void (*process_timeout)(cain_sip_listener_t *user_ctx, const cain_sip_timeout_event_t *event);
	void (*process_transaction_terminated)(cain_sip_listener_t *user_ctx, const cain_sip_transaction_terminated_event_t *event);
CAIN_SIP_DECLARE_INTERFACE_END

#define CAIN_SIP_LISTENER(obj) CAIN_SIP_INTERFACE_CAST(obj,cain_sip_listener_t)


cain_sip_response_t* cain_sip_response_event_get_response(const cain_sip_response_event_t* event);




#endif

