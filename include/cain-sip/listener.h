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

struct cain_sip_dialog_terminated_event{
	cain_sip_provider_t *source;
	cain_sip_dialog_t *dialog;
};

struct cain_sip_io_error_event{
	cain_sip_provider_t *source;
	const char *transport;
	const char *host;
	int port;
};

struct cain_sip_request_event{
	cain_sip_provider_t *source;
	cain_sip_server_transaction_t *server_transaction;
	cain_sip_dialog_t *dialog;
	cain_sip_request_t *request;
};

struct cain_sip_response_event{
	cain_sip_provider_t *source;
	cain_sip_client_transaction_t *client_transaction;
	cain_sip_dialog_t *dialog;
	cain_sip_response_t *response;
};

struct cain_sip_timeout_event{
	cain_sip_provider_t *source;
	cain_sip_transaction_t *transaction;
	int is_server_transaction;
};

struct cain_sip_transaction_terminated_event{
	cain_sip_provider_t *source;
	cain_sip_transaction_t *transaction;
	int is_server_transaction;
};

struct cain_sip_listener{
	void (*process_dialog_terminated)(void *user_ctx, const cain_sip_dialog_terminated_event_t *event);
	void (*process_io_error)(void *user_ctx, const cain_sip_io_error_event_t *event);
	void (*process_request_event)(void *user_ctx, const cain_sip_request_event_t *event);
	void (*process_response_event)(void *user_ctx, const cain_sip_response_event_t *event);
	void (*process_timeout)(void *user_ctx, const cain_sip_timeout_event_t *event);
	void (*process_transaction_terminated)(void *user_ctx, const cain_sip_transaction_terminated_event_t *event);
};


#endif

