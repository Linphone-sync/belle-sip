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
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

/* include all public headers*/
#include "cain-sip/cain-sip.h"

#include "port.h"

#ifdef PACKAGE
#undef PACKAGE
#endif
#ifdef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif
#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif
#ifdef PACKAGE_STRING
#undef PACKAGE_STRING
#endif
#ifdef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif
#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif
#ifdef VERSION
#undef VERSION
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#else

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "0.0.1"
#endif

#endif

#include "port.h"

/*etc*/

#define CAIN_SIP_INTERFACE_GET_METHODS(obj,interface) \
	((CAIN_SIP_INTERFACE_METHODS_TYPE(interface)*)cain_sip_object_get_interface_methods((cain_sip_object_t*)obj,CAIN_SIP_INTERFACE_ID(interface)))

#define __CAIN_SIP_INVOKE_LISTENER_BEGIN(list,interface_name,method) \
	if (list!=NULL) {\
		cain_sip_list_t *__copy=cain_sip_list_copy_with_data((list), (void* (*)(void*))cain_sip_object_ref);\
		const cain_sip_list_t *__elem=__copy;\
		do{\
			void *__method;\
			interface_name *__obj=(interface_name*)__elem->data;\
			__method=CAIN_SIP_INTERFACE_GET_METHODS(__obj,interface_name)->method;\
			if (__method) CAIN_SIP_INTERFACE_GET_METHODS(__obj,interface_name)->

#define __CAIN_SIP_INVOKE_LISTENER_END \
			__elem=__elem->next;\
		}while(__elem!=NULL);\
		cain_sip_list_free_with_data(__copy,cain_sip_object_unref);\
	}

#define CAIN_SIP_INVOKE_LISTENERS_VOID(list,interface_name,method) \
			__CAIN_SIP_INVOKE_LISTENER_BEGIN(list,interface_name,method)\
			method(__obj);\
			__CAIN_SIP_INVOKE_LISTENER_END

#define CAIN_SIP_INVOKE_LISTENERS_ARG(list,interface_name,method,arg) \
	__CAIN_SIP_INVOKE_LISTENER_BEGIN(list,interface_name,method)\
	method(__obj,arg);\
	__CAIN_SIP_INVOKE_LISTENER_END


#define CAIN_SIP_INVOKE_LISTENERS_ARG1_ARG2(list,interface_name,method,arg1,arg2) \
			__CAIN_SIP_INVOKE_LISTENER_BEGIN(list,interface_name,method)\
			method(__obj,arg1,arg2);\
			__CAIN_SIP_INVOKE_LISTENER_END

#define CAIN_SIP_INVOKE_LISTENERS_ARG1_ARG2_ARG3(list,interface_name,method,arg1,arg2,arg3) \
			__CAIN_SIP_INVOKE_LISTENER_BEGIN(list,interface_name)\
			method(__obj,arg1,arg2,arg3);\
			__CAIN_SIP_INVOKE_LISTENER_END

typedef struct weak_ref{
	struct weak_ref *next;
	cain_sip_object_destroy_notify_t notify;
	void *userpointer;
}weak_ref_t;


void *cain_sip_object_get_interface_methods(cain_sip_object_t *obj, cain_sip_interface_id_t ifid);
/*used internally by unref()*/
void cain_sip_object_delete(void *obj);
void cain_sip_object_pool_add(cain_sip_object_pool_t *pool, cain_sip_object_t *obj);
void cain_sip_object_pool_remove(cain_sip_object_pool_t *pool, cain_sip_object_t *obj);


#define CAIN_SIP_OBJECT_VPTR(obj,object_type) ((CAIN_SIP_OBJECT_VPTR_TYPE(object_type)*)(((cain_sip_object_t*)obj)->vptr))
#define cain_sip_object_init(obj)		/*nothing*/


/*list of all vptrs (classes) used in cain-sip*/
CAIN_SIP_DECLARE_VPTR(cain_sip_object_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_stack_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_datagram_listening_point_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_provider_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_main_loop_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_source_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_resolver_context_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_dialog_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_address_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_contact_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_from_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_to_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_via_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_uri_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_message_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_request_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_response_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_parameters_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_call_id_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_cseq_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_content_type_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_route_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_record_route_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_user_agent_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_content_length_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_extension_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_authorization_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_www_authenticate_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_proxy_authenticate_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_proxy_authorization_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_max_forwards_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_expires_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_allow_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_attribute_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_bandwidth_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_connection_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_email_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_info_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_key_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_media_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_media_description_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_origin_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_phone_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_repeate_time_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_session_description_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_session_name_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_time_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_time_description_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_uri_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_version_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_base_description_t);
CAIN_SIP_DECLARE_VPTR(cain_sdp_mime_parameter_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_refresher_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_subscription_state_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_service_route_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_refer_to_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_referred_by_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_replaces_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_date_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_hop_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_object_pool_t);
CAIN_SIP_DECLARE_VPTR(cain_sip_header_p_preferred_identity_t);

typedef void (*cain_sip_source_remove_callback_t)(cain_sip_source_t *);




struct cain_sip_source{
	cain_sip_object_t base;
	cain_sip_list_t node;
	unsigned long id;
	cain_sip_fd_t fd;
	unsigned short events,revents;
	int timeout;
	void *data;
	uint64_t expire_ms;
	int index; /* index in pollfd table */
	cain_sip_source_func_t notify;
	cain_sip_source_remove_callback_t on_remove;
	unsigned char cancelled;
	unsigned char expired;
	unsigned char oneshot;
	unsigned char notify_required; /*for testing purpose, use to ask for being scheduled*/
	cain_sip_socket_t sock;
};

void cain_sip_socket_source_init(cain_sip_source_t *s, cain_sip_source_func_t func, void *data, cain_sip_socket_t fd, unsigned int events, unsigned int timeout_value_ms);
void cain_sip_fd_source_init(cain_sip_source_t *s, cain_sip_source_func_t func, void *data, cain_sip_fd_t fd, unsigned int events, unsigned int timeout_value_ms);
void cain_sip_source_uninit(cain_sip_source_t *s);

#define cain_list_next(elem) ((elem)->next)

/* include private headers */
#include "channel.h"




#define cain_sip_new(type) (type*)cain_sip_malloc(sizeof(type))
#define cain_sip_new0(type) (type*)cain_sip_malloc0(sizeof(type))
	
cain_sip_list_t *cain_sip_list_new(void *data);
cain_sip_list_t*  cain_sip_list_append_link(cain_sip_list_t* elem,cain_sip_list_t *new_elem);
cain_sip_list_t *cain_sip_list_delete_custom(cain_sip_list_t *list, cain_sip_compare_func compare_func, const void *user_data);

#define cain_sip_list_next(elem) ((elem)->next)





#undef MIN
#define MIN(a,b)	((a)>(b) ? (b) : (a))
#undef MAX
#define MAX(a,b)	((a)>(b) ? (a) : (b))


CAINSIP_INTERNAL_EXPORT char * cain_sip_concat (const char *str, ...);

CAINSIP_INTERNAL_EXPORT uint64_t cain_sip_time_ms(void);

CAINSIP_INTERNAL_EXPORT unsigned int cain_sip_random(void);


/*parameters accessors*/
#define GET_SET_STRING(object_type,attribute) \
	const char* object_type##_get_##attribute (const object_type##_t* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type##_t* obj,const char* value) {\
		if (obj->attribute != NULL) cain_sip_free((void*)obj->attribute);\
		if (value) {\
			obj->attribute=cain_sip_strdup(value); \
		} else obj->attribute=NULL;\
	}
/*#define GET_SET_STRING_PARAM_NULL_ALLOWED(object_type,attribute) \
	GET_STRING_PARAM2(object_type,attribute,attribute) \
	void object_type##_set_##func_name (object_type##_t* obj,const char* value) {\
		cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(obj),#attribute,value);\
	}
*/
#define GET_SET_STRING_PARAM(object_type,attribute) GET_SET_STRING_PARAM2(object_type,attribute,attribute)
#define GET_SET_STRING_PARAM2(object_type,attribute,func_name) \
	GET_STRING_PARAM2(object_type,attribute,func_name) \
	void object_type##_set_##func_name (object_type##_t* obj,const char* value) {\
	if (cain_sip_parameters_has_parameter(CAIN_SIP_PARAMETERS(obj),#attribute) && !value) {\
		cain_sip_parameters_remove_parameter(CAIN_SIP_PARAMETERS(obj),#attribute); \
	} else \
		cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(obj),#attribute,value);\
	}

#define GET_STRING_PARAM2(object_type,attribute,func_name) \
	const char* object_type##_get_##func_name (const object_type##_t* obj) {\
	const char* l_value = cain_sip_parameters_get_parameter(CAIN_SIP_PARAMETERS(obj),#attribute);\
	if (l_value == NULL) { \
		/*cain_sip_warning("cannot find parameters [%s]",#attribute);*/\
		return NULL;\
	}\
	return l_value;\
	}

#define DESTROY_STRING(object,attribute) if (object->attribute) cain_sip_free((void*)object->attribute);

#define CLONE_STRING_GENERIC(object_type_src,object_type_dest,attribute,dest,src) \
		if ( object_type_src##_get_##attribute (src)) {\
			object_type_dest##_set_##attribute(dest,object_type_src##_get_##attribute(src));\
		}

#define CLONE_STRING(object_type,attribute,dest,src) CLONE_STRING_GENERIC(object_type,object_type,attribute,dest,src)

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
#define ATO_float(value) (float)strtod(value,NULL)
#define FORMAT_(type) FORMAT_##type
#define FORMAT_int    "%i"
#define FORMAT_float  "%f"

#define GET_SET_INT_PARAM_PRIVATE(object_type,attribute,type,set_prefix) GET_SET_INT_PARAM_PRIVATE2(object_type,attribute,type,set_prefix,attribute)
#define GET_SET_INT_PARAM_PRIVATE2(object_type,attribute,type,set_prefix,func_name) \
	type  object_type##_get_##func_name (const object_type##_t* obj) {\
		const char* l_value = cain_sip_parameters_get_parameter(CAIN_SIP_PARAMETERS(obj),#attribute);\
		if (l_value == NULL) { \
			/*cain_sip_error("cannot find parameters [%s]",#attribute);*/\
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
		return cain_sip_parameters_has_parameter(CAIN_SIP_PARAMETERS(obj),#attribute);\
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
	cain_sip_##object_type##_t* l_parsed_object; \
	input  = antlr3StringStreamNew(\
			(pANTLR3_UINT8)value,\
			ANTLR3_ENC_8BIT,\
			(ANTLR3_UINT32)strlen(value),\
			(pANTLR3_UINT8)#object_type);\
	lex    = cain_sip_messageLexerNew                (input);\
	tokens = antlr3CommonTokenStreamSourceNew  (ANTLR3_SIZE_HINT, TOKENSOURCE(lex));\
	parser = cain_sip_messageParserNew               (tokens);\
	l_parsed_object = parser->object_type(parser);\
	parser ->free(parser);\
	tokens ->free(tokens);\
	lex    ->free(lex);\
	input  ->close(input);\
	if (l_parsed_object == NULL) cain_sip_error(#object_type" parser error for [%s]",value);\
	return l_parsed_object;\
}

#define CAIN_SIP_NEW(object_type,super_type) \
	CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_##object_type##_t); \
	CAIN_SIP_INSTANCIATE_VPTR(	cain_sip_##object_type##_t\
									, cain_sip_##super_type##_t\
									, cain_sip_##object_type##_destroy\
									, cain_sip_##object_type##_clone\
									, cain_sip_##object_type##_marshal, TRUE); \
	cain_sip_##object_type##_t* cain_sip_##object_type##_new () { \
		cain_sip_##object_type##_t* l_object = cain_sip_object_new(cain_sip_##object_type##_t);\
		cain_sip_##super_type##_init((cain_sip_##super_type##_t*)l_object); \
		cain_sip_##object_type##_init((cain_sip_##object_type##_t*) l_object); \
		return l_object;\
	}

	
#define CAIN_SIP_NEW_HEADER(object_type,super_type,name) CAIN_SIP_NEW_HEADER_INIT(object_type,super_type,name,header)
#define CAIN_SIP_NEW_HEADER_INIT(object_type,super_type,name,init_type) \
	CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_##object_type##_t); \
	CAIN_SIP_INSTANCIATE_VPTR(	cain_sip_##object_type##_t\
								, cain_sip_##super_type##_t\
								, cain_sip_##object_type##_destroy\
								, cain_sip_##object_type##_clone\
								, cain_sip_##object_type##_marshal, TRUE); \
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
int cain_sip_param_pair_case_comp_func(const cain_sip_param_pair_t *a, const char*b) ;

cain_sip_param_pair_t* cain_sip_param_pair_ref(cain_sip_param_pair_t* obj);

void cain_sip_param_pair_unref(cain_sip_param_pair_t* obj);


void cain_sip_header_address_set_quoted_displayname(cain_sip_header_address_t* address,const char* value);

/*calss header*/
struct _cain_sip_header {
	cain_sip_object_t base;
	cain_sip_header_t* next;
	char *name;
	char *unparsed_value;
};

void cain_sip_header_set_next(cain_sip_header_t* header,cain_sip_header_t* next);
CAINSIP_INTERNAL_EXPORT cain_sip_header_t* cain_sip_header_get_next(const cain_sip_header_t* headers);
void cain_sip_response_fill_for_dialog(cain_sip_response_t *obj, cain_sip_request_t *req);
void cain_sip_util_copy_headers(cain_sip_message_t *orig, cain_sip_message_t *dest, const char*header, int multiple);

void cain_sip_header_init(cain_sip_header_t* obj);
/*class parameters*/
struct _cain_sip_parameters {
	cain_sip_header_t base;
	cain_sip_list_t* param_list;
	cain_sip_list_t* paramnames_list;
};

void cain_sip_parameters_init(cain_sip_parameters_t *obj);

/*
 * Listening points
*/

#include "listeningpoint_internal.h"


struct cain_sip_hop{
	cain_sip_object_t base;
	char *cname;
	char *host;
	char *transport;
	int port;
};


/*
 cain_sip_stack_t
*/
struct cain_sip_stack{
	cain_sip_object_t base;
	cain_sip_main_loop_t *ml;
	cain_sip_timer_config_t timer_config;
	int transport_timeout;
	int inactive_transport_timeout;
	int dns_timeout;
	int tx_delay; /*used to simulate network transmission delay, for tests*/
	int send_error; /* used to simulate network error. if <0, channel_send will return this value*/
	int resolver_tx_delay; /*used to simulate network transmission delay, for tests*/
	int resolver_send_error;	/* used to simulate network error*/
	int dscp;
	char *dns_user_hosts_file; /* used to load additional hosts file for tests */
};

cain_sip_hop_t* cain_sip_hop_new(const char* transport, const char *cname, const char* host,int port);
cain_sip_hop_t* cain_sip_hop_new_from_uri(const cain_sip_uri_t *uri);

cain_sip_hop_t * cain_sip_stack_get_next_hop(cain_sip_stack_t *stack, cain_sip_request_t *req);
const cain_sip_timer_config_t *cain_sip_stack_get_timer_config(const cain_sip_stack_t *stack);

/*
 cain_sip_provider_t
*/

struct cain_sip_provider{
	cain_sip_object_t base;
	cain_sip_stack_t *stack;
	cain_sip_list_t *lps; /*listening points*/
	cain_sip_list_t *listeners;
	cain_sip_list_t *internal_listeners; /*for transaction internaly managed by cain-sip. I.E by refreshers*/
	cain_sip_list_t *client_transactions;
	cain_sip_list_t *server_transactions;
	cain_sip_list_t *dialogs;
	cain_sip_list_t *auth_contexts;
	int rport_enabled; /*0 if rport should not be set in via header*/
};

cain_sip_provider_t *cain_sip_provider_new(cain_sip_stack_t *s, cain_sip_listening_point_t *lp);
void cain_sip_provider_add_client_transaction(cain_sip_provider_t *prov, cain_sip_client_transaction_t *t);
cain_sip_client_transaction_t *cain_sip_provider_find_matching_client_transaction(cain_sip_provider_t *prov, cain_sip_response_t *resp);
void cain_sip_provider_remove_client_transaction(cain_sip_provider_t *prov, cain_sip_client_transaction_t *t);
void cain_sip_provider_add_server_transaction(cain_sip_provider_t *prov, cain_sip_server_transaction_t *t);
cain_sip_server_transaction_t * cain_sip_provider_find_matching_server_transaction(cain_sip_provider_t *prov, 
                                                                                   cain_sip_request_t *req);
void cain_sip_provider_remove_server_transaction(cain_sip_provider_t *prov, cain_sip_server_transaction_t *t);
void cain_sip_provider_set_transaction_terminated(cain_sip_provider_t *p, cain_sip_transaction_t *t);
cain_sip_channel_t * cain_sip_provider_get_channel(cain_sip_provider_t *p, const cain_sip_hop_t *hop);
void cain_sip_provider_add_dialog(cain_sip_provider_t *prov, cain_sip_dialog_t *dialog);
void cain_sip_provider_remove_dialog(cain_sip_provider_t *prov, cain_sip_dialog_t *dialog);
void cain_sip_provider_release_channel(cain_sip_provider_t *p, cain_sip_channel_t *chan);
void cain_sip_provider_add_internal_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l);
void cain_sip_provider_remove_internal_sip_listener(cain_sip_provider_t *p, cain_sip_listener_t *l);
cain_sip_client_transaction_t * cain_sip_provider_find_matching_client_transaction_from_req(cain_sip_provider_t *prov, cain_sip_request_t *req) ;

typedef struct listener_ctx{
	cain_sip_listener_t *listener;
	void *data;
}listener_ctx_t;

#define CAIN_SIP_PROVIDER_INVOKE_LISTENERS_FOR_TRANSACTION(t,callback,event) \
		CAIN_SIP_PROVIDER_INVOKE_LISTENERS((t)->is_internal?(t)->provider->internal_listeners:(t)->provider->listeners,callback,event)

#define CAIN_SIP_PROVIDER_INVOKE_LISTENERS(listeners,callback,event) \
	CAIN_SIP_INVOKE_LISTENERS_ARG((listeners),cain_sip_listener_t,callback,(event))


/*
 cain_sip_transaction_t
*/

struct cain_sip_transaction{
	cain_sip_object_t base;
	cain_sip_provider_t *provider; /*the provider that created this transaction */
	cain_sip_request_t *request;
	cain_sip_response_t *last_response;
	cain_sip_channel_t *channel;
	cain_sip_dialog_t *dialog;
	char *branch_id;
	cain_sip_transaction_state_t state;
	uint64_t start_time;
	void *appdata;
	unsigned int is_internal:1;
	unsigned int timed_out:1; 
};


CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_transaction_t,cain_sip_object_t)
	void (*on_terminate)(cain_sip_transaction_t *obj);
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

static CAINSIP_INLINE const cain_sip_timer_config_t * cain_sip_transaction_get_timer_config(cain_sip_transaction_t *obj){
	return cain_sip_stack_get_timer_config(obj->provider->stack);
}

static CAINSIP_INLINE void cain_sip_transaction_start_timer(cain_sip_transaction_t *obj, cain_sip_source_t *timer){
	cain_sip_main_loop_add_source(obj->provider->stack->ml,timer);
}
/** */
static CAINSIP_INLINE void cain_sip_transaction_stop_timer(cain_sip_transaction_t *obj, cain_sip_source_t *timer){
	cain_sip_main_loop_remove_source(obj->provider->stack->ml,timer);
}

void cain_sip_transaction_notify_timeout(cain_sip_transaction_t *t);

void cain_sip_transaction_set_dialog(cain_sip_transaction_t *t, cain_sip_dialog_t *dialog);

void cain_sip_transaction_set_state(cain_sip_transaction_t *t, cain_sip_transaction_state_t state);

/*
 *
 *
 *	Client transaction
 *
 *
*/

struct cain_sip_client_transaction{
	cain_sip_transaction_t base;
	cain_sip_uri_t* preset_route; /*use to store outbound proxy, will be helpful for refresher*/
	cain_sip_hop_t* next_hop; /*use to send cancel request*/
};

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_client_transaction_t,cain_sip_transaction_t)
	void (*send_request)(cain_sip_client_transaction_t *);
	void (*on_response)(cain_sip_client_transaction_t *obj, cain_sip_response_t *resp);
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

void cain_sip_client_transaction_init(cain_sip_client_transaction_t *obj, cain_sip_provider_t *prov, cain_sip_request_t *req);
void cain_sip_client_transaction_add_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp);
void cain_sip_client_transaction_notify_response(cain_sip_client_transaction_t *t, cain_sip_response_t *resp);

struct cain_sip_ict{
	cain_sip_client_transaction_t base;
	cain_sip_source_t *timer_A;
	cain_sip_source_t *timer_B;
	cain_sip_source_t *timer_D;
	cain_sip_source_t *timer_M;
	cain_sip_request_t *ack;
};

typedef struct cain_sip_ict cain_sip_ict_t;

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_ict_t,cain_sip_client_transaction_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

cain_sip_ict_t * cain_sip_ict_new(cain_sip_provider_t *prov, cain_sip_request_t *req);

struct cain_sip_nict{
	cain_sip_client_transaction_t base;
	cain_sip_source_t *timer_F;
	cain_sip_source_t *timer_E;
	cain_sip_source_t *timer_K;
};

typedef struct cain_sip_nict cain_sip_nict_t;

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_nict_t,cain_sip_client_transaction_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

cain_sip_nict_t * cain_sip_nict_new(cain_sip_provider_t *prov, cain_sip_request_t *req);


/*
 *
 *
 *	Server transaction
 *
 *
*/

struct cain_sip_server_transaction{
	cain_sip_transaction_t base;
	char to_tag[8];
};

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_server_transaction_t,cain_sip_transaction_t)
	int (*send_new_response)(cain_sip_server_transaction_t *, cain_sip_response_t *resp);
	void (*on_request_retransmission)(cain_sip_server_transaction_t *obj);
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

void cain_sip_server_transaction_init(cain_sip_server_transaction_t *t, cain_sip_provider_t *prov,cain_sip_request_t *req);
void cain_sip_server_transaction_on_request(cain_sip_server_transaction_t *t, cain_sip_request_t *req);

struct cain_sip_ist{
	cain_sip_server_transaction_t base;
	cain_sip_source_t *timer_G;
	cain_sip_source_t *timer_H;
	cain_sip_source_t *timer_I;
	cain_sip_source_t *timer_L;
};

typedef struct cain_sip_ist cain_sip_ist_t;

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_ist_t,cain_sip_server_transaction_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

cain_sip_ist_t * cain_sip_ist_new(cain_sip_provider_t *prov, cain_sip_request_t *req);
/* returns 0 if the ack should be notified to TU, or -1 otherwise*/
int cain_sip_ist_process_ack(cain_sip_ist_t *obj, cain_sip_message_t *ack);

struct cain_sip_nist{
	cain_sip_server_transaction_t base;
	cain_sip_source_t *timer_J;
};

typedef struct cain_sip_nist cain_sip_nist_t;

CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(cain_sip_nist_t,cain_sip_server_transaction_t)
CAIN_SIP_DECLARE_CUSTOM_VPTR_END

cain_sip_nist_t * cain_sip_nist_new(cain_sip_provider_t *prov, cain_sip_request_t *req);


/*
 * Dialogs
 */ 
struct cain_sip_dialog{
	cain_sip_object_t base;
	void *appdata;
	cain_sip_provider_t *provider;
	cain_sip_request_t *last_out_invite;
	cain_sip_request_t *last_out_ack; /*so that it can be retransmitted when needed*/
	cain_sip_response_t *last_200Ok;
	cain_sip_source_t *timer_200Ok;
	cain_sip_source_t *timer_200Ok_end;
	cain_sip_dialog_state_t state;
	cain_sip_dialog_state_t previous_state;
	cain_sip_header_call_id_t *call_id;
	cain_sip_header_address_t *local_party;
	cain_sip_header_address_t *remote_party;
	cain_sip_list_t *route_set;
	cain_sip_header_address_t *remote_target;
	char *local_tag;
	char *remote_tag;
	unsigned int local_cseq;
	unsigned int remote_cseq;
	int is_server:1;
	int is_secure:1;
	int terminate_on_bye:1;
	int needs_ack:1;
	cain_sip_transaction_t* last_transaction;
};

cain_sip_dialog_t *cain_sip_dialog_new(cain_sip_transaction_t *t);
cain_sip_dialog_t * cain_sip_provider_get_new_dialog_internal(cain_sip_provider_t *prov, cain_sip_transaction_t *t,unsigned int check_last_resp);
/*returns 1 if message belongs to the dialog, 0 otherwise */
int _cain_sip_dialog_match(cain_sip_dialog_t *obj, const char *call_id, const char *local_tag, const char *remote_tag);
int cain_sip_dialog_match(cain_sip_dialog_t *obj, cain_sip_message_t *msg, int as_uas);
int cain_sip_dialog_update(cain_sip_dialog_t *obj,cain_sip_transaction_t* transaction, int as_uas);
void cain_sip_dialog_check_ack_sent(cain_sip_dialog_t*obj);
int cain_sip_dialog_handle_ack(cain_sip_dialog_t *obj, cain_sip_request_t *ack);

/*
 cain_sip_response_t
*/
cain_sip_hop_t* cain_sip_response_get_return_hop(cain_sip_response_t *msg);

#define IS_TOKEN(token) \
		(INPUT->toStringTT(INPUT,LT(1),LT(strlen(#token)))->chars ?\
		strcasecmp(#token,(const char*)(INPUT->toStringTT(INPUT,LT(1),LT(strlen(#token)))->chars)) == 0:0)
char* _cain_sip_str_dup_and_unquote_string(const char* quoted_string);

#define IS_HEADER_NAMED(name,compressed_name)    IS_TOKEN(name) || (compressed_name && IS_TOKEN(compressed_name))
/*********************************************************
 * SDP
 */
#define CAIN_SDP_PARSE(object_type) \
cain_sdp_##object_type##_t* cain_sdp_##object_type##_parse (const char* value) { \
	pANTLR3_INPUT_STREAM           input; \
	pcain_sdpLexer               lex; \
	pANTLR3_COMMON_TOKEN_STREAM    tokens; \
	pcain_sdpParser              parser; \
	cain_sdp_##object_type##_t* l_parsed_object; \
	input  = antlr3StringStreamNew	(\
			(pANTLR3_UINT8)value,\
			ANTLR3_ENC_8BIT,\
			(ANTLR3_UINT32)strlen(value),\
			(pANTLR3_UINT8)#object_type);\
	lex    = cain_sdpLexerNew                (input);\
	tokens = antlr3CommonTokenStreamSourceNew  (ANTLR3_SIZE_HINT, TOKENSOURCE(lex));\
	parser = cain_sdpParserNew               (tokens);\
	l_parsed_object = parser->object_type(parser).ret;\
	parser ->free(parser);\
	tokens ->free(tokens);\
	lex    ->free(lex);\
	input  ->close(input);\
	if (l_parsed_object == NULL) cain_sip_error(#object_type" parser error for [%s]",value);\
	return l_parsed_object;\
}
#define CAIN_SDP_NEW(object_type,super_type) \
		CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sdp_##object_type##_t); \
		CAIN_SIP_INSTANCIATE_VPTR(	cain_sdp_##object_type##_t\
									, super_type##_t\
									, cain_sdp_##object_type##_destroy\
									, cain_sdp_##object_type##_clone\
									, cain_sdp_##object_type##_marshal, TRUE); \
		cain_sdp_##object_type##_t* cain_sdp_##object_type##_new () { \
		cain_sdp_##object_type##_t* l_object = cain_sip_object_new(cain_sdp_##object_type##_t);\
		super_type##_init((super_type##_t*)l_object); \
		return l_object;\
	}
#define CAIN_SDP_NEW_WITH_CTR(object_type,super_type) \
		CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sdp_##object_type##_t); \
		CAIN_SIP_INSTANCIATE_VPTR(	cain_sdp_##object_type##_t\
									, super_type##_t\
									, cain_sdp_##object_type##_destroy\
									, cain_sdp_##object_type##_clone\
									, cain_sdp_##object_type##_marshal,TRUE); \
		cain_sdp_##object_type##_t* cain_sdp_##object_type##_new () { \
		cain_sdp_##object_type##_t* l_object = cain_sip_object_new(cain_sdp_##object_type##_t);\
		super_type##_init((super_type##_t*)l_object); \
		cain_sdp_##object_type##_init(l_object); \
		return l_object;\
	}



struct cain_sip_dialog_terminated_event{
	cain_sip_provider_t *source;
	cain_sip_dialog_t *dialog;
};

struct cain_sip_io_error_event{
	cain_sip_object_t *source;  /*the object impacted by this error*/
	const char *transport;
	const char *host;
	unsigned int port;
};

struct cain_sip_request_event{
	cain_sip_provider_t *source;
	cain_sip_server_transaction_t *server_transaction;
	cain_sip_dialog_t *dialog;
	cain_sip_request_t *request;
};

struct cain_sip_response_event{
	cain_sip_provider_t *source;
	cain_sip_client_transaction_t *client_transaction;
	cain_sip_dialog_t *dialog;
	cain_sip_response_t *response;
};

struct cain_sip_timeout_event{
	cain_sip_provider_t *source;
	cain_sip_transaction_t *transaction;
	int is_server_transaction;
};

struct cain_sip_transaction_terminated_event{
	cain_sip_provider_t *source;
	cain_sip_transaction_t *transaction;
	int is_server_transaction;
};

struct cain_sip_auth_event {
	char* username;
	char* userid;
	char* realm;
	char* passwd;
	char* ha1;
};
cain_sip_auth_event_t* cain_sip_auth_event_create(const char* realm,const char* username);

/*
 * refresher
 * */
cain_sip_refresher_t* cain_sip_refresher_new(cain_sip_client_transaction_t* transaction);


/*include private headers */
#include "cain_sip_resolver.h"

/*
 * returns a char, even if entry is escaped*/
int cain_sip_get_char (const char*a,int n,char*out);
/*return an escaped string*/
CAINSIP_INTERNAL_EXPORT	char* cain_sip_to_escaped_string(const char* buff) ;
CAINSIP_INTERNAL_EXPORT	char* cain_sip_to_unescaped_string(const char* buff) ;


#define CAIN_SIP_SOCKET_TIMEOUT 30000

#define CAIN_SIP_BRANCH_ID_LENGTH 10
/*Shall not be less than 32bit */
#define CAIN_SIP_TAG_LENGTH 6
#define CAIN_SIP_MAX_TO_STRING_SIZE 2048

#endif
