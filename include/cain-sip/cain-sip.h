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

#include "cain-sip/object.h"

/**
 * This enum declares all object types used in cain-sip (see cain_sip_object_t)
**/
CAIN_SIP_DECLARE_TYPES_BEGIN(cain_sip,1)
	CAIN_SIP_TYPE_ID(cain_sip_stack_t),
	CAIN_SIP_TYPE_ID(cain_sip_listening_point_t),
	CAIN_SIP_TYPE_ID(cain_sip_datagram_listening_point_t),
	CAIN_SIP_TYPE_ID(cain_sip_udp_listening_point_t),
	CAIN_SIP_TYPE_ID(cain_sip_stream_listening_point_t),
	CAIN_SIP_TYPE_ID(cain_sip_tls_listening_point_t),
	CAIN_SIP_TYPE_ID(cain_sip_channel_t),
	CAIN_SIP_TYPE_ID(cain_sip_udp_channel_t),
	CAIN_SIP_TYPE_ID(cain_sip_stream_channel_t),
	CAIN_SIP_TYPE_ID(cain_sip_tls_channel_t),
	CAIN_SIP_TYPE_ID(cain_sip_provider_t),
	CAIN_SIP_TYPE_ID(cain_sip_main_loop_t),
	CAIN_SIP_TYPE_ID(cain_sip_source_t),
	CAIN_SIP_TYPE_ID(cain_sip_resolver_context_t),
	CAIN_SIP_TYPE_ID(cain_sip_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_server_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_client_transaction_t),
	CAIN_SIP_TYPE_ID(cain_sip_ict_t),
	CAIN_SIP_TYPE_ID(cain_sip_nict_t),
	CAIN_SIP_TYPE_ID(cain_sip_ist_t),
	CAIN_SIP_TYPE_ID(cain_sip_nist_t),
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
	CAIN_SIP_TYPE_ID(cain_sip_header_route_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_record_route_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_user_agent_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_content_length_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_extension_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_authorization_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_proxy_authorization_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_www_authenticate_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_proxy_authenticate_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_max_forwards_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_expires_t),
	CAIN_SIP_TYPE_ID(cain_sip_header_allow_t),
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
	CAIN_SIP_TYPE_ID(cain_sdp_base_description_t),
	CAIN_SIP_TYPE_ID(cain_sdp_mime_parameter_t),
	CAIN_SIP_TYPE_ID(cain_sip_callbacks_t),
CAIN_SIP_DECLARE_TYPES_END


enum cain_sip_interface_ids{
	cain_sip_interface_id_first=1,
	CAIN_SIP_INTERFACE_ID(cain_sip_channel_listener_t),
	CAIN_SIP_INTERFACE_ID(cain_sip_listener_t)
};

CAIN_SIP_BEGIN_DECLS

void *cain_sip_malloc(size_t size);
void *cain_sip_malloc0(size_t size);
void *cain_sip_realloc(void *ptr, size_t size);
void cain_sip_free(void *ptr);
char * cain_sip_strdup(const char *s);

CAIN_SIP_END_DECLS

/*these types are declared here because they are widely used in many headers included after*/

typedef struct cain_sip_listening_point cain_sip_listening_point_t;
typedef struct cain_sip_stack cain_sip_stack_t;
typedef struct cain_sip_provider cain_sip_provider_t;
typedef struct cain_sip_dialog cain_sip_dialog_t;
typedef struct cain_sip_transaction cain_sip_transaction_t;
typedef struct cain_sip_server_transaction cain_sip_server_transaction_t;
typedef struct cain_sip_client_transaction cain_sip_client_transaction_t;
typedef struct _cain_sip_message cain_sip_message_t;
typedef struct _cain_sip_request cain_sip_request_t;
typedef struct _cain_sip_response cain_sip_response_t;

#include "cain-sip/utils.h"
#include "cain-sip/list.h"
#include "cain-sip/auth-info.h"
#include "cain-sip/listener.h"
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
#include "cain-sip/auth-helper.h"


#undef TRUE
#define TRUE 1


#undef FALSE
#define FALSE 0


#define CAIN_SIP_POINTER_TO_INT(p)	((int)(long)(p))
#define CAIN_SIP_INT_TO_POINTER(i)	((void*)(long)(i))

#endif

