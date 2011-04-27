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



#include "cain-sip/headers.h"
#include "cain-sip/parameters.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "cain_sip_messageParser.h"
#include "cain_sip_messageLexer.h"
#include "cain_sip_internal.h"

/************************
 * header
 ***********************/

GET_SET_STRING(cain_sip_header,name);

void cain_sip_header_init(cain_sip_header_t *header) {

}

static void cain_sip_header_destroy(cain_sip_header_t *header){
	if (header->name) cain_sip_free((void*)header->name);
}

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_header_t,cain_sip_object_t,cain_sip_header_destroy,NULL);


/************************
 * header_address
 ***********************/
struct _cain_sip_header_address {
	cain_sip_parameters_t base;
	const char* displayname;
	cain_sip_uri_t* uri;
};

static void cain_sip_header_address_init(cain_sip_header_address_t* object){
	cain_sip_parameters_init((cain_sip_parameters_t*)object); /*super*/
}

static void cain_sip_header_address_destroy(cain_sip_header_address_t* contact) {
	if (contact->displayname) cain_sip_free((void*)(contact->displayname));
	if (contact->uri) cain_sip_object_unref(CAIN_SIP_OBJECT(contact->uri));
}

static void cain_sip_header_address_clone(cain_sip_header_address_t *addr){
}

CAIN_SIP_NEW(header_address,parameters)
GET_SET_STRING(cain_sip_header_address,displayname);

void cain_sip_header_address_set_quoted_displayname(cain_sip_header_address_t* address,const char* value) {
		if (address->displayname != NULL) cain_sip_free((void*)(address->displayname));
		address->displayname=_cain_sip_str_dup_and_unquote_string(value);
}
cain_sip_uri_t* cain_sip_header_address_get_uri(cain_sip_header_address_t* address) {
	return address->uri;
}

void cain_sip_header_address_set_uri(cain_sip_header_address_t* address, cain_sip_uri_t* uri) {
	address->uri=uri;
}



/************************
 * header_contact
 ***********************/
struct _cain_sip_header_contact {
	cain_sip_header_address_t address;
	unsigned int wildcard;
 };

void cain_sip_header_contact_destroy(cain_sip_header_contact_t* contact) {
}

void cain_sip_header_contact_clone(cain_sip_header_contact_t *contact, const cain_sip_header_contact_t *orig){
}

CAIN_SIP_NEW_HEADER(header_contact,header_address,"Contact")
CAIN_SIP_PARSE(header_contact)

GET_SET_INT_PARAM_PRIVATE(cain_sip_header_contact,expires,int,_)
GET_SET_INT_PARAM_PRIVATE(cain_sip_header_contact,q,float,_);
GET_SET_BOOL(cain_sip_header_contact,wildcard,is);


int cain_sip_header_contact_set_expires(cain_sip_header_contact_t* contact, int expires) {
	if (expires < 0 ) {
		 cain_sip_error("bad expires value [%i] for contact",expires);
		return -1;
	}
	_cain_sip_header_contact_set_expires(contact,expires);
	return 0;
 }
int cain_sip_header_contact_set_qvalue(cain_sip_header_contact_t* contact, float qValue) {
	 if (qValue != -1 && qValue < 0 && qValue >1) {
		 cain_sip_error("bad q value [%f] for contact",qValue);
		 return -1;
	 }
	 _cain_sip_header_contact_set_q(contact,qValue);
	 return 0;
}
float	cain_sip_header_contact_get_qvalue(const cain_sip_header_contact_t* contact) {
	return cain_sip_header_contact_get_q(contact);
}
/**************************
* From header object inherent from header_address
****************************
*/
struct _cain_sip_header_from  {
	cain_sip_header_address_t address;
};

static void cain_sip_header_from_destroy(cain_sip_header_from_t* from) {
}

static void cain_sip_header_from_clone(cain_sip_header_from_t* from, const cain_sip_header_from_t* cloned) {
}


CAIN_SIP_NEW_HEADER(header_from,header_address,"From")
CAIN_SIP_PARSE(header_from)
GET_SET_STRING_PARAM(cain_sip_header_from,tag);

/**************************
* To header object inherits from header_address
****************************
*/
struct _cain_sip_header_to  {
	cain_sip_header_address_t address;
};

static void cain_sip_header_to_destroy(cain_sip_header_to_t* to) {
}

void cain_sip_header_to_clone(cain_sip_header_to_t *contact, const cain_sip_header_to_t *orig){
}

CAIN_SIP_NEW_HEADER(header_to,header_address,"To")
CAIN_SIP_PARSE(header_to)
GET_SET_STRING_PARAM(cain_sip_header_to,tag);

/**************************
* Via header object inherits from parameters
****************************
*/
struct _cain_sip_header_via  {
	cain_sip_parameters_t params_list;
	char* protocol;
	char* transport;
	char* host;
	int port;
};

static void cain_sip_header_via_destroy(cain_sip_header_via_t* via) {
	if (via->host) cain_sip_free(via->host);
	if (via->protocol) cain_sip_free(via->protocol);
}

static void cain_sip_header_via_clone(cain_sip_header_via_t* via, const cain_sip_header_via_t*orig){
}

CAIN_SIP_NEW_HEADER(header_via,header_address,"Via")
CAIN_SIP_PARSE(header_via)
GET_SET_STRING(cain_sip_header_via,protocol);
GET_SET_STRING(cain_sip_header_via,transport);
GET_SET_STRING(cain_sip_header_via,host);
GET_SET_INT_PRIVATE(cain_sip_header_via,port,int,_);

GET_SET_STRING_PARAM(cain_sip_header_via,branch);
GET_SET_STRING_PARAM(cain_sip_header_via,maddr);
GET_SET_STRING_PARAM(cain_sip_header_via,received);

GET_SET_INT_PARAM_PRIVATE(cain_sip_header_via,rport,int,_)
GET_SET_INT_PARAM_PRIVATE(cain_sip_header_via,ttl,int,_)

int cain_sip_header_via_set_rport (cain_sip_header_via_t* obj,int  value) {
	if (value ==-1 || (value>0 && value<65536)) {
		_cain_sip_header_via_set_rport(obj,value);
		return 0;
	} else {
		cain_sip_error("bad rport value [%i] for via",value);
		return -1;
	}
}
int cain_sip_header_via_set_ttl (cain_sip_header_via_t* obj,int  value) {
	if (value ==-1 || (value>0 && value<=255)) {
		_cain_sip_header_via_set_ttl(obj,value);
		return 0;
	} else {
		cain_sip_error("bad ttl value [%i] for via",value);
		return -1;
	}
}

int cain_sip_header_via_set_port (cain_sip_header_via_t* obj,int  value) {
	if (value ==-1 || (value>0 && value<65536)) {
		_cain_sip_header_via_set_port(obj,value);
		return 0;
	} else {
		cain_sip_error("bad port value [%i] for via",value);
		return -1;
	}
}

int cain_sip_header_via_get_listening_port(const cain_sip_header_via_t *via){
	int ret=cain_sip_header_via_get_port(via);
	if (ret==-1) ret=cain_sip_listening_point_get_well_known_port(via->protocol);
	return ret;
}

/**************************
* call_id header object inherits from object
****************************
*/
struct _cain_sip_header_call_id  {
	cain_sip_header_t header;
	const char* call_id;
};

static void cain_sip_header_call_id_destroy(cain_sip_header_call_id_t* call_id) {
	if (call_id->call_id) cain_sip_free((void*)call_id->call_id);
}

static void cain_sip_header_call_id_clone(cain_sip_header_call_id_t* call_id,const cain_sip_header_call_id_t *orig){
}

CAIN_SIP_NEW_HEADER(header_call_id,header,"Call-ID")
CAIN_SIP_PARSE(header_call_id)
GET_SET_STRING(cain_sip_header_call_id,call_id);
/**************************
* cseq header object inherent from object
****************************
*/
struct _cain_sip_header_cseq  {
	cain_sip_header_t header;
	const char* method;
	unsigned int seq_number;
};

static void cain_sip_header_cseq_destroy(cain_sip_header_cseq_t* cseq) {
	if (cseq->method) cain_sip_free((void*)cseq->method);
}

static void cain_sip_header_cseq_clone(cain_sip_header_cseq_t* cseq, const cain_sip_header_cseq_t *orig) {
	if (cseq->method) cain_sip_free((void*)cseq->method);
}

CAIN_SIP_NEW_HEADER(header_cseq,header,"CSeq")
CAIN_SIP_PARSE(header_cseq)
GET_SET_STRING(cain_sip_header_cseq,method);
GET_SET_INT(cain_sip_header_cseq,seq_number,unsigned int)
/**************************
* content type header object inherent from parameters
****************************
*/
struct _cain_sip_header_content_type  {
	cain_sip_parameters_t params_list;
	const char* type;
	const char* subtype;
};

static void cain_sip_header_content_type_destroy(cain_sip_header_content_type_t* content_type) {
	if (content_type->type) cain_sip_free((void*)content_type->type);
	if (content_type->subtype) cain_sip_free((void*)content_type->subtype);
}

static void cain_sip_header_content_type_clone(cain_sip_header_content_type_t* content_type, const cain_sip_header_content_type_t* orig){
}

CAIN_SIP_NEW_HEADER(header_content_type,parameters,"Content-Type")
CAIN_SIP_PARSE(header_content_type)
GET_SET_STRING(cain_sip_header_content_type,type);
GET_SET_STRING(cain_sip_header_content_type,subtype);
/**************************
* Route header object inherent from header_address
****************************
*/
struct _cain_sip_header_route  {
	cain_sip_header_address_t address;
};

static void cain_sip_header_route_destroy(cain_sip_header_route_t* route) {
}

static void cain_sip_header_route_clone(cain_sip_header_route_t* route, const cain_sip_header_route_t* orig) {
}

CAIN_SIP_NEW_HEADER(header_route,header_address,"Route")
CAIN_SIP_PARSE(header_route)
/**************************
* Record route header object inherent from header_address
****************************
*/
struct _cain_sip_header_record_route  {
	cain_sip_header_address_t address;
};

static void cain_sip_header_record_route_destroy(cain_sip_header_record_route_t* record_route) {
}

static void cain_sip_header_record_route_clone(cain_sip_header_record_route_t* record_route,
                                const cain_sip_header_record_route_t* orig               ) {
}

CAIN_SIP_NEW_HEADER(header_record_route,header_address,"Record-Route")
CAIN_SIP_PARSE(header_record_route)
/**************************
* content length header object inherent from object
****************************
*/
struct _cain_sip_header_content_length  {
	cain_sip_header_t header;
	unsigned int content_length;
};

static void cain_sip_header_content_length_destroy(cain_sip_header_content_length_t* content_length) {
}

static void cain_sip_header_content_length_clone(cain_sip_header_content_length_t* content_length,
                                                 const cain_sip_header_content_length_t *orig ) {
}


CAIN_SIP_NEW_HEADER(header_content_length,header,"Content-Length")
CAIN_SIP_PARSE(header_content_length)
GET_SET_INT(cain_sip_header_content_length,content_length,unsigned int)
/******************************
 * Extension header hinerite from header
 *
 ******************************/
struct _cain_sip_header_extension  {
	cain_sip_header_t header;
	const char* value;
};

static void cain_sip_header_extension_destroy(cain_sip_header_extension_t* extension) {
	if (extension->value) cain_sip_free((void*)extension->value);
}

static void cain_sip_header_extension_clone(cain_sip_header_extension_t* extension, const cain_sip_header_extension_t* orig){
}

CAIN_SIP_NEW_HEADER(header_extension,header,NULL)

/**
 * special case for this header. I don't know why
 */
cain_sip_header_extension_t* cain_sip_header_extension_parse (const char* value) {
	pANTLR3_INPUT_STREAM           input;
	pcain_sip_messageLexer               lex;
	pANTLR3_COMMON_TOKEN_STREAM    tokens;
	pcain_sip_messageParser              parser;
	input  = antlr3NewAsciiStringCopyStream	(
			(pANTLR3_UINT8)value,
			(ANTLR3_UINT32)strlen(value),
			((void *)0));
	lex    = cain_sip_messageLexerNew                (input);
	tokens = antlr3CommonTokenStreamSourceNew  (1025, lex->pLexer->rec->state->tokSource);
	parser = cain_sip_messageParserNew               (tokens);
	cain_sip_messageParser_header_extension_return l_parsed_object = parser->header_extension(parser,FALSE);
	parser ->free(parser);
	tokens ->free(tokens);
	lex    ->free(lex);
	input  ->close(input);
	if (l_parsed_object.ret == NULL) cain_sip_error("Parser error for [%s]",value);\
	return CAIN_SIP_HEADER_EXTENSION(l_parsed_object.ret);
}
GET_SET_STRING(cain_sip_header_extension,value);
/**************************
*Authorization header object inherent from parameters
****************************
*/
#define AUTH_BASE \
	cain_sip_parameters_t params_list; \
	const char* scheme; \
	const char* realm; \
	const char* nonce; \
	const char* algorithm; \
	const char* opaque; \
	const char* qop; \

#define AUTH_BASE_DESTROY(obj) \
	if (obj->scheme) cain_sip_free((void*)obj->scheme);\
	if (obj->realm) cain_sip_free((void*)obj->realm);\
	if (obj->nonce) cain_sip_free((void*)obj->nonce);\
	if (obj->algorithm) cain_sip_free((void*)obj->algorithm);\
	if (obj->opaque) cain_sip_free((void*)obj->opaque);\
	if (obj->qop) cain_sip_free((void*)obj->qop);\



struct _cain_sip_header_authorization  {
	AUTH_BASE
	const char* username;
	cain_sip_uri_t* uri;
	const char* response;
	const char* cnonce;
	int nonce_count;

};


static void cain_sip_header_authorization_destroy(cain_sip_header_authorization_t* authorization) {
	if (authorization->username) cain_sip_free((void*)authorization->username);
	if (authorization->uri) {
			cain_sip_object_unref(CAIN_SIP_OBJECT(authorization->uri));
	}
	if (authorization->cnonce) cain_sip_free((void*)authorization->cnonce);
	AUTH_BASE_DESTROY(authorization)
}

static void cain_sip_header_authorization_clone(cain_sip_header_authorization_t* authorization,
                                                 const cain_sip_header_authorization_t *orig ) {
}
static void cain_sip_header_authorization_init(cain_sip_header_authorization_t* authorization) {
}

cain_sip_uri_t* cain_sip_header_authorization_get_uri(const cain_sip_header_authorization_t* authorization) {
	return authorization->uri;
}

void cain_sip_header_authorization_set_uri(cain_sip_header_authorization_t* authorization, cain_sip_uri_t* uri) {
	if (authorization->uri) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(authorization->uri));
	}
	authorization->uri=uri;
	if (authorization->uri) cain_sip_object_ref(authorization->uri);
}
CAIN_SIP_NEW_HEADER(header_authorization,parameters,"Authorization")
CAIN_SIP_PARSE(header_authorization)
GET_SET_STRING(cain_sip_header_authorization,scheme);
GET_SET_STRING(cain_sip_header_authorization,username);
GET_SET_STRING(cain_sip_header_authorization,realm);
GET_SET_STRING(cain_sip_header_authorization,nonce);
GET_SET_STRING(cain_sip_header_authorization,response);
GET_SET_STRING(cain_sip_header_authorization,algorithm);
GET_SET_STRING(cain_sip_header_authorization,cnonce);
GET_SET_STRING(cain_sip_header_authorization,opaque);
GET_SET_STRING(cain_sip_header_authorization,qop);
GET_SET_INT(cain_sip_header_authorization,nonce_count,int)

/**************************
*Proxy-Authorization header object inherent from parameters
****************************
*/
struct _cain_sip_header_proxy_authorization  {
	cain_sip_header_authorization_t authorization;
};


static void cain_sip_header_proxy_authorization_destroy(cain_sip_header_proxy_authorization_t* proxy_authorization) {
}

static void cain_sip_header_proxy_authorization_clone(cain_sip_header_proxy_authorization_t* proxy_authorization,
                                                 const cain_sip_header_proxy_authorization_t *orig ) {
}
CAIN_SIP_NEW_HEADER(header_proxy_authorization,header_authorization,"Proxy-Authorization")
CAIN_SIP_PARSE(header_proxy_authorization)
/**************************
*WWW-Authorization header object inherent from parameters
****************************
*/
struct _cain_sip_header_www_authenticate  {
	AUTH_BASE
	const char* domain;
	unsigned int stale;
};


static void cain_sip_header_www_authenticate_destroy(cain_sip_header_www_authenticate_t* www_authenticate) {
}

static void cain_sip_header_www_authenticate_clone(cain_sip_header_www_authenticate_t* www_authenticate,
                                                 const cain_sip_header_www_authenticate_t *orig ) {
}
CAIN_SIP_NEW_HEADER(header_www_authenticate,parameters,"WWW-Authenticdate")
CAIN_SIP_PARSE(header_www_authenticate)
GET_SET_STRING(cain_sip_header_www_authenticate,scheme);
GET_SET_STRING(cain_sip_header_www_authenticate,realm);
GET_SET_STRING(cain_sip_header_www_authenticate,nonce);
GET_SET_STRING(cain_sip_header_www_authenticate,algorithm);
GET_SET_STRING(cain_sip_header_www_authenticate,opaque);
GET_SET_STRING(cain_sip_header_www_authenticate,qop);
GET_SET_STRING(cain_sip_header_www_authenticate,domain)
GET_SET_BOOL(cain_sip_header_www_authenticate,stale,is)
