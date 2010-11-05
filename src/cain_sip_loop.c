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

#include "cain-sip/cain-sip.h"
#include "cain_sip_internal.h"

struct cain_sip_source{
	cain_sip_list_t node;
	int fd;
	int revents;
	int timeout;
	void *data;
	cain_sip_source_func_t notify;
	void (*on_remove)(cain_sip_source_t *);
};

void cain_sip_source_destroy(cain_sip_source_t *obj){
	cain_sip_free(obj);
}

cain_sip_source_t * cain_sip_timeout_source_new(cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms){
	cain_sip_source_t *s=cain_sip_new0(cain_sip_source_t);
	s->fd=-1;
	s->timeout=timeout_value_ms;
	s->data=data;
	s->notify=func;
	return s;
}


struct cain_sip_main_loop{
	cain_sip_source_t *sources;
};


cain_sip_main_loop_t *cain_sip_main_loop_new(void){
	cain_sip_main_loop_t*m=cain_sip_new0(cain_sip_main_loop_t);
	return m;
}

void cain_sip_main_loop_add_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source){
	if (source->node.next || source->node.prev){
		cain_sip_fatal("Source is already linked somewhere else.");
		return;
	}
	ml->sources=(cain_sip_source_t*)cain_sip_list_append_link((cain_sip_list_t*)ml->sources,(cain_sip_list_t*)source);
}

void cain_sip_main_loop_remove_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source){
	ml->sources=(cain_sip_source_t*)cain_sip_list_remove_link((cain_sip_list_t*)ml->sources,(cain_sip_list_t*)source);
}

void cain_sip_main_loop_add_timeout(cain_sip_main_loop_t *ml, cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms){
	cain_sip_source_t * s=cain_sip_timeout_source_new(func,data,timeout_value_ms);
	s->on_remove=cain_sip_source_destroy;
	cain_sip_main_loop_add_source(ml,s);
}
