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


static void cain_sip_source_destroy(cain_sip_source_t *obj){
	if (obj->node.next || obj->node.prev){
		cain_sip_fatal("Destroying source currently used in main loop !");
	}
}



void cain_sip_fd_source_init(cain_sip_source_t *s, cain_sip_source_func_t func, void *data, int fd, unsigned int events, unsigned int timeout_value_ms){
	static unsigned long global_id=1;
	s->node.data=s;
	s->id=global_id++;
	s->fd=fd;
	s->events=events;
	s->timeout=timeout_value_ms;
	s->data=data;
	s->notify=func;
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_source_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_source_t,cain_sip_object_t,cain_sip_source_destroy,NULL,NULL,FALSE);

cain_sip_source_t * cain_sip_fd_source_new(cain_sip_source_func_t func, void *data, int fd, unsigned int events, unsigned int timeout_value_ms){
	cain_sip_source_t *s=cain_sip_object_new(cain_sip_source_t);
	cain_sip_fd_source_init(s,func,data,fd,events,timeout_value_ms);
	return s;
}

cain_sip_source_t * cain_sip_timeout_source_new(cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms){
	return cain_sip_fd_source_new(func,data,-1,0,timeout_value_ms);
}

unsigned long cain_sip_source_get_id(cain_sip_source_t *s){
	return s->id;
}

int cain_sip_source_set_events(cain_sip_source_t* source, int event_mask) {
	source->events = event_mask;
	return 0;
}

cain_sip_fd_t cain_sip_source_get_fd(const cain_sip_source_t* source) {
	return source->fd;
}


struct cain_sip_main_loop{
	cain_sip_object_t base;
	cain_sip_list_t *sources;
	cain_sip_source_t *control;
	int nsources;
	int run;
	int control_fds[2];
};

void cain_sip_main_loop_remove_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source){
	if (!source->node.next && !source->node.prev && &source->node!=ml->sources) return; /*nothing to do*/
	source->cancelled=TRUE;
	ml->sources=cain_sip_list_remove_link(ml->sources,&source->node);
	ml->nsources--;
	
	if (source->on_remove)
		source->on_remove(source);
	cain_sip_object_unref(source);
}


static void cain_sip_main_loop_destroy(cain_sip_main_loop_t *ml){
	cain_sip_main_loop_remove_source(ml,ml->control);
	close(ml->control_fds[0]);
	close(ml->control_fds[1]);
	cain_sip_object_unref(ml->control);
	cain_sip_object_delete_unowned();
}

static int main_loop_done(void *data, unsigned int events){
	cain_sip_main_loop_t * m=(cain_sip_main_loop_t*)data;
	char tmp;
	if (read(m->control_fds[0],&tmp,sizeof(tmp))!=1){
		cain_sip_error("Problem on control fd of main loop.");
	}
	return TRUE;
}


CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_main_loop_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_main_loop_t,cain_sip_object_t,cain_sip_main_loop_destroy,NULL,NULL,FALSE);

cain_sip_main_loop_t *cain_sip_main_loop_new(void){
	cain_sip_main_loop_t*m=cain_sip_object_new(cain_sip_main_loop_t);
	if (pipe(m->control_fds)==-1){
		cain_sip_fatal("Could not create control pipe.");
	}
	m->control=cain_sip_fd_source_new(main_loop_done,m,m->control_fds[0],CAIN_SIP_EVENT_READ,-1);
	cain_sip_object_set_name((cain_sip_object_t*)m->control,"main loop control fd");
	cain_sip_main_loop_add_source(m,m->control);
	return m;
}

void cain_sip_main_loop_add_source(cain_sip_main_loop_t *ml, cain_sip_source_t *source){
	if (source->node.next || source->node.prev){
		cain_sip_fatal("Source is already linked somewhere else.");
		return;
	}
	cain_sip_object_ref(source);
	if (source->timeout>=0){
		source->expire_ms=cain_sip_time_ms()+source->timeout;
	}
	ml->sources=cain_sip_list_append_link(ml->sources,&source->node);
	ml->nsources++;
}


unsigned long cain_sip_main_loop_add_timeout(cain_sip_main_loop_t *ml, cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms){
	cain_sip_source_t * s=cain_sip_timeout_source_new(func,data,timeout_value_ms);
	cain_sip_object_set_name((cain_sip_object_t*)s,"timeout");
	cain_sip_main_loop_add_source(ml,s);
	cain_sip_object_unref(s);
	return s->id;
}

void cain_sip_source_set_timeout(cain_sip_source_t *s, unsigned int value_ms){
	if (!s->expired){
		s->expire_ms=cain_sip_time_ms()+value_ms;
	}
	s->timeout=value_ms;
}

unsigned int cain_sip_source_get_timeout(const cain_sip_source_t *s){
	return s->timeout;
}


static int match_source_id(const void *s, const void *pid){
	if ( ((cain_sip_source_t*)s)->id==(unsigned long)pid){
		return 0;
	}
	return -1;
}

void cain_sip_main_loop_cancel_source(cain_sip_main_loop_t *ml, unsigned long id){
	cain_sip_list_t *elem=cain_sip_list_find_custom(ml->sources,match_source_id,(const void*)id);
	if (elem!=NULL){
		cain_sip_source_t *s=(cain_sip_source_t*)elem->data;
		s->cancelled=TRUE;
	}
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
	cain_sip_source_t *s;
	cain_sip_list_t *elem,*next;
	uint64_t min_time_ms=(uint64_t)-1;
	int duration=-1;
	int ret;
	uint64_t cur;
	cain_sip_list_t *copy;
	
	/*prepare the pollfd table */
	for(elem=ml->sources;elem!=NULL;elem=next){
		next=elem->next;
		s=(cain_sip_source_t*)elem->data;
		if (!s->cancelled){
			if (s->fd!=-1){
				pfd[i].fd=s->fd;
				pfd[i].events=cain_sip_event_to_poll (s->events);
				pfd[i].revents=0;
				s->index=i;
				++i;
			}
			if (s->timeout>=0){
				if (min_time_ms>s->expire_ms){
					min_time_ms=s->expire_ms;
				}
			}
		}else cain_sip_main_loop_remove_source (ml,s);
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
	copy=cain_sip_list_copy_with_data(ml->sources,(void *(*)(void*))cain_sip_object_ref);
	/* examine poll results*/
	for(elem=copy;elem!=NULL;elem=elem->next){
		unsigned revents=0;
		s=(cain_sip_source_t*)elem->data;

		if (!s->cancelled){
			if (s->fd!=-1){
				if (pfd[s->index].revents!=0){
					revents=cain_sip_poll_to_event(pfd[s->index].revents);		
				}
			}
			if (revents!=0 || (s->timeout>=0 && cur>=s->expire_ms)){
				char *objdesc=cain_sip_object_to_string((cain_sip_object_t*)s);
				s->expired=TRUE;
				cain_sip_message("source %s notified revents=%u, timeout=%i",objdesc,revents,s->timeout);
				cain_sip_free(objdesc);
				ret=s->notify(s->data,revents);
				if (ret==0){
					/*this source needs to be removed*/
					cain_sip_main_loop_remove_source(ml,s);
				}else if (revents==0){
					/*timeout needs to be started again */
					s->expire_ms+=s->timeout;
					s->expired=FALSE;
				}
			}
		}else cain_sip_main_loop_remove_source(ml,s);
	}
	cain_sip_list_free_with_data(copy,cain_sip_object_unref);
	cain_sip_object_delete_unowned();
}

void cain_sip_main_loop_run(cain_sip_main_loop_t *ml){
	ml->run=1;
	while(ml->run){
		cain_sip_main_loop_iterate(ml);
	}
}

int cain_sip_main_loop_quit(cain_sip_main_loop_t *ml){
	ml->run=0;
	//if (write(ml->control_fds[1],"a",1)==-1){
	//	cain_sip_error("Fail to write to main loop control fd.");
	//}
	return 0;
}

void cain_sip_main_loop_sleep(cain_sip_main_loop_t *ml, int milliseconds){
	unsigned long timer_id = cain_sip_main_loop_add_timeout(ml,(cain_sip_source_func_t)cain_sip_main_loop_quit,ml,milliseconds);
	cain_sip_main_loop_run(ml);
	cain_sip_main_loop_cancel_source(ml,timer_id);

}
