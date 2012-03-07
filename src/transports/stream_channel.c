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



/*
typedef enum stream_channel_state {
	WAITING_MESSAGE_START=0
	,MESSAGE_AQUISITION=1
	,BODY_AQUISITION=2
}stream_channel_state_t;
*/
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "cain_sip_internal.h"
#include "cain-sip/mainloop.h"
#include "channel.h"



struct cain_sip_stream_channel{
	cain_sip_channel_t base;
};

static void stream_channel_uninit(cain_sip_stream_channel_t *obj){
	cain_sip_fd_t sock = cain_sip_source_get_fd((cain_sip_source_t*)obj);
	if (sock!=-1)
		close_socket(sock);
	 cain_sip_main_loop_remove_source(obj->base.stack->ml,(cain_sip_source_t*)obj);
}

static int stream_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	cain_sip_fd_t sock = cain_sip_source_get_fd((cain_sip_source_t*)obj);
	int err;
	err=send(sock,buf,buflen,0);
	if (err==-1){
		cain_sip_fatal("Could not send stream packet on channel [%p]: %s",obj,strerror(errno));
		return -errno;
	}
	return err;
}

static int stream_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	cain_sip_fd_t sock = cain_sip_source_get_fd((cain_sip_source_t*)obj);
	int err;
	err=recv(sock,buf,buflen,MSG_DONTWAIT);
	if (err==-1 && errno!=EWOULDBLOCK){
		cain_sip_error("Could not receive stream packet: %s",strerror(errno));
		return -errno;
	}
	return err;
}

int stream_channel_connect(cain_sip_channel_t *obj, const struct sockaddr *addr, socklen_t socklen){
	int err;
	int tmp;
	cain_sip_fd_t sock = cain_sip_source_get_fd((cain_sip_source_t*)obj);
	tmp=1;
	err=setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,(char*)&tmp,sizeof(tmp));
	if (err!=0){
		cain_sip_error("setsockopt TCP_NODELAY failed: [%s]",cain_sip_get_socket_error_string());
	}
	fcntl(sock,F_SETFL,fcntl(sock,F_GETFL) | O_NONBLOCK);
	cain_sip_source_set_event((cain_sip_source_t*)obj,CAIN_SIP_EVENT_WRITE|CAIN_SIP_EVENT_ERROR);
	cain_sip_main_loop_add_source(obj->stack->ml,(cain_sip_source_t*)obj);
	err = connect(sock,addr,socklen);
	if (err != 0 && get_socket_error()!=EINPROGRESS) {
		    cain_sip_error("stream connect failed %s",cain_sip_get_socket_error_string());
		    close_socket(sock);
		    return -1;
	}

	return 0;
}

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_stream_channel_t,cain_sip_channel_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_stream_channel_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_stream_channel_t)=
{
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_stream_channel_t,cain_sip_channel_t,FALSE),
			(cain_sip_object_destroy_t)stream_channel_uninit,
			NULL,
			NULL
		},
		"TCP",
		1, /*is_reliable*/
		stream_channel_connect,
		stream_channel_send,
		stream_channel_recv
	}
};

static int process_data(cain_sip_channel_t *obj,unsigned int revents){
	int err, errnum;
	socklen_t optlen=sizeof(errnum);
	cain_sip_fd_t fd=cain_sip_source_get_fd((cain_sip_source_t*)obj);
	if (obj->state == CAIN_SIP_CHANNEL_CONNECTING && revents&CAIN_SIP_EVENT_WRITE) {
		err=getsockopt(fd,SOL_SOCKET,SO_ERROR,&errnum,&optlen);
		if (err!=0){
			cain_sip_error("Failed to retrieve connection status for channel [%p]: cause [%s]",obj,cain_sip_get_socket_error_string());
			goto connect_error;
		}else{
			if (errnum==0){
				/*obtain bind address for client*/
				struct sockaddr_storage ss;
				socklen_t addrlen=sizeof(ss);
				err=getsockname(fd,(struct sockaddr*)&ss,&addrlen);
				if (err<0){
					cain_sip_error("Failed to retrieve sockname  for channel [%p]: cause [%s]",obj,cain_sip_get_socket_error_string());
					goto connect_error;
				}
				cain_sip_source_set_event((cain_sip_source_t*)obj,CAIN_SIP_EVENT_READ|CAIN_SIP_EVENT_ERROR);
				cain_sip_channel_set_ready(obj,(struct sockaddr*)&ss,addrlen);
				return 0;
			}else{
				cain_sip_error("Connection failed  for channel [%p]: cause [%s]",obj,cain_sip_get_socket_error_string());
				goto connect_error;
			}

		}
	} else if ( obj->state == CAIN_SIP_CHANNEL_READY) {
		cain_sip_channel_process_data(obj,revents);
	} else {
		cain_sip_error("Unexpected event for channel [%p]",obj);
	}
	return 0;
connect_error:
	cain_sip_error("Cannot connect to [%s://%s:%s]",cain_sip_channel_get_transport_name(obj),obj->peer_name,obj->peer_port);
				channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
				channel_process_queue(obj);
				return -1;

}
cain_sip_channel_t * cain_sip_channel_new_tcp(cain_sip_stack_t *stack,const char *bindip, int localport, const char *dest, int port){
	cain_sip_stream_channel_t *obj=cain_sip_object_new(cain_sip_stream_channel_t);
	cain_sip_channel_init((cain_sip_channel_t*)obj
							,stack
							,socket(AF_INET, SOCK_STREAM, 0)
							,(cain_sip_source_func_t)process_data
							,bindip,localport,dest,port);
	return (cain_sip_channel_t*)obj;
}













