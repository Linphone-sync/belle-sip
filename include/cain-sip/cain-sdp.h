/*
	cain-sdp - SIP (RFC4566) library.
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

#ifndef CAIN_SDP_H_
#define CAIN_SDP_H_
#include "cain-sip.h"
#define CAIN_SDP_CAST(obj,t) CAIN_SIP_CAST(obj,t)

/***************************************************************************************
 * Attribute
 *
 **************************************************************************************/
typedef struct _cain_sdp_attribute cain_sdp_attribute_t;
cain_sdp_attribute_t* cain_sdp_attribute_new();
cain_sdp_attribute_t* cain_sdp_attribute_parse (const char* attribute);
cain_sdp_attribute_t* cain_sdp_attribute_create (const char* name,const char* value);
const char* cain_sdp_attribute_get_name(const cain_sdp_attribute_t* attribute);
const char* cain_sdp_attribute_get_value(const cain_sdp_attribute_t* attribute);
unsigned int cain_sdp_attribute_as_value(const cain_sdp_attribute_t* attribute);
void cain_sdp_attribute_set_name(cain_sdp_attribute_t* attribute, const char* name);
void cain_sdp_attribute_set_value(cain_sdp_attribute_t* attribute, const char* value);
#define CAIN_SDP_ATTRIBUTE(t) CAIN_SDP_CAST(t,cain_sdp_attribute_t);
/***************************************************************************************
 * Bandwidth
 *
 **************************************************************************************/
typedef struct _cain_sdp_bandwidth cain_sdp_bandwidth_t;
cain_sdp_bandwidth_t* cain_sdp_bandwidth_new();
cain_sdp_bandwidth_t* cain_sdp_bandwidth_parse (const char* bandwidth);
int cain_sdp_bandwidth_get_value(const cain_sdp_bandwidth_t* attribute);
const char* cain_sdp_bandwidth_get_type(const cain_sdp_bandwidth_t* attribute);
void cain_sdp_bandwidth_set_value(cain_sdp_bandwidth_t* attribute, int value);
void cain_sdp_bandwidth_set_type(cain_sdp_bandwidth_t* attribute, const char* type);
#define CAIN_SDP_BANDWIDTH(t) CAIN_SDP_CAST(t,cain_sdp_bandwidth_t);
/***************************************************************************************
 * Connection
 *
 **************************************************************************************/
typedef struct _cain_sdp_connection cain_sdp_connection_t;
cain_sdp_connection_t* cain_sdp_connection_new();
cain_sdp_connection_t* cain_sdp_connection_create(const char* net_type, const char* addr_type, const char* addr);
cain_sdp_connection_t* cain_sdp_connection_parse (const char* connection);
const char* cain_sdp_connection_get_address(const cain_sdp_connection_t* connection);
const char* cain_sdp_connection_get_address_type(const cain_sdp_connection_t* connection);
const char* cain_sdp_connection_get_network_type(const cain_sdp_connection_t* connection);
void cain_sdp_connection_set_address(cain_sdp_connection_t* connection, const char* addr);
void cain_sdp_connection_set_address_type(cain_sdp_connection_t* connection, const char* type);
void cain_sdp_connection_set_network_type(cain_sdp_connection_t* connection, const char* type);
#define CAIN_SDP_CONNECTION(t) CAIN_SDP_CAST(t,cain_sdp_connection_t);
/***************************************************************************************
 * Email
 *
 **************************************************************************************/
typedef struct _cain_sdp_email cain_sdp_email_t;
cain_sdp_email_t* cain_sdp_email_new();
cain_sdp_email_t* cain_sdp_email_parse (const char* email);
const char* cain_sdp_email_get_value(const cain_sdp_email_t* email);
void cain_sdp_email_set_value(cain_sdp_email_t* email, const char* value);
#define CAIN_SDP_EMAIL(t) CAIN_SDP_CAST(t,cain_sdp_email_t);
/***************************************************************************************
 * Info
 *
 **************************************************************************************/
typedef struct _cain_sdp_info cain_sdp_info_t;
cain_sdp_info_t* cain_sdp_info_new();
cain_sdp_info_t* cain_sdp_info_parse (const char* info);
const char* cain_sdp_info_get_value(const cain_sdp_info_t* info);
void cain_sdp_info_set_value(cain_sdp_info_t* info, const char* value);
#define CAIN_SDP_INFO(t) CAIN_SDP_CAST(t,cain_sdp_info_t);
/***************************************************************************************
 * Key
 *
 **************************************************************************************/
//typedef struct _cain_sdp_key cain_sdp_key_t;
//cain_sdp_key_t* cain_sdp_key_new();
//cain_sdp_key_t* cain_sdp_key_parse (const char* key);
//const char* cain_sdp_key_get_key(const cain_sdp_key_t* key);
//const char* cain_sdp_key_get_method(const cain_sdp_key_t* key);
//unsigned int cain_sdp_key_as_key(const cain_sdp_key_t* key);
//void cain_sdp_key_set_key(cain_sdp_key_t* key, const char* keyvalue);
//void cain_sdp_key_set_method(cain_sdp_key_t* key, const char* method);
//#define CAIN_SDP_KEY(t) CAIN_SDP_CAST(t,cain_sdp_key_t);
/***************************************************************************************
 * Media
 *
 **************************************************************************************/
typedef struct _cain_sdp_media cain_sdp_media_t;
cain_sdp_media_t* cain_sdp_media_new();
cain_sdp_media_t* cain_sdp_media_parse (const char* media);
cain_sdp_media_t* cain_sdp_media_create(const char* media_type
                         ,int media_port
                         ,int port_count
                         ,const char* protocol
                         ,cain_sip_list_t* static_media_formats);
cain_sip_list_t*	cain_sdp_media_get_media_formats(const cain_sdp_media_t* media);
int	cain_sdp_media_get_media_port(const cain_sdp_media_t* media);
const char* cain_sdp_media_get_media_type(const cain_sdp_media_t* media);
int	cain_sdp_media_get_port_count(const cain_sdp_media_t* media);
const char* cain_sdp_media_get_protocol(const cain_sdp_media_t* media);
void cain_sdp_media_set_media_formats(cain_sdp_media_t* media, cain_sip_list_t* mediaFormats);
void cain_sdp_media_set_media_port(cain_sdp_media_t* media, int port);
void cain_sdp_media_set_media_type(cain_sdp_media_t* media, const char* mediaType);
void cain_sdp_media_set_port_count(cain_sdp_media_t* media, int port_count);
void cain_sdp_media_set_protocol(cain_sdp_media_t* media, const char* protocole);
#define CAIN_SDP_MEDIA(t) CAIN_SDP_CAST(t,cain_sdp_media_t);

/***************************************************************************************
 * mime_parameter
 *
 **************************************************************************************/
typedef struct _cain_sdp_mime_parameter cain_sdp_mime_parameter_t;
cain_sdp_mime_parameter_t* cain_sdp_mime_parameter_new();
cain_sdp_mime_parameter_t* cain_sdp_mime_parameter_create(const char* type, int media_format, int rate,int channel_count);
int cain_sdp_mime_parameter_get_rate(const cain_sdp_mime_parameter_t* mime_parameter);
void cain_sdp_mime_parameter_set_rate(cain_sdp_mime_parameter_t* mime_parameter,int rate);
int cain_sdp_mime_parameter_get_channel_count(const cain_sdp_mime_parameter_t* mime_parameter);
void cain_sdp_mime_parameter_set_channel_count(cain_sdp_mime_parameter_t* mime_parameter,int count);
int cain_sdp_mime_parameter_get_ptime(const cain_sdp_mime_parameter_t* mime_parameter);
void cain_sdp_mime_parameter_set_ptime(cain_sdp_mime_parameter_t* mime_parameter,int ptime);
int cain_sdp_mime_parameter_get_max_ptime(const cain_sdp_mime_parameter_t* mime_parameter);
void cain_sdp_mime_parameter_set_max_ptime(cain_sdp_mime_parameter_t* mime_parameter,int max_ptime);
const char* cain_sdp_mime_parameter_get_type(const cain_sdp_mime_parameter_t* mime_parameter);
void cain_sdp_mime_parameter_set_type(cain_sdp_mime_parameter_t* mime_parameter,const char* type);
int cain_sdp_mime_parameter_get_media_format(const cain_sdp_mime_parameter_t* mime_parameter);
void cain_sdp_mime_parameter_set_media_format(cain_sdp_mime_parameter_t* mime_parameter,int format);
const char* cain_sdp_mime_parameter_get_parameters(const cain_sdp_mime_parameter_t* mime_parameter);
void cain_sdp_mime_parameter_set_parameters(cain_sdp_mime_parameter_t* mime_parameter,const char* parameters);
#define CAIN_SDP_MIME_PARAMETER(t) CAIN_SDP_CAST(t,cain_sdp_mime_parameter_t);

/***************************************************************************************
 * Media Description
 *
 **************************************************************************************/
typedef struct _cain_sdp_media_description cain_sdp_media_description_t;
cain_sdp_media_description_t* cain_sdp_media_description_new();
cain_sdp_media_description_t* cain_sdp_media_description_parse (const char* media_description);
cain_sdp_media_description_t* cain_sdp_media_description_create(const char* media_type
                         	 	 	 	 	 	 	 	 	 	 ,int media_port
                         	 	 	 	 	 	 	 	 	 	 ,int port_count
                         	 	 	 	 	 	 	 	 	 	 ,const char* protocol
                         	 	 	 	 	 	 	 	 	 	 ,cain_sip_list_t* static_media_formats);
void cain_sdp_media_description_add_dynamic_payloads(cain_sdp_media_description_t* media_description, cain_sip_list_t* payloadNames, cain_sip_list_t* payloadValues);
const char*	cain_sdp_media_description_get_attribute(const cain_sdp_media_description_t* media_description, const char* name);
cain_sip_list_t* cain_sdp_media_description_get_attributes(const cain_sdp_media_description_t* media_description);
int	cain_sdp_media_description_get_bandwidth(const cain_sdp_media_description_t* media_description, const char* name);
cain_sip_list_t* cain_sdp_media_description_get_bandwidths(const cain_sdp_media_description_t* media_description);
cain_sdp_connection_t*	cain_sdp_media_description_get_connection(const cain_sdp_media_description_t* media_description);
cain_sdp_info_t* cain_sdp_media_description_get_info(const cain_sdp_media_description_t* media_description);
/*cain_sdp_key_t*  cain_sdp_media_description_get_key(const cain_sdp_media_description_t* media_description);*/
cain_sdp_media_t* cain_sdp_media_description_get_media(const cain_sdp_media_description_t* media_description);
cain_sip_list_t* cain_sdp_media_description_build_mime_parameters(const cain_sdp_media_description_t* media_description);
/*cain_sip_list_t* cain_sdp_media_description_get_mime_types(const cain_sdp_media_description_t* media_description);*/
void cain_sdp_media_description_remove_attribute(cain_sdp_media_description_t* media_description,const char* attribute);
void cain_sdp_media_description_remove_bandwidth(cain_sdp_media_description_t* media_description,const char* bandwidth);
void cain_sdp_media_description_set_attribute(cain_sdp_media_description_t* media_description, const char* name, const char* value);
void cain_sdp_media_description_add_attribute(cain_sdp_media_description_t* media_description, const cain_sdp_attribute_t* attr);
void cain_sdp_media_description_set_attributes(cain_sdp_media_description_t* media_description, cain_sip_list_t* Attributes);
void cain_sdp_media_description_set_bandwidth(cain_sdp_media_description_t* media_description, const char* name, int value);
void cain_sdp_media_description_add_bandwidth(cain_sdp_media_description_t* media_description, const cain_sdp_bandwidth_t* bandwidth);
void cain_sdp_media_description_set_bandwidths(cain_sdp_media_description_t* media_description, cain_sip_list_t* bandwidths);
void cain_sdp_media_description_set_connection(cain_sdp_media_description_t* media_description, cain_sdp_connection_t* conn);
void cain_sdp_media_description_set_info(cain_sdp_media_description_t* media_description,cain_sdp_info_t* i);
/*void cain_sdp_media_description_set_key(cain_sdp_media_description_t* media_description,cain_sdp_key_t* key);*/
void cain_sdp_media_description_set_media(cain_sdp_media_description_t* media_description, cain_sdp_media_t* media);
void cain_sdp_media_description_append_values_from_mime_parameter(cain_sdp_media_description_t* media_description, cain_sdp_mime_parameter_t* mime_parameter);
#define CAIN_SDP_MEDIA_DESCRIPTION(t) CAIN_SDP_CAST(t,cain_sdp_media_description_t);

/***************************************************************************************
 * Origin
 *
 **************************************************************************************/
typedef struct _cain_sdp_origin cain_sdp_origin_t;
cain_sdp_origin_t* cain_sdp_origin_new();
cain_sdp_origin_t* cain_sdp_origin_parse (const char* origin);
cain_sdp_origin_t* cain_sdp_origin_create(const char* user_name
											, unsigned int session_id
											, unsigned int session_version
											, const char* network_type
											, const char* addr_type
											, const char* address);
const char* cain_sdp_origin_get_address(const cain_sdp_origin_t* origin);
const char* cain_sdp_origin_get_address_type(const cain_sdp_origin_t* origin);
const char* cain_sdp_origin_get_network_type(const cain_sdp_origin_t* origin);
unsigned int cain_sdp_origin_get_session_id(const cain_sdp_origin_t* origin);
unsigned int cain_sdp_origin_get_session_version(const cain_sdp_origin_t* origin);
const char* cain_sdp_origin_get_username(const cain_sdp_origin_t* origin);
void cain_sdp_origin_set_address(cain_sdp_origin_t* origin, const char* address);
void cain_sdp_origin_set_address_type(cain_sdp_origin_t* origin, const char* address);
void cain_sdp_origin_set_network_type(cain_sdp_origin_t* origin, const char* network_type);
void cain_sdp_origin_set_session_id(cain_sdp_origin_t* origin, unsigned int session_id);
void cain_sdp_origin_set_session_version(cain_sdp_origin_t* origin, unsigned int version);
void cain_sdp_origin_set_username(cain_sdp_origin_t* origin, const char* username);
#define CAIN_SDP_ORIGIN(t) CAIN_SDP_CAST(t,cain_sdp_origin_t);
/***************************************************************************************
 * Phone
 *
 **************************************************************************************/
typedef struct _cain_sdp_phone cain_sdp_phone_t;
cain_sdp_phone_t* cain_sdp_phone_new();
cain_sdp_phone_t* cain_sdp_phone_parse (const char* phone);
const char* cain_sdp_phone_get_value(const cain_sdp_phone_t* phone);
void cain_sdp_phone_set_value(cain_sdp_phone_t* phone, const char* value);
#define CAIN_SDP_PHONE(t) CAIN_SDP_CAST(t,cain_sdp_phone_t);
/***************************************************************************************
 * Repeat time
 *
 **************************************************************************************/
typedef struct _cain_sdp_repeate_time cain_sdp_repeate_time_t;
cain_sdp_repeate_time_t* cain_sdp_repeate_time_new();
cain_sdp_repeate_time_t* cain_sdp_repeate_time_parse (const char* repeate_time);
const char* cain_sdp_repeate_time_get_value(const cain_sdp_repeate_time_t* repeate_time);
void cain_sdp_repeate_time_set_value(cain_sdp_repeate_time_t* repeate_time, const char* value);
#define CAIN_SDP_REPEATE_TIME(t) CAIN_SDP_CAST(t,cain_sdp_repeate_time_t);
/***************************************************************************************
 * Session Name
 *
 **************************************************************************************/
typedef struct _cain_sdp_session_name cain_sdp_session_name_t;
cain_sdp_session_name_t* cain_sdp_session_name_new();
cain_sdp_session_name_t* cain_sdp_session_name_create (const char* name);
const char* cain_sdp_session_name_get_value(const cain_sdp_session_name_t* session_name);
void cain_sdp_session_name_set_value(cain_sdp_session_name_t* session_name, const char* value);
#define CAIN_SDP_SESSION_NAME(t) CAIN_SDP_CAST(t,cain_sdp_session_name_t);
/***************************************************************************************
 * Time
 *
 **************************************************************************************/
typedef struct _cain_sdp_time cain_sdp_time_t;
cain_sdp_time_t* cain_sdp_time_new();
cain_sdp_time_t* cain_sdp_time_parse (const char* time);

int cain_sdp_time_get_start(const cain_sdp_time_t* time);
int cain_sdp_time_get_stop(const cain_sdp_time_t* time);
void cain_sdp_time_set_start(cain_sdp_time_t* time, int value);
void cain_sdp_time_set_stop(cain_sdp_time_t* time, int value);
#define CAIN_SDP_TIME(t) CAIN_SDP_CAST(t,cain_sdp_time_t);
/***************************************************************************************
 * Time description
 *
 **************************************************************************************/
typedef struct _cain_sdp_time_description cain_sdp_time_description_t;
cain_sdp_time_description_t* cain_sdp_time_description_new();
cain_sdp_time_description_t* cain_sdp_time_description_parse (const char* time_description);
cain_sdp_time_description_t* cain_sdp_time_description_create (int start,int stop);

cain_sip_list_t* cain_sdp_time_description_get_repeate_times(const cain_sdp_time_description_t* time_description);
cain_sdp_time_t* cain_sdp_time_description_get_time(const cain_sdp_time_description_t* time_description);
void cain_sdp_time_description_set_repeate_times(cain_sdp_time_description_t* time_description, cain_sip_list_t* times);
void cain_sdp_time_description_set_time(cain_sdp_time_description_t* time_description, cain_sdp_time_t* times);
#define CAIN_SDP_TIME_DESCRIPTION(t) CAIN_SDP_CAST(t,cain_sdp_time_description_t);
/***************************************************************************************
 * URI
 *
 **************************************************************************************/
typedef struct _cain_sdp_uri cain_sdp_uri_t;
cain_sdp_uri_t* cain_sdp_uri_new();
cain_sdp_uri_t* cain_sdp_uri_parse (const char* uri);
const char* cain_sdp_uri_get_value(const cain_sdp_uri_t* uri);
void cain_sdp_uri_set_value(cain_sdp_uri_t* uri, const char* value);
#define CAIN_SDP_URI(t) CAIN_SDP_CAST(t,cain_sdp_uri_t);
/***************************************************************************************
 * Version
 *
 **************************************************************************************/
typedef struct _cain_sdp_version cain_sdp_version_t;
cain_sdp_version_t* cain_sdp_version_new();
cain_sdp_version_t* cain_sdp_version_create(int version);
int cain_sdp_version_get_version(const cain_sdp_version_t* version);
void cain_sdp_version_set_version(cain_sdp_version_t* version, int value);
#define CAIN_SDP_VERSION(t) CAIN_SDP_CAST(t,cain_sdp_version_t);

/***************************************************************************************
 * Session Description
 *
 **************************************************************************************/
typedef struct _cain_sdp_session_description cain_sdp_session_description_t;
cain_sdp_session_description_t* cain_sdp_session_description_new();
cain_sdp_session_description_t* cain_sdp_session_description_parse (const char* session_description);
const char*	cain_sdp_session_description_get_attribute(const cain_sdp_session_description_t* session_description, const char* name);
int	cain_sdp_session_description_get_bandwidth(const cain_sdp_session_description_t* session_description, const char* name);
cain_sip_list_t*	cain_sdp_session_description_get_bandwidths(const cain_sdp_session_description_t* session_description);
cain_sdp_connection_t*	cain_sdp_session_description_get_connection(const cain_sdp_session_description_t* session_description);
cain_sip_list_t* cain_sdp_session_description_get_emails(const cain_sdp_session_description_t* session_description);
cain_sdp_info_t* cain_sdp_session_description_get_info(const cain_sdp_session_description_t* session_description);
/*cain_sdp_key_t*	cain_sdp_session_description_get_key(const cain_sdp_session_description_t* session_description);*/
cain_sip_list_t* cain_sdp_session_description_get_media_descriptions(const cain_sdp_session_description_t* session_description);
cain_sdp_origin_t*	cain_sdp_session_description_get_origin(const cain_sdp_session_description_t* session_description);
cain_sip_list_t* cain_sdp_session_description_get_phones(const cain_sdp_session_description_t* session_description);
cain_sdp_session_name_t* cain_sdp_session_description_get_session_name(const cain_sdp_session_description_t* session_description);
cain_sip_list_t* cain_sdp_session_description_get_time_descriptions(const cain_sdp_session_description_t* session_description);
cain_sdp_uri_t* cain_sdp_session_description_get_uri(const cain_sdp_session_description_t* session_description);
cain_sdp_version_t*	cain_sdp_session_description_get_version(const cain_sdp_session_description_t* session_description);
cain_sdp_uri_t* cain_sdp_session_description_get_zone_adjustments(const cain_sdp_session_description_t* session_description);
void cain_sdp_session_description_remove_attribute(cain_sdp_session_description_t* session_description, const char* name);
void cain_sdp_session_description_remove_bandwidth(cain_sdp_session_description_t* session_description, const char* name);
void cain_sdp_session_description_set_attribute(cain_sdp_session_description_t* session_description, const char* name, const char* value);
void cain_sdp_session_description_add_attribute(cain_sdp_session_description_t* session_description, const cain_sdp_attribute_t* attribute);
void cain_sdp_session_description_set_attributes(cain_sdp_session_description_t* session_description, cain_sip_list_t* Attributes);
void cain_sdp_session_description_set_bandwidth(cain_sdp_session_description_t* session_description, const char* name, int value);
void cain_sdp_session_description_set_bandwidths(cain_sdp_session_description_t* session_description, cain_sip_list_t* bandwidths);
void cain_sdp_session_description_add_bandwidth(cain_sdp_session_description_t* session_description, const cain_sdp_bandwidth_t* bandwidth);
void cain_sdp_session_description_set_connection(cain_sdp_session_description_t* session_description, cain_sdp_connection_t* conn);
void cain_sdp_session_description_set_emails(cain_sdp_session_description_t* session_description, cain_sip_list_t* emails);
void cain_sdp_session_description_set_info(cain_sdp_session_description_t* session_description, cain_sdp_info_t* i);
/*void cain_sdp_session_description_set_key(cain_sdp_session_description_t* session_description, cain_sdp_key_t* key);*/
void cain_sdp_session_description_set_media_descriptions(cain_sdp_session_description_t* session_description, cain_sip_list_t* mediaDescriptions);
void cain_sdp_session_description_add_media_description(cain_sdp_session_description_t* session_description, cain_sdp_media_description_t* media_description);
void cain_sdp_session_description_set_origin(cain_sdp_session_description_t* session_description, cain_sdp_origin_t* origin);
void cain_sdp_session_description_set_phones(cain_sdp_session_description_t* session_description, cain_sip_list_t* phones);
void cain_sdp_session_description_set_session_name(cain_sdp_session_description_t* session_description, cain_sdp_session_name_t* sessionName);
void cain_sdp_session_description_set_time_descriptions(cain_sdp_session_description_t* session_description, cain_sip_list_t* times);
void cain_sdp_session_description_set_time_description(cain_sdp_session_description_t* session_description, cain_sdp_time_description_t* time_desc);
void cain_sdp_session_description_set_uri(cain_sdp_session_description_t* session_description, cain_sdp_uri_t* uri);
void cain_sdp_session_description_set_version(cain_sdp_session_description_t* session_description, cain_sdp_version_t* v);
void cain_sdp_session_description_set_zone_adjustments(cain_sdp_session_description_t* session_description, cain_sdp_uri_t* zoneAdjustments);
#define CAIN_SDP_SESSION_DESCRIPTION(t) CAIN_SDP_CAST(t,cain_sdp_session_description_t);
#endif /* CAIN_SDP_H_ */
