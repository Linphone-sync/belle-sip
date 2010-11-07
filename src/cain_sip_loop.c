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

#include <unistd.h>
#include <poll.h>

struct cain_sip_source{
	cain_sip_list_t node;
	int fd;
	unsigned int events;
	int timeout;
	void *data;
	uint64_t expire_ms;
	int index; /* index in pollfd table */
	cain_sip_source_func_t notify;
	void (*on_remove)(cain_sip_source_t *);
};

void cain_sip_source_destroy(cain_sip_source_t *obj){
	if (obj->node.next || obj->node.prev){
		cain_sip_fatal("Destroying source currently used in main loop !");
	}
	cain_sip_free(obj);
}

static cain_sip_source_t * cain_sip_fd_source_new(cain_sip_source_func_t func, void *data, int fd, unsigned int events, unsigned int timeout_value_ms){
	cain_sip_source_t *s=cain_sip_new0(cain_sip_source_t);
	s->fd=fd;
	s->events=events;
	s->timeout=timeout_value_ms;
	s->data=data;
	s->notify=func;
	return s;
}

cain_sip_source_t * cain_sip_timeout_source_new(cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms){
	return cain_sip_fd_source_new(func,data,-1,0,timeout_value_ms);
}

struct cain_sip_main_loop{
	cain_sip_source_t *sources;
	cain_sip_source_t *control;
	int nsources;
	int run;
	int control_fds[2];
};

static int main_loop_done(void *data, unsigned int events){
	cain_sip_debug("Got data on control fd...");
	return TRUE;
}

cain_sip_main_loop_t *cain_sip_main_loop_new(void){
	cain_sip_main_loop_t*m=cain_sip_new0(cain_sip_main_loop_t);
	if (pipe(m->control_fds)==-1){
		cain_sip_fatal("Could not create control pipe.");
	}
	m->control=cain_sip_fd_source_new (main_loop_done,NULL,m->control_fds[0],CAIN_SIP_EVENT_READ,-1);
	cain_sip_main_loop_add_source(m,m->control);
	return m;
}

void cain_sip_main_loop_add_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source){
	if (source->node.next || source->node.prev){
		cain_sip_fatal("Source is already linked somewhere else.");
		return;
	}
	if (source->timeout>0){
		source->expire_ms=cain_sip_time_ms()+source->timeout;
	}
		
	ml->sources=(cain_sip_source_t*)cain_sip_list_append_link((cain_sip_list_t*)ml->sources,(cain_sip_list_t*)source);
	ml->nsources++;
}

void cain_sip_main_loop_remove_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source){
	ml->sources=(cain_sip_source_t*)cain_sip_list_remove_link((cain_sip_list_t*)ml->sources,(cain_sip_list_t*)source);
	ml->nsources--;
	if (source->on_remove)
		source->on_remove(source);
}

void cain_sip_main_loop_add_timeout(cain_sip_main_loop_t *ml, cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms){
	cain_sip_source_t * s=cain_sip_timeout_source_new(func,data,timeout_value_ms);
	s->on_remove=cain_sip_source_destroy;
	cain_sip_main_loop_add_source(ml,s);
}

/*
 Poll() based implementation of event loop.
 */

static int cain_sip_event_to_poll(unsigned int events){
	int ret=0;
	if (events & CAIN_SIP_EVENT_READ)
		ret|=POLLIN;
	if (events & CAIN_SIP_EVENT_WRITE)
		ret|=POLLOUT;
	if (events & CAIN_SIP_EVENT_ERROR)
		ret|=POLLERR;
	return ret;
}

static unsigned int cain_sip_poll_to_event(short events){
	unsigned int ret=0;
	if (events & POLLIN)
		ret|=CAIN_SIP_EVENT_READ;
	if (events & POLLOUT)
		ret|=CAIN_SIP_EVENT_WRITE;
	if (events & POLLERR)
		ret|=CAIN_SIP_EVENT_ERROR;
	return ret;
}

void cain_sip_main_loop_iterate(cain_sip_main_loop_t *ml){
	struct pollfd *pfd=(struct pollfd*)alloca(ml->nsources*sizeof(struct pollfd));
	int i=0;
	cain_sip_source_t *s,*next;
	uint64_t min_time_ms=(uint64_t)-1;
	int duration=-1;
	int ret;
	uint64_t cur;
	
	/*prepare the pollfd table */
	for(s=ml->sources;s!=NULL;s=(cain_sip_source_t*)s->node.next){
		if (s->fd!=-1){
			pfd[i].fd=s->fd;
			pfd[i].events=cain_sip_event_to_poll (s->events);
			pfd[i].revents=0;
			s->index=i;
			++i;
		}
		if (s->timeout>0){
			if (min_time_ms>s->expire_ms){
				min_time_ms=s->expire_ms;
			}
		}
	}
	
	if (min_time_ms!=(uint64_t)-1 ){
		/* compute the amount of time to wait for shortest timeout*/
		cur=cain_sip_time_ms();
		int64_t diff=min_time_ms-cur;
		if (diff>0)
			duration=(int)diff;
		else 
			duration=0;
	}
	/* do the poll */
	ret=poll(pfd,i,duration);
	if (ret==-1 && errno!=EINTR){
		cain_sip_error("poll() error: %s",strerror(errno));
		return;
	}
	cur=cain_sip_time_ms();
	/* examine poll results*/
	for(s=ml->sources;s!=NULL;s=next){
		unsigned revents=0;
		next=(cain_sip_source_t*)s->node.next;
		
		if (s->fd!=-1){
			if (pfd[s->index].revents!=0){
				revents=cain_sip_poll_to_event(pfd[s->index].revents);		
			}
		}
		if (revents!=0 || (s->timeout>0 && cur>=s->expire_ms)){
			ret=s->notify(s->data,revents);
			if (ret==0){
				/*this source needs to be removed*/
				cain_sip_main_loop_remove_source(ml,s);
			}else if (revents==0){
				/*timeout needs to be started again */
				s->expire_ms+=s->timeout;
			}
		}
	}
}

void cain_sip_main_loop_run(cain_sip_main_loop_t *ml){
	ml->run=1;
	while(ml->run){
		cain_sip_main_loop_iterate(ml);
	}
}

void cain_sip_main_loop_quit(cain_sip_main_loop_t *ml){
	ml->run=0;
	write(ml->control_fds[1],"a",1);
}

void cain_sip_main_loop_destroy(cain_sip_main_loop_t *ml){
	cain_sip_main_loop_remove_source (ml,ml->control);
	cain_sip_source_destroy(ml->control);
	close(ml->control_fds[0]);
	close(ml->control_fds[1]);
	cain_sip_free(ml);
}
