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
#include "cain-sip/cain-sip.h"
#include "cain_sdpParser.h"
#include "cain_sdpLexer.h"
#include "cain_sip_internal.h"
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
	if (attribute->name) cain_sip_free((void*)attribute->name);
	if (attribute->value) cain_sip_free((void*)attribute->value);
}

void cain_sdp_attribute_clone(cain_sdp_attribute_t *attribute, const cain_sdp_attribute_t *orig){
	CLONE_STRING(cain_sdp_attribute,name,attribute,orig)
	CLONE_STRING(cain_sdp_attribute,value,attribute,orig)
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
cain_sdp_attribute_t* cain_sdp_attribute_create (const char* name,const char* value) {
	cain_sdp_attribute_t* attribute = cain_sdp_attribute_new();
	cain_sdp_attribute_set_name(attribute,name);
	cain_sdp_attribute_set_value(attribute,value);
	return attribute;
}
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
	if (bandwidth->type) cain_sip_free((void*)bandwidth->type);
}

void cain_sdp_bandwidth_clone(cain_sdp_bandwidth_t *bandwidth, const cain_sdp_bandwidth_t *orig){
	CLONE_STRING(cain_sdp_bandwidth,type,bandwidth,orig)
	bandwidth->value=orig->value;
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
	DESTROY_STRING(connection,network_type)
	DESTROY_STRING(connection,address_type)
	DESTROY_STRING(connection,address)
}

void cain_sdp_connection_clone(cain_sdp_connection_t *connection, const cain_sdp_connection_t *orig){
	CLONE_STRING(cain_sdp_connection,network_type,connection,orig)
	CLONE_STRING(cain_sdp_connection,address_type,connection,orig)
	CLONE_STRING(cain_sdp_connection,address,connection,orig)

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
cain_sdp_connection_t* cain_sdp_connection_create(const char* net_type, const char* addr_type, const char* addr) {
	cain_sdp_connection_t* connection = cain_sdp_connection_new();
	cain_sdp_connection_set_network_type(connection,net_type);
	cain_sdp_connection_set_address_type(connection,addr_type);
	cain_sdp_connection_set_address(connection,addr);
	return connection;
}
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
	DESTROY_STRING(email,value)
}

void cain_sdp_email_clone(cain_sdp_email_t *email, const cain_sdp_email_t *orig){
	CLONE_STRING(cain_sdp_email,value,email,orig)
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
	DESTROY_STRING(info,value)
}

void cain_sdp_info_clone(cain_sdp_info_t *info, const cain_sdp_info_t *orig){
	CLONE_STRING(cain_sdp_info,value,info,orig)
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
	const char* media_type;
	int media_port;
	cain_sip_list_t* media_formats;
	int port_count;
	const char* protocol;
 };
cain_sip_list_t*	cain_sdp_media_get_media_formats(const cain_sdp_media_t* media) {
	return media->media_formats;
}
void cain_sdp_media_set_media_formats( cain_sdp_media_t* media, cain_sip_list_t* formats) {
	/*cain_sip_list_free(media->media_formats); to allow easy list management might be better to add an append format method*/
	media->media_formats = formats;
}
void cain_sdp_media_destroy(cain_sdp_media_t* media) {
	DESTROY_STRING(media,media_type)
	cain_sip_list_free(media->media_formats);
	DESTROY_STRING(media,protocol)
}
static void cain_sdp_media_init(cain_sdp_media_t* media) {
	media->port_count=1;
}

void cain_sdp_media_clone(cain_sdp_media_t *media, const cain_sdp_media_t *orig){
	CLONE_STRING(cain_sdp_media,media_type,media,orig)
	media->media_port=orig->media_port;
	media->media_formats = cain_sip_list_copy(orig->media_formats);
	media->port_count=orig->port_count;
	CLONE_STRING(cain_sdp_media,protocol,media,orig)
}
int cain_sdp_media_marshal(cain_sdp_media_t* media, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	cain_sip_list_t* list=media->media_formats;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"m=%s %i"
								,media->media_type
								,media->media_port
								);
	if (media->port_count>1) {
		current_offset+=snprintf(buff+current_offset
								,buff_size-current_offset
								,"/%i"
								,media->port_count);
	}
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								," %s"
								,media->protocol);
	for(;list!=NULL;list=list->next){
		current_offset+=snprintf(	buff+current_offset
									,buff_size-current_offset
									," %li"
									,(long)list->data);
	}
	return current_offset-offset;
}
CAIN_SDP_NEW_WITH_CTR(media,cain_sip_object)
CAIN_SDP_PARSE(media)
cain_sdp_media_t* cain_sdp_media_create(const char* media_type
                         ,int media_port
                         ,int port_count
                         ,const char* protocol
                         ,cain_sip_list_t* static_media_formats) {
	cain_sdp_media_t* media= cain_sdp_media_new();
	cain_sdp_media_set_media_type(media,media_type);
	cain_sdp_media_set_media_port(media,media_port);
	cain_sdp_media_set_port_count(media,port_count);
	cain_sdp_media_set_protocol(media,protocol);
	if (static_media_formats) cain_sdp_media_set_media_formats(media,static_media_formats);
	return media;
}
GET_SET_STRING(cain_sdp_media,media_type);
GET_SET_STRING(cain_sdp_media,protocol);
GET_SET_INT(cain_sdp_media,media_port,int)
GET_SET_INT(cain_sdp_media,port_count,int)

/************************
 * base_description
 ***********************/
static void cain_sip_object_freefunc(void* obj) {
	cain_sip_object_unref(CAIN_SIP_OBJECT(obj));
}
static void* cain_sip_object_copyfunc(void* obj) {
	return cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(obj));
}

typedef struct _cain_sdp_base_description {
	cain_sip_object_t base;
	cain_sdp_info_t* info;
	cain_sdp_connection_t* connection;
	cain_sip_list_t* bandwidths;
	cain_sip_list_t* attributes;
} cain_sdp_base_description_t;

static void cain_sdp_base_description_destroy(cain_sdp_base_description_t* base_description) {
	if (base_description->info) cain_sip_object_unref(CAIN_SIP_OBJECT(base_description->info));
	if (base_description->connection) cain_sip_object_unref(CAIN_SIP_OBJECT(base_description->connection));
	cain_sip_list_free_with_data(base_description->bandwidths,cain_sip_object_freefunc);
	cain_sip_list_free_with_data(base_description->attributes,cain_sip_object_freefunc);
}
static void cain_sdp_base_description_init(cain_sdp_base_description_t* base_description) {
}
static void cain_sdp_base_description_clone(cain_sdp_base_description_t *base_description, const cain_sdp_base_description_t *orig){
	if (orig->info) base_description->info = CAIN_SDP_INFO(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(orig->info)));
	if (orig->connection) base_description->connection = CAIN_SDP_CONNECTION(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(orig->connection)));
	base_description->bandwidths = cain_sip_list_copy_with_data(orig->bandwidths,cain_sip_object_copyfunc);
	base_description->attributes = cain_sip_list_copy_with_data(orig->attributes,cain_sip_object_copyfunc);

}
int cain_sdp_base_description_marshal(cain_sdp_base_description_t* base_description, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	cain_sip_list_t* bandwidths;
//	cain_sip_list_t* attributes;
	if (base_description->info) {
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(base_description->info),buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	}
	if (base_description->connection) {
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(base_description->connection),buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	}
	for(bandwidths=base_description->bandwidths;bandwidths!=NULL;bandwidths=bandwidths->next){
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(bandwidths->data),buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	}
//	for(attributes=base_description->attributes;attributes!=NULL;attributes=attributes->next){
//		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(attributes->data),buff,current_offset,buff_size);
//		current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
//	}
	return current_offset-offset;
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sdp_base_description_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sdp_base_description_t
							,cain_sip_object_t
							,cain_sdp_base_description_destroy
							,cain_sdp_base_description_clone
							,cain_sdp_base_description_marshal
							,FALSE);

static int cain_sdp_base_description_attribute_comp_func(const cain_sdp_attribute_t* a, const char*b) {
	return strcmp(a->name,b);
}
const cain_sdp_attribute_t*	cain_sdp_base_description_get_attribute(const cain_sdp_base_description_t* base_description, const char* name) {
	cain_sip_list_t* attribute;
	attribute = cain_sip_list_find_custom(base_description->attributes, (cain_sip_compare_func)cain_sdp_base_description_attribute_comp_func, name);
	if (attribute) {
		return ((cain_sdp_attribute_t*)attribute->data);
	} else {
		return NULL;
	}
}
const char*	cain_sdp_base_description_get_attribute_value(const cain_sdp_base_description_t* base_description, const char* name) {
	const cain_sdp_attribute_t* attribute = cain_sdp_base_description_get_attribute(base_description,name);
	if (attribute)
		return cain_sdp_attribute_get_value(attribute);
	else
		return NULL;

}
cain_sip_list_t* cain_sdp_base_description_get_attributes(const cain_sdp_base_description_t* base_description) {
	return base_description->attributes;
}
static int cain_sdp_base_description_bandwidth_comp_func(const cain_sdp_bandwidth_t* a, const char*b) {
	return strcmp(a->type,b);
}

int	cain_sdp_base_description_get_bandwidth(const cain_sdp_base_description_t* base_description, const char* name) {
	cain_sip_list_t* bandwidth;
	bandwidth = cain_sip_list_find_custom(base_description->bandwidths, (cain_sip_compare_func)cain_sdp_base_description_bandwidth_comp_func, name);
	if (bandwidth) {
		return ((cain_sdp_bandwidth_t*)bandwidth->data)->value;
	} else {
		return -1;
	}
}
void cain_sdp_base_description_remove_attribute(cain_sdp_base_description_t* base_description,const char* name) {
	cain_sip_list_t* attribute;
	attribute = cain_sip_list_find_custom(base_description->attributes, (cain_sip_compare_func)cain_sdp_base_description_attribute_comp_func, name);
	if (attribute) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(attribute->data));
		base_description->attributes = cain_sip_list_remove_link(base_description->attributes,attribute);
	}

}
void cain_sdp_base_description_remove_bandwidth(cain_sdp_base_description_t* base_description,const char* name) {
	cain_sip_list_t* bandwidth;
	bandwidth = cain_sip_list_find_custom(base_description->bandwidths, (cain_sip_compare_func)cain_sdp_base_description_bandwidth_comp_func, name);
	if (bandwidth) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(bandwidth->data));
		base_description->bandwidths = cain_sip_list_remove_link(base_description->bandwidths,bandwidth);
	}
}
void cain_sdp_base_description_set_attribute_value(cain_sdp_base_description_t* base_description, const char* name, const char* value) {
	cain_sdp_attribute_t* attribute = cain_sdp_attribute_new();
	cain_sdp_attribute_set_name(attribute,name);
	cain_sdp_attribute_set_value(attribute,value);
	base_description->attributes = cain_sip_list_append(base_description->attributes,cain_sip_object_ref(attribute));
}
void cain_sdp_base_description_add_attribute(cain_sdp_base_description_t* base_description, const cain_sdp_attribute_t* attribute) {
	base_description->attributes = cain_sip_list_append(base_description->attributes,(void*)cain_sip_object_ref(CAIN_SIP_OBJECT(attribute)));
}

#define SET_LIST(list_name,value) \
		cain_sip_list_t* list;\
		if (list_name) {\
			for (list=list_name;list !=NULL; list=list->next) {\
				cain_sip_object_unref(CAIN_SIP_OBJECT(list->data));\
			}\
			cain_sip_list_free(list_name); \
		} \
		for (list=value;list !=NULL; list=list->next) {\
						cain_sip_object_ref(CAIN_SIP_OBJECT(list->data));\
		}\
		list_name=value;


void cain_sdp_base_description_set_attributes(cain_sdp_base_description_t* base_description, cain_sip_list_t* attributes) {
	SET_LIST(base_description->attributes,attributes)
}
void cain_sdp_base_description_set_bandwidth(cain_sdp_base_description_t* base_description, const char* type, int value) {
	cain_sdp_bandwidth_t* bandwidth = cain_sdp_bandwidth_new();
	cain_sdp_bandwidth_set_type(bandwidth,type);
	cain_sdp_bandwidth_set_value(bandwidth,value);
	base_description->bandwidths = cain_sip_list_append(base_description->bandwidths,cain_sip_object_ref(bandwidth));
}
void cain_sdp_base_description_add_bandwidth(cain_sdp_base_description_t* base_description, const cain_sdp_bandwidth_t* bandwidth) {
	base_description->bandwidths = cain_sip_list_append(base_description->bandwidths,(void *)cain_sip_object_ref((void *)bandwidth));
}
void cain_sdp_base_description_set_bandwidths(cain_sdp_base_description_t* base_description, cain_sip_list_t* bandwidths) {
	SET_LIST(base_description->bandwidths,bandwidths)
}

/************************
 * media_description
 ***********************/
struct _cain_sdp_media_description {
	cain_sdp_base_description_t base_description;
	cain_sdp_media_t* media;
};
void cain_sdp_media_description_destroy(cain_sdp_media_description_t* media_description) {
	if (media_description->media) cain_sip_object_unref(CAIN_SIP_OBJECT((media_description->media)));
}

void cain_sdp_media_description_clone(cain_sdp_media_description_t *media_description, const cain_sdp_media_description_t *orig){
	if (orig->media) media_description->media = CAIN_SDP_MEDIA(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT((orig->media))));
}
int cain_sdp_media_description_marshal(cain_sdp_media_description_t* media_description, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	cain_sip_list_t* attributes;
	current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(media_description->media),buff,current_offset,buff_size);
	current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	current_offset+=cain_sdp_base_description_marshal(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),buff,current_offset,buff_size);

	for(attributes=media_description->base_description.attributes;attributes!=NULL;attributes=attributes->next){
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(attributes->data),buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	}
	return current_offset-offset;
}
CAIN_SDP_NEW(media_description,cain_sdp_base_description)
cain_sdp_media_description_t* cain_sdp_media_description_create(const char* media_type
                         	 	 	 	 	 	 	 	 	 	 ,int media_port
                         	 	 	 	 	 	 	 	 	 	 ,int port_count
                         	 	 	 	 	 	 	 	 	 	 ,const char* protocol
                         	 	 	 	 	 	 	 	 	 	 ,cain_sip_list_t* static_media_formats) {
	cain_sdp_media_description_t* media_desc=cain_sdp_media_description_new();
	cain_sdp_media_description_set_media(media_desc,cain_sdp_media_create(media_type,media_port,port_count,protocol,static_media_formats));
	return media_desc;
}
CAIN_SDP_PARSE(media_description)
void cain_sdp_media_description_add_dynamic_payloads(cain_sdp_media_description_t* media_description, cain_sip_list_t* payloadNames, cain_sip_list_t* payloadValues) {

}
const cain_sdp_attribute_t*	cain_sdp_media_description_get_attribute(const cain_sdp_media_description_t* media_description, const char* name) {
	return cain_sdp_base_description_get_attribute(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),name);
}
const char*	cain_sdp_media_description_get_attribute_value(const cain_sdp_media_description_t* media_description, const char* name) {
	return cain_sdp_base_description_get_attribute_value(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),name);
}
cain_sip_list_t* cain_sdp_media_description_get_attributes(const cain_sdp_media_description_t* media_description) {
	return CAIN_SIP_CAST(media_description,cain_sdp_base_description_t)->attributes;
}

int	cain_sdp_media_description_get_bandwidth(const cain_sdp_media_description_t* media_description, const char* name) {
	return cain_sdp_base_description_get_bandwidth(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),name);
}
cain_sip_list_t* cain_sdp_media_description_get_bandwidths(const cain_sdp_media_description_t* media_description) {
	return CAIN_SIP_CAST(media_description,cain_sdp_base_description_t)->bandwidths;
}
cain_sdp_connection_t*	cain_sdp_media_description_get_connection(const cain_sdp_media_description_t* media_description) {
	return CAIN_SIP_CAST(media_description,cain_sdp_base_description_t)->connection;
}
cain_sdp_info_t* cain_sdp_media_description_get_info(const cain_sdp_media_description_t* media_description) {
	return CAIN_SIP_CAST(media_description,cain_sdp_base_description_t)->info;
}
/*cain_sdp_key_t*  cain_sdp_media_description_get_key(const cain_sdp_media_description_t* media_description);*/
cain_sdp_media_t* cain_sdp_media_description_get_media(const cain_sdp_media_description_t* media_description) {
	return media_description->media;
}

struct static_payload {
	unsigned char number;
	int channel_count;
	const char* type;
	int	rate;
};
#define STATIC_PAYLOAD_LIST_LENTH 8
/*
 * rfc 3551
 * PT   encoding    media type  clock rate   channels
                    name                    (Hz)
               ___________________________________________________
               0    PCMU        A            8,000       1
               1    reserved    A
               2    reserved    A
               3    GSM         A            8,000       1
               4    G723        A            8,000       1
               5    DVI4        A            8,000       1
               6    DVI4        A           16,000       1
               7    LPC         A            8,000       1
               8    PCMA        A            8,000       1
               9    G722        A            8,000       1
               10   L16         A           44,100       2
               11   L16         A           44,100       1
               12   QCELP       A            8,000       1
               13   CN          A            8,000       1
               14   MPA         A           90,000       (see text)
               15   G728        A            8,000       1
               16   DVI4        A           11,025       1
               17   DVI4        A           22,050       1
               18   G729        A            8,000       1
               Table 4: Payload types (PT) for audio encodings

  PT      encoding    media type  clock rate
                       name                    (Hz)
               _____________________________________________
               24      unassigned  V
               25      CelB        V           90,000
               26      JPEG        V           90,000
               27      unassigned  V
               28      nv          V           90,000
               29      unassigned  V
               30      unassigned  V
               31      H261        V           90,000
               32      MPV         V           90,000
               33      MP2T        AV          90,000
               34      H263        V           90,000

               Table 5: Payload types (PT) for video and combined
                        encodings


 *
 * */

struct static_payload static_payload_list [] ={
	/*audio*/
	{0,1,"PCMU",8000},
	{3,1,"GSM",8000},
	{4,1,"G723",8000},
	{5,1,"DVI4",8000},
	{6,1,"DVI4",16000},
	{7,1,"LPC",8000},
	{8,1,"PCMA",8000},
	{9,1,"G722",8000},
	{10,2,"L16",44100},
	{11,1,"L16",44100},
	{12,1,"QCELP",8000},
	{13,1,"CN",8000},
	{14,1,"MPA",90000},
	{15,1,"G728",8000},
	{16,1,"DVI4",11025},
	{17,1,"DVI4",22050},
	{18,1,"G729",8000},
	/*video*/
	{25,-1,"CelB",90000},
	{26,-1,"JPEG",90000},
	{28,-1,"nv",90000},
	{31,-1,"H261",90000},
	{32,-1,"MPV",90000},
	{33,-1,"MP2T",90000},
	{34,-1,"H263",90000}
};
static int mime_parameter_fill_from_static(cain_sdp_mime_parameter_t *mime_parameter,int format) {
	struct static_payload* iterator = static_payload_list;
	int i;
	static size_t payload_list_element=sizeof(static_payload_list)/sizeof(struct static_payload);
	for (i=0;i<payload_list_element;i++) {
		if (iterator->number == format) {
			cain_sdp_mime_parameter_set_type(mime_parameter,iterator->type);
			cain_sdp_mime_parameter_set_rate(mime_parameter,iterator->rate);
			cain_sdp_mime_parameter_set_channel_count(mime_parameter,iterator->channel_count);
		} else {
			iterator++;
		}
	}
	return 0;
}
static int mime_parameter_fill_from_rtpmap(cain_sdp_mime_parameter_t *mime_parameter, const char *rtpmap){
	char *mime=cain_sip_strdup(rtpmap);
	char *p=strchr(mime,'/');
	if (p){
		char *chans;
		*p='\0';
		p++;
		chans=strchr(p,'/');
		if (chans){
			*chans='\0';
			chans++;
			cain_sdp_mime_parameter_set_channel_count(mime_parameter,atoi(chans));
		}else cain_sdp_mime_parameter_set_channel_count(mime_parameter,1);
		cain_sdp_mime_parameter_set_rate(mime_parameter,atoi(p));
	}
	cain_sdp_mime_parameter_set_type(mime_parameter,mime);
	cain_sip_free(mime);
	return 0;
}
/* return the value of attr "field" for payload pt at line pos (field=rtpmap,fmtp...)*/
static const char *cain_sdp_media_description_a_attr_value_get_with_pt(const cain_sdp_media_description_t* media_description,int pt,const char *field)
{
	int tmppt=0,scanned=0;
	const char *tmp;
	cain_sdp_attribute_t *attr;
	cain_sip_list_t* attribute_list;
	for (	attribute_list =cain_sdp_media_description_get_attributes(media_description)
						;attribute_list!=NULL
						;attribute_list=attribute_list->next) {

		attr = CAIN_SDP_ATTRIBUTE(attribute_list->data);
		if (strcmp(field,cain_sdp_attribute_get_name(attr))==0 && cain_sdp_attribute_get_value(attr)!=NULL){
			int nb = sscanf(cain_sdp_attribute_get_value(attr),"%i %n",&tmppt,&scanned);
			/* the return value may depend on how %n is interpreted by the libc: see manpage*/
			if (nb == 1 || nb==2 ){
				if (pt==tmppt){
					tmp=cain_sdp_attribute_get_value(attr)+scanned;
					if (strlen(tmp)>0)
						return tmp;
				}
			}else cain_sip_warning("sdp has a strange a= line (%s) nb=%i",cain_sdp_attribute_get_value(attr),nb);
		}
	}
	return NULL;
}

cain_sip_list_t* cain_sdp_media_description_build_mime_parameters(const cain_sdp_media_description_t* media_description) {
	/*First, get media type*/
	cain_sdp_media_t* media = cain_sdp_media_description_get_media(media_description);
	cain_sip_list_t* mime_parameter_list=NULL;
	cain_sip_list_t* media_formats=NULL;
	cain_sdp_mime_parameter_t* mime_parameter;
	const char* rtpmap=NULL;
	const char* fmtp=NULL;
	const char* ptime=NULL;
	const char* max_ptime=NULL;
	int ptime_as_int=-1;
	int max_ptime_as_int=-1;
	if (!media) {
		cain_sip_error("cain_sdp_media_description_build_mime_parameters: no media");
		return NULL;
	}
	ptime = cain_sdp_media_description_get_attribute_value(media_description,"ptime");
	ptime?ptime_as_int=atoi(ptime):-1;
	max_ptime = cain_sdp_media_description_get_attribute_value(media_description,"maxptime");
	max_ptime?max_ptime_as_int=atoi(max_ptime):-1;

	for (media_formats = cain_sdp_media_get_media_formats(media);media_formats!=NULL;media_formats=media_formats->next) {
		/*create mime parameters with format*/
		mime_parameter = cain_sdp_mime_parameter_new();
		cain_sdp_mime_parameter_set_ptime(mime_parameter,ptime_as_int);
		cain_sdp_mime_parameter_set_max_ptime(mime_parameter,max_ptime_as_int);
		cain_sdp_mime_parameter_set_media_format(mime_parameter,(int)(long)media_formats->data);
		mime_parameter_fill_from_static(mime_parameter,cain_sdp_mime_parameter_get_media_format(mime_parameter));
		/*get rtpmap*/
		rtpmap = cain_sdp_media_description_a_attr_value_get_with_pt(media_description
																		,cain_sdp_mime_parameter_get_media_format(mime_parameter)
																		,"rtpmap");
		if (rtpmap) {
			mime_parameter_fill_from_rtpmap(mime_parameter,rtpmap);
		}
		fmtp = cain_sdp_media_description_a_attr_value_get_with_pt(media_description
																		,cain_sdp_mime_parameter_get_media_format(mime_parameter)
																		,"fmtp");
		if (fmtp) {
			cain_sdp_mime_parameter_set_parameters(mime_parameter,fmtp);
		}

		mime_parameter_list=cain_sip_list_append(mime_parameter_list,mime_parameter);
	}
	return mime_parameter_list;
}
#define MAX_FMTP_LENGH 64

void cain_sdp_media_description_append_values_from_mime_parameter(cain_sdp_media_description_t* media_description, cain_sdp_mime_parameter_t* mime_parameter) {
	cain_sdp_media_t* media = cain_sdp_media_description_get_media(media_description);
	char atribute_value [MAX_FMTP_LENGH];
	cain_sdp_media_set_media_formats(media,cain_sip_list_append(cain_sdp_media_get_media_formats(media)
																,(void*)(long)(cain_sdp_mime_parameter_get_media_format(mime_parameter))));
	if (cain_sdp_mime_parameter_get_media_format(mime_parameter) > 34) {
		/*dynamic payload*/

		if (cain_sdp_mime_parameter_get_channel_count(mime_parameter)>1) {
			snprintf(atribute_value,MAX_FMTP_LENGH,"%i %s/%i/%i"
					,cain_sdp_mime_parameter_get_media_format(mime_parameter)
					,cain_sdp_mime_parameter_get_type(mime_parameter)
					,cain_sdp_mime_parameter_get_rate(mime_parameter)
					,cain_sdp_mime_parameter_get_channel_count(mime_parameter));
		} else {
			snprintf(atribute_value,MAX_FMTP_LENGH,"%i %s/%i"
					,cain_sdp_mime_parameter_get_media_format(mime_parameter)
					,cain_sdp_mime_parameter_get_type(mime_parameter)
					,cain_sdp_mime_parameter_get_rate(mime_parameter));
		}
		cain_sdp_media_description_set_attribute_value(media_description,"rtpmap",atribute_value);
		if (cain_sdp_mime_parameter_get_parameters(mime_parameter)) {
			snprintf(atribute_value,MAX_FMTP_LENGH,"%i %s"
					,cain_sdp_mime_parameter_get_media_format(mime_parameter)
					,cain_sdp_mime_parameter_get_parameters(mime_parameter));
			cain_sdp_media_description_set_attribute_value(media_description,"fmtp",atribute_value);
		}

	}

}
cain_sip_list_t* cain_sdp_media_description_get_mime_types(const cain_sdp_media_description_t* media_description) {
	cain_sip_error("cain_sdp_media_description_get_mime_types: not implemented yet");
	return NULL;
}

void cain_sdp_media_description_remove_attribute(cain_sdp_media_description_t* media_description,const char* name) {
	cain_sdp_base_description_remove_attribute(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),name);
}
void cain_sdp_media_description_remove_bandwidth(cain_sdp_media_description_t* media_description,const char* name) {
	cain_sdp_base_description_remove_bandwidth(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),name);
}
void cain_sdp_media_description_set_attribute_value(cain_sdp_media_description_t* media_description, const char* name, const char* value) {
	cain_sdp_base_description_set_attribute_value(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),name,value);
}
void cain_sdp_media_description_set_attributes(cain_sdp_media_description_t* media_description, cain_sip_list_t* value) {
	cain_sdp_base_description_set_attributes(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),value);
}

void cain_sdp_media_description_add_attribute(cain_sdp_media_description_t* media_description, const cain_sdp_attribute_t* attribute) {
	cain_sdp_base_description_add_attribute(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),attribute);
}
void cain_sdp_media_description_set_bandwidth(cain_sdp_media_description_t* media_description, const char* type, int value) {
	cain_sdp_base_description_set_bandwidth(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),type,value);
}
void cain_sdp_media_description_add_bandwidth(cain_sdp_media_description_t* media_description, const cain_sdp_bandwidth_t* bandwidth) {
	cain_sdp_base_description_add_bandwidth(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),bandwidth);
}
void cain_sdp_media_description_set_bandwidths(cain_sdp_media_description_t* media_description, cain_sip_list_t* bandwidths) {
	cain_sdp_base_description_set_bandwidths(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),bandwidths);
}
#define SET_OBJECT(object,param,param_type) \
		param_type** current = &object->param; \
		if (param) cain_sip_object_ref(param); \
		if (*current) { \
			cain_sip_object_unref(CAIN_SIP_OBJECT(*current)); \
		} \
		*current=param; \


void cain_sdp_media_description_set_connection(cain_sdp_media_description_t* media_description, cain_sdp_connection_t* connection) {
	SET_OBJECT(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),connection,cain_sdp_connection_t)
}
void cain_sdp_media_description_set_info(cain_sdp_media_description_t* media_description,cain_sdp_info_t* info) {
	SET_OBJECT(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),info,cain_sdp_info_t)
}
/*void cain_sdp_media_description_set_key(cain_sdp_media_description_t* media_description,cain_sdp_key_t* key);*/
void cain_sdp_media_description_set_media(cain_sdp_media_description_t* media_description, cain_sdp_media_t* media) {
	SET_OBJECT(media_description,media,cain_sdp_media_t)
}

/************************
 * origin
 ***********************/
struct _cain_sdp_origin {
	cain_sip_object_t base;
	const char* address;
	const char* address_type;
	const char* network_type;
	const char* username;
	unsigned int session_id;
	unsigned int session_version;

 };

void cain_sdp_origin_destroy(cain_sdp_origin_t* origin) {
	DESTROY_STRING(origin,address)
	DESTROY_STRING(origin,address_type)
	DESTROY_STRING(origin,network_type)
	DESTROY_STRING(origin,username)
}

void cain_sdp_origin_clone(cain_sdp_origin_t *origin, const cain_sdp_origin_t *orig){
	CLONE_STRING(cain_sdp_origin,username,origin,orig);
	CLONE_STRING(cain_sdp_origin,address,origin,orig);
	CLONE_STRING(cain_sdp_origin,address_type,origin,orig);
	CLONE_STRING(cain_sdp_origin,network_type,origin,orig);
	origin->session_id = orig->session_id;
	origin->session_version = orig->session_version;
}
int cain_sdp_origin_marshal(cain_sdp_origin_t* origin, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
									,buff_size-current_offset
									,"o=%s %i %i %s %s %s"
									,origin->username
									,origin->session_id
									,origin->session_version
									,origin->network_type
									,origin->address_type
									,origin->address);
	return current_offset-offset;
}
CAIN_SDP_NEW(origin,cain_sip_object)
cain_sdp_origin_t* cain_sdp_origin_create(const char* user_name
											, unsigned int session_id
											, unsigned int session_version
											, const char* network_type
											, const char* addr_type
											, const char* address) {
	cain_sdp_origin_t* origin=cain_sdp_origin_new();
	cain_sdp_origin_set_username(origin,user_name);
	cain_sdp_origin_set_session_id(origin,session_id);
	cain_sdp_origin_set_session_version(origin,session_version);
	cain_sdp_origin_set_network_type(origin,network_type);
	cain_sdp_origin_set_address_type(origin,addr_type);
	cain_sdp_origin_set_address(origin,address);
	return origin;
}
CAIN_SDP_PARSE(origin)
GET_SET_STRING(cain_sdp_origin,username);
GET_SET_STRING(cain_sdp_origin,address);
GET_SET_STRING(cain_sdp_origin,address_type);
GET_SET_STRING(cain_sdp_origin,network_type);
GET_SET_INT(cain_sdp_origin,session_id,unsigned int);
GET_SET_INT(cain_sdp_origin,session_version,unsigned int);
/************************
 * session_name
 ***********************/
struct _cain_sdp_session_name {
	cain_sip_object_t base;
	const char* value;
 };

void cain_sdp_session_name_destroy(cain_sdp_session_name_t* session_name) {
	DESTROY_STRING(session_name,value)
}

void cain_sdp_session_name_clone(cain_sdp_session_name_t *session_name, const cain_sdp_session_name_t *orig){
	CLONE_STRING(cain_sdp_session_name,value,session_name,orig);
}
int cain_sdp_session_name_marshal(cain_sdp_session_name_t* session_name, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"s=%s"
								,session_name->value);
	return current_offset-offset;
}
CAIN_SDP_NEW(session_name,cain_sip_object)
cain_sdp_session_name_t* cain_sdp_session_name_create (const char* name) {
	cain_sdp_session_name_t* n=cain_sdp_session_name_new();
	cain_sdp_session_name_set_value(n,name);
	return n;
}
//CAIN_SDP_PARSE(session_name)
GET_SET_STRING(cain_sdp_session_name,value);


/************************
 * session_description
 ***********************/
struct _cain_sdp_session_description {
	cain_sdp_base_description_t base_description;
	cain_sdp_version_t* version;
	cain_sip_list_t* emails;
	cain_sdp_origin_t* origin;
	cain_sdp_session_name_t* session_name;
	cain_sip_list_t* phones;
	cain_sip_list_t* times;
	cain_sdp_uri_t* uri;
	cain_sdp_uri_t* zone_adjustments;
	cain_sip_list_t* media_descriptions;

 };
void cain_sdp_session_description_destroy(cain_sdp_session_description_t* session_description) {
	if (session_description->version) cain_sip_object_unref(CAIN_SIP_OBJECT(session_description->version));
	cain_sip_list_free_with_data(session_description->emails,cain_sip_object_freefunc);
	if (session_description->origin) cain_sip_object_unref(CAIN_SIP_OBJECT(session_description->origin));
	if (session_description->session_name) cain_sip_object_unref(CAIN_SIP_OBJECT(session_description->session_name));
	cain_sip_list_free_with_data(session_description->phones,cain_sip_object_freefunc);
	cain_sip_list_free_with_data(session_description->times,cain_sip_object_freefunc);
	if (session_description->uri) cain_sip_object_unref(CAIN_SIP_OBJECT(session_description->uri));
	if (session_description->zone_adjustments) cain_sip_object_unref(CAIN_SIP_OBJECT(session_description->zone_adjustments));
	cain_sip_list_free_with_data(session_description->media_descriptions,cain_sip_object_freefunc);
}

void cain_sdp_session_description_clone(cain_sdp_session_description_t *session_description, const cain_sdp_session_description_t *orig){
	if (orig->version) session_description->version = CAIN_SDP_VERSION(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(orig->version)));
	session_description->emails = cain_sip_list_copy_with_data(orig->emails,cain_sip_object_copyfunc);
	if (orig->origin) session_description->origin = CAIN_SDP_ORIGIN(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(orig->origin)));
	if (orig->session_name) session_description->session_name = CAIN_SDP_SESSION_NAME(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(orig->session_name)));
	session_description->phones = cain_sip_list_copy_with_data(orig->phones,cain_sip_object_copyfunc);
	session_description->times = cain_sip_list_copy_with_data(orig->times,cain_sip_object_copyfunc);
	if (orig->uri) session_description->uri = CAIN_SDP_URI(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(orig->uri)));
	if (orig->zone_adjustments) session_description->zone_adjustments = CAIN_SDP_URI(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(orig->zone_adjustments)));
	session_description->media_descriptions = cain_sip_list_copy_with_data(orig->media_descriptions,cain_sip_object_copyfunc);
}
int cain_sdp_session_description_marshal(cain_sdp_session_description_t* session_description, char* buff,unsigned int offset,unsigned int buff_size) {
/*session_description:   proto_version CR LF
                         origin_field
                         session_name_field
                         (info CR LF)?
                         uri_field?
                         (email CR LF)*
                         phone_field*
                         (connection CR LF)?
                         (bandwidth CR LF)*
                         time_field
                         (repeat_time CR LF)?
                         (zone_adjustments CR LF)?
                         (key_field CR LF)?
                         (attribute CR LF)*
                         media_descriptions;
 */
	unsigned int current_offset=offset;
	cain_sip_list_t* media_descriptions;
	cain_sip_list_t* times;
	cain_sip_list_t* attributes;

	current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(session_description->version),buff,current_offset,buff_size);
	current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");

	current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(session_description->origin),buff,current_offset,buff_size);
	current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");

	current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(session_description->session_name),buff,current_offset,buff_size);
	current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");

	current_offset+=cain_sdp_base_description_marshal((cain_sdp_base_description_t*)(&session_description->base_description),buff,current_offset,buff_size);

	current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "t=");
	for(times=session_description->times;times!=NULL;times=times->next){
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(times->data),buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	}

	for(attributes=session_description->base_description.attributes;attributes!=NULL;attributes=attributes->next){
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(attributes->data),buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	}

	for(media_descriptions=session_description->media_descriptions;media_descriptions!=NULL;media_descriptions=media_descriptions->next){
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(media_descriptions->data),buff,current_offset,buff_size);
	}

	return current_offset-offset;
}
CAIN_SDP_NEW(session_description,cain_sdp_base_description)
CAIN_SDP_PARSE(session_description)

cain_sdp_session_description_t* cain_sdp_session_description_create(cain_sip_message_t* message) {
	cain_sdp_session_description_t* session_desc=NULL;
	cain_sip_header_content_type_t* content_type=cain_sip_message_get_header_by_type(message,cain_sip_header_content_type_t);
	if (content_type
		&& strcmp("application",cain_sip_header_content_type_get_type(content_type))==0
		&&	strcmp("sdp",cain_sip_header_content_type_get_subtype(content_type))==0) {
		session_desc=cain_sdp_session_description_parse(cain_sip_message_get_body(message));
	}
	return session_desc;
}
const char*	cain_sdp_session_description_get_attribute_value(const cain_sdp_session_description_t* session_description, const char* name) {
	return cain_sdp_base_description_get_attribute_value(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),name);
}
int	cain_sdp_session_description_get_bandwidth(const cain_sdp_session_description_t* session_description, const char* name) {
	return cain_sdp_base_description_get_bandwidth(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),name);
}
cain_sip_list_t*	cain_sdp_session_description_get_bandwidths(const cain_sdp_session_description_t* session_description) {
	return CAIN_SIP_CAST(session_description,cain_sdp_base_description_t)->bandwidths;
}
cain_sdp_connection_t*	cain_sdp_session_description_get_connection(const cain_sdp_session_description_t* session_description) {
	return CAIN_SIP_CAST(session_description,cain_sdp_base_description_t)->connection;
}
cain_sip_list_t* cain_sdp_session_description_get_emails(const cain_sdp_session_description_t* session_description){
	return session_description->emails;
}
cain_sdp_info_t* cain_sdp_session_description_get_info(const cain_sdp_session_description_t* session_description) {
	return CAIN_SIP_CAST(session_description,cain_sdp_base_description_t)->info;
}
/*cain_sdp_key_t*	cain_sdp_session_description_get_key(const cain_sdp_session_description_t* session_description);*/
cain_sip_list_t* cain_sdp_session_description_get_media_descriptions(const cain_sdp_session_description_t* session_description) {
	return session_description->media_descriptions;
}
cain_sdp_origin_t*	cain_sdp_session_description_get_origin(const cain_sdp_session_description_t* session_description){
	return session_description->origin;
}
cain_sip_list_t* cain_sdp_session_description_get_phones(const cain_sdp_session_description_t* session_description) {
	return session_description->phones;
}
cain_sdp_session_name_t* cain_sdp_session_description_get_session_name(const cain_sdp_session_description_t* session_description) {
	return session_description->session_name;
}
cain_sip_list_t* cain_sdp_session_description_get_time_descriptions(const cain_sdp_session_description_t* session_description) {
	return session_description->times;
}
cain_sdp_uri_t* cain_sdp_session_description_get_uri(const cain_sdp_session_description_t* session_description) {
	return session_description->uri;
}
cain_sdp_version_t*	cain_sdp_session_description_get_version(const cain_sdp_session_description_t* session_description) {
	return session_description->version;
}
cain_sdp_uri_t* cain_sdp_session_description_get_zone_adjustments(const cain_sdp_session_description_t* session_description) {
	return session_description->zone_adjustments;
}
void cain_sdp_session_description_remove_attribute(cain_sdp_session_description_t* session_description, const char* name) {
	cain_sdp_base_description_remove_attribute(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),name);
}
void cain_sdp_session_description_remove_bandwidth(cain_sdp_session_description_t* session_description, const char* name) {
	cain_sdp_base_description_remove_bandwidth(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),name);
}
void cain_sdp_session_description_set_attribute_value(cain_sdp_session_description_t* session_description, const char* name, const char* value) {
	cain_sdp_base_description_set_attribute_value(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),name,value);
}
void cain_sdp_session_description_set_attributes(cain_sdp_session_description_t* session_description, cain_sip_list_t* attributes) {
	cain_sdp_base_description_set_attributes(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),attributes);
}
void cain_sdp_session_description_add_attribute(cain_sdp_session_description_t* session_description, const cain_sdp_attribute_t* attribute) {
	cain_sdp_base_description_add_attribute(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),attribute);
}
void cain_sdp_session_description_set_bandwidth(cain_sdp_session_description_t* session_description, const char* type, int value) {
	cain_sdp_base_description_set_bandwidth(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),type,value);
}
void cain_sdp_session_description_set_bandwidths(cain_sdp_session_description_t* session_description, cain_sip_list_t* bandwidths) {
	cain_sdp_base_description_set_bandwidths(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),bandwidths);
}
void cain_sdp_session_description_add_bandwidth(cain_sdp_session_description_t* session_description, const cain_sdp_bandwidth_t* bandwidth) {
	cain_sdp_base_description_add_bandwidth(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),bandwidth);
}
void cain_sdp_session_description_set_connection(cain_sdp_session_description_t* session_description, cain_sdp_connection_t* connection) {
	SET_OBJECT(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),connection,cain_sdp_connection_t)
}
void cain_sdp_session_description_set_emails(cain_sdp_session_description_t* session_description, cain_sip_list_t* emails) {
	SET_LIST(session_description->emails,emails)
}
void cain_sdp_session_description_set_info(cain_sdp_session_description_t* session_description, cain_sdp_info_t* info) {
	SET_OBJECT(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),info,cain_sdp_info_t)
}
/*void cain_sdp_session_description_set_key(cain_sdp_session_description_t* session_description, cain_sdp_key_t* key);*/
void cain_sdp_session_description_set_media_descriptions(cain_sdp_session_description_t* session_description, cain_sip_list_t* media_descriptions) {
	SET_LIST(session_description->media_descriptions,media_descriptions)
}
void cain_sdp_session_description_add_media_description(cain_sdp_session_description_t* session_description, cain_sdp_media_description_t* media_description) {
	session_description->media_descriptions = cain_sip_list_append(session_description->media_descriptions,cain_sip_object_ref(media_description));
}

void cain_sdp_session_description_set_origin(cain_sdp_session_description_t* session_description, cain_sdp_origin_t* origin) {
	SET_OBJECT(session_description,origin,cain_sdp_origin_t)
}
void cain_sdp_session_description_set_phones(cain_sdp_session_description_t* session_description, cain_sip_list_t* phones) {
	SET_LIST(session_description->phones,phones)
}
void cain_sdp_session_description_set_session_name(cain_sdp_session_description_t* session_description, cain_sdp_session_name_t* session_name) {
	SET_OBJECT(session_description,session_name,cain_sdp_session_name_t)
}
void cain_sdp_session_description_set_time_descriptions(cain_sdp_session_description_t* session_description, cain_sip_list_t* times) {
	SET_LIST(session_description->times,times)
}
void cain_sdp_session_description_set_time_description(cain_sdp_session_description_t* session_description, cain_sdp_time_description_t* time_desc) {
	cain_sdp_session_description_set_time_descriptions(session_description,cain_sip_list_new(time_desc));
}
void cain_sdp_session_description_set_uri(cain_sdp_session_description_t* session_description, cain_sdp_uri_t* uri) {
	SET_OBJECT(session_description,uri,cain_sdp_uri_t)
}
void cain_sdp_session_description_set_version(cain_sdp_session_description_t* session_description, cain_sdp_version_t* version) {
	SET_OBJECT(session_description,version,cain_sdp_version_t)
}
void cain_sdp_session_description_set_zone_adjustments(cain_sdp_session_description_t* session_description, cain_sdp_uri_t* zone_adjustments) {
	SET_OBJECT(session_description,zone_adjustments,cain_sdp_uri_t)
}
/************************
 * time
 ***********************/
struct _cain_sdp_time {
	cain_sip_object_t base;
	int start;
	int stop;
 };

void cain_sdp_time_destroy(cain_sdp_time_t* time) {

}

void cain_sdp_time_clone(cain_sdp_time_t *time, const cain_sdp_time_t *orig){
	time->start=orig->start;
	time->stop=orig->stop;
}
int cain_sdp_time_marshal(cain_sdp_time_t* time, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"%i %i"
								,time->start
								,time->stop);
	return current_offset-offset;
}
CAIN_SDP_NEW(time,cain_sip_object)
//CAIN_SDP_PARSE(version)
GET_SET_INT(cain_sdp_time,start,int);
GET_SET_INT(cain_sdp_time,stop,int);

/************************
 * time description
 ***********************/
struct _cain_sdp_time_description {
	cain_sip_object_t base;
	cain_sdp_time_t* time;

 };

void cain_sdp_time_description_destroy(cain_sdp_time_description_t* time_description) {
	if (time_description->time) cain_sip_object_unref(CAIN_SIP_OBJECT(time_description->time));
}

void cain_sdp_time_description_clone(cain_sdp_time_description_t *time_description, const cain_sdp_time_description_t *orig){
	if (orig->time) time_description->time = CAIN_SDP_TIME(cain_sip_object_clone_and_ref(CAIN_SIP_OBJECT(orig->time)));
}
int cain_sdp_time_description_marshal(cain_sdp_time_description_t* time_description, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(time_description->time),buff,current_offset,buff_size);
	return current_offset-offset;
}
CAIN_SDP_NEW(time_description,cain_sip_object)

cain_sdp_time_description_t* cain_sdp_time_description_create (int start,int stop) {
	cain_sdp_time_description_t* time_desc= cain_sdp_time_description_new();
	cain_sdp_time_t* time = cain_sdp_time_new();
	cain_sdp_time_set_start(time,start);
	cain_sdp_time_set_stop(time,stop);
	cain_sdp_time_description_set_time(time_desc,time);
	return time_desc;
}
cain_sip_list_t* cain_sdp_time_description_get_repeate_times(const cain_sdp_time_description_t* time_description) {
	return NULL;
}
cain_sdp_time_t* cain_sdp_time_description_get_time(const cain_sdp_time_description_t* time_description) {
	return time_description->time;
}
void cain_sdp_time_description_set_repeate_times(cain_sdp_time_description_t* time_description, cain_sip_list_t* times) {
	cain_sip_error("time description repeat time not implemented");
}
void cain_sdp_time_description_set_time(cain_sdp_time_description_t* time_description, cain_sdp_time_t* time) {
	SET_OBJECT(time_description,time,cain_sdp_time_t)
}
#define CAIN_SDP_TIME_DESCRIPTION(t) CAIN_SDP_CAST(t,cain_sdp_time_description_t);

/************************
 * version
 ***********************/
struct _cain_sdp_version {
	cain_sip_object_t base;
	int version;
 };

void cain_sdp_version_destroy(cain_sdp_version_t* version) {

}

void cain_sdp_version_clone(cain_sdp_version_t *version, const cain_sdp_version_t *orig){
	version->version = orig->version;
}
int cain_sdp_version_marshal(cain_sdp_version_t* version, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=snprintf(	buff+current_offset
								,buff_size-current_offset
								,"v=%i"
								,version->version);
	return current_offset-offset;
}
CAIN_SDP_NEW(version,cain_sip_object)
cain_sdp_version_t* cain_sdp_version_create(int version) {
	cain_sdp_version_t* v = cain_sdp_version_new();
	cain_sdp_version_set_version(v,version);
	return v;
}
//CAIN_SDP_PARSE(version)
GET_SET_INT(cain_sdp_version,version,int);

/***************************************************************************************
 * mime_parameter
 *
 **************************************************************************************/
struct _cain_sdp_mime_parameter {
	cain_sip_object_t base;
	int rate;
	int channel_count;
	int ptime;
	int max_ptime;
	int media_format;
	const char* type;
	const char* parameters;

};
static void cain_sdp_mime_parameter_destroy(cain_sdp_mime_parameter_t *mime_parameter) {
	if (mime_parameter->type) cain_sip_free((void*)mime_parameter->type);
	if (mime_parameter->parameters) cain_sip_free((void*)mime_parameter->parameters);
}
static void cain_sdp_mime_parameter_clone(cain_sdp_mime_parameter_t *mime_parameter,cain_sdp_mime_parameter_t *orig) {
	mime_parameter->rate = orig->rate;
	mime_parameter->channel_count = orig->channel_count;
	mime_parameter->ptime = orig->ptime;
	mime_parameter->max_ptime = orig->max_ptime;
	mime_parameter->media_format = orig->media_format;
	CLONE_STRING(cain_sdp_mime_parameter,type,mime_parameter,orig);
	CLONE_STRING(cain_sdp_mime_parameter,parameters,mime_parameter,orig);
}
CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sdp_mime_parameter_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sdp_mime_parameter_t
							,cain_sip_object_t
							,cain_sdp_mime_parameter_destroy
							,cain_sdp_mime_parameter_clone
							,NULL
							,TRUE);

cain_sdp_mime_parameter_t* cain_sdp_mime_parameter_new() {
	cain_sdp_mime_parameter_t* l_param = cain_sip_object_new(cain_sdp_mime_parameter_t);
	l_param->ptime = -1;
	l_param->max_ptime = -1;
	return l_param;
}
cain_sdp_mime_parameter_t* cain_sdp_mime_parameter_create(const char* type, int media_format, int rate,int channel_count) {
	cain_sdp_mime_parameter_t* mime_param= cain_sdp_mime_parameter_new();
	cain_sdp_mime_parameter_set_type(mime_param,type);
	cain_sdp_mime_parameter_set_media_format(mime_param,media_format);
	cain_sdp_mime_parameter_set_rate(mime_param,rate);
	cain_sdp_mime_parameter_set_channel_count(mime_param,channel_count);
	return mime_param;
}
GET_SET_INT(cain_sdp_mime_parameter,rate,int);
GET_SET_INT(cain_sdp_mime_parameter,channel_count,int);
GET_SET_INT(cain_sdp_mime_parameter,ptime,int);
GET_SET_INT(cain_sdp_mime_parameter,max_ptime,int);
GET_SET_INT(cain_sdp_mime_parameter,media_format,int);
GET_SET_STRING(cain_sdp_mime_parameter,type);
GET_SET_STRING(cain_sdp_mime_parameter,parameters);

