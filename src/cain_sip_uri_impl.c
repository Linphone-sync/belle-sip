/*
 * SipUri.cpp
 *
 *  Created on: 18 sept. 2010
 *      Author: jehanmonnier
 */

#include "cain_sip_uri.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "cain_sip_uriParser.h"
#include "cain_sip_uriLexer.h"
#include "cain_sip_utils.h"

#define GET_SET_STRING(object_type,attribute) \
	const char* object_type##_get_##attribute (object_type* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type* obj,const char* value) {\
		if (obj->attribute != NULL) free((void*)obj->attribute);\
		obj->attribute=malloc(strlen(value)+1);\
		strcpy((char*)(obj->attribute),value);\
	}

#define SIP_URI_GET_SET_STRING(attribute) GET_SET_STRING(cain_sip_uri,attribute)

#define GET_SET_UINT(object_type,attribute) \
	unsigned int object_type##_get_##attribute (object_type* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type* obj,unsigned int value) {\
		obj->attribute=value;\
	}

#define SIP_URI_GET_SET_UINT(attribute) GET_SET_UINT(cain_sip_uri,attribute)

#define GET_SET_BOOL(object_type,attribute,getter) \
	unsigned int object_type##_##getter##_##attribute (object_type* obj) {\
		return obj->attribute;\
	}\
	void object_type##_set_##attribute (object_type* obj,unsigned int value) {\
		obj->attribute=value;\
	}

#define SIP_URI_GET_SET_BOOL(attribute) GET_SET_BOOL(cain_sip_uri,attribute,is)
#define SIP_URI_HAS_SET_BOOL(attribute) GET_SET_BOOL(cain_sip_uri,attribute,has)




struct _cain_sip_uri {
	const char* user;
	const char* host;
	const char* transport_param;
	unsigned int port;
	unsigned int secure;
	unsigned int lr_param;
	const char* maddr_param;
	cain_sip_list* header_list;
	cain_sip_list* headernames_list;
};
void cain_sip_uri_delete(cain_sip_uri* uri) {
	if (uri->user) free (uri->user);
	if (uri->host) free (uri->host);
	if (uri->transport_param) free (uri->transport_param);
	if (uri->maddr_param) free (uri->maddr_param);
	if (uri->header_list) cain_sip_list_free (uri->header_list);
	if (uri->headernames_list) cain_sip_list_free (uri->headernames_list);
	free(uri);
}

typedef struct _header_pair {
	const char* name;
	const char* value;

} header_pair;
static const header_pair* header_pair_new(const char* name,const char* value) {
	header_pair* lPair = (header_pair*)malloc( sizeof(header_pair));
	lPair->name=strdup(name);
	lPair->value=strdup(value);
	return lPair;
}
static void header_pair_delete(header_pair*  pair) {
	free(pair->name);
	free(pair->value);
	free (pair);
}
static int header_pair_comp_func(const header_pair *a, const char*b) {
	return strcmp(a->name,b);
}

cain_sip_uri* cain_sip_uri_parse (const char* uri) {
	pANTLR3_INPUT_STREAM           input;
	pcain_sip_uriLexer               lex;
	pANTLR3_COMMON_TOKEN_STREAM    tokens;
	pcain_sip_uriParser              parser;
	input  = antlr3NewAsciiStringCopyStream	(
			(pANTLR3_UINT8)uri,
			(ANTLR3_UINT32)strlen(uri),
			NULL);
	lex    = cain_sip_uriLexerNew                (input);
	tokens = antlr3CommonTokenStreamSourceNew  (ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
	parser = cain_sip_uriParserNew               (tokens);

	cain_sip_uri* l_parsed_uri = parser->an_sip_uri(parser);

	// Must manually clean up
	//
	parser ->free(parser);
	tokens ->free(tokens);
	lex    ->free(lex);
	input  ->close(input);
	return l_parsed_uri;
}
cain_sip_uri* cain_sip_uri_new () {
	cain_sip_uri* lUri = (cain_sip_uri*)malloc(sizeof(cain_sip_uri));
	memset(lUri,0,sizeof(cain_sip_uri));
	return lUri;
}



char*	cain_sip_uri_to_string(cain_sip_uri* uri)  {
	return cain_sip_concat(	"sip:"
					,(uri->user?uri->user:"")
					,(uri->user?"@":"")
					,uri->host
					,(uri->transport_param?";transport=":"")
					,(uri->transport_param?uri->transport_param:"")
					,NULL);
}


const char*	cain_sip_uri_get_header(cain_sip_uri* uri,const char* name) {
	cain_sip_list * lResult = cain_sip_list_find_custom(uri->header_list, header_pair_comp_func, name);
	if (lResult) {
		return ((header_pair*)(lResult->data))->value;
	}
	else {
		return NULL;
	}
}
void	cain_sip_uri_set_header(cain_sip_uri* uri,const char* name,const char* value) {
	/*1 check if present*/
	cain_sip_list * lResult = cain_sip_list_find_custom(uri->headernames_list, strcmp, name);
	/* first remove from headersnames list*/
	if (lResult) {
		cain_sip_list_remove_link(uri->headernames_list,lResult);
	}
	/* next from header list*/
	lResult = cain_sip_list_find_custom(uri->header_list, header_pair_comp_func, name);
	if (lResult) {
		header_pair_delete(lResult->data);
		cain_sip_list_remove_link(uri->header_list,lResult);
	}
	/* 2 insert*/
	header_pair* lNewpair = header_pair_new(name,value);
	uri->header_list=cain_sip_list_append(uri->header_list,lNewpair);
	uri->headernames_list=cain_sip_list_append(uri->headernames_list,lNewpair->name);
}

cain_sip_list*	cain_sip_uri_get_header_names(cain_sip_uri* uri) {
	return uri->headernames_list;
}




SIP_URI_GET_SET_STRING(user)
SIP_URI_GET_SET_STRING(host)
SIP_URI_GET_SET_STRING(transport_param)
SIP_URI_GET_SET_STRING(maddr_param)
SIP_URI_GET_SET_UINT(port)
SIP_URI_GET_SET_BOOL(secure)
SIP_URI_HAS_SET_BOOL(lr_param)
