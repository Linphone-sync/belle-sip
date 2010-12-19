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

#include "cain_sip_internal.h"

static void cain_sip_sender_task_uninit(cain_sip_sender_task_t *t){
	cain_sip_stack_t *stack=cain_sip_provider_get_sip_stack(t->provider);
	cain_sip_object_unref(t->request);
	if (t->channel) cain_sip_object_unref(t->channel);
	if (t->source) {
		cain_sip_main_loop_cancel_source(stack->ml,cain_sip_source_get_id(t->source));
		cain_sip_object_unref(t->source);
	}
	if (t->resolver_id>0) cain_sip_main_loop_cancel_source (stack->ml,t->resolver_id);
}

cain_sip_sender_task_t * cain_sip_sender_task_new(cain_sip_provider_t *provider, cain_sip_request_t *req, cain_sip_sender_task_callback_t cb, void *data){
	cain_sip_sender_task_t *t=cain_sip_object_new(cain_sip_sender_task_t,cain_sip_sender_task_uninit);
	t->provider=provider;
	t->request=(cain_sip_request_t*)cain_sip_object_ref(req);
	t->cb=cb;
	t->cb_data=data;
	return t;
}

static int do_send(cain_sip_sender_task_t *t){
	int err=cain_sip_channel_send(t->channel,t->buf,strlen(t->buf));
	if (err>0){
		t->cb(t->cb_data,0);
	}else if (err!=-EWOULDBLOCK){
		t->cb(t->cb_data,-1);
	}
	return err;
}

static int retry_send(cain_sip_sender_task_t *t, unsigned int revents){
	if (revents & CAIN_SIP_EVENT_WRITE){
		do_send(t);
		return 0;
	}
	/*timeout : notify the failure*/
	t->cb(t->cb_data,-1);
	return 0;
}

static void sender_task_send(cain_sip_sender_task_t *t){
	int err;
	err=do_send(t);
	if (err==-EWOULDBLOCK){
		cain_sip_stack_t *stack=cain_sip_provider_get_sip_stack(t->provider);
		/*need to retry later*/
		if (t->source==NULL){
			t->source=cain_sip_channel_create_source(t->channel,CAIN_SIP_EVENT_WRITE,CAIN_SIP_SOCKET_TIMEOUT,
			                                         (cain_sip_source_func_t)retry_send,t);
		}
		cain_sip_main_loop_add_source (stack->ml,t->source);
	}
}

static void sender_task_find_channel(cain_sip_sender_task_t *t){
	cain_sip_listening_point_t *lp=cain_sip_provider_get_listening_point(t->provider,t->hop.transport);
	if (lp==NULL){
		cain_sip_error("No listening point available for transport %s",t->hop.transport);
		goto error;
	}else{
		cain_sip_channel_t *chan=cain_sip_listening_point_find_output_channel (lp,t->dest);
		if (chan==NULL) goto error;
		t->channel=(cain_sip_channel_t*)cain_sip_object_ref(chan);
		t->buf=cain_sip_message_to_string(CAIN_SIP_MESSAGE(t->request));
		sender_task_send(t);
	}
	return;
	error:
		t->cb(t->cb_data,-1);
}

static void sender_task_res_done(void *data, const char *name, struct addrinfo *res){
	cain_sip_sender_task_t *t=(cain_sip_sender_task_t*)data;
	t->resolver_id=0;
	if (res){
		t->dest=res;
		sender_task_find_channel(t);
	}else{
		t->cb(t->cb_data,-1);
	}
}

void cain_sip_sender_task_send(cain_sip_sender_task_t *t){
	cain_sip_stack_t *stack=cain_sip_provider_get_sip_stack(t->provider);

	if (t->buf!=NULL){
		/*retransmission, everything already done*/
		sender_task_send(t);
	}
	cain_sip_stack_get_next_hop(stack,t->request,&t->hop);
	t->resolver_id=cain_sip_resolve(t->hop.host,t->hop.port,0,sender_task_res_done,t,stack->ml);
}


