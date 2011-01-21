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

#include "cain_sip_internal.h"

static uint8_t *find_type(cain_sip_object_t *obj, cain_sip_type_id_t id){
	int i;
	for(i=0;i<sizeof(obj->type_ids);++i){
		if (obj->type_ids[i]==(uint8_t)id)
			return &obj->type_ids[i];
	}
	return NULL;
}

void _cain_sip_object_init_type(cain_sip_object_t *obj, cain_sip_type_id_t id){
	uint8_t * t=find_type(obj,id);
	if (t!=NULL) cain_sip_fatal("This object already inherits type %i",id);
	t=find_type(obj,0);
	if (t==NULL) cain_sip_fatal("This object has too much inheritance !");
	*t=id;
}

cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_type_id_t id, void *vptr, cain_sip_object_destroy_t destroy_func, int initially_unowed){
	cain_sip_object_t *obj=(cain_sip_object_t *)cain_sip_malloc0(objsize);
	obj->type_ids[0]=id;
	obj->ref=initially_unowed ? 0 : 1;
	obj->vptr=vptr;
	obj->destroy=destroy_func;
	return obj;
}

int cain_sip_object_is_unowed(const cain_sip_object_t *obj){
	return obj->ref==0;
}

cain_sip_object_t * cain_sip_object_ref(void *obj){
	CAIN_SIP_OBJECT(obj)->ref++;
	return obj;
}

void cain_sip_object_unref(void *ptr){
	cain_sip_object_t *obj=CAIN_SIP_OBJECT(ptr);
	if (obj->ref==0){
		cain_sip_warning("Destroying unowed object");
		cain_sip_object_destroy(obj);
		return;
	}
	obj->ref--;
	if (obj->ref==0){
		cain_sip_object_destroy(obj);
	}
}

void cain_sip_object_destroy(void *ptr){
	cain_sip_object_t *obj=CAIN_SIP_OBJECT(ptr);
	if (obj->ref!=0){
		cain_sip_error("Destroying referenced object !");
		if (obj->destroy) obj->destroy(obj);
		cain_sip_free(obj);
	}
}

void *cain_sip_object_cast(cain_sip_object_t *obj, cain_sip_type_id_t id, const char *castname, const char *file, int fileno){
	if (find_type(obj,id)==NULL){
		cain_sip_fatal("Bad cast to %s at %s:%i",castname,file,fileno);
		return NULL;
	}
	return obj;
}
void cain_sip_object_init(cain_sip_object_t *obj) {
	cain_sip_object_init_type(obj,cain_sip_object_t);
}
void cain_sip_object_set_name(cain_sip_object_t* object,const char* name) {
	if (name==NULL) return;
	if (object->name) cain_sip_free((void*)object->name);
	object->name=cain_sip_strdup(name);
}
const char* cain_sip_object_get_name(cain_sip_object_t* object) {
	return object->name;
}
