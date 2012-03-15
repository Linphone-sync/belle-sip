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

#ifndef CAIN_SIP_UTILS_H
#define CAIN_SIP_UTILS_H

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

/***************/
/* logging api */
/***************/

typedef enum {
	CAIN_SIP_LOG_FATAL=1,
	CAIN_SIP_LOG_ERROR=1<<1,
	CAIN_SIP_LOG_WARNING=1<<2,
        CAIN_SIP_LOG_MESSAGE=1<<3,
        CAIN_SIP_LOG_DEBUG=1<<4,
        CAIN_SIP_LOG_END=1<<5
} cain_sip_log_level;


typedef void (*cain_sip_log_function_t)(cain_sip_log_level lev, const char *fmt, va_list args);

CAIN_SIP_BEGIN_DECLS

extern cain_sip_log_function_t cain_sip_logv_out;

extern unsigned int __cain_sip_log_mask;

#define cain_sip_log_level_enabled(level)   (__cain_sip_log_mask & (level))

#if !defined(WIN32) && !defined(_WIN32_WCE)
#define cain_sip_logv(level,fmt,args) \
{\
        if (cain_sip_logv_out!=NULL && cain_sip_log_level_enabled(level)) \
                cain_sip_logv_out(level,fmt,args);\
        if ((level)==CAIN_SIP_LOG_FATAL) abort();\
}while(0)
#else
void cain_sip_logv(int level, const char *fmt, va_list args);
#endif


#ifdef CAIN_SIP_DEBUG_MODE
static inline void cain_sip_debug(const char *fmt,...)
{
  va_list args;
  va_start (args, fmt);
  cain_sip_logv(CAIN_SIP_LOG_DEBUG, fmt, args);
  va_end (args);
}
#else

#define cain_sip_debug(...)

#endif

#ifdef CAIN_SIP_NOMESSAGE_MODE

#define cain_sip_log(...)
#define cain_sip_message(...)
#define cain_sip_warning(...)

#else

static inline void cain_sip_log(cain_sip_log_level lev, const char *fmt,...){
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(lev, fmt, args);
        va_end (args);
}

static inline void cain_sip_message(const char *fmt,...)
{
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(CAIN_SIP_LOG_MESSAGE, fmt, args);
        va_end (args);
}

static inline void cain_sip_warning(const char *fmt,...)
{
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(CAIN_SIP_LOG_WARNING, fmt, args);
        va_end (args);
}

#endif

static inline void cain_sip_error(const char *fmt,...)
{
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(CAIN_SIP_LOG_ERROR, fmt, args);
        va_end (args);
}

static inline void cain_sip_fatal(const char *fmt,...)
{
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(CAIN_SIP_LOG_FATAL, fmt, args);
        va_end (args);
}



void cain_sip_set_log_file(FILE *file);
void cain_sip_set_log_handler(cain_sip_log_function_t func);


void cain_sip_set_log_level(int level);

char * cain_sip_random_token(char *ret, size_t size);

#if defined(WIN32) || defined(WIN32_WCE)

typedef SOCKET cain_sip_fd_t;
static inline void close_socket(cain_sip_fd_t s){
	closesocket(s);
}

static inline int get_socket_error(void){
	return WSAGetLastError();
}

const char *getSocketErrorString();
#define cain_sip_get_socket_error_string() getSocketErrorString()
#define cain_sip_get_socket_error_string_from_code(code) getSocketErrorString()
#define usleep(us) Sleep((us)/1000)
static inline int inet_aton(const char *ip, struct in_addr *p){
	*(long*)p=inet_addr(ip);
	return 0;
}

#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEINPROGRESS

#else

typedef int cain_sip_fd_t;
static inline void close_socket(cain_sip_fd_t s){
	close(s);
}

static inline int get_socket_error(void){
	return errno;
}

#define cain_sip_get_socket_error_string() strerror(errno)
#define cain_sip_get_socket_error_string_from_code(code) strerror(code)

#endif

CAIN_SIP_END_DECLS

#endif

