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

#ifndef CAIN_SIP_MAINLOOP_H
#define CAIN_SIP_MAINLOOP_H


#define CAIN_SIP_EVENT_READ 1
#define CAIN_SIP_EVENT_WRITE (1<<1)
#define CAIN_SIP_EVENT_ERROR (1<<2)
#define CAIN_SIP_EVENT_TIMEOUT (1<<3)

typedef struct cain_sip_source cain_sip_source_t;


/**
 * Callback function prototype for main loop notifications.
 * Return value is important:
 * 0 => source is removed from main loop.
 * non zero value => source is kept.
**/
typedef int (*cain_sip_source_func_t)(void *user_data, unsigned int events);

typedef struct cain_sip_main_loop cain_sip_main_loop_t;

#define CAIN_SIP_CONTINUE	TRUE
#define CAIN_SIP_STOP			FALSE

CAIN_SIP_BEGIN_DECLS

void cain_sip_main_loop_add_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source);

/**
 * Creates a mainloop.
**/
cain_sip_main_loop_t *cain_sip_main_loop_new(void);

/**
 * Adds a timeout into the main loop
 * @param ml
 * @param func a callback function to be called to notify timeout expiration
 * @param data a pointer to be passed to the callback
 * @param timeout_value_ms duration of the timeout.
 * @returns timeout id
**/
unsigned long cain_sip_main_loop_add_timeout(cain_sip_main_loop_t *ml, cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms);


/**
 * Creates a timeout source, similarly to cain_sip_main_loop_add_timeout().
 * However in this case the timeout must be entered manually using cain_sip_main_loop_add_source().
 * Its pointer can be used to remove it from the source (that is cancelling it).
**/
cain_sip_source_t * cain_sip_timeout_source_new(cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms);


cain_sip_source_t * cain_sip_fd_source_new(cain_sip_source_func_t func, void *data, int fd, unsigned int events, unsigned int timeout_value_ms);

unsigned long cain_sip_source_get_id(cain_sip_source_t *s);

/**
 * Executes the main loop forever (or until cain_sip_main_loop_quit() is called)
**/
void cain_sip_main_loop_run(cain_sip_main_loop_t *ml);

/**
 * Executes the main loop for the time specified in milliseconds.
**/
void cain_sip_main_loop_sleep(cain_sip_main_loop_t *ml, int milliseconds);

/**
 * Break out the main loop.
**/
void cain_sip_main_loop_quit(cain_sip_main_loop_t *ml);

/**
 * Cancel (removes) a source. It is not freed.
**/
void cain_sip_main_loop_cancel_source(cain_sip_main_loop_t *ml, unsigned long id);

CAIN_SIP_END_DECLS

#endif
