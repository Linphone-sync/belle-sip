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

#ifndef cain_sip_port_h
#define cain_sip_port_h

#ifndef WIN32
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>

#else

#include <ws2tcpip.h>
#include <winsock2.h>

/*AI_NUMERICSERV is not defined for windows XP. Since it is not essential, we define it to 0 (does nothing)*/
#ifndef AI_NUMERICSERV
#define AI_NUMERICSERV 0
#endif

#endif

/*
 * Socket abstraction layer
 */

int cain_sip_init_sockets(void);
void cain_sip_uninit_sockets(void);
int cain_sip_socket_set_nonblocking (cain_sip_socket_t sock);
 
#if defined(WIN32)

int cain_sip_init_sockets(void);
void cain_sip_uninit_sockets(void);

static inline void close_socket(cain_sip_socket_t s){
	closesocket(s);
}

static inline int get_socket_error(void){
	return WSAGetLastError();
}

const char *cain_sip_get_socket_error_string();
const char *cain_sip_get_socket_error_string_from_code();

#define usleep(us) Sleep((us)/1000)
static inline int inet_aton(const char *ip, struct in_addr *p){
	*(long*)p=inet_addr(ip);
	return 0;
}

#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEINPROGRESS

#else

static inline void close_socket(cain_sip_socket_t s){
	close(s);
}

static inline int get_socket_error(void){
	return errno;
}
#define cain_sip_get_socket_error_string() strerror(errno)
#define cain_sip_get_socket_error_string_from_code(code) strerror(code)

#endif

/*
 * Thread abstraction layer
 */

#ifdef WIN32

typedef HANDLE cain_sip_thread_t;
int cain_sip_thread_join(cain_sip_thread_t thread, void **retptr);
int cain_sip_thread_create(cain_sip_thread_t *thread, void *attr, void * (*routine)(void*), void *arg);

#else

#include <pthread.h>

typedef pthread_t cain_sip_thread_t;
#define cain_sip_thread_join(thread,retptr) pthread_join(thread,retptr)
#define cain_sip_thread_create(thread,attr,routine,arg) pthread_create(thread,attr,routine,arg)

#endif




#endif
