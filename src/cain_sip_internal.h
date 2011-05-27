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
#ifndef cain_utils_h
#define cain_utils_h

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>

/* include all public headers*/
#include "cain-sip/cain-sip.h"

typedef void (*cain_sip_object_destroy_t)(cain_sip_object_t*);
typedef void (*cain_sip_object_clone_t)(cain_sip_object_t* obj, const cain_sip_object_t *orig);
typedef int (*cain_sip_object_marshal_t)(cain_sip_object_t* obj, char* buff,unsigned int offset,size_t buff_size);

struct _cain_sip_object_vptr{
	cain_sip_type_id_t id;
	struct _cain_sip_object_vptr *parent;
	void *interfaces;	/*unused for the moment*/
	cain_sip_object_destroy_t destroy;
	cain_sip_object_clone_t clone;
	cain_sip_object_marshal_t marshal;

};

typedef struct _cain_sip_object_vptr cain_sip_object_vptr_t;

extern cain_sip_object_vptr_t cain_sip_object_t_vptr;

#define CAIN_SIP_OBJECT_VPTR_NAME(object_type)	object_type##_vptr

#define CAIN_SIP_DECLARE_VPTR(object_type) \
	extern cain_sip_object_vptr_t CAIN_SIP_OBJECT_VPTR_NAME(object_type);

#define CAIN_SIP_INSTANCIATE_CUSTOM_VPTR_BEGIN(vptr_type,object_type, parent_type, destroy, clone) \
	vptr_type object_type##_vptr={ {\
		CAIN_SIP_TYPE_ID(object_type), \
		(cain_sip_object_vptr_t*)&CAIN_SIP_OBJECT_VPTR_NAME(parent_type), \
		NULL, \
		(cain_sip_object_destroy_t)destroy,	\
		(cain_sip_object_clone_t)clone	},

#define CAIN_SIP_INSTANCIATE_CUSTOM_VPTR_END };

#define CAIN_SIP_INSTANCIATE_VPTR(object_type,parent_type,destroy,clone,marshal) \
		cain_sip_object_vptr_t object_type##_vptr={ \
		CAIN_SIP_TYPE_ID(object_type), \
		(cain_sip_object_vptr_t*)&CAIN_SIP_OBJECT_VPTR_NAME(parent_type), \
		NULL, \
		(cain_sip_object_destroy_t)destroy,	\
		(cain_sip_object_clone_t)clone,	\
		(cain_sip_object_marshal_t)marshal\
		}




struct _cain_sip_object{
	cain_sip_object_vptr_t *vptr;
	size_t size;
	int ref;
	char* name;
};

cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_object_vptr_t *vptr, int initially_unowed);
int cain_sip_object_marshal(cain_sip_object_t* obj, char* buff,unsigned int offset,size_t buff_size);

#define cain_sip_object_new(_type) (_type*)_cain_sip_object_new(sizeof(_type),(cain_sip_object_vptr_t*)&CAIN_SIP_OBJECT_VPTR_NAME(_type),0)
#define cain_sip_object_new_unowed(_type,destroy)(_type*)_cain_sip_object_new(sizeof(_type),(cain_sip_object_vptr_t*)&CAIN_SIP_OBJECT_VPTR_NAME(_type),1)

#define CAIN_SIP_OBJECT_VPTR(obj,vptr_type) ((vptr_type*)(((cain_sip_object_t*)obj)->vptr))
#define cain_sip_object_init(obj)		/*nothing*/


/*list of all vptrs (classes) used in cain-sip*/
CAIN_SIP_DECLARE_VPTR(cain_sip_object_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_uri_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_parameters_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_contact_t);

struct _cain_sip_list {
	struct _cain_sip_list *next;
	struct _cain_sip_list *prev;
	void *data;
};

typedef void (*cain_sip_source_remove_callback_t)(cain_sip_source_t *);

struct cain_sip_source{
	cain_sip_object_t base;
	cain_sip_list_t node;
	unsigned long id;
	int fd;
	unsigned int events;
	int timeout;
	void *data;
	uint64_t expire_ms;
	int index; /* index in pollfd table */
	cain_sip_source_func_t notify;
	cain_sip_source_remove_callback_t on_remove;
	int cancelled:1;
	int expired:1;
};

void cain_sip_fd_source_init(cain_sip_source_t *s, cain_sip_source_func_t func, void *data, int fd, unsigned int events, unsigned int timeout_value_ms);

#define cain_list_next(elem) ((elem)->next)

/* include private headers */
#include "sender_task.h"

#ifdef __cplusplus
extern "C"{
#endif



#define cain_sip_new(type) (type*)cain_sip_malloc(sizeof(type))
#define cain_sip_new0(type) (type*)cain_sip_malloc0(sizeof(type))
	
cain_sip_list_t *cain_sip_list_new(void *data);
cain_sip_list_t*  cain_sip_list_append_link(cain_sip_list_t* elem,cain_sip_list_t *new_elem);
cain_sip_list_t *cain_sip_list_find_custom(cain_sip_list_t *list, cain_sip_compare_func compare_func, const void *user_data);
cain_sip_list_t *cain_sip_list_remove_custom(cain_sip_list_t *list, cain_sip_compare_func compare_func, const void *user_data);
cain_sip_list_t * cain_sip_list_free(cain_sip_list_t *list);
#define cain_sip_list_next(elem) ((elem)->next)
/***************/
/* logging api */
/***************/

typedef enum {
        CAIN_SIP_DEBUG=1,
        CAIN_SIP_MESSAGE=1<<1,
        CAIN_SIP_WARNING=1<<2,
        CAIN_SIP_ERROR=1<<3,
        CAIN_SIP_FATAL=1<<4,
        CAIN_SIP_LOGLEV_END=1<<5
} cain_sip_log_level;


typedef void (*cain_sip_log_function_t)(cain_sip_log_level lev, const char *fmt, va_list args);

void cain_sip_set_log_file(FILE *file);
void cain_sip_set_log_handler(cain_sip_log_function_t func);

extern cain_sip_log_function_t cain_sip_logv_out;

extern unsigned int __cain_sip_log_mask;

#define cain_sip_log_level_enabled(level)   (__cain_sip_log_mask & (level))

#if !defined(WIN32) && !defined(_WIN32_WCE)
#define cain_sip_logv(level,fmt,args) \
{\
        if (cain_sip_logv_out!=NULL && cain_sip_log_level_enabled(level)) \
                cain_sip_logv_out(level,fmt,args);\
        if ((level)==CAIN_SIP_FATAL) abort();\
}while(0)
#else
void cain_sip_logv(int level, const char *fmt, va_list args);
#endif

void cain_sip_set_log_level_mask(int levelmask);

#ifdef CAIN_SIP_DEBUG_MODE
static inline void cain_sip_debug(const char *fmt,...)
{
  va_list args;
  va_start (args, fmt);
  cain_sip_logv(CAIN_SIP_DEBUG, fmt, args);
  va_end (args);
}
#else

#define cain_sip_debug(...)

#endif

#ifdef CAIN_SIP_NOMESSAGE_MODE

#define cain_sip_log(...)
#define cain_sip_message(...)
#define cain_sip_warning(...)

#else

static inline void cain_sip_log(cain_sip_log_level lev, const char *fmt,...){
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(lev, fmt, args);
        va_end (args);
}

static inline void cain_sip_message(const char *fmt,...)
{
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(CAIN_SIP_MESSAGE, fmt, args);
        va_end (args);
}

static inline void cain_sip_warning(const char *fmt,...)
{
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(CAIN_SIP_WARNING, fmt, args);
        va_end (args);
}

#endif

static inline void cain_sip_error(const char *fmt,...)
{
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(CAIN_SIP_ERROR, fmt, args);
        va_end (args);
}

static inline void cain_sip_fatal(const char *fmt,...)
{
        va_list args;
        va_start (args, fmt);
        cain_sip_logv(CAIN_SIP_FATAL, fmt, args);
        va_end (args);
}





#undef MIN
#define MIN(a,b)	((a)>(b) ? (b) : (a))
#undef MAX
#define MAX(a,b)	((a)>(b) ? (a) : (b))


char * cain_sip_concat (const char *str, ...);

uint64_t cain_sip_time_ms(void);

/*parameters accessors*/
#define GET_SET_STRING(object_type,attribute) \
	const char* object_type##_get_##attribute (const object_type##_t* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type##_t* obj,const char* value) {\
		if (obj->attribute != NULL) free((void*)obj->attribute);\
		obj->attribute=malloc(strlen(value)+1);\
		strcpy((char*)(obj->attribute),value);\
	}
#define GET_SET_STRING_PARAM(object_type,attribute) GET_SET_STRING_PARAM2(object_type,attribute,attribute)
#define GET_SET_STRING_PARAM2(object_type,attribute,func_name) \
	const char* object_type##_get_##func_name (const object_type##_t* obj) {\
	const char* l_value = cain_sip_parameters_get_parameter(CAIN_SIP_PARAMETERS(obj),#attribute);\
	if (l_value == NULL) { \
		cain_sip_warning("cannot find parameters [%s]",#attribute);\
		return NULL;\
	}\
	return l_value;\
	}\
	void object_type##_set_##func_name (object_type##_t* obj,const char* value) {\
		cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(obj),#attribute,value);\
	}


#define GET_SET_INT(object_type,attribute,type) GET_SET_INT_PRIVATE(object_type,attribute,type,)

#define GET_SET_INT_PRIVATE(object_type,attribute,type,set_prefix) \
	type  object_type##_get_##attribute (const object_type##_t* obj) {\
		return obj->attribute;\
	}\
	void set_prefix##object_type##_set_##attribute (object_type##_t* obj,type  value) {\
		obj->attribute=value;\
	}
#define GET_SET_INT_PARAM(object_type,attribute,type) GET_SET_INT_PARAM_PRIVATE(object_type,attribute,type,)
#define GET_SET_INT_PARAM2(object_type,attribute,type,func_name) GET_SET_INT_PARAM_PRIVATE2(object_type,attribute,type,,func_name)

#define ATO_(type,value) ATO_##type(value)
#define ATO_int(value) atoi(value)
#define ATO_float(value) strtof(value,NULL)
#define FORMAT_(type) FORMAT_##type
#define FORMAT_int    "%i"
#define FORMAT_float  "%f"

#define GET_SET_INT_PARAM_PRIVATE(object_type,attribute,type,set_prefix) GET_SET_INT_PARAM_PRIVATE2(object_type,attribute,type,set_prefix,attribute)
#define GET_SET_INT_PARAM_PRIVATE2(object_type,attribute,type,set_prefix,func_name) \
	type  object_type##_get_##func_name (const object_type##_t* obj) {\
		const char* l_value = cain_sip_parameters_get_parameter(CAIN_SIP_PARAMETERS(obj),#attribute);\
		if (l_value == NULL) { \
			cain_sip_error("cannot find parameters [%s]",#attribute);\
			return -1;\
		}\
		return ATO_(type,l_value);\
	}\
	void set_prefix##object_type##_set_##func_name (object_type##_t* obj,type  value) {\
		char l_str_value[16];\
		if (value == -1) { \
			cain_sip_parameters_remove_parameter(CAIN_SIP_PARAMETERS(obj),#attribute);\
			return;\
		}\
		snprintf(l_str_value,16,FORMAT_(type),value);\
		cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(obj),#attribute,(const char*)l_str_value);\
	}

#define GET_SET_BOOL(object_type,attribute,getter) \
	unsigned int object_type##_##getter##_##attribute (const object_type##_t* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type##_t* obj,unsigned int value) {\
		obj->attribute=value;\
	}
#define GET_SET_BOOL_PARAM2(object_type,attribute,getter,func_name) \
	unsigned int object_type##_##getter##_##func_name (const object_type##_t* obj) {\
		return cain_sip_parameters_is_parameter(CAIN_SIP_PARAMETERS(obj),#attribute);\
	}\
	void object_type##_set_##func_name (object_type##_t* obj,unsigned int value) {\
		cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(obj),#attribute,NULL);\
	}

#define CAIN_SIP_PARSE(object_type) \
cain_sip_##object_type##_t* cain_sip_##object_type##_parse (const char* value) { \
	pANTLR3_INPUT_STREAM           input; \
	pcain_sip_messageLexer               lex; \
	pANTLR3_COMMON_TOKEN_STREAM    tokens; \
	pcain_sip_messageParser              parser; \
	input  = antlr3NewAsciiStringCopyStream	(\
			(pANTLR3_UINT8)value,\
			(ANTLR3_UINT32)strlen(value),\
			NULL);\
	lex    = cain_sip_messageLexerNew                (input);\
	tokens = antlr3CommonTokenStreamSourceNew  (ANTLR3_SIZE_HINT, TOKENSOURCE(lex));\
	parser = cain_sip_messageParserNew               (tokens);\
	cain_sip_##object_type##_t* l_parsed_object = parser->object_type(parser);\
	parser ->free(parser);\
	tokens ->free(tokens);\
	lex    ->free(lex);\
	input  ->close(input);\
	if (l_parsed_object == NULL) cain_sip_error(#object_type" parser error for [%s]",value);\
	return l_parsed_object;\
}

#define CAIN_SIP_NEW(object_type,super_type) CAIN_SIP_NEW_HEADER(object_type,super_type,NULL)
#define CAIN_SIP_NEW_HEADER(object_type,super_type,name) CAIN_SIP_NEW_HEADER_INIT(object_type,super_type,name,header)
#define CAIN_SIP_NEW_HEADER_INIT(object_type,super_type,name,init_type) \
		CAIN_SIP_INSTANCIATE_VPTR(	cain_sip_##object_type##_t\
									, cain_sip_##super_type##_t\
									, cain_sip_##object_type##_destroy\
									, cain_sip_##object_type##_clone\
									, cain_sip_##object_type##_marshal); \
		cain_sip_##object_type##_t* cain_sip_##object_type##_new () { \
		cain_sip_##object_type##_t* l_object = cain_sip_object_new(cain_sip_##object_type##_t);\
		cain_sip_##super_type##_init((cain_sip_##super_type##_t*)l_object); \
		cain_sip_##init_type##_init((cain_sip_##init_type##_t*) l_object); \
		if (name) cain_sip_header_set_name(CAIN_SIP_HEADER(l_object),name);\
		return l_object;\
	}
typedef struct cain_sip_param_pair_t {
	int ref;
	char* name;
	char* value;
} cain_sip_param_pair_t;


cain_sip_param_pair_t* cain_sip_param_pair_new(const char* name,const char* value);

void cain_sip_param_pair_destroy(cain_sip_param_pair_t*  pair) ;

int cain_sip_param_pair_comp_func(const cain_sip_param_pair_t *a, const char*b) ;

cain_sip_param_pair_t* cain_sip_param_pair_ref(cain_sip_param_pair_t* obj);

void cain_sip_param_pair_unref(cain_sip_param_pair_t* obj);


void cain_sip_header_address_set_quoted_displayname(cain_sip_header_address_t* address,const char* value);

/*calss header*/
struct _cain_sip_header {
	cain_sip_object_t base;
	cain_sip_header_t* next;
	const char* name;
};

void cain_sip_header_set_next(cain_sip_header_t* header,cain_sip_header_t* next);
cain_sip_header_t* cain_sip_header_get_next(const cain_sip_header_t* headers);

void cain_sip_header_init(cain_sip_header_t* obj);
/*class parameters*/
struct _cain_sip_parameters {
	cain_sip_header_t base;
	cain_sip_list_t* param_list;
	cain_sip_list_t* paramnames_list;
};

void cain_sip_parameters_init(cain_sip_parameters_t *obj);

/*
 * Listening points and channels
*/

struct cain_sip_channel{
	cain_sip_object_t base;
	cain_sip_listening_point_t *lp; /* the listening point this channel belongs */
	struct addrinfo peer;
	struct sockaddr_storage peer_addr;
};

typedef struct cain_sip_udp_listening_point cain_sip_udp_listening_point_t;

#define CAIN_SIP_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_listening_point_t)
#define CAIN_SIP_UDP_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_udp_listening_point_t)
#define CAIN_SIP_CHANNEL(obj)		CAIN_SIP_CAST(obj,cain_sip_channel_t)

cain_sip_listening_point_t * cain_sip_udp_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port);
cain_sip_channel_t *cain_sip_listening_point_find_output_channel(cain_sip_listening_point_t *ip, const struct addrinfo *dest); 
cain_sip_source_t *cain_sip_channel_create_source(cain_sip_channel_t *, unsigned int events, int timeout, cain_sip_source_func_t callback, void *data);
int cain_sip_listening_point_get_well_known_port(const char *transport);


/*
 cain_sip_stack_t
*/
struct cain_sip_stack{
	cain_sip_object_t base;
	cain_sip_main_loop_t *ml;
	cain_sip_list_t *lp;/*list of listening points*/
	cain_sip_timer_config_t timer_config;
};

void cain_sip_stack_get_next_hop(cain_sip_stack_t *stack, cain_sip_request_t *req, cain_sip_hop_t *hop);

const cain_sip_timer_config_t *cain_sip_stack_get_timer_config(const cain_sip_stack_t *stack);

/*
 cain_sip_provider_t
*/

struct cain_sip_provider{
	cain_sip_object_t base;
	cain_sip_stack_t *stack;
	cain_sip_list_t *lps; /*listening points*/
	cain_sip_list_t *listeners;
};

cain_sip_provider_t *cain_sip_provider_new(cain_sip_stack_t *s, cain_sip_listening_point_t *lp);
void cain_sip_provider_set_transaction_terminated(cain_sip_provider_t *p, cain_sip_transaction_t *t);

typedef struct listener_ctx{
	cain_sip_listener_t *listener;
	void *data;
}listener_ctx_t;

#define CAIN_SIP_PROVIDER_INVOKE_LISTENERS(provider,callback,event) \
{ \
	cain_sip_list_t *_elem; \
	for(_elem=(provider)->listeners;_elem!=NULL;_elem=_elem->next){ \
		listener_ctx_t *_lctx=(listener_ctx_t*)_elem->data; \
		_lctx->listener->callback(_lctx->data,(event)); \
	} \
}

/*
 cain_sip_transaction_t
*/

struct cain_sip_transaction{
	cain_sip_object_t base;
	cain_sip_provider_t *provider; /*the provider that created this transaction */
	cain_sip_request_t *request;
	cain_sip_response_t *prov_response;
	cain_sip_response_t *final_response;
	char *branch_id;
	cain_sip_transaction_state_t state;
	cain_sip_sender_task_t *stask;
	uint64_t start_time;
	cain_sip_source_t *timer;
	int interval;
	int is_reliable:1;
	int is_server:1;
	int is_invite:1;
	void *appdata;
};


cain_sip_client_transaction_t * cain_sip_client_transaction_new(cain_sip_provider_t *prov,cain_sip_request_t *req);
cain_sip_server_transaction_t * cain_sip_server_transaction_new(cain_sip_provider_t *prov,cain_sip_request_t *req);
void cain_sip_client_transaction_add_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp);

/*
 cain_sip_response_t
*/
void cain_sip_response_get_return_hop(cain_sip_response_t *msg, cain_sip_hop_t *hop);

#define IS_TOKEN(token) \
		(INPUT->toStringTT(INPUT,LT(1),LT(strlen(#token)))->chars ?\
		strcmp(#token,(const char*)(INPUT->toStringTT(INPUT,LT(1),LT(strlen(#token)))->chars)) == 0:0)
char* _cain_sip_str_dup_and_unquote_string(const char* quoted_string);

/*********************************************************
 * SDP
 */
#define CAIN_SDP_PARSE(object_type) \
cain_sdp_##object_type##_t* cain_sdp_##object_type##_parse (const char* value) { \
	pANTLR3_INPUT_STREAM           input; \
	pcain_sdpLexer               lex; \
	pANTLR3_COMMON_TOKEN_STREAM    tokens; \
	pcain_sdpParser              parser; \
	input  = antlr3NewAsciiStringCopyStream	(\
			(pANTLR3_UINT8)value,\
			(ANTLR3_UINT32)strlen(value),\
			NULL);\
	lex    = cain_sdpLexerNew                (input);\
	tokens = antlr3CommonTokenStreamSourceNew  (ANTLR3_SIZE_HINT, TOKENSOURCE(lex));\
	parser = cain_sdpParserNew               (tokens);\
	cain_sdp_##object_type##_t* l_parsed_object = parser->object_type(parser).ret;\
	parser ->free(parser);\
	tokens ->free(tokens);\
	lex    ->free(lex);\
	input  ->close(input);\
	if (l_parsed_object == NULL) cain_sip_error(#object_type" parser error for [%s]",value);\
	return l_parsed_object;\
}
#define CAIN_SDP_NEW(object_type,super_type) \
		CAIN_SIP_INSTANCIATE_VPTR(	cain_sdp_##object_type##_t\
									, super_type##_t\
									, cain_sdp_##object_type##_destroy\
									, cain_sdp_##object_type##_clone\
									, cain_sdp_##object_type##_marshal); \
		cain_sdp_##object_type##_t* cain_sdp_##object_type##_new () { \
		cain_sdp_##object_type##_t* l_object = cain_sip_object_new(cain_sdp_##object_type##_t);\
		super_type##_init((super_type##_t*)l_object); \
		return l_object;\
	}






#ifdef __cplusplus
}
#endif


/*include private headers */
#include "cain_sip_resolver.h"
#include "sender_task.h"

#define CAIN_SIP_SOCKET_TIMEOUT 30000

#endif
