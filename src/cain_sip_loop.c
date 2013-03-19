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


#ifndef WIN32
#include <unistd.h>
#include <poll.h>
typedef struct pollfd cain_sip_pollfd_t;

static int cain_sip_poll(cain_sip_pollfd_t *pfd, int count, int duration){
	int err;
	err=poll(pfd,count,duration);
	if (err==-1 && errno!=EINTR)
		cain_sip_error("poll() error: %s",strerror(errno));
	return err;
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

static unsigned int cain_sip_poll_to_event(cain_sip_pollfd_t * pfd){
	unsigned int ret=0;
	short events=pfd->revents;
	if (events & POLLIN)
		ret|=CAIN_SIP_EVENT_READ;
	if (events & POLLOUT)
		ret|=CAIN_SIP_EVENT_WRITE;
	if (events & POLLERR)
		ret|=CAIN_SIP_EVENT_ERROR;
	return ret;
}

static void cain_sip_source_to_poll(cain_sip_source_t *s, cain_sip_pollfd_t *pfd, int i){
	pfd[i].fd=s->fd;
	pfd[i].events=cain_sip_event_to_poll(s->events);
	pfd[i].revents=0;
	s->index=i;
}

static unsigned int cain_sip_source_get_revents(cain_sip_source_t *s,cain_sip_pollfd_t *pfd){
	return cain_sip_poll_to_event(&pfd[s->index]);
}

#else


#include <malloc.h>


typedef HANDLE cain_sip_pollfd_t;

static void cain_sip_source_to_poll(cain_sip_source_t *s, cain_sip_pollfd_t *pfd,int i){
	s->index=i;
	pfd[i]=s->fd;
	
	/*special treatments for windows sockets*/
	if (s->sock!=(cain_sip_socket_t)-1){
		int err;
		long events=0;
		
		if (s->events & CAIN_SIP_EVENT_READ)
			events|=FD_READ;
		if (s->events & CAIN_SIP_EVENT_WRITE)
			events|=FD_WRITE;
		
		err=WSAEventSelect(s->sock,s->fd,events);
		if (err!=0) cain_sip_error("WSAEventSelect() failed: %s",cain_sip_get_socket_error_string());
	}
}

static unsigned int cain_sip_source_get_revents(cain_sip_source_t *s,cain_sip_pollfd_t *pfd){
	WSANETWORKEVENTS revents={0};
	int err;
	unsigned int ret=0;
	
	/*special treatments for windows sockets*/
	if (s->sock!=(cain_sip_socket_t)-1){
		err=WSAEnumNetworkEvents(s->sock,NULL,&revents);
		if (err!=0){
			cain_sip_error("WSAEnumNetworkEvents() failed: %s socket=%x",cain_sip_get_socket_error_string(),(unsigned int)s->sock);
			return 0;
		}
		if (revents.lNetworkEvents & FD_READ)
			ret|=CAIN_SIP_EVENT_READ;
		if (revents.lNetworkEvents & FD_WRITE)
			ret|=CAIN_SIP_EVENT_WRITE;
	}else{
		if (WaitForSingleObjectEx(s->fd,0,FALSE)==WAIT_OBJECT_0){
			ret=CAIN_SIP_EVENT_READ;
			ResetEvent(s->fd);
		}
	}
	return ret;
}

static int cain_sip_poll(cain_sip_pollfd_t *pfd, int count, int duration){
	DWORD ret;
	
	if (count == 0) {
		cain_sip_sleep(duration);
		return 0;
	}

	ret=WaitForMultipleObjectsEx(count,pfd,FALSE,duration,FALSE);
	if (ret==WAIT_FAILED){
		cain_sip_error("WaitForMultipleObjectsEx() failed.");
		return -1;
	}
	if (ret==WAIT_TIMEOUT){
		return 0;
	}
	return ret-WAIT_OBJECT_0;
}

#endif

static void cain_sip_source_destroy(cain_sip_source_t *obj){
	if (obj->node.next || obj->node.prev){
		cain_sip_fatal("Destroying source currently used in main loop !");
	}
	cain_sip_source_uninit(obj);
}

static void cain_sip_source_init(cain_sip_source_t *s, cain_sip_source_func_t func, void *data, cain_sip_fd_t fd, unsigned int events, unsigned int timeout_value_ms){
	static unsigned long global_id=1;
	s->node.data=s;
	s->id=global_id++;
	s->fd=fd;
	s->events=events;
	s->timeout=timeout_value_ms;
	s->data=data;
	s->notify=func;
	s->sock=(cain_sip_socket_t)-1;
}

void cain_sip_source_uninit(cain_sip_source_t *obj){
#ifdef WIN32
	if (obj->sock!=(cain_sip_socket_t)-1){
		WSACloseEvent(obj->fd);
		obj->fd=(WSAEVENT)-1;
	}
#endif
	obj->fd=(cain_sip_fd_t)-1;
	obj->sock=(cain_sip_socket_t)-1;
}

void cain_sip_socket_source_init(cain_sip_source_t *s, cain_sip_source_func_t func, void *data, cain_sip_socket_t sock, unsigned int events, unsigned int timeout_value_ms){
#ifdef WIN32
	/*on windows, the fd to poll is not the socket */
	cain_sip_fd_t fd=(cain_sip_fd_t)-1;
	if (sock!=(cain_sip_socket_t)-1)
		fd=WSACreateEvent();
	else
		fd=(WSAEVENT)-1;
	cain_sip_source_init(s,func,data,fd,events,timeout_value_ms);
	
#else
	cain_sip_source_init(s,func,data,sock,events,timeout_value_ms);
#endif
	s->sock=sock;
}

void cain_sip_fd_source_init(cain_sip_source_t *s, cain_sip_source_func_t func, void *data, cain_sip_fd_t fd, unsigned int events, unsigned int timeout_value_ms){
	cain_sip_source_init(s,func,data,fd,events,timeout_value_ms);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_source_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_source_t,cain_sip_object_t,cain_sip_source_destroy,NULL,NULL,FALSE);

cain_sip_source_t * cain_sip_socket_source_new(cain_sip_source_func_t func, void *data, cain_sip_socket_t sock, unsigned int events, unsigned int timeout_value_ms){
	cain_sip_source_t *s=cain_sip_object_new(cain_sip_source_t);
	cain_sip_socket_source_init(s,func,data,sock,events,timeout_value_ms);
	return s;
}

cain_sip_source_t * cain_sip_fd_source_new(cain_sip_source_func_t func, void *data, cain_sip_fd_t fd, unsigned int events, unsigned int timeout_value_ms){
	cain_sip_source_t *s=cain_sip_object_new(cain_sip_source_t);
	cain_sip_fd_source_init(s,func,data,fd,events,timeout_value_ms);
	return s;
}

cain_sip_source_t * cain_sip_timeout_source_new(cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms){
	return cain_sip_socket_source_new(func,data,(cain_sip_socket_t)-1,0,timeout_value_ms);
}

unsigned long cain_sip_source_get_id(cain_sip_source_t *s){
	return s->id;
}

int cain_sip_source_set_events(cain_sip_source_t* source, int event_mask) {
	source->events = event_mask;
	return 0;
}

cain_sip_socket_t cain_sip_source_get_socket(const cain_sip_source_t* source) {
	return source->sock;
}


struct cain_sip_main_loop{
	cain_sip_object_t base;
	cain_sip_list_t *sources;
	cain_sip_object_pool_t *pool;
	int nsources;
	int run;
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
	while (ml->sources){
		cain_sip_main_loop_remove_source(ml,(cain_sip_source_t*)ml->sources->data);
	}
	if (cain_sip_object_pool_cleanable(ml->pool)){
		cain_sip_object_unref(ml->pool);
	}
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_main_loop_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_main_loop_t,cain_sip_object_t,cain_sip_main_loop_destroy,NULL,NULL,FALSE);

cain_sip_main_loop_t *cain_sip_main_loop_new(void){
	cain_sip_main_loop_t*m=cain_sip_object_new(cain_sip_main_loop_t);
	m->pool=cain_sip_object_pool_push();
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

cain_sip_source_t* cain_sip_main_loop_create_timeout(cain_sip_main_loop_t *ml
														, cain_sip_source_func_t func
														, void *data
														, unsigned int timeout_value_ms
														,const char* timer_name) {
	cain_sip_source_t * s=cain_sip_timeout_source_new(func,data,timeout_value_ms);
	cain_sip_object_set_name((cain_sip_object_t*)s,timer_name);
	cain_sip_main_loop_add_source(ml,s);
	return s;
}

unsigned long cain_sip_main_loop_add_timeout(cain_sip_main_loop_t *ml, cain_sip_source_func_t func, void *data, unsigned int timeout_value_ms){
	cain_sip_source_t * s=cain_sip_main_loop_create_timeout(ml,func,data,timeout_value_ms,"Timer");
	cain_sip_object_unref(s);
	return s->id;
}

void cain_sip_main_loop_do_later(cain_sip_main_loop_t *ml, cain_sip_callback_t func, void *data){
	cain_sip_source_t * s=cain_sip_main_loop_create_timeout(ml,(cain_sip_source_func_t)func,data,0,"defered task");
	s->oneshot=TRUE;
	cain_sip_object_unref(s);
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

cain_sip_source_t *cain_sip_main_loop_find_source(cain_sip_main_loop_t *ml, unsigned long id){
	cain_sip_list_t *elem=cain_sip_list_find_custom(ml->sources,match_source_id,(const void*)id);
	if (elem!=NULL){
		return (cain_sip_source_t*)elem->data;
	}
	return NULL;
}

void cain_sip_main_loop_cancel_source(cain_sip_main_loop_t *ml, unsigned long id){
	cain_sip_source_t *s=cain_sip_main_loop_find_source(ml,id);
	if (s) s->cancelled=TRUE;
}

void cain_sip_main_loop_iterate(cain_sip_main_loop_t *ml){
	size_t pfd_size = ml->nsources * sizeof(cain_sip_pollfd_t);
	cain_sip_pollfd_t *pfd=(cain_sip_pollfd_t*)alloca(pfd_size);
	int i=0;
	cain_sip_source_t *s;
	cain_sip_list_t *elem,*next;
	uint64_t min_time_ms=(uint64_t)-1;
	int duration=-1;
	int ret;
	uint64_t cur;
	cain_sip_list_t *copy;
	int can_clean=cain_sip_object_pool_cleanable(ml->pool); /*iterate might not be called by the thread that created the main loop*/ 
	cain_sip_object_pool_t *tmp_pool=NULL;
	
	if (!can_clean){
		/*Push a temporary pool for the time of the iterate loop*/
		tmp_pool=cain_sip_object_pool_push();
	}
	
	/*prepare the pollfd table */
	memset(pfd, 0, pfd_size);
	for(elem=ml->sources;elem!=NULL;elem=next){
		next=elem->next;
		s=(cain_sip_source_t*)elem->data;
		if (!s->cancelled){
			if (s->fd!=(cain_sip_fd_t)-1){
				cain_sip_source_to_poll(s,pfd,i);
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
		int64_t diff;
		/* compute the amount of time to wait for shortest timeout*/
		cur=cain_sip_time_ms();
		diff=min_time_ms-cur;
		if (diff>0)
			duration=(int)diff;
		else 
			duration=0;
	}
	/* do the poll */
	ret=cain_sip_poll(pfd,i,duration);
	if (ret==-1){
		return;
	}
	cur=cain_sip_time_ms();
	copy=cain_sip_list_copy_with_data(ml->sources,(void *(*)(void*))cain_sip_object_ref);
	/* examine poll results*/
	for(elem=copy;elem!=NULL;elem=elem->next){
		unsigned revents=0;
		s=(cain_sip_source_t*)elem->data;

		if (!s->cancelled){
			if (s->fd!=(cain_sip_fd_t)-1){
				if (s->notify_required) { /*for testing purpose to force channel to read*/
					revents=CAIN_SIP_EVENT_READ;
					s->notify_required=0; /*reset*/
				} else {
					revents=cain_sip_source_get_revents(s,pfd);
				}
			}
			if (revents!=0 || (s->timeout>=0 && cur>=s->expire_ms)){
				char *objdesc=cain_sip_object_to_string((cain_sip_object_t*)s);
				s->expired=TRUE;
				if (revents==0)
					revents=CAIN_SIP_EVENT_TIMEOUT;
				if (s->timeout>0)/*to avoid too many traces*/ cain_sip_debug("source %s notified revents=%u, timeout=%i",objdesc,revents,s->timeout);
				cain_sip_free(objdesc);
				ret=s->notify(s->data,revents);
				if (ret==CAIN_SIP_STOP || s->oneshot){
					/*this source needs to be removed*/
					cain_sip_main_loop_remove_source(ml,s);
				}else if (revents==CAIN_SIP_EVENT_TIMEOUT){
					/*timeout needs to be started again */
					s->expire_ms+=s->timeout;
					s->expired=FALSE;
				}
			}
		}else cain_sip_main_loop_remove_source(ml,s);
	}
	cain_sip_list_free_with_data(copy,cain_sip_object_unref);
	if (can_clean) cain_sip_object_pool_clean(ml->pool);
	else if (tmp_pool) cain_sip_object_unref(tmp_pool);
}

void cain_sip_main_loop_run(cain_sip_main_loop_t *ml){
	ml->run=1;
	while(ml->run){
		cain_sip_main_loop_iterate(ml);
	}
}

int cain_sip_main_loop_quit(cain_sip_main_loop_t *ml){
	ml->run=0;
	return CAIN_SIP_STOP;
}

void cain_sip_main_loop_sleep(cain_sip_main_loop_t *ml, int milliseconds){
	unsigned long timer_id = cain_sip_main_loop_add_timeout(ml,(cain_sip_source_func_t)cain_sip_main_loop_quit,ml,milliseconds);
	cain_sip_main_loop_run(ml);
	cain_sip_main_loop_cancel_source(ml,timer_id);
}

