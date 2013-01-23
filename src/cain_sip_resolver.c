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

#include "cain_sip_resolver.h"


int cain_sip_addrinfo_to_ip(const struct addrinfo *ai, char *ip, size_t ip_size, int *port){
	char serv[16];
	int err=getnameinfo(ai->ai_addr,ai->ai_addrlen,ip,ip_size,serv,sizeof(serv),NI_NUMERICHOST|NI_NUMERICSERV);
	if (err!=0){
		cain_sip_error("getnameinfo() error: %s",gai_strerror(err));
		strncpy(ip,"<bug!!>",ip_size);
	}
	if (port) *port=atoi(serv);
	return 0;
}

struct addrinfo * cain_sip_ip_address_to_addrinfo(const char *ipaddress, int port){
	struct addrinfo *res=NULL;
	struct addrinfo hints={0};
	char serv[10];
	int err;

	snprintf(serv,sizeof(serv),"%i",port);
	hints.ai_family=AF_UNSPEC;
	hints.ai_flags=AI_NUMERICSERV|AI_NUMERICHOST;
	err=getaddrinfo(ipaddress,serv,&hints,&res);
	if (err!=0){
		return NULL;
	}
	return res;
}


static void cain_sip_resolver_context_destroy(cain_sip_resolver_context_t *ctx){
	if (ctx->thread!=0){
		cain_sip_thread_join(ctx->thread,NULL);
	}
	if (ctx->name)
		cain_sip_free(ctx->name);
	if (ctx->ai){
		freeaddrinfo(ctx->ai);
	}
#ifndef WIN32
	close(ctx->ctlpipe[0]);
	close(ctx->ctlpipe[1]);
#else
	if (ctx->ctlevent!=(cain_sip_fd_t)-1)
		CloseHandle(ctx->ctlevent);
#endif
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_resolver_context_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_resolver_context_t, cain_sip_source_t,cain_sip_resolver_context_destroy, NULL, NULL,FALSE);

static int resolver_callback(cain_sip_resolver_context_t *ctx){
	cain_sip_message("resolver_callback() for %p (%s) called, done=%i, cancelled=%i",ctx->cb_data,ctx->name,(int)ctx->done,(int)ctx->cancelled);
	if (!ctx->cancelled){
		ctx->cb(ctx->cb_data, ctx->name, ctx->ai);
		ctx->ai=NULL;
	}
#ifndef WIN32
	{
		char tmp;
		if (read(ctx->source.fd,&tmp,1)!=1){
			cain_sip_fatal("Unexpected read from resolver_callback");
		}
	}
#endif
	/*by returning stop, we'll be removed from main loop and destroyed. */
	return CAIN_SIP_STOP;
}

cain_sip_resolver_context_t *cain_sip_resolver_context_new(){
	cain_sip_resolver_context_t *ctx=cain_sip_object_new(cain_sip_resolver_context_t);
#ifdef WIN32
	ctx->ctlevent=(cain_sip_fd_t)-1;
#endif
	return ctx;
}

static void *cain_sip_resolver_thread(void *ptr){
	cain_sip_resolver_context_t *ctx=(cain_sip_resolver_context_t *)ptr;
	struct addrinfo *res=NULL;
	struct addrinfo hints={0};
	char serv[10];
	int err;

	/*the thread owns a ref on the resolver context*/
	cain_sip_object_ref(ctx);
	
	cain_sip_message("Resolver thread started.");
	snprintf(serv,sizeof(serv),"%i",ctx->port);
	hints.ai_family=ctx->family;
	hints.ai_flags=AI_NUMERICSERV;
	err=getaddrinfo(ctx->name,serv,&hints,&res);
	if (err!=0){
		cain_sip_error("DNS resolution of %s failed: %s",ctx->name,gai_strerror(err));
	}else{
		char host[64];
		cain_sip_addrinfo_to_ip(res,host,sizeof(host),NULL);
		cain_sip_message("%s has address %s.",ctx->name,host);
		ctx->ai=res;
	}
	ctx->done=TRUE;
#ifndef WIN32
	if (write(ctx->ctlpipe[1],"q",1)==-1){
		cain_sip_error("cain_sip_resolver_thread(): Fail to write on pipe.");
	}
#else
	SetEvent(ctx->ctlevent);
#endif
	cain_sip_object_unref(ctx);
	return NULL;
}

static void cain_sip_resolver_context_start(cain_sip_resolver_context_t *ctx){
	cain_sip_fd_t fd=(cain_sip_fd_t)-1;
	cain_sip_thread_create(&ctx->thread,NULL,cain_sip_resolver_thread,ctx);
#ifndef WIN32
	if (pipe(ctx->ctlpipe)==-1){
		cain_sip_fatal("pipe() failed: %s",strerror(errno));
	}
	fd=ctx->ctlpipe[0];
#else
	/*we don't use the thread handle itself, because it is not a manual-reset event.
	The mainloop implementation can only work with manual-reset events*/
	ctx->ctlevent=CreateEvent(NULL,TRUE,FALSE,NULL);
	/*use CreateEventEx on wp8*/
	fd=(HANDLE)ctx->ctlevent;
#endif
	cain_sip_fd_source_init(&ctx->source,(cain_sip_source_func_t)resolver_callback,ctx,fd,CAIN_SIP_EVENT_READ,-1);
}

unsigned long cain_sip_resolve(const char *name, int port, int family, cain_sip_resolver_callback_t cb , void *data, cain_sip_main_loop_t *ml){
	struct addrinfo *res=cain_sip_ip_address_to_addrinfo (name, port);
	if (res==NULL){
		/*then perform asynchronous DNS query */
		cain_sip_resolver_context_t *ctx=cain_sip_resolver_context_new();
		ctx->cb_data=data;
		ctx->cb=cb;
		ctx->name=cain_sip_strdup(name);
		ctx->port=port;
		if (family==0) family=AF_UNSPEC;
		ctx->family=family;
		
		cain_sip_resolver_context_start(ctx);
		/*the resolver context must never be removed manually from the main loop*/
		cain_sip_main_loop_add_source(ml,(cain_sip_source_t*)ctx);
		cain_sip_object_unref(ctx);/*the main loop and the thread have a ref on it*/
		return ctx->source.id;
	}else{
		cb(data,name,res);
		return 0;
	}
}

void cain_sip_resolve_cancel(cain_sip_main_loop_t *ml, unsigned long id){
	if (id!=0){
		cain_sip_source_t *s=cain_sip_main_loop_find_source(ml,id);
		if (s){
			cain_sip_resolver_context_t *res=CAIN_SIP_RESOLVER_CONTEXT(s);
			res->cancelled=1;
		}
	}
}

void cain_sip_get_src_addr_for(const struct sockaddr *dest, socklen_t destlen, struct sockaddr *src, socklen_t *srclen){
	int af_type=dest->sa_family;
	int sock=socket(af_type,SOCK_DGRAM,IPPROTO_UDP);
	
	if (sock==(cain_sip_socket_t)-1){
		cain_sip_fatal("Could not create socket: %s",cain_sip_get_socket_error_string());
		goto fail;
	}
	if (connect(sock,dest,destlen)==-1){
		cain_sip_error("cain_sip_get_src_addr_for: connect() failed: %s",cain_sip_get_socket_error_string());
		goto fail;
	}
	if (getsockname(sock,src,srclen)==-1){
		cain_sip_error("cain_sip_get_src_addr_for: getsockname() failed: %s",cain_sip_get_socket_error_string());
		goto fail;
	}
	close_socket(sock);
	return;
fail:
	{
		struct addrinfo hints={0},*res=NULL;
		int err;
		hints.ai_family=af_type;
		err=getaddrinfo(af_type==AF_INET ? "0.0.0.0" : "::0","0",&hints,&res);
		if (err!=0) cain_sip_fatal("cain_sip_get_src_addr_for(): getaddrinfo failed: %s",cain_sip_get_socket_error_string_from_code(err));
		memcpy(src,res->ai_addr,MIN(*srclen,res->ai_addrlen));
		*srclen=res->ai_addrlen;
		freeaddrinfo(res);
	}
	if (sock==(cain_sip_socket_t)-1) close_socket(sock);
}

