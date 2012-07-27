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

cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_object_vptr_t *vptr){
	cain_sip_object_t *obj=(cain_sip_object_t *)cain_sip_malloc0(objsize);
	obj->ref=vptr->initially_unowned ? 0 : 1;
	obj->vptr=vptr;
	obj->size=objsize;
	return obj;
}

int cain_sip_object_is_initially_unowned(const cain_sip_object_t *obj){
	return obj->vptr->initially_unowned;
}

cain_sip_object_t * cain_sip_object_ref(void *obj){
	CAIN_SIP_OBJECT(obj)->ref++;
	return obj;
}

void cain_sip_object_unref(void *ptr){
	cain_sip_object_t *obj=CAIN_SIP_OBJECT(ptr);
	if (obj->ref==-1) cain_sip_fatal("Object of type [%s] freed twice !",obj->name);
	if (obj->ref==0){
		obj->ref=-1;
		cain_sip_object_delete(obj);
		return;
	}
	obj->ref--;
	if (obj->ref==0){
		obj->ref=-1;
		cain_sip_object_delete(obj);
	}
}

static weak_ref_t *weak_ref_new(cain_sip_object_destroy_notify_t destroy_notify, void *userpointer){
	weak_ref_t *r=cain_sip_new(weak_ref_t);
	r->next=NULL;
	r->notify=destroy_notify;
	r->userpointer=userpointer;
	return r;
}

cain_sip_object_t *cain_sip_object_weak_ref(void *obj, cain_sip_object_destroy_notify_t destroy_notify, void *userpointer){
	cain_sip_object_t *o=CAIN_SIP_OBJECT(obj);
	weak_ref_t *old=o->weak_refs;
	o->weak_refs=weak_ref_new(destroy_notify,userpointer);
	o->weak_refs->next=old;
	return o;
}

void cain_sip_object_weak_unref(void *obj, cain_sip_object_destroy_notify_t destroy_notify, void *userpointer){
	cain_sip_object_t *o=CAIN_SIP_OBJECT(obj);
	weak_ref_t *ref,*prevref=NULL,*next=NULL;

	if (o->ref==-1) return; /*too late and avoid recursions*/
	for(ref=o->weak_refs;ref!=NULL;ref=next){
		next=ref->next;
		if (ref->notify==destroy_notify && ref->userpointer==userpointer){
			cain_sip_message("cain_sip_object_weak_unref(): prefref=%p",prevref);
			if (prevref==NULL) o->weak_refs=next;
			else prevref->next=next;
			cain_sip_free(ref);
			return;
		}else{
			prevref=ref;
		}
	}
	cain_sip_fatal("Could not find weak_ref, you're a looser.");
}

static void cain_sip_object_loose_weak_refs(cain_sip_object_t *obj){
	weak_ref_t *ref,*next;
	for(ref=obj->weak_refs;ref!=NULL;ref=next){
		next=ref->next;
		ref->notify(ref->userpointer,obj);
		cain_sip_free(ref);
	}
	obj->weak_refs=NULL;
}

static void _cain_sip_object_uninit(cain_sip_object_t *obj){
	if (obj->name)
		cain_sip_free(obj->name);
}

static void _cain_sip_object_clone(cain_sip_object_t *obj, const cain_sip_object_t *orig){
	if (orig->name!=NULL) obj->name=cain_sip_strdup(obj->name);
}

static int _cain_object_marshall(cain_sip_object_t* obj, char* buff,unsigned int offset,size_t buff_size) {
	return snprintf(buff+offset,buff_size,"{%s::%s %p}",obj->vptr->type_name,obj->name ? obj->name : "(no name)",obj);
}

cain_sip_object_vptr_t cain_sip_object_t_vptr={
	CAIN_SIP_TYPE_ID(cain_sip_object_t),
	"cain_sip_object_t",
	FALSE,
	NULL, /*no parent, it's god*/
	NULL,
	_cain_sip_object_uninit,
	_cain_sip_object_clone,
	_cain_object_marshall
};

void cain_sip_object_delete(void *ptr){
	cain_sip_object_t *obj=CAIN_SIP_OBJECT(ptr);
	cain_sip_object_vptr_t *vptr;
	
	cain_sip_object_loose_weak_refs(obj);
	vptr=obj->vptr;
	while(vptr!=NULL){
		if (vptr->destroy) vptr->destroy(obj);
		vptr=vptr->parent;
	}
	cain_sip_free(obj);
}

static cain_sip_object_vptr_t *find_common_floor(cain_sip_object_vptr_t *vptr1, cain_sip_object_vptr_t *vptr2){
	cain_sip_object_vptr_t *it1,*it2;
	for (it1=vptr1;it1!=NULL;it1=it1->parent){
		if (it1==vptr2)
			return vptr2;
	}
	for(it2=vptr2;it2!=NULL;it2=it2->parent){
		if (vptr1==it2)
			return vptr1;
	}
	return find_common_floor(vptr1->parent,vptr2);
}

/*copy the content of ref object to new object, for the part they have in common in their inheritence diagram*/
void _cain_sip_object_copy(cain_sip_object_t *newobj, const cain_sip_object_t *ref){
	cain_sip_object_vptr_t *vptr;
	vptr=find_common_floor(newobj->vptr,ref->vptr);
	if (vptr==NULL){
		cain_sip_fatal("Should not happen");
	}
	while(vptr!=NULL){
		if (vptr->clone==NULL){
			cain_sip_fatal("Object of type %s cannot be cloned, it does not provide a clone() implementation.",vptr->type_name);
			return;
		}else vptr->clone(newobj,ref);
		vptr=vptr->parent;
	}
}

cain_sip_object_t *cain_sip_object_clone(const cain_sip_object_t *obj){
	cain_sip_object_t *newobj;
	
	newobj=cain_sip_malloc0(obj->size);
	newobj->ref=obj->vptr->initially_unowned ? 0 : 1;
	newobj->vptr=obj->vptr;
	newobj->size=obj->size;
	
	_cain_sip_object_copy(newobj,obj);
	
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
			cain_sip_interface_desc_t **ifaces=vptr->interfaces;
			if (ifaces!=NULL){
				for(;*ifaces!=0;++ifaces){
					if ((*ifaces)->id==ifid){
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

void *cain_sip_object_interface_cast(cain_sip_object_t *obj, cain_sip_interface_id_t ifid, const char *castname, const char *file, int fileno){
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
	return cain_sip_strdup(buff);

}

char * _cain_sip_object_describe_type(cain_sip_object_vptr_t *vptr){
	const int maxbufsize=2048;
	char *ret=cain_sip_malloc(maxbufsize);
	cain_sip_object_vptr_t *it;
	int pos=0;
	cain_sip_list_t *l=NULL,*elem;
	pos+=snprintf(ret+pos,maxbufsize-pos,"Ownership:\n");
	pos+=snprintf(ret+pos,maxbufsize-pos,"\t%s is created initially %s\n",vptr->type_name,
	              vptr->initially_unowned ? "unowned" : "owned");
	pos+=snprintf(ret+pos,maxbufsize-pos,"\nInheritance diagram:\n");
	for(it=vptr;it!=NULL;it=it->parent){
		l=cain_sip_list_prepend(l,it);
	}
	for(elem=l;elem!=NULL;elem=elem->next){
		it=(cain_sip_object_vptr_t*)elem->data;
		pos+=snprintf(ret+pos,maxbufsize-pos,"\t%s\n",it->type_name);
		if (elem->next)
			pos+=snprintf(ret+pos,maxbufsize-pos,"\t        |\n");
	}
	cain_sip_list_free(l);
	pos+=snprintf(ret+pos,maxbufsize-pos,"\nImplemented interfaces:\n");
	for(it=vptr;it!=NULL;it=it->parent){
		cain_sip_interface_desc_t **desc=it->interfaces;
		if (desc!=NULL){
			for(;*desc!=NULL;desc++){
				pos+=snprintf(ret+pos,maxbufsize-pos,"\t* %s\n",(*desc)->ifname);
			}
		}
	}
	return ret;
}

char *cain_sip_object_describe(void *obj){
	cain_sip_object_t *o=CAIN_SIP_OBJECT(obj);
	return _cain_sip_object_describe_type(o->vptr);
}

#include <dlfcn.h>

char *cain_sip_object_describe_type_from_name(const char *name){
	char *vptr_name;
	void *handle;
	void *symbol;
	
	handle=dlopen(NULL,RTLD_LAZY);
	if (handle==NULL){
		cain_sip_error("cain_sip_object_describe_type_from_name: dlopen() failed: %s",dlerror());
		return NULL;
	}
	vptr_name=cain_sip_strdup_printf("%s_vptr",name);
	symbol=dlsym(handle,vptr_name);
	cain_sip_free(vptr_name);
	dlclose(handle);
	if (symbol==NULL){
		cain_sip_error("cain_sip_object_describe_type_from_name: could not find vptr for type %s",name);
		return NULL;
	}
	return _cain_sip_object_describe_type((cain_sip_object_vptr_t*)symbol);
}
