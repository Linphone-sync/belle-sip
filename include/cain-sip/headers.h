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

#ifndef HEADERS_H_
#define HEADERS_H_

#include "cain-sip/uri.h"



/***************************************************************************************
 * header address
 *
 **************************************************************************************/

typedef struct _cain_sip_header_address cain_sip_header_address_t;

cain_sip_header_address_t* cain_sip_header_address_new();

cain_sip_header_address_t* cain_sip_header_address_parse (const char* address) ;

/**
 *
 */
cain_sip_uri_t* cain_sip_header_address_get_uri(const cain_sip_header_address_t* address);
/**
 *
 */
void cain_sip_header_address_set_uri(cain_sip_header_address_t* address, cain_sip_uri_t* uri);

/**
 *
 */
const char* cain_sip_header_address_get_displayname(const cain_sip_header_address_t* address);
/**
 *
 */
void cain_sip_header_address_set_displayname(cain_sip_header_address_t* address, const char* uri);

#define CAIN_SIP_HEADER_ADDRESS(t) CAIN_SIP_CAST(t,cain_sip_header_address_t)



/***************************************************************************************
 * header common
 *
 **************************************************************************************/

typedef struct _cain_sip_header cain_sip_header_t;
const char* cain_sip_header_get_name (const cain_sip_header_t* obj);
void cain_sip_header_set_name (cain_sip_header_t* obj,const char* value);
int cain_sip_header_marshal(cain_sip_header_t* header, char* buff, unsigned int offset,unsigned int buff_size);

#define CAIN_SIP_HEADER(t) CAIN_SIP_CAST(t,cain_sip_header_t)

/******************************
 *
 * Allow header inherit from header
 *
 ******************************/
typedef struct _cain_sip_header_allow cain_sip_header_allow_t;

cain_sip_header_allow_t* cain_sip_header_allow_new();

cain_sip_header_allow_t* cain_sip_header_allow_parse (const char* allow) ;
const char* cain_sip_header_allow_get_method(const cain_sip_header_allow_t* allow);
void cain_sip_header_allow_set_method(cain_sip_header_allow_t* allow,const char* method);
#define CAIN_SIP_HEADER_ALLOW(t) CAIN_SIP_CAST(t,cain_sip_header_allow_t)

/***********************
 * Contact header object
 ************************/
typedef struct _cain_sip_header_contact cain_sip_header_contact_t;

cain_sip_header_contact_t* cain_sip_header_contact_new();


cain_sip_header_contact_t* cain_sip_header_contact_parse (const char* contact) ;


/**
* Returns the value of the expires parameter or -1 if no expires parameter was specified or if the parameter value cannot be parsed as an int.
*@returns value of the expires parameter measured in delta-seconds, O implies removal of Registration specified in Contact Header.
*
*/
 int	cain_sip_header_contact_get_expires(const cain_sip_header_contact_t* contact);
/**
 * Returns the value of the q-value parameter of this ContactHeader. The q-value parameter indicates the relative preference amongst a set of locations. q-values are decimal numbers from 0 to 1, with higher values indicating higher preference.
 * @return the q-value parameter of this ContactHeader, -1 if the q-value is not set.
 */
 float	cain_sip_header_contact_get_qvalue(const cain_sip_header_contact_t* contact);
 /**
  * Returns a boolean value that indicates if the contact header has the format of Contact: *.
  * @return true if this is a wildcard address, false otherwise.
  */
 unsigned int cain_sip_header_contact_is_wildcard(const cain_sip_header_contact_t* contact);
 /**
 *
 */
 int cain_sip_header_contact_set_expires(cain_sip_header_contact_t* contact, int expires);
/**
 *  Sets the qValue value of the Name Address.
 */
 int cain_sip_header_contact_set_qvalue(cain_sip_header_contact_t* contact, float qvalue);
/**
 * Sets a wildcard on this contact address that is "*" is assigned to the contact header so that the header will have the format of Contact: *.
 *
 */
 void cain_sip_header_contact_set_wildcard(cain_sip_header_contact_t* contact,unsigned int is_wildcard);

#define CAIN_SIP_HEADER_CONTACT(t) CAIN_SIP_CAST(t,cain_sip_header_contact_t)

 /******************************
 * From header object inherent from header_address
 *
 ******************************/
 typedef struct _cain_sip_header_from cain_sip_header_from_t;

 cain_sip_header_from_t* cain_sip_header_from_new();

 cain_sip_header_from_t* cain_sip_header_from_create(const char *address, const char *tag);

 cain_sip_header_from_t* cain_sip_header_from_parse (const char* from) ;


 void cain_sip_header_from_set_tag(cain_sip_header_from_t* from, const char* tag);

 const char* cain_sip_header_from_get_tag(const cain_sip_header_from_t* from);

#define CAIN_SIP_HEADER_FROM(t) CAIN_SIP_CAST(t,cain_sip_header_from_t)
 /******************************
 * To header object inherent from header_address
 *
 ******************************/
 typedef struct _cain_sip_header_to cain_sip_header_to_t;

 cain_sip_header_to_t* cain_sip_header_to_new();

 cain_sip_header_to_t* cain_sip_header_to_parse(const char* to) ;

 cain_sip_header_to_t* cain_sip_header_to_create(const char *address, const char *tag);

 void cain_sip_header_to_set_tag(cain_sip_header_to_t* from, const char* tag);

 const char* cain_sip_header_to_get_tag(const cain_sip_header_to_t* from);

#define CAIN_SIP_HEADER_TO(t) CAIN_SIP_CAST(t,cain_sip_header_to_t)

/******************************
 * Via header object inherent from header_address
 *
 ******************************/
typedef struct _cain_sip_header_via cain_sip_header_via_t;

cain_sip_header_via_t* cain_sip_header_via_new();
cain_sip_header_via_t* cain_sip_header_via_create(const char *host, int port, const char *transport, const char *branch);
cain_sip_header_via_t* cain_sip_header_via_parse (const char* via) ;
const char*	cain_sip_header_via_get_branch(const cain_sip_header_via_t* via);
const char*	cain_sip_header_via_get_transport(const cain_sip_header_via_t* via);
const char*	cain_sip_header_via_get_host(const cain_sip_header_via_t* via);
int cain_sip_header_via_get_port(const cain_sip_header_via_t* via);
int cain_sip_header_via_get_listening_port(const cain_sip_header_via_t *via);

const char*	cain_sip_header_via_get_maddr(const cain_sip_header_via_t* via);
const char*	cain_sip_header_via_get_protocol(const cain_sip_header_via_t* via);
const char*	cain_sip_header_via_get_received(const cain_sip_header_via_t* via);
int cain_sip_header_via_get_rport(const cain_sip_header_via_t* via);
int	cain_sip_header_via_get_ttl(const cain_sip_header_via_t* via);

void cain_sip_header_via_set_branch(cain_sip_header_via_t* via,const char* branch);
void cain_sip_header_via_set_host(cain_sip_header_via_t* via, const char* host);
int cain_sip_header_via_set_port(cain_sip_header_via_t* via,int port);
void cain_sip_header_via_set_maddr(cain_sip_header_via_t* via, const char* maddr);
void cain_sip_header_via_set_protocol(cain_sip_header_via_t* via, const char* protocol);
void cain_sip_header_via_set_received(cain_sip_header_via_t* via, const char* received);
int cain_sip_header_via_set_rport(cain_sip_header_via_t* via,int rport);
void cain_sip_header_via_set_transport(cain_sip_header_via_t* via,const char* transport);
int cain_sip_header_via_set_ttl(cain_sip_header_via_t* via, int ttl);
#define CAIN_SIP_HEADER_VIA(t) CAIN_SIP_CAST(t,cain_sip_header_via_t)

/******************************
 * Call id object inherent from object
 *
 ******************************/
typedef struct _cain_sip_header_call_id cain_sip_header_call_id_t;

cain_sip_header_call_id_t* cain_sip_header_call_id_new();

cain_sip_header_call_id_t* cain_sip_header_call_id_parse (const char* call_id) ;
const char*	cain_sip_header_call_id_get_call_id(const cain_sip_header_call_id_t* call_id);
void cain_sip_header_call_id_set_call_id(cain_sip_header_call_id_t* via,const char* call_id);
#define CAIN_SIP_HEADER_CALL_ID(t) CAIN_SIP_CAST(t,cain_sip_header_call_id_t)
/******************************
 * cseq object inherent from object
 *
 ******************************/
typedef struct _cain_sip_header_cseq cain_sip_header_cseq_t;

cain_sip_header_cseq_t* cain_sip_header_cseq_new();
cain_sip_header_cseq_t* cain_sip_header_cseq_create(unsigned int number, const char *method);
cain_sip_header_cseq_t* cain_sip_header_cseq_parse (const char* cseq) ;
const char*	cain_sip_header_cseq_get_method(const cain_sip_header_cseq_t* cseq);
void cain_sip_header_cseq_set_method(cain_sip_header_cseq_t* cseq,const char* method);
unsigned int	cain_sip_header_cseq_get_seq_number(const cain_sip_header_cseq_t* cseq);
void cain_sip_header_cseq_set_seq_number(cain_sip_header_cseq_t* cseq,unsigned int seq_number);
#define CAIN_SIP_HEADER_CSEQ(t) CAIN_SIP_CAST(t,cain_sip_header_cseq_t)
/******************************
 * content type object inherent from parameters
 *
 ******************************/
typedef struct _cain_sip_header_content_type cain_sip_header_content_type_t;

cain_sip_header_content_type_t* cain_sip_header_content_type_new();

cain_sip_header_content_type_t* cain_sip_header_content_type_parse (const char* content_type) ;
const char*	cain_sip_header_content_type_get_type(const cain_sip_header_content_type_t* content_type);
void cain_sip_header_content_type_set_type(cain_sip_header_content_type_t* content_type,const char* type);
const char*	cain_sip_header_content_type_get_subtype(const cain_sip_header_content_type_t* content_type);
void cain_sip_header_content_type_set_subtype(cain_sip_header_content_type_t* content_type,const char* sub_type);
#define CAIN_SIP_HEADER_CONTENT_TYPE(t) CAIN_SIP_CAST(t,cain_sip_header_content_type_t)
/******************************
 *
 * Expires inherit from header
 *
 ******************************/
typedef struct _cain_sip_header_expires cain_sip_header_expires_t;

cain_sip_header_expires_t* cain_sip_header_expires_new();

cain_sip_header_expires_t* cain_sip_header_expires_parse (const char* expires) ;
int cain_sip_header_expires_get_expires(const cain_sip_header_expires_t* expires);
void cain_sip_header_expires_set_expires(cain_sip_header_expires_t* expires,int value);
int cain_sip_header_expires_decrement_expires(cain_sip_header_expires_t* expires);
cain_sip_header_expires_t* cain_sip_header_expires_create(int expires);
#define CAIN_SIP_HEADER_EXPIRES(t) CAIN_SIP_CAST(t,cain_sip_header_expires_t)

/******************************
 * Route header object inherent from header_address
 *
 ******************************/
 typedef struct _cain_sip_header_route cain_sip_header_route_t;

 cain_sip_header_route_t* cain_sip_header_route_new();
 cain_sip_header_route_t* cain_sip_header_route_parse (const char* route) ;

#define CAIN_SIP_HEADER_ROUTE(t) CAIN_SIP_CAST(t,cain_sip_header_route_t)
/******************************
 * Record route header object inherent from header_address
 *
 ******************************/
 typedef struct _cain_sip_header_record_route cain_sip_header_record_route_t;

 cain_sip_header_record_route_t* cain_sip_header_record_route_new();
 cain_sip_header_record_route_t* cain_sip_header_record_route_parse (const char* route) ;

#define CAIN_SIP_HEADER_RECORD_ROUTE(t) CAIN_SIP_CAST(t,cain_sip_header_record_route_t)
 /******************************
  *
  * user-Agent header inherit from header
  *
  ******************************/
 typedef struct _cain_sip_header_user_agent cain_sip_header_user_agent_t;

 cain_sip_header_user_agent_t* cain_sip_header_user_agent_new();

 cain_sip_header_user_agent_t* cain_sip_header_user_agent_parse (const char* user_agent) ;
 cain_sip_list_t* cain_sip_header_user_agent_get_products(const cain_sip_header_user_agent_t* user_agent);
 void cain_sip_header_user_agent_set_products(cain_sip_header_user_agent_t* user_agent,cain_sip_list_t* value);
 void cain_sip_header_user_agent_add_product(cain_sip_header_user_agent_t* user_agent,const char* product);
 #define CAIN_SIP_HEADER_USER_AGENT(t) CAIN_SIP_CAST(t,cain_sip_header_user_agent_t)

 /******************************
 * Content length inherent from object
 *
 ******************************/
typedef struct _cain_sip_header_content_length cain_sip_header_content_length_t;

cain_sip_header_content_length_t* cain_sip_header_content_length_new();

cain_sip_header_content_length_t* cain_sip_header_content_length_parse (const char* content_length) ;
unsigned int cain_sip_header_content_length_get_content_length(const cain_sip_header_content_length_t* content_length);
void cain_sip_header_content_length_set_content_length(cain_sip_header_content_length_t* content_length,unsigned int length);
#define CAIN_SIP_HEADER_CONTENT_LENGTH(t) CAIN_SIP_CAST(t,cain_sip_header_content_length_t)

/******************************
 * authorization header inherit from parameters
 *
 ******************************/
typedef struct _cain_sip_header_authorization cain_sip_header_authorization_t;

cain_sip_header_authorization_t* cain_sip_header_authorization_new();
cain_sip_header_authorization_t* cain_sip_header_authorization_parse(const char* authorization);
const char*	cain_sip_header_authorization_get_algorithm(const cain_sip_header_authorization_t* authorization );
const char*	cain_sip_header_authorization_get_cnonce(const cain_sip_header_authorization_t* authorization );
const char* cain_sip_header_authorization_get_nonce(const cain_sip_header_authorization_t* authorization);
int	cain_sip_header_authorization_get_nonce_count(const cain_sip_header_authorization_t* authorization);
const char*	cain_sip_header_authorization_get_opaque(const cain_sip_header_authorization_t* authorization);
const char*	cain_sip_header_authorization_get_qop(const cain_sip_header_authorization_t* authorization);
const char*	cain_sip_header_authorization_get_realm(const cain_sip_header_authorization_t* authorization);
const char*	cain_sip_header_authorization_get_response(const cain_sip_header_authorization_t* authorization);
const char*	cain_sip_header_authorization_get_scheme(const cain_sip_header_authorization_t* authorization);
cain_sip_uri_t* cain_sip_header_authorization_get_uri(const cain_sip_header_authorization_t* authorization);
const char*	cain_sip_header_authorization_get_username(const cain_sip_header_authorization_t* authorization);
void cain_sip_header_authorization_set_algorithm(cain_sip_header_authorization_t* authorization, const char* algorithm);
void cain_sip_header_authorization_set_cnonce(cain_sip_header_authorization_t* authorization, const char* cNonce);
void cain_sip_header_authorization_set_nonce(cain_sip_header_authorization_t* authorization, const char* nonce);
void cain_sip_header_authorization_set_nonce_count(cain_sip_header_authorization_t* authorization, int nonceCount);
void cain_sip_header_authorization_set_opaque(cain_sip_header_authorization_t* authorization, const char* opaque);
void cain_sip_header_authorization_set_qop(cain_sip_header_authorization_t* authorization, const char* qop);
void cain_sip_header_authorization_add_qop(cain_sip_header_authorization_t* authorization, const char* qop);
void cain_sip_header_authorization_set_realm(cain_sip_header_authorization_t* authorization, const char* realm);
void cain_sip_header_authorization_set_response(cain_sip_header_authorization_t* authorization, const char* response);
void cain_sip_header_authorization_set_scheme(cain_sip_header_authorization_t* authorization, const char* scheme);
void cain_sip_header_authorization_set_uri(cain_sip_header_authorization_t* authorization, cain_sip_uri_t* uri);
void cain_sip_header_authorization_set_username(cain_sip_header_authorization_t* authorization, const char* username);

#define CAIN_SIP_HEADER_AUTHORIZATION(t) CAIN_SIP_CAST(t,cain_sip_header_authorization_t)

/*******************************
 * proxy_authorization inherit from Authorization
 */
typedef struct _cain_sip_header_proxy_authorization cain_sip_header_proxy_authorization_t;
cain_sip_header_proxy_authorization_t* cain_sip_header_proxy_authorization_new();
cain_sip_header_proxy_authorization_t* cain_sip_header_proxy_authorization_parse(const char* proxy_authorization);
#define CAIN_SIP_HEADER_PROXY_AUTHORIZATION(t) CAIN_SIP_CAST(t,cain_sip_header_proxy_authorization_t)

/*******************************
 * www_authenticate inherit from parameters
 */
typedef struct _cain_sip_header_www_authenticate cain_sip_header_www_authenticate_t;
cain_sip_header_www_authenticate_t* cain_sip_header_www_authenticate_new();
cain_sip_header_www_authenticate_t* cain_sip_header_www_authenticate_parse(const char* www_authenticate);
const char*	cain_sip_header_www_authenticate_get_algorithm(const cain_sip_header_www_authenticate_t* www_authenticate );
const char* cain_sip_header_www_authenticate_get_nonce(const cain_sip_header_www_authenticate_t* www_authenticate);
const char*	cain_sip_header_www_authenticate_get_opaque(const cain_sip_header_www_authenticate_t* www_authenticate);
cain_sip_list_t* cain_sip_header_www_authenticate_get_qop(const cain_sip_header_www_authenticate_t* www_authetication);
const char*	cain_sip_header_www_authenticate_get_realm(const cain_sip_header_www_authenticate_t* www_authenticate);
const char*	cain_sip_header_www_authenticate_get_scheme(const cain_sip_header_www_authenticate_t* www_authenticate);
const char*	cain_sip_header_www_authenticate_get_domain(const cain_sip_header_www_authenticate_t* www_authenticate);
unsigned int cain_sip_header_www_authenticate_is_stale(const cain_sip_header_www_authenticate_t* www_authenticate);
void cain_sip_header_www_authenticate_set_algorithm(cain_sip_header_www_authenticate_t* www_authenticate, const char* algorithm);
void cain_sip_header_www_authenticate_set_nonce(cain_sip_header_www_authenticate_t* www_authenticate, const char* nonce);
void cain_sip_header_www_authenticate_set_opaque(cain_sip_header_www_authenticate_t* www_authenticate, const char* opaque);
void cain_sip_header_www_authenticate_set_qop(cain_sip_header_www_authenticate_t* www_authentication, cain_sip_list_t*  qop);
void cain_sip_header_www_authenticate_add_qop(cain_sip_header_www_authenticate_t* www_authentication, const char*  qop_param);
void cain_sip_header_www_authenticate_set_realm(cain_sip_header_www_authenticate_t* www_authenticate, const char* realm);
void cain_sip_header_www_authenticate_set_scheme(cain_sip_header_www_authenticate_t* www_authenticate, const char* scheme);
void cain_sip_header_www_authenticate_set_domain(cain_sip_header_www_authenticate_t* www_authenticate,const char* domain);
void cain_sip_header_www_authenticate_set_stale(cain_sip_header_www_authenticate_t* www_authenticate, unsigned int enable);
#define CAIN_SIP_HEADER_WWW_AUTHENTICATE(t) CAIN_SIP_CAST(t,cain_sip_header_www_authenticate_t)

/*******************************
 * proxy_authenticate inherit from www_authenticate
 */
typedef struct _cain_sip_header_proxy_authenticate cain_sip_header_proxy_authenticate_t;
cain_sip_header_proxy_authenticate_t* cain_sip_header_proxy_authenticate_new();
cain_sip_header_proxy_authenticate_t* cain_sip_header_proxy_authenticate_parse(const char* proxy_authenticate);
#define CAIN_SIP_HEADER_PROXY_AUTHENTICATE(t) CAIN_SIP_CAST(t,cain_sip_header_proxy_authenticate_t)


/******************************
 *
 * Extension header inherit from header
 *
 ******************************/
typedef struct _cain_sip_header_extension cain_sip_header_extension_t;

cain_sip_header_extension_t* cain_sip_header_extension_new();

cain_sip_header_extension_t* cain_sip_header_extension_parse (const char* extension) ;
const char* cain_sip_header_extension_get_value(const cain_sip_header_extension_t* extension);
void cain_sip_header_extension_set_value(cain_sip_header_extension_t* extension,const char* value);
#define CAIN_SIP_HEADER_EXTENSION(t) CAIN_SIP_CAST(t,cain_sip_header_extension_t)
/******************************
 *
 * Max forward inherit from header
 *
 ******************************/
typedef struct _cain_sip_header_max_forwards cain_sip_header_max_forwards_t;

cain_sip_header_max_forwards_t* cain_sip_header_max_forwards_new();

cain_sip_header_max_forwards_t* cain_sip_header_max_forwards_parse (const char* max_forwards) ;
int cain_sip_header_max_forwards_get_max_forwards(const cain_sip_header_max_forwards_t* max_forwards);
void cain_sip_header_max_forwards_set_max_forwards(cain_sip_header_max_forwards_t* max_forwards,int value);
int cain_sip_header_max_forwards_decrement_max_forwards(cain_sip_header_max_forwards_t* max_forwards);
#define CAIN_SIP_HEADER_MAX_FORWARDS(t) CAIN_SIP_CAST(t,cain_sip_header_max_forwards_t)


#endif /* HEADERS_H_ */
