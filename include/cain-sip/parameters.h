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

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "cain-sip/utils.h"
#include "cain-sip/list.h"
/***
 *  parameters
 *
 */

typedef struct _cain_sip_parameters cain_sip_parameters_t;

cain_sip_parameters_t* cain_sip_parameters_new();
/*
 * remove all parameters */
CAINSIP_EXPORT void cain_sip_parameters_clean(cain_sip_parameters_t* params);


CAINSIP_EXPORT const char*	cain_sip_parameters_get_parameter(const cain_sip_parameters_t* obj,const char* name);
/*
 * same as #cain_sip_parameters_get_parameter but name is case insensitive */
CAINSIP_EXPORT const char*	cain_sip_parameters_get_case_parameter(const cain_sip_parameters_t* params,const char* name);

/**
 * returns 0 if not found
 */
CAINSIP_EXPORT unsigned int cain_sip_parameters_has_parameter(const cain_sip_parameters_t* obj,const char* name);

CAINSIP_EXPORT void	cain_sip_parameters_set_parameter(cain_sip_parameters_t* obj,const char* name,const char* value);

CAINSIP_EXPORT const cain_sip_list_t *	cain_sip_parameters_get_parameter_names(const cain_sip_parameters_t* obj);

CAINSIP_EXPORT const cain_sip_list_t *	cain_sip_parameters_get_parameters(const cain_sip_parameters_t* obj);

CAINSIP_EXPORT void	cain_sip_parameters_remove_parameter(cain_sip_parameters_t* obj,const char* name);

CAINSIP_EXPORT cain_sip_error_code cain_sip_parameters_marshal(const cain_sip_parameters_t* obj, char* buff, size_t buff_size, size_t *offset);

#define CAIN_SIP_PARAMETERS(obj) CAIN_SIP_CAST(obj,cain_sip_parameters_t)

#endif /*PARAMETERS_H_*/

