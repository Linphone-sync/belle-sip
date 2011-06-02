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
#include "cain-sip/cain-sdp.h"
#include "cain_sip_internal.h"
#include "cain_sdpParser.h"
#include "cain_sdpLexer.h"

/***************************************************************************************
 * Attribute
 *
 **************************************************************************************/
struct _cain_sdp_attribute {
	cain_sip_object_t base;
	const char* name;
	const char* value;
};
void cain_sdp_attribute_destroy(cain_sdp_attribute_t* attribute) {
}

void cain_sdp_attribute_clone(cain_sdp_attribute_t *attribute, const cain_sdp_attribute_t *orig){
}
int cain_sdp_attribute_marshal(cain_sdp_attribute_t* attribute, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"a=%s"
								,attribute->name);
	if (attribute->value) {
		current_offset+=snprintf(	buff+current_offset
									,buff_size-current_offset
									,":%s"
									,attribute->value);
	}
	return current_offset-offset;
}
CAIN_SDP_NEW(attribute,cain_sip_object)
CAIN_SDP_PARSE(attribute)
GET_SET_STRING(cain_sdp_attribute,name);
GET_SET_STRING(cain_sdp_attribute,value);
unsigned int cain_sdp_attribute_as_value(const cain_sdp_attribute_t* attribute) {
	return attribute->value!=NULL;
}
/***************************************************************************************
 * Bandwidth
 *
 **************************************************************************************/
struct _cain_sdp_bandwidth {
	cain_sip_object_t base;
	const char* type;
	int value;
};
void cain_sdp_bandwidth_destroy(cain_sdp_bandwidth_t* bandwidth) {
}

void cain_sdp_bandwidth_clone(cain_sdp_bandwidth_t *bandwidth, const cain_sdp_bandwidth_t *orig){
}
int cain_sdp_bandwidth_marshal(cain_sdp_bandwidth_t* bandwidth, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"b=%s:%i"
								,bandwidth->type,bandwidth->value);

	return current_offset-offset;
}
CAIN_SDP_NEW(bandwidth,cain_sip_object)
CAIN_SDP_PARSE(bandwidth)
GET_SET_STRING(cain_sdp_bandwidth,type);
GET_SET_INT(cain_sdp_bandwidth,value,int)

/************************
 * connection
 ***********************/
struct _cain_sdp_connection {
	cain_sip_object_t base;
	const char* network_type;
	const char* address_type;
	const char* address;
 };

void cain_sdp_connection_destroy(cain_sdp_connection_t* connection) {
}

void cain_sdp_connection_clone(cain_sdp_connection_t *connection, const cain_sdp_connection_t *orig){
}
int cain_sdp_connection_marshal(cain_sdp_connection_t* connection, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"c=%s %s %s"
								,connection->network_type
								,connection->address_type
								,connection->address);
	return current_offset-offset;
}
CAIN_SDP_NEW(connection,cain_sip_object)
CAIN_SDP_PARSE(connection)
GET_SET_STRING(cain_sdp_connection,network_type);
GET_SET_STRING(cain_sdp_connection,address_type);
GET_SET_STRING(cain_sdp_connection,address);
/************************
 * email
 ***********************/
struct _cain_sdp_email {
	cain_sip_object_t base;
	const char* value;
 };

void cain_sdp_email_destroy(cain_sdp_email_t* email) {
}

void cain_sdp_email_clone(cain_sdp_email_t *email, const cain_sdp_email_t *orig){
}
int cain_sdp_email_marshal(cain_sdp_email_t* email, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"e=%s"
								,email->value);
	return current_offset-offset;
}
CAIN_SDP_NEW(email,cain_sip_object)
CAIN_SDP_PARSE(email)
GET_SET_STRING(cain_sdp_email,value);
/************************
 * info
 ***********************/
struct _cain_sdp_info {
	cain_sip_object_t base;
	const char* value;
 };

void cain_sdp_info_destroy(cain_sdp_info_t* info) {
}

void cain_sdp_info_clone(cain_sdp_info_t *info, const cain_sdp_info_t *orig){
}
int cain_sdp_info_marshal(cain_sdp_info_t* info, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"i=%s"
								,info->value);
	return current_offset-offset;
}
CAIN_SDP_NEW(info,cain_sip_object)
CAIN_SDP_PARSE(info)
GET_SET_STRING(cain_sdp_info,value);
/************************
 * media
 ***********************/
struct _cain_sdp_media {
	cain_sip_object_t base;
	const char* value;
	int media_port;
	cain_sip_list_t* media_formats;
	const char* media_type;
	int port_count;
	const char* media_protocol;
 };

void cain_sdp_media_destroy(cain_sdp_media_t* media) {
}

void cain_sdp_media_clone(cain_sdp_media_t *media, const cain_sdp_media_t *orig){
}
int cain_sdp_media_marshal(cain_sdp_media_t* media, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"m=%s"
								,media->value);
	return current_offset-offset;
}
CAIN_SDP_NEW(media,cain_sip_object)
CAIN_SDP_PARSE(media)
GET_SET_STRING(cain_sdp_media,value);
GET_SET_STRING(cain_sdp_media,media_type);
GET_SET_STRING(cain_sdp_media,media_protocol);
GET_SET_INT(cain_sdp_media,media_port,int)
GET_SET_INT(cain_sdp_media,port_count,int)
