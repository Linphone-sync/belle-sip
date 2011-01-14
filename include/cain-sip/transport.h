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
#ifndef CAIN_SIP_TRANSPORT_H
#define CAIN_SIP_TRANSPORT_H


typedef struct cain_sip_transport cain_sip_transport_t;

struct cain_sip_transport_class{
	char *name; /*udp,tcp*/
	int is_reliable;
	int (*connect)(cain_sip_transport_t *t, const struct sockaddr *addr, socklen_t addrlen);
	int (*recvfrom)(cain_sip_transport_t *t, void *buf, size_t buflen, struct sockaddr *addr, socklen_t *addrlen);
	int (*sendto)(cain_sip_transport_t *t, const void *buf, size_t buflen, const struct sockaddr *addr, socklen_t addrlen);
	/**
	 * Method to know if this transport is suitable for supplied transport name and address.
	 * Should return 1 if suitable, 0 otherwise.
	 **/
	int (*matches)(cain_sip_transport_t *, const char *name, const struct sockaddr *addr, socklen_t addrlen);
	void (*close)(cain_sip_transport_t *);
};

typedef struct cain_sip_transport_class cain_sip_transport_class_t;


struct cain_sip_transport{
	int magic;
	cain_sip_transport_class_t *klass;
	int refcnt;
};

const char *cain_sip_transport_get_name(const cain_sip_transport_t *t);
int cain_sip_transport_is_reliable(const cain_sip_transport_t *t);
int cain_sip_transport_matches(cain_sip_transport_t *t, const char *name, const struct sockaddr *addr, socklen_t addrlen);
int cain_sip_transport_sendto(cain_sip_transport_t *t, const void *buf, size_t buflen, const struct sockaddr *addr, socklen_t addrlen);
int cain_sip_transport_recvfrom(cain_sip_transport_t *t, void *buf, size_t buflen, struct sockaddr *addr, socklen_t *addrlen);
void cain_sip_transport_ref(cain_sip_transport_t *t);
void cain_sip_transport_unref(cain_sip_transport_t *t);

CAIN_SIP_DECLARE_CAST(cain_sip_transport_t);

#define CAIN_SIP_TRANSPORT(obj) CAIN_SIP_CAST(obj,cain_sip_transport_t)

#endif
