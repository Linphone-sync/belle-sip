/*
	cain-sip - SIP (RFC3261) library.
    Copyright (C) 2013  Belledonne Communications SARL

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
#include "stream_channel.h"

#ifdef HAVE_POLARSSL

/* Uncomment to get very verbose polarssl logs*/
//#define ENABLE_POLARSSL_LOGS

#include <polarssl/error.h>

/*************tls********/

static int tls_process_data(cain_sip_channel_t *obj,unsigned int revents);

struct cain_sip_tls_channel{
	cain_sip_stream_channel_t base;
	ssl_context sslctx;
	x509_cert root_ca;
	struct sockaddr_storage ss;
	socklen_t socklen;
	int socket_connected;
	char *cur_debug_msg;
};

static void tls_channel_close(cain_sip_tls_channel_t *obj){
	cain_sip_socket_t sock = cain_sip_source_get_socket((cain_sip_source_t*)obj);
	if (sock!=-1)
		ssl_close_notify(&obj->sslctx);
	stream_channel_close((cain_sip_stream_channel_t*)obj);
}

static void tls_channel_uninit(cain_sip_tls_channel_t *obj){
	cain_sip_socket_t sock = cain_sip_source_get_socket((cain_sip_source_t*)obj);
	if (sock!=-1)
		tls_channel_close(obj);
	ssl_free(&obj->sslctx);
	x509_free(&obj->root_ca);
	if (obj->cur_debug_msg)
		cain_sip_free(obj->cur_debug_msg);
}

static int tls_channel_send(cain_sip_channel_t *obj, const void *buf, size_t buflen){
	cain_sip_tls_channel_t* channel = (cain_sip_tls_channel_t*)obj;
	int err = ssl_write(&channel->sslctx,buf,buflen);
	if (err<0){
		cain_sip_error("Channel [%p]: error in ssl_write().",obj);
	}
	return err;
}

static int tls_channel_recv(cain_sip_channel_t *obj, void *buf, size_t buflen){
	cain_sip_tls_channel_t* channel = (cain_sip_tls_channel_t*)obj;
	int err = ssl_read(&channel->sslctx,buf,buflen);
	if (err<0){
		cain_sip_error("Channel [%p]: error in ssl_read().",obj);
	}
	return err;
}

static int tls_channel_connect(cain_sip_channel_t *obj, const struct addrinfo *ai){
	int err= stream_channel_connect((cain_sip_stream_channel_t*)obj,ai);
	if (err==0){
		cain_sip_socket_t sock=cain_sip_source_get_socket((cain_sip_source_t*)obj);
		cain_sip_channel_set_socket(obj,sock,(cain_sip_source_func_t)tls_process_data);
		return 0;
	}
	return -1;
}

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_tls_channel_t,cain_sip_stream_channel_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_tls_channel_t);

CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_tls_channel_t)=
{
	{
		{
			{
				CAIN_SIP_VPTR_INIT(cain_sip_tls_channel_t,cain_sip_stream_channel_t,FALSE),
				(cain_sip_object_destroy_t)tls_channel_uninit,
				NULL,
				NULL
			},
			"TLS",
			1, /*is_reliable*/
			tls_channel_connect,
			tls_channel_send,
			tls_channel_recv,
			(void (*)(cain_sip_channel_t*))tls_channel_close
		}
	}
};

static int tls_process_data(cain_sip_channel_t *obj,unsigned int revents){
	cain_sip_tls_channel_t* channel=(cain_sip_tls_channel_t*)obj;
	int err;

	if (obj->state == CAIN_SIP_CHANNEL_CONNECTING ) {
		if (!channel->socket_connected && (revents & CAIN_SIP_EVENT_WRITE)) {
			channel->socklen=sizeof(channel->ss);
			if (finalize_stream_connection((cain_sip_stream_channel_t*)obj,(struct sockaddr*)&channel->ss,&channel->socklen)) {
				goto process_error;
			}
			cain_sip_source_set_events((cain_sip_source_t*)channel,CAIN_SIP_EVENT_READ|CAIN_SIP_EVENT_ERROR);
			channel->socket_connected=1;
			cain_sip_message("Channel [%p]: Connected at TCP level, now doing TLS handshake",obj);
		}
		err=ssl_handshake(&channel->sslctx);
		if (err==0){
			cain_sip_message("Channel [%p]: SSL handshake finished.",obj);
			cain_sip_channel_set_ready(obj,(struct sockaddr*)&channel->ss,channel->socklen);
		}else if (err==POLARSSL_ERR_NET_WANT_READ || err==POLARSSL_ERR_NET_WANT_WRITE){
			cain_sip_message("Channel [%p]: SSL handshake in progress...",obj);
		}else{
			char tmp[128];
			error_strerror(err,tmp,sizeof(tmp));
			cain_sip_error("Channel [%p]: SSL handlshake failed : %s",obj,tmp);
			goto process_error;
		}
		
	} else if ( obj->state == CAIN_SIP_CHANNEL_READY) {
		cain_sip_channel_process_data(obj,revents);
	} else {
		cain_sip_warning("Unexpected event [%i], for channel [%p]",revents,channel);
	}
	return CAIN_SIP_CONTINUE;
	
process_error:
	cain_sip_error("Cannot connect to [%s://%s:%i]",cain_sip_channel_get_transport_name(obj),obj->peer_name,obj->peer_port);
	channel_set_state(obj,CAIN_SIP_CHANNEL_ERROR);
	return CAIN_SIP_STOP;
}

static int polarssl_read(void * ctx, unsigned char *buf, size_t len ){
	cain_sip_stream_channel_t *super=(cain_sip_stream_channel_t *)ctx;
	
	int ret = stream_channel_recv(super,buf,len);

	if (ret<0){
		ret=-ret;
		if (ret==CAINSIP_EWOULDBLOCK || ret==CAINSIP_EINPROGRESS || ret == EINTR )
			return POLARSSL_ERR_NET_WANT_READ;
		return POLARSSL_ERR_NET_CONN_RESET;
	}
	return ret;
}

static int polarssl_write(void * ctx, const unsigned char *buf, size_t len ){
	cain_sip_stream_channel_t *super=(cain_sip_stream_channel_t *)ctx;
	
	int ret = stream_channel_send(super, buf, len);

	if (ret<0){
		ret=-ret;
		if (ret==CAINSIP_EWOULDBLOCK || ret==CAINSIP_EINPROGRESS || ret == EINTR )
			return POLARSSL_ERR_NET_WANT_WRITE;
		return POLARSSL_ERR_NET_CONN_RESET;
	}
	return ret;
}

static int random_generator(void *ctx, unsigned char *ptr, size_t size){
	cain_sip_random_bytes(ptr, size);
	return 0;
}

static const char *polarssl_certflags_to_string(char *buf, size_t size, int flags){
	int i=0;
	
	memset(buf,0,size);
	size--;
	
	if (i<size && (flags & BADCERT_EXPIRED))
		i+=snprintf(buf+i,size-i,"expired ");
	if (i<size && (flags & BADCERT_REVOKED))
		i+=snprintf(buf+i,size-i,"revoked ");
	if (i<size && (flags & BADCERT_CN_MISMATCH))
		i+=snprintf(buf+i,size-i,"CN-mismatch ");
	if (i<size && (flags & BADCERT_NOT_TRUSTED))
		i+=snprintf(buf+i,size-i,"not-trusted ");
	if (i<size && (flags & BADCERT_MISSING))
		i+=snprintf(buf+i,size-i,"missing ");
	if (i<size && (flags & BADCRL_NOT_TRUSTED))
		i+=snprintf(buf+i,size-i,"crl-not-trusted ");
	if (i<size && (flags & BADCRL_EXPIRED))
		i+=snprintf(buf+i,size-i,"crl-not-expired ");
	return buf;
}

static int cain_sip_ssl_verify(void *data , x509_cert *cert , int depth, int *flags){
	cain_sip_tls_listening_point_t *lp=(cain_sip_tls_listening_point_t*)data;
	char tmp[256];
	char flags_str[128];
	
	x509parse_cert_info(tmp,sizeof(tmp),"",cert);
	cain_sip_message("Found certificate depth=[%i], flags=[%s]:\n%s",
		depth,polarssl_certflags_to_string(flags_str,sizeof(flags_str),*flags),tmp);
	if (lp->verify_exceptions==CAIN_SIP_TLS_LISTENING_POINT_BADCERT_ANY_REASON){
		*flags=0;
	}else if (lp->verify_exceptions & CAIN_SIP_TLS_LISTENING_POINT_BADCERT_CN_MISMATCH){
		*flags&=~BADCERT_CN_MISMATCH;
	}
	return 0;
}

static int cain_sip_tls_channel_load_root_ca(cain_sip_tls_channel_t *obj, const char *path){
	struct stat statbuf; 
	if (stat(path,&statbuf)==0){
		if (statbuf.st_mode & S_IFDIR){
			if (x509parse_crtpath(&obj->root_ca,path)<0){
				cain_sip_error("Failed to load root ca from directory %s",path);
				return -1;
			}
		}else{
			if (x509parse_crtfile(&obj->root_ca,path)<0){
				cain_sip_error("Failed to load root ca from file %s",path);
				return -1;
			}
		}
		return 0;
	}
	cain_sip_error("Could not load root ca from %s: %s",path,strerror(errno));
	return -1;
}

#ifdef ENABLE_POLARSSL_LOGS
/*
 * polarssl does a lot of logs, some with newline, some without.
 * We need to concatenate logs without new line until a new line is found.
 */
static void ssl_debug_to_cain_sip(void *context, int level, const char *str){
	cain_sip_tls_channel_t *chan=(cain_sip_tls_channel_t*)context;
	int len=strlen(str);
	
	if (len>0 && (str[len-1]=='\n' || str[len-1]=='\r')){
		/*eliminate the newline*/
		char *tmp=cain_sip_strdup(str);
		tmp[len-1]=0;
		if (chan->cur_debug_msg){
			cain_sip_message("ssl: %s%s",chan->cur_debug_msg,tmp);
			cain_sip_free(chan->cur_debug_msg);
			chan->cur_debug_msg=NULL;
		}else cain_sip_message("ssl: %s",tmp);
		cain_sip_free(tmp);
	}else{
		if (chan->cur_debug_msg){
			char *tmp=cain_sip_strdup_printf("%s%s",chan->cur_debug_msg,str);
			cain_sip_free(chan->cur_debug_msg);
			chan->cur_debug_msg=tmp;
		}else chan->cur_debug_msg=cain_sip_strdup(str);
	}
}

#endif

cain_sip_channel_t * cain_sip_channel_new_tls(cain_sip_tls_listening_point_t *lp,const char *bindip, int localport, const char *peer_cname, const char *dest, int port){
	cain_sip_tls_channel_t *obj=cain_sip_object_new(cain_sip_tls_channel_t);
	cain_sip_stream_channel_t* super=(cain_sip_stream_channel_t*)obj;

	cain_sip_stream_channel_init_client(super
					,((cain_sip_listening_point_t*)lp)->stack
					,bindip,localport,peer_cname,dest,port);
	ssl_init(&obj->sslctx);
#ifdef ENABLE_POLARSSL_LOGS
	ssl_set_dbg(&obj->sslctx,ssl_debug_to_cain_sip,obj);
#endif
	ssl_set_endpoint(&obj->sslctx,SSL_IS_CLIENT);
	ssl_set_authmode(&obj->sslctx,SSL_VERIFY_REQUIRED);
	ssl_set_bio(&obj->sslctx,polarssl_read,obj,polarssl_write,obj);
	if (lp->root_ca && cain_sip_tls_channel_load_root_ca(obj,lp->root_ca)==0){
		ssl_set_ca_chain(&obj->sslctx,&obj->root_ca,NULL,super->base.peer_cname);
	}
	ssl_set_rng(&obj->sslctx,random_generator,NULL);
	ssl_set_verify(&obj->sslctx,cain_sip_ssl_verify,lp);
	return (cain_sip_channel_t*)obj;
}

#endif


