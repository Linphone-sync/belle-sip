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
#define CAIN_SIP_EVENT_ERROR (1<<1)

typedef struct cain_sip_source cain_sip_source_t;

typedef int (*cain_sip_source_func_t)(void *user_data, int events);

cain_sip_source_t * cain_sip_timeout_source_new(cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms);

typedef struct cain_sip_main_loop cain_sip_main_loop_t;

cain_sip_main_loop_t *cain_sip_main_loop_new(void);

void cain_sip_main_loop_add_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source);

void cain_sip_main_loop_remove_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source);

void cain_sip_main_loop_add_timeout(cain_sip_main_loop_t *ml, cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms);

#endif
