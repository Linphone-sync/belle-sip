/*
	belle-sip - SIP (RFC3261) library.
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

#include "belle_sip_internal.h"

static int has_type(belle_sip_object_t *obj, belle_sip_type_id_t id){
	belle_sip_object_vptr_t *vptr=obj->vptr;
	
	while(vptr!=NULL){
		if (vptr->id==id) return TRUE;
		vptr=vptr->parent;
	}
	return FALSE;
}

int belle_sip_object_is_instance_of(belle_sip_object_t * obj,belle_sip_type_id_t id) {
	return has_type(obj,id);
}

belle_sip_object_t * _belle_sip_object_new(size_t objsize, belle_sip_object_vptr_t *vptr){
	belle_sip_object_t *obj=(belle_sip_object_t *)belle_sip_malloc0(objsize);
	obj->ref=vptr->initially_unowned ? 0 : 1;
	obj->vptr=vptr;
	obj->size=objsize;
	return obj;
}

int belle_sip_object_is_initially_unowned(const belle_sip_object_t *obj){
	return obj->vptr->initially_unowned;
}

belle_sip_object_t * belle_sip_object_ref(void *obj){
	BELLE_SIP_OBJECT(obj)->ref++;
	return obj;
}

void belle_sip_object_unref(void *ptr){
	belle_sip_object_t *obj=BELLE_SIP_OBJECT(ptr);
	if (obj->ref==0){
		/*might hide a double unref belle_sip_warning("Destroying unowed object");*/
		belle_sip_object_delete(obj);
		return;
	}
	obj->ref--;
	if (obj->ref==0){
		belle_sip_object_delete(obj);
	}
}

static weak_ref_t *weak_ref_new(belle_sip_object_destroy_notify_t destroy_notify, void *userpointer){
	weak_ref_t *r=belle_sip_new(weak_ref_t);
	r->next=NULL;
	r->notify=destroy_notify;
	r->userpointer=userpointer;
	return r;
}

belle_sip_object_t *belle_sip_object_weak_ref(void *obj, belle_sip_object_destroy_notify_t destroy_notify, void *userpointer){
	belle_sip_object_t *o=BELLE_SIP_OBJECT(obj);
	weak_ref_t *old=o->weak_refs;
	o->weak_refs=weak_ref_new(destroy_notify,userpointer);
	o->weak_refs->next=old;
	return o;
}

void belle_sip_object_weak_unref(void *obj, belle_sip_object_destroy_notify_t destroy_notify, void *userpointer){
	belle_sip_object_t *o=BELLE_SIP_OBJECT(obj);
	weak_ref_t *ref,*prevref=NULL,*next=NULL;

	if (o->ref==-1) return; /*too late and avoid recursions*/
	for(ref=o->weak_refs;ref!=NULL;ref=next){
		next=ref->next;
		if (ref->notify==destroy_notify && ref->userpointer==userpointer){
			if (prevref==NULL) o->weak_refs=next;
			else prevref->next=next;
			belle_sip_free(ref);
			return;
		}else{
			prevref=ref;
		}
	}
	belle_sip_fatal("Could not find weak_ref, you're a looser.");
}

static void belle_sip_object_loose_weak_refs(belle_sip_object_t *obj){
	weak_ref_t *ref,*next;
	for(ref=obj->weak_refs;ref!=NULL;ref=next){
		next=ref->next;
		ref->notify(ref->userpointer,obj);
		belle_sip_free(ref);
	}
	obj->weak_refs=NULL;
}

static void _belle_sip_object_uninit(belle_sip_object_t *obj){
	if (obj->name)
		belle_sip_free(obj->name);
}

static void _belle_sip_object_clone(belle_sip_object_t *obj, const belle_sip_object_t *orig){
	if (orig->name!=NULL) obj->name=belle_sip_strdup(obj->name);
}

belle_sip_object_vptr_t belle_sip_object_t_vptr={
	BELLE_SIP_TYPE_ID(belle_sip_object_t),
	"belle_sip_object_t",
	FALSE,
	NULL, /*no parent, it's god*/
	NULL,
	_belle_sip_object_uninit,
	_belle_sip_object_clone,
	NULL
};

void belle_sip_object_delete(void *ptr){
	belle_sip_object_t *obj=BELLE_SIP_OBJECT(ptr);
	belle_sip_object_vptr_t *vptr;
	
	if (obj->ref!=0){
		belle_sip_error("Destroying referenced object !");
	}
	obj->ref=-1;
	belle_sip_object_loose_weak_refs(obj);
	vptr=obj->vptr;
	while(vptr!=NULL){
		if (vptr->destroy) vptr->destroy(obj);
		vptr=vptr->parent;
	}
	belle_sip_free(obj);
}

belle_sip_object_t *belle_sip_object_clone(const belle_sip_object_t *obj){
	belle_sip_object_t *newobj;
	belle_sip_object_vptr_t *vptr;
	
	newobj=belle_sip_malloc0(obj->size);
	newobj->ref=obj->vptr->initially_unowned ? 0 : 1;
	newobj->vptr=obj->vptr;
	
	vptr=obj->vptr;
	while(vptr!=NULL){
		if (vptr->clone==NULL){
			belle_sip_fatal("Object of type %i cannot be cloned, it does not provide a clone() implementation.",vptr->id);
			return NULL;
		}else vptr->clone(newobj,obj);
		vptr=vptr->parent;
	}
	return newobj;
}

void *belle_sip_object_cast(belle_sip_object_t *obj, belle_sip_type_id_t id, const char *castname, const char *file, int fileno){
	if (obj!=NULL){
		if (has_type(obj,id)==0){
			belle_sip_fatal("Bad cast to %s at %s:%i",castname,file,fileno);
			return NULL;
		}
	}
	return obj;
}

void *belle_sip_object_get_interface_methods(belle_sip_object_t *obj, belle_sip_interface_id_t ifid){
	if (obj!=NULL){
		belle_sip_object_vptr_t *vptr;
		for (vptr=obj->vptr;vptr!=NULL;vptr=vptr->parent){
			belle_sip_interface_desc_t **ifaces=vptr->interfaces;
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

int belle_sip_object_implements(belle_sip_object_t *obj, belle_sip_interface_id_t id){
	return belle_sip_object_get_interface_methods(obj,id)!=NULL;
}

void *belle_sip_object_interface_cast(belle_sip_object_t *obj, belle_sip_interface_id_t ifid, const char *castname, const char *file, int fileno){
	if (obj!=NULL){
		if (belle_sip_object_get_interface_methods(obj,ifid)==0){
			belle_sip_fatal("Bad cast to interface %s at %s:%i",castname,file,fileno);
			return NULL;
		}
	}
	return obj;
}

void belle_sip_object_set_name(belle_sip_object_t* object,const char* name) {
	if (object->name) {
		belle_sip_free(object->name);
		object->name=NULL;
	}
	if (name)
		object->name=belle_sip_strdup(name);
}

const char* belle_sip_object_get_name(belle_sip_object_t* object) {
	return object->name;
}

int belle_sip_object_marshal(belle_sip_object_t* obj, char* buff,unsigned int offset,size_t buff_size) {
	belle_sip_object_vptr_t *vptr=obj->vptr;
	while (vptr != NULL) {
		if (vptr->marshal != NULL) {
			return vptr->marshal(obj,buff,offset,buff_size);
		} else {
			vptr=vptr->parent;
		}
	}
	return -1; /*no implementation found*/
}

char* belle_sip_object_to_string(belle_sip_object_t* obj) {
	char buff[2048]; /*to be optimized*/
	int size = belle_sip_object_marshal(obj,buff,0,sizeof(buff));
	buff[size]='\0';
	return belle_sip_strdup(buff);

}

char * _belle_sip_object_describe_type(belle_sip_object_vptr_t *vptr){
	const int maxbufsize=2048;
	char *ret=belle_sip_malloc(maxbufsize);
	belle_sip_object_vptr_t *it;
	int pos=0;
	belle_sip_list_t *l=NULL,*elem;
	pos+=snprintf(ret+pos,maxbufsize-pos,"Ownership:\n");
	pos+=snprintf(ret+pos,maxbufsize-pos,"\t%s is created initially %s\n",vptr->type_name,
	              vptr->initially_unowned ? "unowned" : "owned");
	pos+=snprintf(ret+pos,maxbufsize-pos,"\nInheritance diagram:\n");
	for(it=vptr;it!=NULL;it=it->parent){
		l=belle_sip_list_prepend(l,it);
	}
	for(elem=l;elem!=NULL;elem=elem->next){
		it=(belle_sip_object_vptr_t*)elem->data;
		pos+=snprintf(ret+pos,maxbufsize-pos,"\t%s\n",it->type_name);
		if (elem->next)
			pos+=snprintf(ret+pos,maxbufsize-pos,"\t        |\n");
	}
	belle_sip_list_free(l);
	pos+=snprintf(ret+pos,maxbufsize-pos,"\nImplemented interfaces:\n");
	for(it=vptr;it!=NULL;it=it->parent){
		belle_sip_interface_desc_t **desc=it->interfaces;
		if (desc!=NULL){
			for(;*desc!=NULL;desc++){
				pos+=snprintf(ret+pos,maxbufsize-pos,"\t* %s\n",(*desc)->ifname);
			}
		}
	}
	return ret;
}

char *belle_sip_object_describe(void *obj){
	belle_sip_object_t *o=BELLE_SIP_OBJECT(obj);
	return _belle_sip_object_describe_type(o->vptr);
}

#include <dlfcn.h>

char *belle_sip_object_describe_type_from_name(const char *name){
	char *vptr_name;
	void *handle;
	void *symbol;
	
	handle=dlopen(NULL,RTLD_LAZY);
	if (handle==NULL){
		belle_sip_error("belle_sip_object_describe_type_from_name: dlopen() failed: %s",dlerror());
		return NULL;
	}
	vptr_name=belle_sip_strdup_printf("%s_vptr",name);
	symbol=dlsym(handle,vptr_name);
	belle_sip_free(vptr_name);
	dlclose(handle);
	if (symbol==NULL){
		belle_sip_error("belle_sip_object_describe_type_from_name: could not find vptr for type %s",name);
		return NULL;
	}
	return _belle_sip_object_describe_type((belle_sip_object_vptr_t*)symbol);
}