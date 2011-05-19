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

#ifdef __cplusplus
#define CAIN_SIP_BEGIN_DECLS		extern "C"{
#define CAIN_SIP_END_DECLS		}
#else
#define CAIN_SIP_BEGIN_DECLS
#define CAIN_SIP_END_DECLS
#endif

#define CAIN_SIP_TYPE_ID(_type) _type##_id

typedef enum cain_sip_type_id{
	cain_sip_type_id_first=1,
	CAIN_SIP_TYPE_ID(cain_sip_stack_t),
	CAIN_SIP_TYPE_ID(cain_sip_listening_point_t),
	CAIN_SIP_TYPE_ID(cain_sip_datagram_listening_point_t),
	CAIN_SIP_TYPE_ID(cain_sip_udp_listening_point_t),
	CAIN_SIP_TYPE_ID(cain_sip_channel_t),
	CAIN_SIP_TYPE_ID(cain_sip_udp_channel_t),
	CAIN_SIP_TYPE_ID(cain_sip_provider_t),
	CAIN_SIP_TYPE_ID(cain_sip_main_loop_t),
	CAIN_SIP_TYPE_ID(cain_sip_source_t),
	CAIN_SIP_TYPE_ID(cain_sip_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_server_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_client_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_dialog_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_address_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_contact_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_from_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_to_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_via_t),
	CAIN_SIP_TYPE_ID(cain_sip_uri_t),
	CAIN_SIP_TYPE_ID(cain_sip_message_t),
	CAIN_SIP_TYPE_ID(cain_sip_request_t),
	CAIN_SIP_TYPE_ID(cain_sip_response_t),
	CAIN_SIP_TYPE_ID(cain_sip_object_t),
	CAIN_SIP_TYPE_ID(cain_sip_parameters_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_call_id_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_cseq_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_content_type_t),
	CAIN_SIP_TYPE_ID(cain_sip_sender_task_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_route_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_record_route_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_content_length_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_extension_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_authorization_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_www_authenticate_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_proxy_authorization_t),
	CAIN_SIP_TYPE_ID(cain_sdp_attribute_t),
	CAIN_SIP_TYPE_ID(cain_sdp_bandwidth_t),
	CAIN_SIP_TYPE_ID(cain_sdp_connection_t),
	CAIN_SIP_TYPE_ID(cain_sdp_email_t),
	CAIN_SIP_TYPE_ID(cain_sdp_info_t),
	CAIN_SIP_TYPE_ID(cain_sdp_key_t),
	CAIN_SIP_TYPE_ID(cain_sdp_media_t),
	CAIN_SIP_TYPE_ID(cain_sdp_media_description_t),
	CAIN_SIP_TYPE_ID(cain_sdp_origin_t),
	CAIN_SIP_TYPE_ID(cain_sdp_phone_t),
	CAIN_SIP_TYPE_ID(cain_sdp_repeate_time_t),
	CAIN_SIP_TYPE_ID(cain_sdp_session_description_t),
	CAIN_SIP_TYPE_ID(cain_sdp_session_name_t),
	CAIN_SIP_TYPE_ID(cain_sdp_time_t),
	CAIN_SIP_TYPE_ID(cain_sdp_time_description_t),
	CAIN_SIP_TYPE_ID(cain_sdp_uri_t),
	CAIN_SIP_TYPE_ID(cain_sdp_version_t),
	cain_sip_type_id_end
}cain_sip_type_id_t;


/**
 * cain_sip_object_t is the base object.
 * It is the base class for all cain sip non trivial objects.
 * It owns a reference count which allows to trigger the destruction of the object when the last
 * user of it calls cain_sip_object_unref().
**/

typedef struct _cain_sip_object cain_sip_object_t;

CAIN_SIP_BEGIN_DECLS

int cain_sip_object_is_unowed(const cain_sip_object_t *obj);

/**
 * Increments reference counter, which prevents the object from being destroyed.
 * If the object is initially unowed, this acquires the first reference.
**/
cain_sip_object_t * cain_sip_object_ref(void *obj);

/**
 * Decrements the reference counter. When it drops to zero, the object is destroyed.
**/
void cain_sip_object_unref(void *obj);
/**
 * Set object name.
**/
void cain_sip_object_set_name(cain_sip_object_t *obj,const char* name);
/**
 * Get object name.
**/
const char* cain_sip_object_get_name(cain_sip_object_t *obj);

cain_sip_object_t *cain_sip_object_clone(const cain_sip_object_t *obj);

/**
 * Delete the object: this function is intended for unowed object, that is objects
 * that were created with a 0 reference count. For all others, use cain_sip_object_unref().
**/
void cain_sip_object_delete(void *obj);

void *cain_sip_object_cast(cain_sip_object_t *obj, cain_sip_type_id_t id, const char *castname, const char *file, int fileno);

char* cain_sip_object_to_string(cain_sip_object_t* obj);

unsigned int cain_sip_object_is_instance_of(cain_sip_object_t * obj,cain_sip_type_id_t id);
CAIN_SIP_END_DECLS

#define CAIN_SIP_CAST(obj,_type) 		((_type*)cain_sip_object_cast((cain_sip_object_t *)(obj), _type##_id, #_type, __FILE__, __LINE__))
#define CAIN_SIP_OBJECT(obj) CAIN_SIP_CAST(obj,cain_sip_object_t)
#define CAIN_SIP_IS_INSTANCE_OF(obj,_type) cain_sip_object_is_instance_of(obj,_type##_id)


typedef struct cain_sip_listening_point cain_sip_listening_point_t;
typedef struct cain_sip_stack cain_sip_stack_t;
typedef struct cain_sip_provider cain_sip_provider_t;
typedef struct cain_sip_listener cain_sip_listener_t;
typedef struct cain_sip_dialog cain_sip_dialog_t;

#include "cain-sip/list.h"
#include "cain-sip/mainloop.h"
#include "cain-sip/uri.h"
#include "cain-sip/headers.h"
#include "cain-sip/parameters.h"
#include "cain-sip/message.h"
#include "cain-sip/transaction.h"
#include "cain-sip/dialog.h"
#include "cain-sip/sipstack.h"
#include "cain-sip/listeningpoint.h"
#include "cain-sip/provider.h"
#include "cain-sip/listener.h"

#undef TRUE
#define TRUE 1


#undef FALSE
#define FALSE 0

#endif
