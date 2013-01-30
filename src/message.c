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
#include "cain_sip_messageParser.h"
#include "cain_sip_messageLexer.h"
#include "cain_sip_internal.h"


typedef struct _headers_container {
	char* name;
	cain_sip_list_t* header_list;
} headers_container_t;

static headers_container_t* cain_sip_message_headers_container_new(const char* name) {
	headers_container_t* headers_container = cain_sip_new0(headers_container_t);
	headers_container->name= cain_sip_strdup(name);
	return  headers_container;
}

static void cain_sip_headers_container_delete(headers_container_t *obj){
	cain_sip_free(obj->name);
	cain_sip_list_free_with_data(obj->header_list,(void (*)(void*))cain_sip_object_unref);
	cain_sip_free(obj);
}

struct _cain_sip_message {
	cain_sip_object_t base;
	cain_sip_list_t* header_list;
	char* body;
	unsigned int body_length;
};

static void cain_sip_message_destroy(cain_sip_message_t *msg){
	cain_sip_list_free_with_data(msg->header_list,(void (*)(void*))cain_sip_headers_container_delete);
	if (msg->body)
		cain_sip_free(msg->body);
}

/*very sub-optimal clone method */
static void cain_sip_message_clone(cain_sip_message_t *obj, const cain_sip_message_t *orig){
	headers_container_t *c;
	const cain_sip_list_t *l;
	for(l=orig->header_list;l!=NULL;l=l->next){
		c=(headers_container_t*)l->data;
		if (c->header_list){
			cain_sip_list_t * ll=cain_sip_list_copy_with_data(c->header_list,(void *(*)(void*))cain_sip_object_clone);
			cain_sip_message_add_headers(obj,ll);
			cain_sip_list_free(ll);
		}
	}
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_message_t);

CAIN_SIP_INSTANCIATE_VPTR(cain_sip_message_t,cain_sip_object_t,cain_sip_message_destroy,cain_sip_message_clone,NULL,TRUE);

cain_sip_message_t* cain_sip_message_parse (const char* value) {
	size_t message_length;
	return cain_sip_message_parse_raw(value,strlen(value),&message_length);
}

cain_sip_message_t* cain_sip_message_parse_raw (const char* buff, size_t buff_length,size_t* message_length ) { \
	pANTLR3_INPUT_STREAM           input;
	pcain_sip_messageLexer               lex;
	pANTLR3_COMMON_TOKEN_STREAM    tokens;
	pcain_sip_messageParser              parser;
	input  = antlr3NewAsciiStringCopyStream	(
			(pANTLR3_UINT8)buff,
			(ANTLR3_UINT32)buff_length,
			((void *)0));
	lex    = cain_sip_messageLexerNew                (input);
	tokens = antlr3CommonTokenStreamSourceNew  (1025, lex->pLexer->rec->state->tokSource);
	parser = cain_sip_messageParserNew               (tokens);
	cain_sip_message_t* l_parsed_object = parser->message_raw(parser,message_length);
/*	if (*message_length < buff_length) {*/
		/*there is a body*/
/*		l_parsed_object->body_length=buff_length-*message_length;
		l_parsed_object->body = cain_sip_malloc(l_parsed_object->body_length+1);
		memcpy(l_parsed_object->body,buff+*message_length,l_parsed_object->body_length);
		l_parsed_object->body[l_parsed_object->body_length]='\0';
	}*/
	parser ->free(parser);
	tokens ->free(tokens);
	lex    ->free(lex);
	input  ->close(input);
	return l_parsed_object;
}

static int cain_sip_headers_container_comp_func(const headers_container_t *a, const char*b) {
	return strcasecmp(a->name,b);
}

static void cain_sip_message_init(cain_sip_message_t *message){
	
}

headers_container_t* cain_sip_headers_container_get(const cain_sip_message_t* message,const char* header_name) {
	cain_sip_list_t *  result = cain_sip_list_find_custom(	message->header_list
															, (cain_sip_compare_func)cain_sip_headers_container_comp_func
															, header_name);
	return result?(headers_container_t*)(result->data):NULL;
}

headers_container_t * get_or_create_container(cain_sip_message_t *message, const char *header_name){
	// first check if already exist
	headers_container_t* headers_container = cain_sip_headers_container_get(message,header_name);
	if (headers_container == NULL) {
		headers_container = cain_sip_message_headers_container_new(header_name);
		message->header_list=cain_sip_list_append(message->header_list,headers_container);
	}
	return headers_container;
}

void cain_sip_message_add_header(cain_sip_message_t *message,cain_sip_header_t* header) {
	headers_container_t *headers_container=get_or_create_container(message,cain_sip_header_get_name(header));
	headers_container->header_list=cain_sip_list_append(headers_container->header_list,cain_sip_object_ref(header));
}

void cain_sip_message_add_headers(cain_sip_message_t *message, const cain_sip_list_t *header_list){
	const char *hname=cain_sip_header_get_name(CAIN_SIP_HEADER((header_list->data)));
	headers_container_t *headers_container=get_or_create_container(message,hname);
	for(;header_list!=NULL;header_list=header_list->next){
		cain_sip_header_t *h=CAIN_SIP_HEADER(header_list->data);
		if (strcmp(cain_sip_header_get_name(h),hname)!=0){
			cain_sip_fatal("Bad use of cain_sip_message_add_headers(): all headers of the list must be of the same type.");
			return ;
		}
		headers_container->header_list=cain_sip_list_append(headers_container->header_list,cain_sip_object_ref(h));
	}
}

void cain_sip_message_set_header(cain_sip_message_t *msg, cain_sip_header_t* header){
	headers_container_t *headers_container=get_or_create_container(msg,cain_sip_header_get_name(header));
	cain_sip_object_ref(header);
	headers_container->header_list=cain_sip_list_free_with_data(headers_container->header_list,cain_sip_object_unref);
	headers_container->header_list=cain_sip_list_append(headers_container->header_list,header);
}

const cain_sip_list_t* cain_sip_message_get_headers(const cain_sip_message_t *message,const char* header_name) {
	headers_container_t* headers_container = cain_sip_headers_container_get(message,header_name);
	return headers_container ? headers_container->header_list:NULL;
}

cain_sip_object_t *_cain_sip_message_get_header_by_type_id(const cain_sip_message_t *message, cain_sip_type_id_t id){
	const cain_sip_list_t *e1;
	for(e1=message->header_list;e1!=NULL;e1=e1->next){
		headers_container_t* headers_container=(headers_container_t*)e1->data;
		if (headers_container->header_list){
			cain_sip_object_t *ret=headers_container->header_list->data;
			if (ret->vptr->id==id) return ret;
		}
	}
	return NULL;
}

void cain_sip_message_remove_first(cain_sip_message_t *msg, const char *header_name){
	headers_container_t* headers_container = cain_sip_headers_container_get(msg,header_name);
	if (headers_container && headers_container->header_list){
		cain_sip_list_t *to_be_removed=headers_container->header_list;
		headers_container->header_list=cain_sip_list_remove_link(headers_container->header_list,to_be_removed);
		cain_sip_list_free_with_data(to_be_removed,cain_sip_object_unref);
	}
}

void cain_sip_message_remove_last(cain_sip_message_t *msg, const char *header_name){
	headers_container_t* headers_container = cain_sip_headers_container_get(msg,header_name);
	if (headers_container && headers_container->header_list){
		cain_sip_list_t *to_be_removed=cain_sip_list_last_elem(headers_container->header_list);
		headers_container->header_list=cain_sip_list_remove_link(headers_container->header_list,to_be_removed);
		cain_sip_list_free_with_data(to_be_removed,cain_sip_object_unref);
	}
}

void cain_sip_message_remove_header(cain_sip_message_t *msg, const char *header_name){
	headers_container_t* headers_container = cain_sip_headers_container_get(msg,header_name);
	if (headers_container){
		cain_sip_headers_container_delete(headers_container);
		cain_sip_list_remove(msg->header_list,headers_container);
	}
}


/*
int cain_sip_message_named_headers_marshal(cain_sip_message_t *message, const char* header_name, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	cain_sip_list_t* header_list = cain_sip_message_get_headers(message,header_name);
	if (!header_list) {
		cain_sip_error("headers [%s] not found",header_name);
		return 0;
	}
	for(;header_list!=NULL;header_list=header_list->next){
		cain_sip_header_t *h=CAIN_SIP_HEADER(header_list->data);
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(h),buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s","\r\n");
	}
	return current_offset-offset;
}

#define MARSHAL_AND_CHECK_HEADER(header) \
		if (current_offset == (current_offset+=(header))) {\
			cain_sip_error("missing mandatory header");\
			return current_offset;\
		} else {\
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s","\r\n");\
		}
*/
typedef void (*each_header_cb)(const cain_sip_header_t* header,void* userdata);

static void cain_sip_message_for_each_header(const cain_sip_message_t *message,each_header_cb cb,void* user_data) {
	cain_sip_list_t* headers_list;
	cain_sip_list_t* header_list;
	for(headers_list=message->header_list;headers_list!=NULL;headers_list=headers_list->next){
		for(header_list=((headers_container_t*)(headers_list->data))->header_list
				;header_list!=NULL
				;header_list=header_list->next)	{
			cb(CAIN_SIP_HEADER(header_list->data),user_data);
		}
	}
	return;
}
static void append_header(const cain_sip_header_t* header,void* user_data) {
	*(cain_sip_list_t**)user_data=cain_sip_list_append((*(cain_sip_list_t**)user_data),(void*)header);
}

cain_sip_list_t* cain_sip_message_get_all_headers(const cain_sip_message_t *message) {
	cain_sip_list_t* headers=NULL;
	cain_sip_message_for_each_header(message,append_header,&headers);
	return headers;
}

int cain_sip_headers_marshal(cain_sip_message_t *message, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	/*FIXME, replace this code by cain_sip_message_for_each_header*/
	cain_sip_list_t* headers_list;
	cain_sip_list_t* header_list;
	for(headers_list=message->header_list;headers_list!=NULL;headers_list=headers_list->next){
		for(header_list=((headers_container_t*)(headers_list->data))->header_list
				;header_list!=NULL
				;header_list=header_list->next)	{
			cain_sip_header_t *h=CAIN_SIP_HEADER(header_list->data);
			current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(h),buff,current_offset,buff_size);
			current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s","\r\n");
		}
	}
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s","\r\n");
	return current_offset-offset;
}

struct _cain_sip_request {
	cain_sip_message_t message;
	char* method;
	cain_sip_uri_t* uri;
};

static void cain_sip_request_destroy(cain_sip_request_t* request) {
	if (request->method) cain_sip_free(request->method);
	if (request->uri) cain_sip_object_unref(request->uri);
}

static void cain_sip_request_init(cain_sip_request_t *message){	
}

static void cain_sip_request_clone(cain_sip_request_t *request, const cain_sip_request_t *orig){
	if (orig->method) request->method=cain_sip_strdup(orig->method);
	if (orig->uri) request->uri=(cain_sip_uri_t*)cain_sip_object_ref(cain_sip_object_clone((cain_sip_object_t*)orig->uri));
}

int cain_sip_request_marshal(cain_sip_request_t* request, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset,"%s ",cain_sip_request_get_method(request));
	current_offset+=cain_sip_uri_marshal(cain_sip_request_get_uri(request),buff,current_offset,buff_size);
	current_offset+=snprintf(buff+current_offset,buff_size-current_offset," %s","SIP/2.0\r\n");
	current_offset+=cain_sip_headers_marshal(CAIN_SIP_MESSAGE(request),buff,current_offset,buff_size);
	if (CAIN_SIP_MESSAGE(request)->body) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset, "%s",CAIN_SIP_MESSAGE(request)->body);
	}
	return current_offset-offset;
}

CAIN_SIP_NEW(request,message)
CAIN_SIP_PARSE(request)
GET_SET_STRING(cain_sip_request,method);

void cain_sip_request_set_uri(cain_sip_request_t* request,cain_sip_uri_t* uri) {
	cain_sip_object_ref(uri);
	if (request->uri) {
		cain_sip_object_unref(request->uri);
	}
	request->uri=uri;
}

cain_sip_uri_t * cain_sip_request_get_uri(cain_sip_request_t *request){
	return request->uri;
}

cain_sip_uri_t* cain_sip_request_extract_origin(const cain_sip_request_t* req) {
	cain_sip_header_via_t* via_header = cain_sip_message_get_header_by_type(req,cain_sip_header_via_t);
	cain_sip_uri_t* uri=NULL;
	const char* received = cain_sip_header_via_get_received(via_header);
	int rport = cain_sip_header_via_get_rport(via_header);
	uri = cain_sip_uri_new();
	if (received!=NULL) {
		cain_sip_uri_set_host(uri,received);
	} else {
		cain_sip_uri_set_host(uri,cain_sip_header_via_get_host(via_header));
	}
	if (rport>0) {
		cain_sip_uri_set_port(uri,rport);
	} else if (cain_sip_header_via_get_port(via_header)) {
		cain_sip_uri_set_port(uri,cain_sip_header_via_get_port(via_header));
	}
	if (cain_sip_header_via_get_transport(via_header)) {
		cain_sip_uri_set_transport_param(uri,cain_sip_header_via_get_transport_lowercase(via_header));
	}
	return uri;
}

int cain_sip_message_is_request(cain_sip_message_t *msg){
	return CAIN_SIP_IS_INSTANCE_OF(CAIN_SIP_OBJECT(msg),cain_sip_request_t);
}

int cain_sip_message_is_response(const cain_sip_message_t *msg){
	return CAIN_SIP_IS_INSTANCE_OF(CAIN_SIP_OBJECT(msg),cain_sip_response_t);
}

cain_sip_header_t *cain_sip_message_get_header(const cain_sip_message_t *msg, const char *header_name){
	const cain_sip_list_t *l=cain_sip_message_get_headers(msg,header_name);
	if (l!=NULL)
		return (cain_sip_header_t*)l->data;
	return NULL;
}

char *cain_sip_message_to_string(cain_sip_message_t *msg){
	return cain_sip_object_to_string(CAIN_SIP_OBJECT(msg));
}

const char* cain_sip_message_get_body(cain_sip_message_t *msg) {
	return msg->body;
}

void cain_sip_message_set_body(cain_sip_message_t *msg,const char* body,unsigned int size) {
	if (msg->body) {
		cain_sip_free((void*)body);
	}
	msg->body = cain_sip_malloc(size+1);
	memcpy(msg->body,body,size);
	msg->body[size]='\0';
}
void cain_sip_message_assign_body(cain_sip_message_t *msg, char* body) {
	if (msg->body) {
		cain_sip_free((void*)body);
	}
	msg->body = body;
}
struct _cain_sip_response{
	cain_sip_message_t base;
	char *sip_version;
	int status_code;
	char *reason_phrase;
};

typedef struct code_phrase{
	int code;
	const char *phrase;
} code_phrase_t;

static code_phrase_t well_known_codes[]={
	{	100		,		"Trying"	},
	{	101		,		"Dialog establishment"	},
	{	180		,		"Ringing"				},
	{	181		,		"Call is being forwarded"	},
	{	182		,		"Queued"	},
	{	183		,		"Session progress"	},
	{	200		,		"Ok"				},
	{	202		,		"Accepted"	},
	{	300		,		"Multiple choices"	},
	{	301		,		"Moved permanently"	},
	{	302		,		"Moved temporarily"	},
	{	305		,		"Use proxy"	},
	{	380		,		"Alternate contact"	},
	{	400		,		"Bad request"		},
	{	401		,		"Unauthorized"		},
	{	402		,		"Payment required"	},
	{	403		,		"Forbidden"	},
	{	404		,		"Not found"	},
	{	405		,		"Method not allowed"	},
	{	406		,		"Not acceptable"	},
	{	407		,		"Proxy authentication required"	},
	{	408		,		"Request timeout"	},
	{	410		,		"Gone"	},
	{	413		,		"Request entity too large"	},
	{	414		,		"Request-URI too long"	},
	{	415		,		"Unsupported media type"	},
	{	416		,		"Unsupported URI scheme"	},
	{	420		,		"Bad extension"	},
	{	421		,		"Extension required"	},
	{	423		,		"Interval too brief"	},
	{	480		,		"Temporarily unavailable"	},
	{	481		,		"Call/transaction does not exist"	},
	{	482		,		"Loop detected"	},
	{	483		,		"Too many hops"	},
	{	484		,		"Address incomplete"	},
	{	485		,		"Ambiguous"	},
	{	486		,		"Busy here"	},
	{	487		,		"Request terminated"	},
	{	488		,		"Not acceptable here"	},
	{	491		,		"Request pending"	},
	{	493		,		"Undecipherable"	},
	{	500		,		"Server internal error"	},
	{	501		,		"Not implemented"	},
	{	502		,		"Bad gateway"	},
	{	503		,		"Service unavailable"	},
	{	504		,		"Server time-out"	},
	{	505		,		"Version not supported"	},
	{	513		,		"Message too large"	},
	{	600		,		"Busy everywhere"	},
	{	603		,		"Decline"	},
	{	604		,		"Does not exist anywhere"	},
	{	606		,		"Not acceptable"	},
	{	0			,		NULL	}
};

const char *cain_sip_get_well_known_reason_phrase(int status_code){
	int i;
	for(i=0;well_known_codes[i].code!=0;++i){
		if (well_known_codes[i].code==status_code)
			return well_known_codes[i].phrase;
	}
	return "Unknown reason";
}

void cain_sip_response_destroy(cain_sip_response_t *resp){
	if (resp->sip_version) cain_sip_free(resp->sip_version);
	if (resp->reason_phrase) cain_sip_free(resp->reason_phrase);
}

static void cain_sip_response_init(cain_sip_response_t *resp){
}

static void cain_sip_response_clone(cain_sip_response_t *resp, const cain_sip_response_t *orig){
	if (orig->sip_version) resp->sip_version=cain_sip_strdup(orig->sip_version);
	if (orig->reason_phrase) resp->reason_phrase=cain_sip_strdup(orig->reason_phrase);
}
int cain_sip_response_marshal(cain_sip_response_t *resp, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"SIP/2.0 %i %s\r\n"
								,cain_sip_response_get_status_code(resp)
								,cain_sip_response_get_reason_phrase(resp));
	current_offset+=cain_sip_headers_marshal(CAIN_SIP_MESSAGE(resp),buff,current_offset,buff_size);
	if (CAIN_SIP_MESSAGE(resp)->body) {
		current_offset+=snprintf(buff+current_offset,buff_size-current_offset, "%s",CAIN_SIP_MESSAGE(resp)->body);
	}
	return current_offset-offset;
}
CAIN_SIP_NEW(response,message);
CAIN_SIP_PARSE(response)
GET_SET_STRING(cain_sip_response,reason_phrase);
GET_SET_INT(cain_sip_response,status_code,int)

cain_sip_request_t* cain_sip_request_create(cain_sip_uri_t *requri, const char* method,
                                         cain_sip_header_call_id_t *callid,
                                         cain_sip_header_cseq_t * cseq,
                                         cain_sip_header_from_t *from,
                                         cain_sip_header_to_t *to,
                                         cain_sip_header_via_t *via,
                                         int max_forward)
{
	cain_sip_request_t *ret=cain_sip_request_new();
	cain_sip_header_max_forwards_t *mf=cain_sip_header_max_forwards_new();
	if (max_forward==0) max_forward=70;
	cain_sip_header_max_forwards_set_max_forwards(mf,max_forward);

	cain_sip_request_set_uri(ret,requri);
	cain_sip_request_set_method(ret,method);
	cain_sip_message_add_header((cain_sip_message_t*)ret,CAIN_SIP_HEADER(via));
	cain_sip_message_add_header((cain_sip_message_t*)ret,CAIN_SIP_HEADER(from));
	cain_sip_message_add_header((cain_sip_message_t*)ret,CAIN_SIP_HEADER(to));
	cain_sip_message_add_header((cain_sip_message_t*)ret,CAIN_SIP_HEADER(cseq));
	cain_sip_message_add_header((cain_sip_message_t*)ret,CAIN_SIP_HEADER(callid));
	cain_sip_message_add_header((cain_sip_message_t*)ret,CAIN_SIP_HEADER(mf));
	return ret;
}

static void cain_sip_response_init_default(cain_sip_response_t *resp, int status_code, const char *phrase){
	resp->status_code=status_code;
	resp->sip_version=cain_sip_strdup("SIP/2.0");
	if (phrase==NULL) phrase=cain_sip_get_well_known_reason_phrase(status_code);
	resp->reason_phrase=cain_sip_strdup(phrase);
}

cain_sip_response_t *cain_sip_response_create_from_request(cain_sip_request_t *req, int status_code){
	cain_sip_response_t *resp=cain_sip_response_new();
	cain_sip_header_t *h;
	cain_sip_header_to_t *to;
	cain_sip_response_init_default(resp,status_code,NULL);
	if (status_code==100 && (h=cain_sip_message_get_header((cain_sip_message_t*)req,"timestamp"))){
		cain_sip_message_add_header((cain_sip_message_t*)resp,h);
	}
	cain_sip_message_add_headers((cain_sip_message_t*)resp,cain_sip_message_get_headers ((cain_sip_message_t*)req,"via"));
	cain_sip_message_add_header((cain_sip_message_t*)resp,cain_sip_message_get_header((cain_sip_message_t*)req,"from"));
	h=cain_sip_message_get_header((cain_sip_message_t*)req,"to");
	if (status_code!=100){
		//so that to tag can be added
		to=(cain_sip_header_to_t*)cain_sip_object_clone((cain_sip_object_t*)h);
	}else{
		to=(cain_sip_header_to_t*)h;
	}
	cain_sip_message_add_header((cain_sip_message_t*)resp,(cain_sip_header_t*)to);
	h=cain_sip_message_get_header((cain_sip_message_t*)req,"call-id");
	cain_sip_message_add_header((cain_sip_message_t*)resp,h);
	cain_sip_message_add_header((cain_sip_message_t*)resp,cain_sip_message_get_header((cain_sip_message_t*)req,"cseq"));
	return resp;
}
/*
12.1.1 UAS behavior

   When a UAS responds to a request with a response that establishes a
   dialog (such as a 2xx to INVITE), the UAS MUST copy all Record-Route
   header field values from the request into the response (including the
   URIs, URI parameters, and any Record-Route header field parameters,
   whether they are known or unknown to the UAS) and MUST maintain the
   order of those values.
   */
void cain_sip_response_fill_for_dialog(cain_sip_response_t *obj, cain_sip_request_t *req){
	const cain_sip_list_t *rr=cain_sip_message_get_headers((cain_sip_message_t*)req,CAIN_SIP_RECORD_ROUTE);
	cain_sip_header_contact_t *ct=cain_sip_message_get_header_by_type(obj,cain_sip_header_contact_t);
	cain_sip_message_remove_header((cain_sip_message_t*)obj,CAIN_SIP_RECORD_ROUTE);
	if (rr)
		cain_sip_message_add_headers((cain_sip_message_t*)obj,rr);
	if (cain_sip_response_get_status_code(obj)>=200 && cain_sip_response_get_status_code(obj)<300 && !ct){
		/*add a dummy contact to be filled by channel later*/
		cain_sip_message_add_header((cain_sip_message_t*)obj,(cain_sip_header_t*)cain_sip_header_contact_new());
	}	
}

void cain_sip_response_get_return_hop(cain_sip_response_t *msg, cain_sip_hop_t *hop){
	cain_sip_header_via_t *via=CAIN_SIP_HEADER_VIA(cain_sip_message_get_header(CAIN_SIP_MESSAGE(msg),"via"));
	const char *host;
	hop->transport=cain_sip_strdup(cain_sip_header_via_get_transport_lowercase(via));
	host=cain_sip_header_via_get_received(via);
	if (host==NULL)
		host=cain_sip_header_via_get_host(via);
	hop->host=cain_sip_strdup(host);
	hop->port=cain_sip_header_via_get_rport(via);
	if (hop->port==-1)
		hop->port=cain_sip_header_via_get_listening_port(via);
}
int cain_sip_response_fix_contact(const cain_sip_response_t* response,cain_sip_header_contact_t* contact) {
	cain_sip_header_via_t* via_header;
	cain_sip_uri_t* contact_uri;
	const char* received;
	int rport;
	int contact_port;
	/*first check received/rport*/
	via_header= (cain_sip_header_via_t*)cain_sip_message_get_header(CAIN_SIP_MESSAGE(response),CAIN_SIP_VIA);
	received = cain_sip_header_via_get_received(via_header);
	rport = cain_sip_header_via_get_rport(via_header);
	if (received!=NULL || rport>0) {
		contact_uri=cain_sip_header_address_get_uri(CAIN_SIP_HEADER_ADDRESS(contact));
		if (received && strcmp(received,cain_sip_uri_get_host(contact_uri))!=0) {
			/*need to update host*/
			cain_sip_uri_set_host(contact_uri,received);
		}
		contact_port =  cain_sip_uri_get_port(contact_uri);
		if (rport>0 && rport!=contact_port && (contact_port+rport)!=5060) {
			/*need to update port*/
			cain_sip_uri_set_port(contact_uri,rport);
		}
		/*try to fix transport if needed (very unlikely)*/
		if (strcasecmp(cain_sip_header_via_get_transport(via_header),"UDP")!=0) {
			if (!cain_sip_uri_get_transport_param(contact_uri)
					||strcasecmp(cain_sip_uri_get_transport_param(contact_uri),cain_sip_header_via_get_transport(via_header))!=0) {
				cain_sip_uri_set_transport_param(contact_uri,cain_sip_header_via_get_transport_lowercase(via_header));
			}
		} else {
			if (cain_sip_uri_get_transport_param(contact_uri)) {
				cain_sip_uri_set_transport_param(contact_uri,NULL);
			}
		}
	}
	return 0;
}
