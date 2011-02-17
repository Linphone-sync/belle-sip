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

void cain_sip_parameters_destroy(cain_sip_parameters_t* params) {
	if (params->param_list) cain_sip_list_free (params->param_list);
	if (params->paramnames_list) cain_sip_list_free (params->paramnames_list);
	cain_sip_header_destroy(CAIN_SIP_HEADER(params));
}
void cain_sip_parameters_init(cain_sip_parameters_t *obj) {
	cain_sip_object_init_type(obj,cain_sip_parameters_t);
	cain_sip_header_init((cain_sip_header_t*)obj);
}

cain_sip_parameters_t* cain_sip_parameters_new() {
	cain_sip_parameters_t* l_object = (cain_sip_parameters_t*)cain_sip_object_new(cain_sip_parameters_t,(cain_sip_object_destroy_t)cain_sip_parameters_destroy);
	cain_sip_header_init((cain_sip_header_t*)l_object);
	return l_object;
}

const char*	cain_sip_parameters_get_parameter(cain_sip_parameters_t* params,const char* name) {
	cain_sip_list_t *  lResult = cain_sip_list_find_custom(params->param_list, (cain_sip_compare_func)cain_sip_param_pair_comp_func, name);
	if (lResult) {
		return ((cain_sip_param_pair_t*)(lResult->data))->value;
	}
	else {
		return NULL;
	}
}
unsigned int cain_sip_parameters_is_parameter(cain_sip_parameters_t* params,const char* name) {
	return cain_sip_list_find_custom(params->param_list, (cain_sip_compare_func)cain_sip_param_pair_comp_func, name) != NULL;
}
void	cain_sip_parameters_set_parameter(cain_sip_parameters_t* params,const char* name,const char* value) {
	/*1 check if present*/
	cain_sip_list_t *  lResult = cain_sip_list_find_custom(params->paramnames_list, (cain_sip_compare_func)strcmp, name);
	/* first remove from header names list*/
	if (lResult) {
		cain_sip_list_remove_link(params->paramnames_list,lResult);
	}
	/* next from header list*/
	lResult = cain_sip_list_find_custom(params->param_list, (cain_sip_compare_func)cain_sip_param_pair_comp_func, name);
	if (lResult) {
		cain_sip_param_pair_destroy(lResult->data);
		cain_sip_list_remove_link(params->param_list,lResult);
	}
	/* 2 insert*/
	cain_sip_param_pair_t* lNewpair = cain_sip_param_pair_new(name,value);
	params->param_list=cain_sip_list_append(params->param_list,lNewpair);
	params->paramnames_list=cain_sip_list_append(params->paramnames_list,lNewpair->name);
}

const cain_sip_list_t*	cain_sip_parameters_get_parameter_names(cain_sip_parameters_t* params) {
	return params->paramnames_list;
}
void	cain_sip_parameters_remove_parameter(cain_sip_parameters_t* params,const char* name) {
	/*1 check if present*/
	cain_sip_list_t *  lResult = cain_sip_list_find_custom(params->paramnames_list, (cain_sip_compare_func)strcmp, name);
	/* first remove from header names list*/
	if (lResult) {
		cain_sip_list_remove_link(params->paramnames_list,lResult);
	} else {
		cain_sip_warning("cannot remove param \%s because not present",name);
	}
}

