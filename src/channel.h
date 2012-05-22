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
#ifndef CAIN_SIP_CHANNEL_H
#define CAIN_SIP_CHANNEL_H

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#else

#endif

static const int cain_sip_network_buffer_size=65535;

typedef enum cain_sip_channel_state{
	CAIN_SIP_CHANNEL_INIT,
	CAIN_SIP_CHANNEL_RES_IN_PROGRESS,
	CAIN_SIP_CHANNEL_RES_DONE,
	CAIN_SIP_CHANNEL_CONNECTING,
	CAIN_SIP_CHANNEL_READY,
	CAIN_SIP_CHANNEL_ERROR,
	CAIN_SIP_CHANNEL_DISCONNECTED
}cain_sip_channel_state_t;

const char * cain_sip_channel_state_to_string(cain_sip_channel_state_t state);

/**
* cain_sip_channel_t is an object representing a single communication channel ( socket or file descriptor), 
* unlike the cain_sip_listening_point_t that can owns several channels for TCP or TLS (incoming server child sockets or 
* outgoing client sockets).
**/
typedef struct cain_sip_channel cain_sip_channel_t;

CAIN_SIP_DECLARE_INTERFACE_BEGIN(cain_sip_channel_listener_t)
void (*on_state_changed)(cain_sip_channel_listener_t *l, cain_sip_channel_t *, cain_sip_channel_state_t state);
int (*on_event)(cain_sip_channel_listener_t *l, cain_sip_channel_t *obj, unsigned revents);
void (*on_sending)(cain_sip_channel_listener_t *l, cain_sip_channel_t *obj, cain_sip_message_t *msg);
CAIN_SIP_DECLARE_INTERFACE_END

#define CAIN_SIP_CHANNEL_LISTENER(obj) CAIN_SIP_INTERFACE_CAST(obj,cain_sip_channel_listener_t)
#define MAX_CHANNEL_BUFF_SIZE 64000 + 1500 + 1

typedef enum input_stream_state {
	WAITING_MESSAGE_START=0
	,MESSAGE_AQUISITION=1
	,BODY_AQUISITION=2
}input_stream_state_t;

typedef struct cain_sip_channel_input_stream{
	input_stream_state_t state;
	char buff[MAX_CHANNEL_BUFF_SIZE];
	char* read_ptr;
	char* write_ptr;
	cain_sip_message_t *msg;
}cain_sip_channel_input_stream_t;

typedef struct cain_sip_stream_channel cain_sip_stream_channel_t;
typedef struct cain_sip_tls_channel cain_sip_tls_channel_t;

struct cain_sip_channel{
	cain_sip_source_t base;
	cain_sip_stack_t *stack;
	cain_sip_channel_state_t state;
	cain_sip_list_t *listeners;
	char *peer_name;
	int peer_port;
	char *local_ip;
	int local_port;
	int prepare;
	unsigned long resolver_id;
	struct addrinfo *peer;
	cain_sip_message_t *msg;
	cain_sip_list_t* incoming_messages;
	cain_sip_channel_input_stream_t input_stream;
};

#define CAIN_SIP_CHANNEL(obj)		CAIN_SIP_CAST(obj,cain_sip_channel_t)


void cain_sip_channel_add_listener(cain_sip_channel_t *chan, cain_sip_channel_listener_t *l);

void cain_sip_channel_remove_listener(cain_sip_channel_t *obj, cain_sip_channel_listener_t *l);

int cain_sip_channel_matches(const cain_sip_channel_t *obj, const char *peername, int peerport, const struct addrinfo *addr);

void cain_sip_channel_resolve(cain_sip_channel_t *obj);

void cain_sip_channel_connect(cain_sip_channel_t *obj);

void cain_sip_channel_prepare(cain_sip_channel_t *obj);

int cain_sip_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen);

int cain_sip_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen);
/*only used by channels implementation*/
void cain_sip_channel_set_ready(cain_sip_channel_t *obj, const struct sockaddr *addr, socklen_t slen);
void cain_sip_channel_init(cain_sip_channel_t *obj, cain_sip_stack_t *stack, int fd, cain_sip_source_func_t process_data, const char *bindip,int localport,const char *peername, int peer_port);
/*end of channel implementations*/
/**
 * pickup last received message. This method take the ownership of the message.
 */
cain_sip_message_t* cain_sip_channel_pick_message(cain_sip_channel_t *obj);

int cain_sip_channel_queue_message(cain_sip_channel_t *obj, cain_sip_message_t *msg);

int cain_sip_channel_is_reliable(const cain_sip_channel_t *obj);

const char * cain_sip_channel_get_transport_name(const cain_sip_channel_t *obj);
const char * cain_sip_channel_get_transport_name_lower_case(const cain_sip_channel_t *obj);

const struct addrinfo * cain_sip_channel_get_peer(cain_sip_channel_t *obj);

const char *cain_sip_channel_get_local_address(cain_sip_channel_t *obj, int *port);

#define cain_sip_channel_get_state(chan) ((chan)->state)

void channel_set_state(cain_sip_channel_t *obj, cain_sip_channel_state_t state);

void channel_process_queue(cain_sip_channel_t *obj);

/*just invokes the listeners to process data*/
void cain_sip_channel_process_data(cain_sip_channel_t *obj,unsigned int revents);

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_channel_t,cain_sip_source_t)
	const char *transport;
	int reliable;
	int (*connect)(cain_sip_channel_t *obj, const struct sockaddr *, socklen_t socklen);
	int (*channel_send)(cain_sip_channel_t *obj, const void *buf, size_t buflen);
	int (*channel_recv)(cain_sip_channel_t *obj, void *buf, size_t buflen);
CAIN_SIP_DECLARE_CUSTOM_VPTR_END





#endif
