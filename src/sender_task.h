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

#ifndef sender_task_h
#define sender_task_h

typedef void (*cain_sip_sender_task_callback_t)(void *data, int retcode);

struct cain_sip_sender_task{
	cain_sip_object_t base;
	cain_sip_provider_t *provider;
	cain_sip_request_t *request;
	cain_sip_source_t *source;
	cain_sip_channel_t *channel;
	cain_sip_hop_t hop;
	struct addrinfo *dest;
	unsigned long resolver_id;
	char *buf;
	cain_sip_sender_task_callback_t cb;
	void *cb_data;
};

typedef struct cain_sip_sender_task cain_sip_sender_task_t;



cain_sip_sender_task_t * cain_sip_sender_task_new(cain_sip_provider_t *provider, cain_sip_request_t *req, cain_sip_sender_task_callback_t cb, void *data);

void cain_sip_sender_task_send(cain_sip_sender_task_t *task);


#endif

