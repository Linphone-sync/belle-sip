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
	media->media_formats = formats;
}
void cain_sdp_media_destroy(cain_sdp_media_t* media) {
}
static void cain_sdp_media_init(cain_sdp_media_t* media) {
	media->port_count=1;
}

void cain_sdp_media_clone(cain_sdp_media_t *media, const cain_sdp_media_t *orig){
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
GET_SET_STRING(cain_sdp_media,media_type);
GET_SET_STRING(cain_sdp_media,protocol);
GET_SET_INT(cain_sdp_media,media_port,int)
GET_SET_INT(cain_sdp_media,port_count,int)

/************************
 * base_description
 ***********************/

typedef struct _cain_sdp_base_description {
	cain_sip_object_t base;
	cain_sdp_info_t* info;
	cain_sdp_connection_t* connection;
	cain_sip_list_t* bandwidths;
	cain_sip_list_t* attributes;
} cain_sdp_base_description_t;

void cain_sdp_base_description_destroy(cain_sdp_base_description_t* base_description) {
}
void cain_sdp_base_description_init(cain_sdp_base_description_t* base_description) {
}
void cain_sdp_base_description_clone(cain_sdp_base_description_t *base_description, const cain_sdp_base_description_t *orig){
}
int cain_sdp_base_description_marshal(cain_sdp_base_description_t* base_description, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	cain_sip_list_t* bandwidths;
	cain_sip_list_t* attributes;
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
	for(attributes=base_description->attributes;attributes!=NULL;attributes=attributes->next){
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(attributes->data),buff,current_offset,buff_size);
		current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	}
	return current_offset-offset;
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sdp_base_description_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sdp_base_description_t,cain_sip_object_t,cain_sdp_base_description_destroy,NULL,cain_sdp_base_description_marshal,FALSE);

static int cain_sdp_base_description_attribute_comp_func(const cain_sdp_attribute_t* a, const char*b) {
	return strcmp(a->name,b);
}
const char*	cain_sdp_base_description_get_attribute(const cain_sdp_base_description_t* base_description, const char* name) {
	cain_sip_list_t* attribute;
	attribute = cain_sip_list_find_custom(base_description->attributes, (cain_sip_compare_func)cain_sdp_base_description_attribute_comp_func, name);
	if (attribute) {
		return ((cain_sdp_attribute_t*)attribute->data)->value;
	} else {
		return NULL;
	}
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
void cain_sdp_base_description_set_attribute(cain_sdp_base_description_t* base_description, const char* name, const char* value) {
	cain_sdp_attribute_t* attribute = cain_sdp_attribute_new();
	cain_sdp_attribute_set_name(attribute,name);
	cain_sdp_attribute_set_value(attribute,value);
	base_description->attributes = cain_sip_list_append(base_description->attributes,attribute);
}
void cain_sdp_base_description_add_attribute(cain_sdp_base_description_t* base_description, const cain_sdp_attribute_t* attribute) {
	base_description->attributes = cain_sip_list_append(base_description->attributes,(void*)attribute);
}

#define SET_LIST(list_name,value) \
		cain_sip_list_t* list;\
		if (list_name) {\
			for (list=list_name;list !=NULL; list=list->next) {\
				cain_sip_object_unref(CAIN_SIP_OBJECT(list->data));\
			}\
			cain_sip_list_free(list_name); \
		} \
		list_name=value;


void cain_sdp_base_description_set_attributes(cain_sdp_base_description_t* base_description, cain_sip_list_t* attributes) {
	SET_LIST(base_description->attributes,attributes)
}
void cain_sdp_base_description_set_bandwidth(cain_sdp_base_description_t* base_description, const char* type, int value) {
	cain_sdp_bandwidth_t* bandwidth = cain_sdp_bandwidth_new();
	cain_sdp_bandwidth_set_type(bandwidth,type);
	cain_sdp_bandwidth_set_value(bandwidth,value);
	base_description->bandwidths = cain_sip_list_append(base_description->bandwidths,bandwidth);
}
void cain_sdp_base_description_add_bandwidth(cain_sdp_base_description_t* base_description, const cain_sdp_bandwidth_t* bandwidth) {
	base_description->bandwidths = cain_sip_list_append(base_description->bandwidths,(void *)bandwidth);
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
}

void cain_sdp_media_description_clone(cain_sdp_media_description_t *media_description, const cain_sdp_media_description_t *orig){
}
int cain_sdp_media_description_marshal(cain_sdp_media_description_t* media_description, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(media_description->media),buff,current_offset,buff_size);
	current_offset+=snprintf(buff+current_offset, buff_size-current_offset, "\r\n");
	current_offset+=cain_sdp_base_description_marshal(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),buff,current_offset,buff_size);
	return current_offset-offset;
}
CAIN_SDP_NEW(media_description,cain_sdp_base_description)
CAIN_SDP_PARSE(media_description)
void cain_sdp_media_description_add_dynamic_payloads(cain_sdp_media_description_t* media_description, cain_sip_list_t* payloadNames, cain_sip_list_t* payloadValues) {

}
const char*	cain_sdp_media_description_get_attribute(const cain_sdp_media_description_t* media_description, const char* name) {
	return cain_sdp_base_description_get_attribute(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),name);
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
struct static_payload static_payload_list [STATIC_PAYLOAD_LIST_LENTH] ={
	{0,1,"PCMU",8000},
	{3,1,"GSM",8000},
	{4,1,"G723",8000},
	{5,1,"DVI4",8000},
	{6,1,"DVI4",16000},
	{8,1,"PCMA",8000},
	{9,1,"G722",8000},
	{34,-1,"H263",90000}
};
static int mime_parameter_fill_from_static(cain_sdp_mime_parameter_t *mime_parameter,int format) {
	struct static_payload* iterator = static_payload_list;
	int i;
	for (i=0;i<STATIC_PAYLOAD_LIST_LENTH;i++) {
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
	ptime = cain_sdp_media_description_get_attribute(media_description,"ptime");
	ptime?ptime_as_int=atoi(ptime):-1;
	max_ptime = cain_sdp_media_description_get_attribute(media_description,"maxptime");
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
		cain_sdp_media_description_set_attribute(media_description,"rtpmap",atribute_value);
		if (cain_sdp_mime_parameter_get_parameters(mime_parameter)) {
			snprintf(atribute_value,MAX_FMTP_LENGH,"%i %s"
					,cain_sdp_mime_parameter_get_media_format(mime_parameter)
					,cain_sdp_mime_parameter_get_parameters(mime_parameter));
			cain_sdp_media_description_set_attribute(media_description,"fmtp",atribute_value);
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
void cain_sdp_media_description_set_attribute(cain_sdp_media_description_t* media_description, const char* name, const char* value) {
	cain_sdp_base_description_set_attribute(CAIN_SIP_CAST(media_description,cain_sdp_base_description_t),name,value);
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

void cain_sdp_media_description_set_connection(cain_sdp_media_description_t* media_description, cain_sdp_connection_t* conn) {
	cain_sdp_connection_t** current = &CAIN_SIP_CAST(media_description,cain_sdp_base_description_t)->connection;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=conn;
}
void cain_sdp_media_description_set_info(cain_sdp_media_description_t* media_description,cain_sdp_info_t* i) {
	cain_sdp_info_t** current = &CAIN_SIP_CAST(media_description,cain_sdp_base_description_t)->info;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=i;
}
/*void cain_sdp_media_description_set_key(cain_sdp_media_description_t* media_description,cain_sdp_key_t* key);*/
void cain_sdp_media_description_set_media(cain_sdp_media_description_t* media_description, cain_sdp_media_t* media) {
	cain_sdp_media_t** current = &media_description->media;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=media;
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
	int session_id;
	int session_version;

 };

void cain_sdp_origin_destroy(cain_sdp_origin_t* origin) {
}

void cain_sdp_origin_clone(cain_sdp_origin_t *origin, const cain_sdp_origin_t *orig){
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
CAIN_SDP_PARSE(origin)
GET_SET_STRING(cain_sdp_origin,username);
GET_SET_STRING(cain_sdp_origin,address);
GET_SET_STRING(cain_sdp_origin,address_type);
GET_SET_STRING(cain_sdp_origin,network_type);
GET_SET_INT(cain_sdp_origin,session_id,int);
GET_SET_INT(cain_sdp_origin,session_version,int);
/************************
 * session_name
 ***********************/
struct _cain_sdp_session_name {
	cain_sip_object_t base;
	const char* value;
 };

void cain_sdp_session_name_destroy(cain_sdp_session_name_t* session_name) {
}

void cain_sdp_session_name_clone(cain_sdp_session_name_t *session_name, const cain_sdp_session_name_t *orig){
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
}

void cain_sdp_session_description_clone(cain_sdp_session_description_t *session_description, const cain_sdp_session_description_t *orig){
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

	for(media_descriptions=session_description->media_descriptions;media_descriptions!=NULL;media_descriptions=media_descriptions->next){
		current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(media_descriptions->data),buff,current_offset,buff_size);
	}

	return current_offset-offset;
}
CAIN_SDP_NEW(session_description,cain_sdp_base_description)
CAIN_SDP_PARSE(session_description)

const char*	cain_sdp_session_description_get_attribute(const cain_sdp_session_description_t* session_description, const char* name) {
	return cain_sdp_base_description_get_attribute(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),name);
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
void cain_sdp_session_description_set_attribute(cain_sdp_session_description_t* session_description, const char* name, const char* value) {
	cain_sdp_base_description_set_attribute(CAIN_SIP_CAST(session_description,cain_sdp_base_description_t),name,value);
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
void cain_sdp_session_description_set_connection(cain_sdp_session_description_t* session_description, cain_sdp_connection_t* conn) {
	cain_sdp_connection_t** current = &CAIN_SIP_CAST(session_description,cain_sdp_base_description_t)->connection;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=conn;
}
void cain_sdp_session_description_set_emails(cain_sdp_session_description_t* session_description, cain_sip_list_t* emails) {
	SET_LIST(session_description->emails,emails)
}
void cain_sdp_session_description_set_info(cain_sdp_session_description_t* session_description, cain_sdp_info_t* i) {
	cain_sdp_info_t** current = &CAIN_SIP_CAST(session_description,cain_sdp_base_description_t)->info;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=i;
}
/*void cain_sdp_session_description_set_key(cain_sdp_session_description_t* session_description, cain_sdp_key_t* key);*/
void cain_sdp_session_description_set_media_descriptions(cain_sdp_session_description_t* session_description, cain_sip_list_t* media_descriptions) {
	SET_LIST(session_description->media_descriptions,media_descriptions)
}
void cain_sdp_session_description_add_media_description(cain_sdp_session_description_t* session_description, cain_sdp_media_description_t* media_description) {
	session_description->media_descriptions = cain_sip_list_append(session_description->media_descriptions,media_description);
}

void cain_sdp_session_description_set_origin(cain_sdp_session_description_t* session_description, cain_sdp_origin_t* origin) {
	cain_sdp_origin_t** current = &session_description->origin;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=origin;
}
void cain_sdp_session_description_set_phones(cain_sdp_session_description_t* session_description, cain_sip_list_t* phones) {
	SET_LIST(session_description->phones,phones)
}
void cain_sdp_session_description_set_session_name(cain_sdp_session_description_t* session_description, cain_sdp_session_name_t* session_name) {
	cain_sdp_session_name_t** current = &session_description->session_name;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=session_name;
}
void cain_sdp_session_description_set_time_descriptions(cain_sdp_session_description_t* session_description, cain_sip_list_t* times) {
	SET_LIST(session_description->times,times)
}
void cain_sdp_session_description_set_uri(cain_sdp_session_description_t* session_description, cain_sdp_uri_t* uri) {
	cain_sdp_uri_t** current = &session_description->uri;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=uri;
}
void cain_sdp_session_description_set_version(cain_sdp_session_description_t* session_description, cain_sdp_version_t* version) {
	cain_sdp_version_t** current = &session_description->version;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=version;
}
void cain_sdp_session_description_set_zone_adjustments(cain_sdp_session_description_t* session_description, cain_sdp_uri_t* zone_adjustments) {
	cain_sdp_uri_t** current = &session_description->zone_adjustments;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=zone_adjustments;
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
}

void cain_sdp_time_description_clone(cain_sdp_time_description_t *time_description, const cain_sdp_time_description_t *orig){
}
int cain_sdp_time_description_marshal(cain_sdp_time_description_t* time_description, char* buff,unsigned int offset,unsigned int buff_size) {
	unsigned int current_offset=offset;
	current_offset+=cain_sip_object_marshal(CAIN_SIP_OBJECT(time_description->time),buff,current_offset,buff_size);
	return current_offset-offset;
}
CAIN_SDP_NEW(time_description,cain_sip_object)

cain_sip_list_t* cain_sdp_time_description_get_repeate_times(const cain_sdp_time_description_t* time_description) {
	return NULL;
}
cain_sdp_time_t* cain_sdp_time_description_get_time(const cain_sdp_time_description_t* time_description) {
	return time_description->time;
}
void cain_sdp_time_description_set_repeate_times(cain_sdp_time_description_t* time_description, cain_sip_list_t* times) {
	cain_sip_error("time description repeat time not implemented");
}
void cain_sdp_time_description_set_time(cain_sdp_time_description_t* time_description, cain_sdp_time_t* value) {
	cain_sdp_time_t** current = &time_description->time;
	if (*current) {
		cain_sip_object_unref(CAIN_SIP_OBJECT(*current));
	}
	*current=value;
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

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sdp_mime_parameter_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sdp_mime_parameter_t,cain_sip_object_t,cain_sdp_mime_parameter_destroy,NULL,NULL,TRUE);

cain_sdp_mime_parameter_t* cain_sdp_mime_parameter_new() {
	cain_sdp_mime_parameter_t* l_param = cain_sip_object_new(cain_sdp_mime_parameter_t);
	l_param->ptime = -1;
	l_param->max_ptime = -1;
	return l_param;
}
GET_SET_INT(cain_sdp_mime_parameter,rate,int);
GET_SET_INT(cain_sdp_mime_parameter,channel_count,int);
GET_SET_INT(cain_sdp_mime_parameter,ptime,int);
GET_SET_INT(cain_sdp_mime_parameter,max_ptime,int);
GET_SET_INT(cain_sdp_mime_parameter,media_format,int);
GET_SET_STRING(cain_sdp_mime_parameter,type);
GET_SET_STRING(cain_sdp_mime_parameter,parameters);

