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
	cain_sip_object_destroy_t destroy;
};

cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_type_id_t id, cain_sip_object_destroy_t destroy_func, int initially_unowed);
void _cain_sip_object_init_type(cain_sip_object_t *obj, cain_sip_type_id_t id);

#define cain_sip_object_new(_type,destroy) (_type*)_cain_sip_object_new(sizeof(_type),CAIN_SIP_TYPE_ID(_type),destroy,0)
#define cain_sip_object_new_unowed(_type,destroy) (_type*)_cain_sip_object_new(sizeof(_type),CAIN_SIP_TYPE_ID(_type),destroy,1)
#define cain_sip_object_init_type(obj, _type) _cain_sip_object_init_type((cain_sip_object_t*)obj, CAIN_SIP_TYPE_ID(_type))

struct _cain_sip_list {
	struct _cain_sip_list *next;
	struct _cain_sip_list *prev;
	void *data;
};

typedef void (*cain_sip_source_remove_callback_t)(cain_sip_source_t *);

struct cain_sip_source{
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
#define GET_SET_INT(object_type,attribute,type) GET_SET_INT_PRIVATE(object_type,attribute,type,)

#define GET_SET_INT_PRIVATE(object_type,attribute,type,set_prefix) \
	type  object_type##_get_##attribute (object_type##_t* obj) {\
		return obj->attribute;\
	}\
	void set_prefix##object_type##_set_##attribute (object_type##_t* obj,type  value) {\
		obj->attribute=value;\
	}

#define GET_SET_BOOL(object_type,attribute,getter) \
	unsigned int object_type##_##getter##_##attribute (object_type##_t* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type##_t* obj,unsigned int value) {\
		obj->attribute=value;\
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

#define CAIN_SIP_REF(object_type) \
cain_sip_##object_type##_t* cain_sip_##object_type##_ref (cain_sip_##object_type##_t* obj) { \
	obj->ref++;\
	return obj;\
}\
void cain_sip_##object_type##_unref (cain_sip_##object_type##_t* obj) { \
	obj->ref--; \
	if (obj->ref < 0) {\
		cain_sip_##object_type##_delete(obj);\
	}\
}


typedef struct cain_sip_param_pair_t {
	int ref;
	char* name;
	char* value;
} cain_sip_param_pair_t;


cain_sip_param_pair_t* cain_sip_param_pair_new(const char* name,const char* value);

void cain_sip_param_pair_delete(cain_sip_param_pair_t*  pair) ;

int cain_sip_param_pair_comp_func(const cain_sip_param_pair_t *a, const char*b) ;

cain_sip_param_pair_t* cain_sip_param_pair_ref(cain_sip_param_pair_t* obj);

void cain_sip_param_pair_unref(cain_sip_param_pair_t* obj);



#ifdef __cplusplus
}
#endif

/*include private headers */
#include "cain_sip_resolver.h"

#endif
