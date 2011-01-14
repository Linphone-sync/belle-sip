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
 * header_address
 ***********************/
struct _cain_sip_header_address {
	cain_sip_parameters_t params;
	const char* displayname;
	cain_sip_uri_t* uri;
};
static void cain_sip_header_address_init(cain_sip_header_address_t* object){
	cain_sip_object_init_type(object,cain_sip_header_address_t);
	cain_sip_parameters_init((cain_sip_parameters_t*)object); /*super*/
}

static void cain_sip_header_address_destroy(cain_sip_header_address_t* contact) {
	if (contact->displayname) cain_sip_free((void*)(contact->displayname));
	if (contact->uri) cain_sip_object_unref(CAIN_SIP_OBJECT(contact->uri));
}

CAIN_SIP_NEW(header_address,object)
GET_SET_STRING(cain_sip_header_address,displayname);

void cain_sip_header_address_set_quoted_displayname(cain_sip_header_address_t* address,const char* value) {
		if (address->displayname != NULL) cain_sip_free((void*)(address->displayname));
		size_t value_size = strlen(value);
		address->displayname=cain_sip_malloc0(value_size-2+1);
		strncpy((char*)(address->displayname),value+1,value_size-2);
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
	cain_sip_header_address_destroy(CAIN_SIP_HEADER_ADDRESS(contact));
}

CAIN_SIP_NEW(header_contact,header_address)
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
float	cain_sip_header_contact_get_qvalue(cain_sip_header_contact_t* contact) {
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
	cain_sip_header_address_destroy(CAIN_SIP_HEADER_ADDRESS(from));
}

CAIN_SIP_NEW(header_from,header_address)
CAIN_SIP_PARSE(header_from)
GET_SET_STRING_PARAM(cain_sip_header_from,tag);

/**************************
* To header object inherent from header_address
****************************
*/
struct _cain_sip_header_to  {
	cain_sip_header_address_t address;
};

static void cain_sip_header_to_destroy(cain_sip_header_to_t* to) {
	cain_sip_header_address_destroy(CAIN_SIP_HEADER_ADDRESS(to));
}

CAIN_SIP_NEW(header_to,header_address)
CAIN_SIP_PARSE(header_to)
GET_SET_STRING_PARAM(cain_sip_header_to,tag);

/**************************
* Viq header object inherent from parameters
****************************
*/
struct _cain_sip_header_via  {
	cain_sip_parameters_t params_list;
	const char* protocol;
	const char* transport;
	const char* host;
	int port;
};

static void cain_sip_header_via_destroy(cain_sip_header_via_t* via) {
	cain_sip_parameters_destroy(CAIN_SIP_PARAMETERS(via));
	if (via->host) cain_sip_free((void*)via->host);
	if (via->protocol) cain_sip_free((void*)via->protocol);
}

CAIN_SIP_NEW(header_via,header_address)
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
/**************************
* callid header object inherent from object
****************************
*/
struct _cain_sip_header_callid  {
	cain_sip_object_t base;
	const char* callid;
};

static void cain_sip_header_callid_destroy(cain_sip_header_callid_t* callid) {
	cain_sip_object_destroy(CAIN_SIP_OBJECT(callid));
	if (callid->callid) cain_sip_free((void*)callid->callid);

}

CAIN_SIP_NEW(header_callid,object)
CAIN_SIP_PARSE(header_callid)
GET_SET_STRING(cain_sip_header_callid,callid);
/**************************
* cseq header object inherent from object
****************************
*/
struct _cain_sip_header_cseq  {
	cain_sip_object_t base;
	const char* method;
	unsigned int seq_number;
};

static void cain_sip_header_cseq_destroy(cain_sip_header_cseq_t* cseq) {
	cain_sip_object_destroy(CAIN_SIP_OBJECT(cseq));
	if (cseq->method) cain_sip_free((void*)cseq->method);

}

CAIN_SIP_NEW(header_cseq,object)
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
	cain_sip_parameters_destroy(CAIN_SIP_PARAMETERS(content_type));
	if (content_type->type) cain_sip_free((void*)content_type->type);
	if (content_type->subtype) cain_sip_free((void*)content_type->subtype);
}

CAIN_SIP_NEW(header_content_type,parameters)
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
	cain_sip_header_address_destroy(CAIN_SIP_HEADER_ADDRESS(route));
}

CAIN_SIP_NEW(header_route,header_address)
CAIN_SIP_PARSE(header_route)
/**************************
* Record route header object inherent from header_address
****************************
*/
struct _cain_sip_header_record_route  {
	cain_sip_header_address_t address;
};

static void cain_sip_header_record_route_destroy(cain_sip_header_record_route_t* record_route) {
	cain_sip_header_address_destroy(CAIN_SIP_HEADER_ADDRESS(record_route));
}

CAIN_SIP_NEW(header_record_route,header_address)
CAIN_SIP_PARSE(header_record_route)

