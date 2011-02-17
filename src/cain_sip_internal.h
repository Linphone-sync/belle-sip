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

struct _cain_sip_object{
	uint8_t type_ids[8]; /*table of cain_sip_type_id_t for all inheritance chain*/
	int ref;
	void *vptr;
	cain_sip_object_destroy_t destroy;
	const char* name;
};

cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_type_id_t id, void *vptr, cain_sip_object_destroy_t destroy_func, int initially_unowed);
void _cain_sip_object_init_type(cain_sip_object_t *obj, cain_sip_type_id_t id);
void cain_sip_object_init(cain_sip_object_t *obj);


#define cain_sip_object_new(_type,destroy) (_type*)_cain_sip_object_new(sizeof(_type),CAIN_SIP_TYPE_ID(_type),NULL,(cain_sip_object_destroy_t)destroy,0)
#define cain_sip_object_new_unowed(_type,destroy) (_type*)_cain_sip_object_new(sizeof(_type),CAIN_SIP_TYPE_ID(_type),NULL,(cain_sip_object_destroy_t)destroy,1)
#define cain_sip_object_init_type(obj, _type) _cain_sip_object_init_type((cain_sip_object_t*)obj, CAIN_SIP_TYPE_ID(_type))

#define cain_sip_object_new_with_vptr(_type,vptr,destroy) (_type*)_cain_sip_object_new(sizeof(_type),CAIN_SIP_TYPE_ID(_type),vptr,(cain_sip_object_destroy_t)destroy,0)
#define cain_sip_object_new_unowed_with_vptr(_type,vptr,destroy) (_type*)_cain_sip_object_new(sizeof(_type),CAIN_SIP_TYPE_ID(_type),vptr,(cain_sip_object_destroy_t)destroy,1)

#define CAIN_SIP_OBJECT_VPTR(obj,vptr_type) ((vptr_type*)(((cain_sip_object_t*)obj)->vptr))
		

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
	int cancelled;
};

void cain_sip_fd_source_init(cain_sip_source_t *s, cain_sip_source_func_t func, void *data, int fd, unsigned int events, unsigned int timeout_value_ms);

#define cain_list_next(elem) ((elem)->next)


#ifdef __cplusplus
extern "C"{
#endif

void *cain_sip_malloc(size_t size);
void *cain_sip_malloc0(size_t size);
void *cain_sip_realloc(void *ptr, size_t size);
void cain_sip_free(void *ptr);
char * cain_sip_strdup(const char *s);

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
	const char* object_type##_get_##attribute (object_type##_t* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type##_t* obj,const char* value) {\
		if (obj->attribute != NULL) free((void*)obj->attribute);\
		obj->attribute=malloc(strlen(value)+1);\
		strcpy((char*)(obj->attribute),value);\
	}
#define GET_SET_STRING_PARAM(object_type,attribute) GET_SET_STRING_PARAM2(object_type,attribute,attribute)
#define GET_SET_STRING_PARAM2(object_type,attribute,func_name) \
	const char* object_type##_get_##func_name (object_type##_t* obj) {\
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
	type  object_type##_get_##attribute (object_type##_t* obj) {\
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
	type  object_type##_get_##func_name (object_type##_t* obj) {\
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
	unsigned int object_type##_##getter##_##attribute (object_type##_t* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type##_t* obj,unsigned int value) {\
		obj->attribute=value;\
	}
#define GET_SET_BOOL_PARAM2(object_type,attribute,getter,func_name) \
	unsigned int object_type##_##getter##_##func_name (object_type##_t* obj) {\
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
	return l_parsed_object;\
}

#define CAIN_SIP_NEW(object_type,super_type) CAIN_SIP_NEW_WITH_NAME(object_type,super_type,NULL)

#define CAIN_SIP_NEW_WITH_NAME(object_type,super_type,name) \
		cain_sip_##object_type##_t* cain_sip_##object_type##_new () { \
		cain_sip_##object_type##_t* l_object = cain_sip_object_new(cain_sip_##object_type##_t, (cain_sip_object_destroy_t)cain_sip_##object_type##_destroy);\
		cain_sip_##super_type##_init((cain_sip_##super_type##_t*)l_object); \
		cain_sip_object_set_name(CAIN_SIP_OBJECT(l_object),name);\
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
	const char* name;
};
void cain_sip_header_destroy(cain_sip_header_t *header);
void cain_sip_header_init(cain_sip_header_t* obj);
/*class parameters*/
struct _cain_sip_parameters {
	cain_sip_header_t base;
	cain_sip_list_t* param_list;
	cain_sip_list_t* paramnames_list;
};

void cain_sip_parameters_init(cain_sip_parameters_t *obj);
void cain_sip_parameters_destroy(cain_sip_parameters_t* params);

typedef struct cain_sip_udp_listening_point cain_sip_udp_listening_point_t;

#define CAIN_SIP_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_listening_point_t)
#define CAIN_SIP_UDP_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_udp_listening_point_t)
#define CAIN_SIP_CHANNEL(obj)		CAIN_SIP_CAST(obj,cain_sip_channel_t)

cain_sip_listening_point_t * cain_sip_udp_listening_point_new(cain_sip_stack_t *s, const char *ipaddress, int port);
cain_sip_channel_t *cain_sip_listening_point_find_output_channel(cain_sip_listening_point_t *ip, const struct addrinfo *dest); 
cain_sip_source_t *cain_sip_channel_create_source(cain_sip_channel_t *, unsigned int events, int timeout, cain_sip_source_func_t callback, void *data);

/*
 cain_sip_stack_t
*/
struct cain_sip_stack{
	cain_sip_object_t base;
	cain_sip_main_loop_t *ml;
	cain_sip_list_t *lp;/*list of listening points*/
};

void cain_sip_stack_get_next_hop(cain_sip_stack_t *stack, cain_sip_request_t *req, cain_sip_hop_t *hop);


/*
 cain_sip_provider_t
*/
cain_sip_provider_t *cain_sip_provider_new(cain_sip_stack_t *s, cain_sip_listening_point_t *lp);

typedef struct listener_ctx{
	cain_sip_listener_t *listener;
	void *data;
}listener_ctx_t;

#define CAIN_SIP_PROVIDER_INVOKE_LISTENERS(provider,callback,event) \
{ \
	cain_sip_list_t *_elem; \
	for(_elem=(provider)->listeners;_elem!=NULL;_elem=_elem->next){ \
		listener_ctx_t *_lctx=(listener_ctx_t*)_elem->data; \
		_lctx->##callback(_lctx->data,event); \
	} \
}

/*
 cain_sip_transaction_t
*/

cain_sip_client_transaction_t * cain_sip_client_transaction_new(cain_sip_provider_t *prov,cain_sip_request_t *req);
cain_sip_server_transaction_t * cain_sip_server_transaction_new(cain_sip_provider_t *prov,cain_sip_request_t *req);

#ifdef __cplusplus
}
#endif

/*include private headers */
#include "cain_sip_resolver.h"
#include "sender_task.h"

#define CAIN_SIP_SOCKET_TIMEOUT 30000

#endif
