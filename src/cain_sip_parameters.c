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
#include "cain-sip/cain-sip.h"
#include "cain-sip/parameters.h"
#include "cain_sip_internal.h"
#include "cain-sip/headers.h"

void cain_sip_parameters_init(cain_sip_parameters_t *obj){
}

void cain_sip_parameters_clean(cain_sip_parameters_t* params) {
	if (params->param_list) cain_sip_list_free_with_data (params->param_list, (void (*)(void*))cain_sip_param_pair_destroy);
	if (params->paramnames_list) cain_sip_list_free(params->paramnames_list);
	params->paramnames_list=NULL;
	params->param_list=NULL;
}
static void cain_sip_parameters_destroy(cain_sip_parameters_t* params) {
	cain_sip_parameters_clean(params);
}

static void cain_sip_parameters_clone(cain_sip_parameters_t *params, const cain_sip_parameters_t *orig){
	cain_sip_list_t* list=orig->param_list;
	for(;list!=NULL;list=list->next){
		cain_sip_param_pair_t* container = (cain_sip_param_pair_t* )(list->data);
		cain_sip_parameters_set_parameter( params,container->name,container->value);
	}
}

int cain_sip_parameters_marshal(const cain_sip_parameters_t* params, char* buff,unsigned int offset,unsigned int buff_size) {
	cain_sip_list_t* list=params->param_list;
	unsigned int curent_offset=offset;
	for(;list!=NULL;list=list->next){
		cain_sip_param_pair_t* container = (cain_sip_param_pair_t* )(list->data);
		if (container->value) {
			curent_offset+=snprintf(buff+curent_offset,buff_size-curent_offset,";%s=%s",container->name,container->value);
		} else {
			curent_offset+=snprintf(buff+curent_offset,buff_size-curent_offset,";%s",container->name);
		}
	}
	return curent_offset-offset;
}
CAIN_SIP_NEW_HEADER(parameters,header,"parameters")
const cain_sip_list_t *	cain_sip_parameters_get_parameters(const cain_sip_parameters_t* obj) {
	return obj->param_list;
}

const char*	cain_sip_parameters_get_parameter_base(const cain_sip_parameters_t* params,const char* name,cain_sip_compare_func func) {
	cain_sip_list_t *  lResult = cain_sip_list_find_custom(params->param_list, func, name);
	if (lResult) {
		return ((cain_sip_param_pair_t*)(lResult->data))->value;
	}
	else {
		return NULL;
	}
}
const char* cain_sip_parameters_get_parameter(const cain_sip_parameters_t* params,const char* name) {
	return cain_sip_parameters_get_parameter_base(params,name,(cain_sip_compare_func)cain_sip_param_pair_comp_func);
}
const char* cain_sip_parameters_get_case_parameter(const cain_sip_parameters_t* params,const char* name) {
	return cain_sip_parameters_get_parameter_base(params,name,(cain_sip_compare_func)cain_sip_param_pair_case_comp_func);
}

unsigned int cain_sip_parameters_has_parameter(const cain_sip_parameters_t* params,const char* name) {
	return cain_sip_list_find_custom(params->param_list, (cain_sip_compare_func)cain_sip_param_pair_comp_func, name) != NULL;
}

void cain_sip_parameters_set_parameter(cain_sip_parameters_t* params,const char* name,const char* value) {
	/*1 check if present*/
	cain_sip_param_pair_t* lNewpair;
	cain_sip_list_t *  lResult = cain_sip_list_find_custom(params->paramnames_list, (cain_sip_compare_func)strcmp, name);
	/* first remove from header names list*/
	if (lResult) {
		params->paramnames_list=cain_sip_list_delete_link(params->paramnames_list,lResult);
	}
	/* next from header list*/
	lResult = cain_sip_list_find_custom(params->param_list, (cain_sip_compare_func)cain_sip_param_pair_comp_func, name);
	if (lResult) {
		cain_sip_param_pair_destroy(lResult->data);
		params->param_list=cain_sip_list_delete_link(params->param_list,lResult);
	}
	/* 2 insert*/
	lNewpair = cain_sip_param_pair_new(name,value);
	params->param_list=cain_sip_list_append(params->param_list,lNewpair);
	params->paramnames_list=cain_sip_list_append(params->paramnames_list,lNewpair->name);
}

const cain_sip_list_t*	cain_sip_parameters_get_parameter_names(const cain_sip_parameters_t* params) {
	return params?params->paramnames_list:NULL;
}

void cain_sip_parameters_remove_parameter(cain_sip_parameters_t* params,const char* name) {
	/*1 check if present*/
	cain_sip_list_t *  lResult = cain_sip_list_find_custom(params->paramnames_list, (cain_sip_compare_func)strcmp, name);
	/* first remove from header names list*/
	if (lResult) {
		params->paramnames_list=cain_sip_list_delete_link(params->paramnames_list,lResult);
		/*next remove node*/
		lResult = cain_sip_list_find_custom(params->param_list, (cain_sip_compare_func)cain_sip_param_pair_comp_func, name);
		if (lResult) {
			cain_sip_param_pair_destroy(lResult->data);
			params->param_list=cain_sip_list_delete_link(params->param_list,lResult);
		}
	} else {
		cain_sip_warning("cannot remove param %s because not present",name);
	}
}

