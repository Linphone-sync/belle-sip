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

#ifndef STREAM_CHANNEL_H_
#define STREAM_CHANNEL_H_
#include "channel.h"

struct cain_sip_stream_channel{
	cain_sip_channel_t base;
};

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_stream_channel_t,cain_sip_channel_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

void stream_channel_close(cain_sip_channel_t *obj);
int stream_channel_connect(cain_sip_channel_t *obj, const struct addrinfo *ai);
/*return 0 if succeed*/
int finalize_stream_connection (cain_sip_fd_t fd, struct sockaddr *addr, socklen_t* slen);
int stream_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen);
int stream_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen);
#endif /* STREAM_CHANNEL_H_ */
