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
#include "listeningpoint_internal.h"

/************************
 * header
 ***********************/

GET_SET_STRING(cain_sip_header,name);

cain_sip_header_t* cain_sip_header_create (const char* name,const char* value) {
	return CAIN_SIP_HEADER(cain_sip_header_extension_create(name,value));
}
void cain_sip_header_init(cain_sip_header_t *header) {

}
static void cain_sip_header_clone(cain_sip_header_t *header, const cain_sip_header_t *orig){
	CLONE_STRING(cain_sip_header,name,header,orig)
	if (cain_sip_header_get_next(orig)) {
		cain_sip_header_set_next(header,CAIN_SIP_HEADER(cain_sip_object_clone(CAIN_SIP_OBJECT(cain_sip_header_get_next(orig))))) ;
	}
}
static void cain_sip_header_destroy(cain_sip_header_t *header){
	if (header->name) cain_sip_free((void*)header->name);
	if (header->next) cain_sip_object_unref(CAIN_SIP_OBJECT(header->next));
}
void cain_sip_header_set_next(cain_sip_header_t* header,cain_sip_header_t* next) {
	header->next = next;
}
cain_sip_header_t* cain_sip_header_get_next(const cain_sip_header_t* header) {
	return header->next;
}

int cain_sip_header_marshal(cain_sip_header_t* header, char* buff,unsigned int offset,unsigned int buff_size) {
	if (header->name) {
		return snprintf(buff+offset,buff_size-offset,"%s: ",header->name);
	} else {
		cain_sip_warning("no header name found");
		return 0;
	}
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_header_t);

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_header_t,cain_sip_object_t,cain_sip_header_destroy,cain_sip_header_clone,cain_sip_header_marshal,TRUE);


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

static void cain_sip_header_address_destroy(cain_sip_header_address_t* address) {
	if (address->displayname) cain_sip_free((void*)(address->displayname));
	if (address->uri) cain_sip_object_unref(CAIN_SIP_OBJECT(address->uri));
}

static void cain_sip_header_address_clone(cain_sip_header_address_t *addr, const cain_sip_header_address_t *orig){
	CLONE_STRING(cain_sip_header_address,displayname,addr,orig)
	if (cain_sip_header_address_get_uri(orig)) {
		cain_sip_header_address_set_uri(addr,CAIN_SIP_URI(cain_sip_object_clone(CAIN_SIP_OBJECT(cain_sip_header_address_get_uri(orig)))));
	}
}

int cain_sip_header_address_marshal(cain_sip_header_address_t* header, char* buff,unsigned int offset,unsigned int buff_size) {
	/*1 display name*/
	unsigned int current_offset=offset;
	if (header->displayname) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"\"%s\" ",header->displayname);;
	}
	if (header->uri) {
		/*cases where < is required*/
		if (header->displayname
			|| cain_sip_parameters_get_parameter_names((cain_sip_parameters_t*)header->uri)
			|| cain_sip_uri_get_header_names(header->uri)
			|| cain_sip_parameters_get_parameter_names(&header->base)) {
			current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s","<");
		}
		current_offset+=cain_sip_uri_marshal(header->uri,buff,current_offset,buff_size);
		if (header->displayname
				|| cain_sip_parameters_get_parameter_names((cain_sip_parameters_t*)header->uri)
				|| cain_sip_uri_get_header_names(header->uri)
				|| cain_sip_parameters_get_parameter_names(&header->base)) {
			current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s",">");
		}
	}
	current_offset+=cain_sip_parameters_marshal(&header->base,buff,current_offset,buff_size);
	return current_offset-offset;
}

CAIN_SIP_NEW_HEADER(header_address,parameters,"header_address")
CAIN_SIP_PARSE(header_address)
GET_SET_STRING(cain_sip_header_address,displayname);

void cain_sip_header_address_set_quoted_displayname(cain_sip_header_address_t* address,const char* value) {
		if (address->displayname != NULL) cain_sip_free((void*)(address->displayname));
		address->displayname=_cain_sip_str_dup_and_unquote_string(value);
}
cain_sip_uri_t* cain_sip_header_address_get_uri(const cain_sip_header_address_t* address) {
	return address->uri;
}

void cain_sip_header_address_set_uri(cain_sip_header_address_t* address, cain_sip_uri_t* uri) {
	address->uri=(cain_sip_uri_t*)cain_sip_object_ref(uri);
}

cain_sip_header_address_t* cain_sip_header_address_create(const char* display, cain_sip_uri_t* uri) {
	cain_sip_header_address_t* address = cain_sip_header_address_new();
	cain_sip_header_address_set_displayname(address,display);
	cain_sip_header_address_set_uri(address,uri);
	return address;
}

/******************************
 * Extension header inherits from header
 *
 ******************************/
struct _cain_sip_header_allow  {
	cain_sip_header_t header;
	const char* method;
};
static void cain_sip_header_allow_clone(cain_sip_header_allow_t *allow, const cain_sip_header_allow_t *orig){
	CLONE_STRING(cain_sip_header_allow,method,allow,orig)
}
static void cain_sip_header_allow_destroy(cain_sip_header_allow_t* allow) {
	if (allow->method) cain_sip_free((void*)allow->method);
}


int cain_sip_header_allow_marshal(cain_sip_header_allow_t* allow, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(allow), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s",allow->method);
	return current_offset-offset;

}
CAIN_SIP_NEW_HEADER(header_allow,header,"Allow")
CAIN_SIP_PARSE(header_allow)
cain_sip_header_allow_t* cain_sip_header_allow_create (const char* methods) {
	cain_sip_header_allow_t* allow = cain_sip_header_allow_new();
	cain_sip_header_allow_set_method(allow,methods);
	return allow;
}
GET_SET_STRING(cain_sip_header_allow,method);



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
	contact->wildcard=orig->wildcard;
}
int cain_sip_header_contact_marshal(cain_sip_header_contact_t* contact, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(contact), buff,offset, buff_size);
	if (contact->wildcard) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s","*");
	} else {
		current_offset+=cain_sip_header_address_marshal(&contact->address, buff,current_offset, buff_size);
	}
	return current_offset-offset;
}
CAIN_SIP_NEW_HEADER(header_contact,header_address,CAIN_SIP_CONTACT)
CAIN_SIP_PARSE(header_contact)
cain_sip_header_contact_t* cain_sip_header_contact_create (const cain_sip_header_address_t* contact) {
	cain_sip_header_contact_t* header = cain_sip_header_contact_new();
	cain_sip_header_address_clone(CAIN_SIP_HEADER_ADDRESS(header),contact);
	return header;
}
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
#define CAIN_SIP_FROM_LIKE_MARSHAL(header) \
		unsigned int current_offset=offset; \
		current_offset+=cain_sip_##header_marshal(CAIN_SIP_HEADER(header), buff,current_offset, buff_size);\
		current_offset+=cain_sip_header_address_marshal(&header->address, buff,current_offset, buff_size); \
		return current_offset-offset;

struct _cain_sip_header_from  {
	cain_sip_header_address_t address;
};

static void cain_sip_header_from_destroy(cain_sip_header_from_t* from) {
}

static void cain_sip_header_from_clone(cain_sip_header_from_t* from, const cain_sip_header_from_t* cloned) {
}
int cain_sip_header_from_marshal(cain_sip_header_from_t* from, char* buff,unsigned int offset,unsigned int buff_size) {
	CAIN_SIP_FROM_LIKE_MARSHAL(from);
}

cain_sip_header_from_t* cain_sip_header_from_create2(const char *address, const char *tag){
	char *tmp=cain_sip_strdup_printf("From: %s",address);
	cain_sip_header_from_t *from=cain_sip_header_from_parse(tmp);
	if (from){
		if (tag) cain_sip_header_from_set_tag(from,tag);
	}
	cain_sip_free(tmp);
	return from;
}
cain_sip_header_from_t* cain_sip_header_from_create(const cain_sip_header_address_t* address, const char *tag) {
	cain_sip_header_from_t* header= cain_sip_header_from_new();
	_cain_sip_object_copy((cain_sip_object_t*)header,(cain_sip_object_t*)address);
	/*cain_sip_header_set_name(CAIN_SIP_HEADER(header),CAIN_SIP_FROM);*/ /*restaure header name*/
	if (tag) cain_sip_header_from_set_tag(header,tag);
	return header;
}
CAIN_SIP_NEW_HEADER(header_from,header_address,CAIN_SIP_FROM)
CAIN_SIP_PARSE(header_from)
GET_SET_STRING_PARAM2(cain_sip_header_from,tag,raw_tag);

void cain_sip_header_from_set_random_tag(cain_sip_header_from_t *obj){
	char tmp[8];
	/*not less than 32bit */
	cain_sip_header_from_set_raw_tag(obj,cain_sip_random_token(tmp,sizeof(tmp)));
}

void cain_sip_header_from_set_tag(cain_sip_header_from_t *obj, const char *tag){
	if (tag==CAIN_SIP_RANDOM_TAG) cain_sip_header_from_set_random_tag(obj);
	else cain_sip_header_from_set_raw_tag(obj,tag);
}

const char *cain_sip_header_from_get_tag(const cain_sip_header_from_t *obj){
	return cain_sip_header_from_get_raw_tag(obj);
}

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
int cain_sip_header_to_marshal(cain_sip_header_to_t* to, char* buff,unsigned int offset,unsigned int buff_size) {
	CAIN_SIP_FROM_LIKE_MARSHAL(to)
}

CAIN_SIP_NEW_HEADER(header_to,header_address,CAIN_SIP_TO)
CAIN_SIP_PARSE(header_to)
GET_SET_STRING_PARAM2(cain_sip_header_to,tag,raw_tag);

cain_sip_header_to_t* cain_sip_header_to_create2(const char *address, const char *tag){
	char *tmp=cain_sip_strdup_printf("To: %s",address);
	cain_sip_header_to_t *to=cain_sip_header_to_parse(tmp);
	if (to){
		if (tag) cain_sip_header_to_set_tag(to,tag);
	}
	cain_sip_free(tmp);
	return to;
}
cain_sip_header_to_t* cain_sip_header_to_create(const cain_sip_header_address_t* address, const char *tag) {
	cain_sip_header_to_t* header= cain_sip_header_to_new();
	_cain_sip_object_copy((cain_sip_object_t*)header,(cain_sip_object_t*)address);
	cain_sip_header_set_name(CAIN_SIP_HEADER(header),CAIN_SIP_TO); /*restaure header name*/
	if (tag) cain_sip_header_to_set_tag(header,tag);
	return header;
}
void cain_sip_header_to_set_random_tag(cain_sip_header_to_t *obj){
	char tmp[8];
	/*not less than 32bit */
	cain_sip_header_to_set_tag(obj,cain_sip_random_token(tmp,sizeof(tmp)));
}

void cain_sip_header_to_set_tag(cain_sip_header_to_t *obj, const char *tag){
	if (tag==CAIN_SIP_RANDOM_TAG) cain_sip_header_to_set_random_tag(obj);
	else cain_sip_header_to_set_raw_tag(obj,tag);
}

const char *cain_sip_header_to_get_tag(const cain_sip_header_to_t *obj){
	return cain_sip_header_to_get_raw_tag(obj);
}


/******************************
 * User-Agent header inherits from header
 *
 ******************************/
struct _cain_sip_header_user_agent  {
	cain_sip_header_t header;
	cain_sip_list_t* products;
};

static void cain_sip_header_user_agent_destroy(cain_sip_header_user_agent_t* user_agent) {
	cain_sip_header_user_agent_set_products(user_agent,NULL);
}

static void cain_sip_header_user_agent_clone(cain_sip_header_user_agent_t* user_agent, const cain_sip_header_user_agent_t* orig){
	cain_sip_list_t* list=orig->products;
	for(;list!=NULL;list=list->next){
		cain_sip_header_user_agent_add_product(user_agent,(const char *)list->data);
	}
}
int cain_sip_header_user_agent_marshal(cain_sip_header_user_agent_t* user_agent, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	cain_sip_list_t* list = user_agent->products;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(user_agent), buff,current_offset, buff_size);
	for(;list!=NULL;list=list->next){
		current_offset+=snprintf(	buff+current_offset
									,buff_size-current_offset
									," %s"
									,(const char *)list->data);
	}
	return current_offset-offset;

}
CAIN_SIP_NEW_HEADER(header_user_agent,header,"User-Agent")
CAIN_SIP_PARSE(header_user_agent)
cain_sip_list_t* cain_sip_header_user_agent_get_products(const cain_sip_header_user_agent_t* user_agent) {
	return user_agent->products;
}
void cain_sip_header_user_agent_set_products(cain_sip_header_user_agent_t* user_agent,cain_sip_list_t* products) {
	cain_sip_list_t* list;
	if (user_agent->products) {
		for (list=user_agent->products;list !=NULL; list=list->next) {
			cain_sip_free((void*)list->data);

		}
		cain_sip_list_free(user_agent->products);
	}
	user_agent->products=products;
}
void cain_sip_header_user_agent_add_product(cain_sip_header_user_agent_t* user_agent,const char* product) {
	user_agent->products = cain_sip_list_append(user_agent->products ,cain_sip_strdup(product));
}

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
	if (via->protocol) cain_sip_free(via->protocol);
	if (via->transport) cain_sip_free(via->transport);
	if (via->host) cain_sip_free(via->host);
}

static void cain_sip_header_via_clone(cain_sip_header_via_t* via, const cain_sip_header_via_t*orig){
	CLONE_STRING(cain_sip_header_via,protocol,via,orig)
	CLONE_STRING(cain_sip_header_via,transport,via,orig)
	CLONE_STRING(cain_sip_header_via,host,via,orig)
	via->port=orig->port;
}

int cain_sip_header_via_marshal(cain_sip_header_via_t* via, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(via), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s/%s",via->protocol,via->transport);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset," %s",via->host);
	if (via->port > 0) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,":%i",via->port);
	}
	current_offset+=cain_sip_parameters_marshal(&via->params_list, buff,current_offset, buff_size);
	return current_offset-offset;
}

cain_sip_header_via_t* cain_sip_header_via_create(const char *host, int port, const char *transport, const char *branch){
	cain_sip_header_via_t *via=cain_sip_header_via_new();
	via->host=cain_sip_strdup(host);
	via->port=port;
	via->transport=cain_sip_strdup(transport);
	via->protocol=cain_sip_strdup("SIP/2.0");
	cain_sip_header_via_set_branch(via,branch);
	return via;
}

CAIN_SIP_NEW_HEADER(header_via,parameters,CAIN_SIP_VIA)
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
	CLONE_STRING(cain_sip_header_call_id,call_id,call_id,orig);
}
int cain_sip_header_call_id_marshal(cain_sip_header_call_id_t* call_id, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(call_id), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s",call_id->call_id);
	return current_offset-offset;
}

CAIN_SIP_NEW_HEADER(header_call_id,header,CAIN_SIP_CALL_ID)
CAIN_SIP_PARSE(header_call_id)
GET_SET_STRING(cain_sip_header_call_id,call_id);
/**************************
* cseq header object inherent from object
****************************
*/
struct _cain_sip_header_cseq  {
	cain_sip_header_t header;
	char* method;
	unsigned int seq_number;
};

static void cain_sip_header_cseq_destroy(cain_sip_header_cseq_t* cseq) {
	if (cseq->method) cain_sip_free(cseq->method);
}

static void cain_sip_header_cseq_clone(cain_sip_header_cseq_t* cseq, const cain_sip_header_cseq_t *orig) {
	CLONE_STRING(cain_sip_header_cseq,method,cseq,orig)
	cseq->seq_number=orig->seq_number;
}
int cain_sip_header_cseq_marshal(cain_sip_header_cseq_t* cseq, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(cseq), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%i %s",cseq->seq_number,cseq->method);
	return current_offset-offset;
}
cain_sip_header_cseq_t * cain_sip_header_cseq_create(unsigned int number, const char *method){
	cain_sip_header_cseq_t *cseq=cain_sip_header_cseq_new();
	cain_sip_header_cseq_set_method(cseq,method);
	cseq->seq_number=number;
	return cseq;
}
CAIN_SIP_NEW_HEADER(header_cseq,header,CAIN_SIP_CSEQ)
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
	CLONE_STRING(cain_sip_header_content_type,type,content_type,orig);
	CLONE_STRING(cain_sip_header_content_type,subtype,content_type,orig);
}
int cain_sip_header_content_type_marshal(cain_sip_header_content_type_t* content_type, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(content_type), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s/%s",content_type->type, content_type->subtype);
	current_offset+=cain_sip_parameters_marshal(&content_type->params_list, buff,current_offset, buff_size);
	return current_offset-offset;
}
CAIN_SIP_NEW_HEADER(header_content_type,parameters,"Content-Type")
CAIN_SIP_PARSE(header_content_type)
cain_sip_header_content_type_t* cain_sip_header_content_type_create (const char* type,const char* sub_type) {
	cain_sip_header_content_type_t* header = cain_sip_header_content_type_new();
	cain_sip_header_content_type_set_type(header,type);
	cain_sip_header_content_type_set_subtype(header,sub_type);
	return header;
}
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
int cain_sip_header_route_marshal(cain_sip_header_route_t* route, char* buff,unsigned int offset,unsigned int buff_size) {
	CAIN_SIP_FROM_LIKE_MARSHAL(route)
}
CAIN_SIP_NEW_HEADER(header_route,header_address,CAIN_SIP_ROUTE)
CAIN_SIP_PARSE(header_route)
cain_sip_header_route_t* cain_sip_header_route_create(const cain_sip_header_address_t* route) {
	cain_sip_header_route_t* header= cain_sip_header_route_new();
	_cain_sip_object_copy((cain_sip_object_t*)header,(cain_sip_object_t*)route);
	cain_sip_header_set_name(CAIN_SIP_HEADER(header),CAIN_SIP_ROUTE); /*restaure header name*/
	return header;
}
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
int cain_sip_header_record_route_marshal(cain_sip_header_record_route_t* record_route, char* buff,unsigned int offset,unsigned int buff_size) {
	CAIN_SIP_FROM_LIKE_MARSHAL(record_route)
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
	content_length->content_length=orig->content_length;
}

int cain_sip_header_content_length_marshal(cain_sip_header_content_length_t* content_length, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(content_length), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%i",content_length->content_length);
	return current_offset-offset;
}
CAIN_SIP_NEW_HEADER(header_content_length,header,CAIN_SIP_CONTENT_LENGTH)
CAIN_SIP_PARSE(header_content_length)
GET_SET_INT(cain_sip_header_content_length,content_length,unsigned int)
cain_sip_header_content_length_t* cain_sip_header_content_length_create (int content_length)  {
	cain_sip_header_content_length_t* obj;
	obj = cain_sip_header_content_length_new();
	cain_sip_header_content_length_set_content_length(obj,content_length);
	return obj;
}
/**************************
* Expires header object inherent from header
****************************
*/
struct _cain_sip_header_expires  {
	cain_sip_header_t header;
	int expires;
};

static void cain_sip_header_expires_destroy(cain_sip_header_expires_t* expires) {
}

static void cain_sip_header_expires_clone(cain_sip_header_expires_t* expires,
                                                 const cain_sip_header_expires_t *orig ) {
	expires->expires=orig->expires;
}

int cain_sip_header_expires_marshal(cain_sip_header_expires_t* expires, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(expires), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%i",expires->expires);
	return current_offset-offset;
}
CAIN_SIP_NEW_HEADER(header_expires,header,CAIN_SIP_EXPIRES)
CAIN_SIP_PARSE(header_expires)
GET_SET_INT(cain_sip_header_expires,expires,int)
cain_sip_header_expires_t* cain_sip_header_expires_create(int expires) {
	cain_sip_header_expires_t* obj = cain_sip_header_expires_new();
	cain_sip_header_expires_set_expires(obj,expires);
	return obj;
}
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
	CLONE_STRING(cain_sip_header_extension,value,extension,orig)
}
int cain_sip_header_extension_marshal(cain_sip_header_extension_t* extension, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(extension), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s",extension->value);
	return current_offset-offset;

}
CAIN_SIP_NEW_HEADER(header_extension,header,NULL)

cain_sip_header_extension_t* cain_sip_header_extension_create (const char* name,const char* value) {
	cain_sip_header_extension_t* ext = cain_sip_header_extension_new();
	cain_sip_header_set_name(CAIN_SIP_HEADER(ext),name);
	cain_sip_header_extension_set_value(ext,value);
	return ext;

}
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
	const char* opaque;




#define AUTH_BASE_DESTROY(obj) \
	if (obj->scheme) cain_sip_free((void*)obj->scheme);\
	if (obj->realm) cain_sip_free((void*)obj->realm);\
	if (obj->nonce) cain_sip_free((void*)obj->nonce);\
	if (obj->algorithm) cain_sip_free((void*)obj->algorithm);\
	if (obj->opaque) cain_sip_free((void*)obj->opaque);\

	/*if (obj->params_list) FIXME free list*/

#define AUTH_BASE_CLONE(object_type,dest,src) \
		CLONE_STRING(object_type,scheme,dest,src)\
		CLONE_STRING(object_type,realm,dest,src)\
		CLONE_STRING(object_type,nonce,dest,src)\
		CLONE_STRING(object_type,algorithm,dest,src)\
		CLONE_STRING(object_type,opaque,dest,src) \


#define AUTH_BASE_MARSHAL(header) \
	unsigned int current_offset=offset;\
	char* border=" ";\
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(header), buff,current_offset, buff_size);\
	const cain_sip_list_t* list=cain_sip_parameters_get_parameters(&header->params_list);\
	if (header->scheme) { \
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset," %s",header->scheme);\
		} else { \
			cain_sip_error("missing mandatory scheme"); \
		} \
	for(;list!=NULL;list=list->next){\
		cain_sip_param_pair_t* container = list->data;\
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s%s=%s",border, container->name,container->value);\
		border=", ";\
	}\
	if (header->realm) {\
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%srealm=\"%s\"",border,header->realm);\
		border=", ";\
		}\
	if (header->nonce) {\
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%snonce=\"%s\"",border,header->nonce);\
		border=", ";\
		}\
	if (header->algorithm) {\
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%salgorithm=%s",border,header->algorithm);\
		border=", ";\
		}\
	if (header->opaque) {\
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%sopaque=\"%s\"",border,header->opaque);\
		border=", ";\
		}

	struct _cain_sip_header_authorization  {
	AUTH_BASE
	const char* username;
	cain_sip_uri_t* uri;
	const char* response;
	const char* cnonce;
	int nonce_count;
	const char* qop;

};


static void cain_sip_header_authorization_destroy(cain_sip_header_authorization_t* authorization) {
	if (authorization->username) cain_sip_free((void*)authorization->username);
	if (authorization->uri) {
			cain_sip_object_unref(authorization->uri);
	}
	if (authorization->cnonce) cain_sip_free((void*)authorization->cnonce);
	AUTH_BASE_DESTROY(authorization)
	DESTROY_STRING(authorization,response);
	DESTROY_STRING(authorization,qop);
}

static void cain_sip_header_authorization_clone(cain_sip_header_authorization_t* authorization,
                                                 const cain_sip_header_authorization_t *orig ) {
	AUTH_BASE_CLONE(cain_sip_header_authorization,authorization,orig)
	CLONE_STRING(cain_sip_header_authorization,username,authorization,orig)
	if (cain_sip_header_authorization_get_uri(orig)) {
		cain_sip_header_authorization_set_uri(authorization,CAIN_SIP_URI(cain_sip_object_clone(CAIN_SIP_OBJECT(cain_sip_header_authorization_get_uri(orig)))));
	}
	CLONE_STRING(cain_sip_header_authorization,response,authorization,orig)
	CLONE_STRING(cain_sip_header_authorization,cnonce,authorization,orig)
	authorization->nonce_count=orig->nonce_count;
	CLONE_STRING(cain_sip_header_authorization,qop,authorization,orig)
}
static void cain_sip_header_authorization_init(cain_sip_header_authorization_t* authorization) {
}

cain_sip_uri_t* cain_sip_header_authorization_get_uri(const cain_sip_header_authorization_t* authorization) {
	return authorization->uri;
}

void cain_sip_header_authorization_set_uri(cain_sip_header_authorization_t* authorization, cain_sip_uri_t* uri) {
	if (uri) cain_sip_object_ref(uri);
	if (authorization->uri) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(authorization->uri));
	}
	authorization->uri=uri;
}
int cain_sip_header_authorization_marshal(cain_sip_header_authorization_t* authorization, char* buff,unsigned int offset,unsigned int buff_size) {
	char nonce_count[10];
	AUTH_BASE_MARSHAL(authorization)
	if (authorization->username) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%susername=\"%s\"",border,authorization->username);\
		border=", ";
		}
	if (authorization->uri) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s uri=\"",border);
		border=", ";
		current_offset+=cain_sip_uri_marshal(authorization->uri,buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s","\"");
		}
	if (authorization->algorithm) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%salgorithm=%s",border,authorization->algorithm);
		border=", ";
		}
	if (authorization->response) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%sresponse=\"%s\"",border,authorization->response);
		border=", ";
		}
	if (authorization->cnonce) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%scnonce=\"%s\"",border,authorization->cnonce);
		border=", ";
		}
	if (authorization->nonce_count>0) {
		cain_sip_header_authorization_get_nonce_count_as_string(authorization,nonce_count);
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%snc=%s",border,nonce_count);
		border=", ";
	}
	if (authorization->qop) {
	   current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%sqop=%s",border,authorization->qop);
	}
	return current_offset-offset;
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

int cain_sip_header_authorization_get_nonce_count_as_string(const cain_sip_header_authorization_t* authorization,char nounce_count[9]) {
	nounce_count[0]='\0';
	if (authorization->nonce_count>0) {
		snprintf(nounce_count,9,"%08x",authorization->nonce_count);
		return 0;
	} else {
		return -1;
	}
}
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
int cain_sip_header_proxy_authorization_marshal(cain_sip_header_proxy_authorization_t* proxy_authorization, char* buff,unsigned int offset,unsigned int buff_size) {
	return cain_sip_header_authorization_marshal(&proxy_authorization->authorization,buff,offset,buff_size);
}
CAIN_SIP_NEW_HEADER(header_proxy_authorization,header_authorization,"Proxy-Authorization")
CAIN_SIP_PARSE(header_proxy_authorization)
/**************************
*WWW-Authenticate header object inherent from parameters
****************************
*/
struct _cain_sip_header_www_authenticate  {
	AUTH_BASE
	const char* domain;
	unsigned int stale;
	cain_sip_list_t* qop;
};


static void cain_sip_header_www_authenticate_destroy(cain_sip_header_www_authenticate_t* www_authenticate) {
	AUTH_BASE_DESTROY(www_authenticate)
	if (www_authenticate->domain) cain_sip_free((void*)www_authenticate->domain);
	if (www_authenticate->qop) cain_sip_list_free_with_data(www_authenticate->qop,cain_sip_free);
}
void cain_sip_header_www_authenticate_init(cain_sip_header_www_authenticate_t* www_authenticate) {
	www_authenticate->stale=-1;
}
static void cain_sip_header_www_authenticate_clone(cain_sip_header_www_authenticate_t* www_authenticate,
                                                 const cain_sip_header_www_authenticate_t *orig ) {
	AUTH_BASE_CLONE(cain_sip_header_www_authenticate,www_authenticate,orig)
	CLONE_STRING(cain_sip_header_www_authenticate,domain,www_authenticate,orig)
	www_authenticate->stale=orig->stale;
	www_authenticate->qop=cain_sip_list_copy_with_data(orig->qop,(void* (*)(void*))cain_sip_strdup);
}
int cain_sip_header_www_authenticate_marshal(cain_sip_header_www_authenticate_t* www_authenticate, char* buff,unsigned int offset,unsigned int buff_size) {
	cain_sip_list_t* qops=www_authenticate->qop;
	AUTH_BASE_MARSHAL(www_authenticate)
	if (www_authenticate->domain) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%sdomain=\"%s\"",border,www_authenticate->domain);\
		border=", ";
		}
	if (www_authenticate->stale>=0) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%sstale=%s",border,www_authenticate->stale?"true":"false");
		}
	if (qops!=NULL && qops->data!=NULL) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%sqop=\"",border);
		border="";
		for(;qops!=NULL;qops=qops->next){
			current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s%s",border, (const char*)qops->data);
			border=",";
		}\
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"\"");
		border=", ";
	}
	return current_offset-offset;
}
#define SET_ADD_STRING_LIST(header,name) \
void header##_set_##name(header##_t* obj, cain_sip_list_t*  value) {\
	if (obj->name) {\
		cain_sip_list_free_with_data(obj->name,cain_sip_free);\
	} \
	obj->name=value;\
}\
void header##_add_name(header##_t* obj, const char*  value) {\
	obj->name=cain_sip_list_append(obj->name,strdup(value));\
}

CAIN_SIP_NEW_HEADER_INIT(header_www_authenticate,parameters,CAIN_SIP_WWW_AUTHENTICATE,header_www_authenticate)
CAIN_SIP_PARSE(header_www_authenticate)
GET_SET_STRING(cain_sip_header_www_authenticate,scheme);
GET_SET_STRING(cain_sip_header_www_authenticate,realm);
GET_SET_STRING(cain_sip_header_www_authenticate,nonce);
GET_SET_STRING(cain_sip_header_www_authenticate,algorithm);
GET_SET_STRING(cain_sip_header_www_authenticate,opaque);
/*GET_SET_STRING(cain_sip_header_www_authenticate,qop);*/
SET_ADD_STRING_LIST(cain_sip_header_www_authenticate,qop)
GET_SET_STRING(cain_sip_header_www_authenticate,domain)
GET_SET_BOOL(cain_sip_header_www_authenticate,stale,is)
cain_sip_list_t* cain_sip_header_www_authenticate_get_qop(const cain_sip_header_www_authenticate_t* www_authetication) {
	return www_authetication->qop;
}

/**************************
*Proxy-authenticate header object inherent from www_authenticate
****************************
*/
struct _cain_sip_header_proxy_authenticate  {
	cain_sip_header_www_authenticate_t www_authenticate;
};


static void cain_sip_header_proxy_authenticate_destroy(cain_sip_header_proxy_authenticate_t* proxy_authenticate) {
}

static void cain_sip_header_proxy_authenticate_clone(cain_sip_header_proxy_authenticate_t* proxy_authenticate,
                                                 const cain_sip_header_proxy_authenticate_t *orig ) {
}
int cain_sip_header_proxy_authenticate_marshal(cain_sip_header_proxy_authenticate_t* proxy_authenticate, char* buff,unsigned int offset,unsigned int buff_size) {
	return cain_sip_header_www_authenticate_marshal(&proxy_authenticate->www_authenticate,buff,offset,buff_size);
}
CAIN_SIP_NEW_HEADER(header_proxy_authenticate,header_www_authenticate,CAIN_SIP_PROXY_AUTHENTICATE)
CAIN_SIP_PARSE(header_proxy_authenticate)

/**************************
* max forwards header object inherent from header
****************************
*/
struct _cain_sip_header_max_forwards  {
	cain_sip_header_t header;
	int max_forwards;
};

static void cain_sip_header_max_forwards_destroy(cain_sip_header_max_forwards_t* max_forwards) {
}

static void cain_sip_header_max_forwards_clone(cain_sip_header_max_forwards_t* max_forwards,
                                                 const cain_sip_header_max_forwards_t *orig ) {
	max_forwards->max_forwards=orig->max_forwards;
}

int cain_sip_header_max_forwards_marshal(cain_sip_header_max_forwards_t* max_forwards, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_header_marshal(CAIN_SIP_HEADER(max_forwards), buff,current_offset, buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%i",max_forwards->max_forwards);
	return current_offset-offset;
}
CAIN_SIP_NEW_HEADER(header_max_forwards,header,"Max-Forwards")
CAIN_SIP_PARSE(header_max_forwards)
GET_SET_INT(cain_sip_header_max_forwards,max_forwards,int)
int cain_sip_header_max_forwards_decrement_max_forwards(cain_sip_header_max_forwards_t* max_forwards) {
	return max_forwards->max_forwards--;
}
