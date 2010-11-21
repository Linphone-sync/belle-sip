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

void cain_sip_resolver_context_destroy(cain_sip_resolver_context_t *ctx){
	if (ctx->thread!=0){
		if (!ctx->exited){
			ctx->cancelled=1;
			pthread_cancel(ctx->thread);
		}
		pthread_join(ctx->thread,NULL);
	}
	if (ctx->name)
		cain_sip_free(ctx->name);
	if (ctx->ai){
		freeaddrinfo(ctx->ai);
	}
	cain_sip_free(ctx);
}

static int resolver_callback(cain_sip_resolver_context_t *ctx){
	ctx->cb(ctx->cb_data, ctx->name, ctx->ai);
	return 0;
}

cain_sip_resolver_context_t *cain_sip_resolver_context_new(){
	cain_sip_resolver_context_t *ctx=cain_sip_new0(cain_sip_resolver_context_t);
	if (pipe(ctx->ctlpipe)==-1){
		cain_sip_fatal("pipe() failed: %s",strerror(errno));
	}
	cain_sip_fd_source_init(&ctx->source,(cain_sip_source_func_t)resolver_callback,ctx,ctx->ctlpipe[0],CAIN_SIP_EVENT_READ,-1);
	ctx->source.on_remove=(cain_sip_source_remove_callback_t)cain_sip_resolver_context_destroy;
	return ctx;
}

static void *cain_sip_resolver_thread(void *ptr){
	cain_sip_resolver_context_t *ctx=(cain_sip_resolver_context_t *)ptr;
	if (write(ctx->ctlpipe[1],"q",1)==-1){
		cain_sip_error("cain_sip_resolver_thread(): Fail to write on pipe.");
	}
	return NULL;
}

unsigned long cain_sip_resolve(const char *name, unsigned int hints, cain_sip_resolver_callback_t cb , void *data, cain_sip_main_loop_t *ml){
	cain_sip_resolver_context_t *ctx=cain_sip_resolver_context_new();
	ctx->cb_data=data;
	ctx->cb=cb;
	ctx->name=cain_sip_strdup(name);
	ctx->hints=hints;
	cain_sip_main_loop_add_source(ml,(cain_sip_source_t*)ctx);
	pthread_create(&ctx->thread,NULL,cain_sip_resolver_thread,ctx);
	return ctx->source.id;
}

