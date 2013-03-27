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

#ifdef HAVE_POLARSSL

static void cain_sip_tls_listening_point_uninit(cain_sip_tls_listening_point_t *lp){
	x509_free(&lp->root_ca);
}

static cain_sip_channel_t *tls_create_channel(cain_sip_listening_point_t *lp, const cain_sip_hop_t *hop){

	cain_sip_channel_t *chan=cain_sip_channel_new_tls(CAIN_SIP_TLS_LISTENING_POINT(lp)
				,cain_sip_uri_get_host(lp->listening_uri)
				,cain_sip_uri_get_port(lp->listening_uri)
				,hop->cname
				,hop->host,hop->port);
	return chan;
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_tls_listening_point_t);
CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(cain_sip_tls_listening_point_t)={
	{
		{
			CAIN_SIP_VPTR_INIT(cain_sip_tls_listening_point_t, cain_sip_stream_listening_point_t,TRUE),
			(cain_sip_object_destroy_t)cain_sip_tls_listening_point_uninit,
			NULL,
			NULL
		},
		"TLS",
		tls_create_channel
	}
};

static int on_new_connection(void *userdata, unsigned int revents){
	cain_sip_socket_t child;
	struct sockaddr_storage addr;
	socklen_t slen=sizeof(addr);
	cain_sip_tls_listening_point_t *lp=(cain_sip_tls_listening_point_t*)userdata;
	cain_sip_stream_listening_point_t *super=(cain_sip_stream_listening_point_t*)lp;
	
	child=accept(super->server_sock,(struct sockaddr*)&addr,&slen);
	if (child==(cain_sip_socket_t)-1){
		cain_sip_error("Listening point [%p] accept() failed on TLS server socket: %s",lp,cain_sip_get_socket_error_string());
		cain_sip_stream_listening_point_destroy_server_socket(super);
		cain_sip_stream_listening_point_setup_server_socket(super,on_new_connection);
		return CAIN_SIP_STOP;
	}
	cain_sip_message("New connection arriving on TLS, not handled !");
	close_socket(child);
	return CAIN_SIP_CONTINUE;
}

cain_sip_listening_point_t * cain_sip_tls_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port){
	cain_sip_tls_listening_point_t *lp=cain_sip_object_new(cain_sip_tls_listening_point_t);
	cain_sip_stream_listening_point_init((cain_sip_stream_listening_point_t*)lp,s,ipaddress,port,on_new_connection);
	
	lp->verify_exceptions=0;
	/*try to load "system" default root ca, wihtout warranty...*/
#ifdef __linux
	cain_sip_tls_listening_point_set_root_ca(lp,"/etc/ssl/certs");
#elif defined(__APPLE__)
	cain_sip_tls_listening_point_set_root_ca(lp,"/opt/local/share/curl/curl-ca-bundle.crt");
#endif
	return CAIN_SIP_LISTENING_POINT(lp);
}

int cain_sip_tls_listening_point_set_root_ca(cain_sip_tls_listening_point_t *lp, const char *path){
	struct stat statbuf; 
	if (stat(path,&statbuf)==0){
		if (statbuf.st_mode & S_IFDIR){
			if (x509parse_crtpath(&lp->root_ca,path)<0){
				cain_sip_error("Failed to load root ca from directory %s",path);
				return -1;
			}
		}else{
			if (x509parse_crtfile(&lp->root_ca,path)<0){
				cain_sip_error("Failed to load root ca from file %s",path);
				return -1;
			}
		}
		return 0;
	}
	cain_sip_error("Could not load root ca from %s: %s",path,strerror(errno));
	return -1;
}

int cain_sip_tls_listening_point_set_verify_exceptions(cain_sip_tls_listening_point_t *lp, int flags){
	lp->verify_exceptions=flags;
	return 0;
}

#else

cain_sip_listening_point_t * cain_sip_tls_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port){
	return NULL;
}

int cain_sip_tls_listening_point_set_root_ca(cain_sip_tls_listening_point_t *s, const char *path){
	return -1;
}

int cain_sip_tls_listening_point_set_verify_exceptions(cain_sip_tls_listening_point_t *s, int value){
	return -1;
}

#endif
