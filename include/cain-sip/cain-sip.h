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
#ifndef CAIN_SIP_H
#define CAIN_SIP_H

#include <stdlib.h>

#define CAIN_SIP_TYPE_ID(_type) _type##_id

typedef enum cain_sip_type_id{
	cain_sip_type_id_first=1,
	CAIN_SIP_TYPE_ID(cain_sip_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_server_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_client_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_transport_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_address_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_contact_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_from_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_to_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_via_t),
	CAIN_SIP_TYPE_ID(cain_sip_uri_t),
	CAIN_SIP_TYPE_ID(cain_sip_object_t),
	CAIN_SIP_TYPE_ID(cain_sip_parameters_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_callid_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_cseq_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_content_type_t),
	cain_sip_type_id_end
}cain_sip_type_id_t;


/**
 * cain_sip_object_t is the base object.
 * It is the base class for all cain sip non trivial objects.
 * It owns a reference count which allows to trigger the destruction of the object when the last
 * user of it calls cain_sip_object_unref().
**/

typedef struct _cain_sip_object cain_sip_object_t;

int cain_sip_object_is_unowed(const cain_sip_object_t *obj);

/**
 * Increments reference counter, which prevents the object from being destroyed.
 * If the object is initially unowed, this acquires the first reference.
**/
#define cain_sip_object_ref(obj) _cain_sip_object_ref((cain_sip_object_t*)obj)
cain_sip_object_t * _cain_sip_object_ref(cain_sip_object_t *obj);

/**
 * Decrements the reference counter. When it drops to zero, the object is destroyed.
**/
#define cain_sip_object_unref(obj) _cain_sip_object_unref((cain_sip_object_t*)obj)
void _cain_sip_object_unref(cain_sip_object_t *obj);

/**
 * Destroy the object: this function is intended for unowed object, that is objects
 * that were created with a 0 reference count.
**/
#define cain_sip_object_destroy(obj) _cain_sip_object_destroy((cain_sip_object_t*)obj)
void _cain_sip_object_destroy(cain_sip_object_t *obj);

void *cain_sip_object_cast(cain_sip_object_t *obj, cain_sip_type_id_t id, const char *castname, const char *file, int fileno);

#define CAIN_SIP_CAST(obj,_type) (_type*)cain_sip_object_cast((cain_sip_object_t *)(obj), _type##_id, #_type, __FILE__, __LINE__)
#define CAIN_SIP_OBJECT(obj) CAIN_SIP_CAST(obj,cain_sip_object_t)

#include "cain-sip/list.h"
#include "cain-sip/mainloop.h"
#include "cain-sip/uri.h"
#include "cain-sip/headers.h"
#include "cain-sip/parameters.h"
#include "cain-sip/message.h"
#include "cain-sip/transaction.h"
#undef TRUE
#define TRUE 1


#undef FALSE
#define FALSE 0

#endif
