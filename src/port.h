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

#include <sys/stat.h>

#ifndef WIN32
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>

#else

#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef _MSC_VER
typedef unsigned long long uint64_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
#define strcasecmp(a,b) _stricmp(a,b)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strdup _strdup
#else
#include <stdint.h>
#endif

/*AI_NUMERICSERV is not defined for windows XP. Since it is not essential, we define it to 0 (does nothing)*/
#ifndef AI_NUMERICSERV
#define AI_NUMERICSERV 0
#endif

#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)
#ifdef CAINSIP_INTERNAL_EXPORTS
#define CAINSIP_INTERNAL_EXPORT __declspec(dllexport)
#else
#define CAINSIP_INTERNAL_EXPORT
#endif
#else
#define CAINSIP_INTERNAL_EXPORT extern
#endif

/*
 * Socket abstraction layer
 */

CAINSIP_INTERNAL_EXPORT int cain_sip_init_sockets(void);
CAINSIP_INTERNAL_EXPORT void cain_sip_uninit_sockets(void);
int cain_sip_socket_set_nonblocking (cain_sip_socket_t sock);
int cain_sip_socket_set_dscp(cain_sip_socket_t sock, int ai_family, int dscp);
 
#if defined(WIN32)

typedef HANDLE cain_sip_thread_t;

#define cain_sip_thread_self()		GetCurrentThread()

typedef DWORD cain_sip_thread_key_t;
int cain_sip_thread_key_create(cain_sip_thread_key_t *key, void (*destructor)(void*) );
int cain_sip_thread_setspecific(cain_sip_thread_key_t key,const void *value);
const void* cain_sip_thread_getspecific(cain_sip_thread_key_t key);
int cain_sip_thread_key_delete(cain_sip_thread_key_t key);


static CAINSIP_INLINE void close_socket(cain_sip_socket_t s){
	closesocket(s);
}

static CAINSIP_INLINE int get_socket_error(void){
	return WSAGetLastError();
}

const char *cain_sip_get_socket_error_string();
const char *cain_sip_get_socket_error_string_from_code(int code);

#ifdef WINAPI_FAMILY_PHONE_APP
void WINAPI Sleep(DWORD ms);
#define cain_sip_sleep Sleep
#else
#define cain_sip_sleep Sleep
#endif

#define usleep(us) cain_sip_sleep((us)/1000)
static CAINSIP_INLINE int inet_aton(const char *ip, struct in_addr *p){
	*(long*)p=inet_addr(ip);
	return 0;
}

#define CAINSIP_EWOULDBLOCK WSAEWOULDBLOCK
#define CAINSIP_EINPROGRESS WSAEINPROGRESS

#else

typedef pthread_t cain_sip_thread_t;
#define cain_sip_thread_self()			pthread_self()

typedef pthread_key_t cain_sip_thread_key_t;
#define cain_sip_thread_key_create(key,destructor)		pthread_key_create(key,destructor)
#define cain_sip_thread_setspecific(key,value)			pthread_setspecific(key,value)
#define cain_sip_thread_getspecific(key)			pthread_getspecific(key)
#define cain_sip_thread_key_delete(key)				pthread_key_delete(key)

static CAINSIP_INLINE void close_socket(cain_sip_socket_t s){
	close(s);
}

static CAINSIP_INLINE int get_socket_error(void){
	return errno;
}
#define cain_sip_get_socket_error_string() strerror(errno)
#define cain_sip_get_socket_error_string_from_code(code) strerror(code)

#define CAINSIP_EWOULDBLOCK EWOULDBLOCK
#define CAINSIP_EINPROGRESS EINPROGRESS

#endif

#define cain_sip_error_code_is_would_block(err) ((err)==CAINSIP_EWOULDBLOCK || (err)==CAINSIP_EINPROGRESS)

#endif

