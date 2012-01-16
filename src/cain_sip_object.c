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

static int has_type(cain_sip_object_t *obj, cain_sip_type_id_t id){
	cain_sip_object_vptr_t *vptr=obj->vptr;
	
	while(vptr!=NULL){
		if (vptr->id==id) return TRUE;
		vptr=vptr->parent;
	}
	return FALSE;
}

int cain_sip_object_is_instance_of(cain_sip_object_t * obj,cain_sip_type_id_t id) {
	return has_type(obj,id);
}

cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_object_vptr_t *vptr, int initially_unowed){
	cain_sip_object_t *obj=(cain_sip_object_t *)cain_sip_malloc0(objsize);
	obj->ref=initially_unowed ? 0 : 1;
	obj->vptr=vptr;
	obj->size=objsize;
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
		cain_sip_object_delete(obj);
		return;
	}
	obj->ref--;
	if (obj->ref==0){
		cain_sip_object_delete(obj);
	}
}

static void _cain_sip_object_uninit(cain_sip_object_t *obj){
	if (obj->name)
		cain_sip_free(obj->name);
}

static void _cain_sip_object_clone(cain_sip_object_t *obj, const cain_sip_object_t *orig){
	if (orig->name!=NULL) obj->name=cain_sip_strdup(obj->name);
}

cain_sip_object_vptr_t cain_sip_object_t_vptr={
	CAIN_SIP_TYPE_ID(cain_sip_object_t),
	NULL, /*no parent, it's god*/
	NULL,
	_cain_sip_object_uninit,
	_cain_sip_object_clone,
	NULL
};

void cain_sip_object_delete(void *ptr){
	cain_sip_object_t *obj=CAIN_SIP_OBJECT(ptr);
	cain_sip_object_vptr_t *vptr;
	if (obj->ref!=0){
		cain_sip_error("Destroying referenced object !");
		vptr=obj->vptr;
		while(vptr!=NULL){
			if (vptr->destroy) vptr->destroy(obj);
			vptr=vptr->parent;
		}
		cain_sip_free(obj);
	}
}

cain_sip_object_t *cain_sip_object_clone(const cain_sip_object_t *obj){
	cain_sip_object_t *newobj;
	cain_sip_object_vptr_t *vptr;
	
	newobj=cain_sip_malloc0(obj->size);
	newobj->ref=1;
	newobj->vptr=obj->vptr;
	
	vptr=obj->vptr;
	while(vptr!=NULL){
		if (vptr->clone==NULL){
			cain_sip_fatal("Object of type %i cannot be cloned, it does not provide a clone() implementation.",vptr->id);
			return NULL;
		}else vptr->clone(newobj,obj);
		vptr=vptr->parent;
	}
	return newobj;
}

void *cain_sip_object_cast(cain_sip_object_t *obj, cain_sip_type_id_t id, const char *castname, const char *file, int fileno){
	if (obj!=NULL){
		if (has_type(obj,id)==0){
			cain_sip_fatal("Bad cast to %s at %s:%i",castname,file,fileno);
			return NULL;
		}
	}
	return obj;
}

void *cain_sip_object_get_interface_methods(cain_sip_object_t *obj, cain_sip_interface_id_t ifid){
	if (obj!=NULL){
		cain_sip_object_vptr_t *vptr;
		for (vptr=obj->vptr;vptr!=NULL;vptr=vptr->parent){
			cain_sip_interface_id_t **ifaces=vptr->interfaces;
			if (ifaces!=NULL){
				for(;*ifaces!=0;++ifaces){
					if (**ifaces==ifid){
						return *ifaces;
					}
				}
			}
		}
	}
	return NULL;
}

int cain_sip_object_implements(cain_sip_object_t *obj, cain_sip_interface_id_t id){
	return cain_sip_object_get_interface_methods(obj,id)!=NULL;
}

void *cain_sip_object_cast_to_interface(cain_sip_object_t *obj, cain_sip_interface_id_t ifid, const char *castname, const char *file, int fileno){
	if (obj!=NULL){
		if (cain_sip_object_get_interface_methods(obj,ifid)==0){
			cain_sip_fatal("Bad cast to interface %s at %s:%i",castname,file,fileno);
			return NULL;
		}
	}
	return obj;
}

void cain_sip_object_set_name(cain_sip_object_t* object,const char* name) {
	if (object->name) {
		cain_sip_free(object->name);
		object->name=NULL;
	}
	if (name)
		object->name=cain_sip_strdup(name);
}

const char* cain_sip_object_get_name(cain_sip_object_t* object) {
	return object->name;
}

int cain_sip_object_marshal(cain_sip_object_t* obj, char* buff,unsigned int offset,size_t buff_size) {
	cain_sip_object_vptr_t *vptr=obj->vptr;
	while (vptr != NULL) {
		if (vptr->marshal != NULL) {
			return vptr->marshal(obj,buff,offset,buff_size);
		} else {
			vptr=vptr->parent;
		}
	}
	return -1; /*no implementation found*/
}
char* cain_sip_object_to_string(cain_sip_object_t* obj) {
	char buff[2048]; /*to be optimized*/
	int size = cain_sip_object_marshal(obj,buff,0,sizeof(buff));
	buff[size]='\0';
	return strdup(buff);

}
