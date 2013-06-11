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

static void _cain_sip_object_pool_remove_from_stack(cain_sip_object_pool_t *pool);

static int _cain_sip_object_marshal_check_enabled = FALSE;

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

void cain_sip_object_enable_marshal_check(int enable) {
	_cain_sip_object_marshal_check_enabled = (enable) ? TRUE : FALSE;
}

cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_object_vptr_t *vptr){
	cain_sip_object_t *obj=(cain_sip_object_t *)cain_sip_malloc0(objsize);
	obj->ref=vptr->initially_unowned ? 0 : 1;
	obj->vptr=vptr;
	obj->size=objsize;
	if (obj->ref==0){
		cain_sip_object_pool_t *pool=cain_sip_object_pool_get_current();
		if (pool) cain_sip_object_pool_add(pool,obj);
	}
	return obj;
}

int cain_sip_object_is_initially_unowned(const cain_sip_object_t *obj){
	return obj->vptr->initially_unowned;
}

cain_sip_object_t * cain_sip_object_ref(void *obj){
	cain_sip_object_t *o=CAIN_SIP_OBJECT(obj);
	if (o->ref==0 && o->pool){
		cain_sip_object_pool_remove(o->pool,obj);
	}
	o->ref++;
	return obj;
}

void cain_sip_object_unref(void *ptr){
	cain_sip_object_t *obj=CAIN_SIP_OBJECT(ptr);
	if (obj->ref==-1) cain_sip_fatal("Object with name [%s] freed twice !",obj->name);
	if (obj->ref==0 && obj->pool){
		cain_sip_object_pool_remove(obj->pool,obj);
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

static cain_sip_error_code _cain_object_marshal(cain_sip_object_t* obj, char* buff, size_t buff_size, size_t *offset) {
	return cain_sip_snprintf(buff,buff_size,offset,"{%s::%s %p}",obj->vptr->type_name,obj->name ? obj->name : "(no name)",obj);
}

cain_sip_object_vptr_t cain_sip_object_t_vptr={
	CAIN_SIP_TYPE_ID(cain_sip_object_t),
	"cain_sip_object_t",
	FALSE,
	NULL, /*no parent, it's god*/
	NULL,
	_cain_sip_object_uninit,
	_cain_sip_object_clone,
	_cain_object_marshal
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
	if (newobj->ref==0){
		cain_sip_object_pool_t *pool=cain_sip_object_pool_get_current();
		if (pool) cain_sip_object_pool_add(pool,newobj);
	}
	return newobj;
}

cain_sip_object_t *cain_sip_object_clone_and_ref(const cain_sip_object_t *obj) {
	return cain_sip_object_ref(cain_sip_object_clone(obj));
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

static cain_sip_error_code checked_marshal(cain_sip_object_vptr_t *vptr, cain_sip_object_t* obj, char* buff, size_t buff_size, size_t *offset){
	size_t tmp_buf_size=buff_size*2;
	char *p=(char*)cain_sip_malloc0(tmp_buf_size);
	size_t i;
	size_t initial_offset=*offset;
	cain_sip_error_code error=vptr->marshal(obj,p,buff_size,offset);
	size_t written;

	for (i=initial_offset;i<buff_size;++i){
		if (p[i]=='\0') break;
	}
	written=i-initial_offset;
	if (error==CAIN_SIP_BUFFER_OVERFLOW){
		cain_sip_error("Object of type %s commited a buffer overflow by marshalling %i bytes",
			vptr->type_name,*offset-initial_offset);
	} else if (error!=CAIN_SIP_OK){
		cain_sip_error("Object of type %s produced an error during marshalling: %i",
			vptr->type_name,error);
	}
	if (written!=(*offset-initial_offset) && written!=(buff_size-initial_offset-1)){ /*this is because snprintf won't allow you to write a non null character at the end of the buffer*/
		cain_sip_fatal("Object of type %s marshalled %i bytes but said it marshalled %i bytes !",
			vptr->type_name,written,*offset-initial_offset);
	}
	memcpy(buff+initial_offset,p+initial_offset,*offset-initial_offset);
	cain_sip_free(p);
	return error;
}

cain_sip_error_code cain_sip_object_marshal(cain_sip_object_t* obj, char* buff, size_t buff_size, size_t *offset) {
	cain_sip_object_vptr_t *vptr=obj->vptr;
	while (vptr != NULL) {
		if (vptr->marshal != NULL) {
			if (_cain_sip_object_marshal_check_enabled == TRUE)
				return checked_marshal(vptr,obj,buff,buff_size,offset);
			else
				return vptr->marshal(obj,buff,buff_size,offset);
		} else {
			vptr=vptr->parent;
		}
	}
	return CAIN_SIP_NOT_IMPLEMENTED; /*no implementation found*/
}

 
static char * cain_sip_object_to_alloc_string(cain_sip_object_t *obj, int size_hint){
	char *buf=cain_sip_malloc(size_hint);
	size_t offset=0;
	cain_sip_error_code error = cain_sip_object_marshal(obj,buf,size_hint-1,&offset);
	obj->vptr->tostring_bufsize_hint=size_hint;
	if (error==CAIN_SIP_BUFFER_OVERFLOW){
		cain_sip_message("cain_sip_object_to_alloc_string(): hint buffer was too short while doing to_string() for %s, retrying", obj->vptr->type_name);
		cain_sip_free(buf);
		return cain_sip_object_to_alloc_string(obj,2*size_hint);
	}
	buf=cain_sip_realloc(buf,offset+1);
	buf[offset]='\0';
	return buf;
}

static int get_hint_size(int size){
	if (size<128)
		return 128;
	return size;
}

char* cain_sip_object_to_string(void* _obj) {
	cain_sip_object_t *obj=CAIN_SIP_OBJECT(_obj);
	if (obj->vptr->tostring_bufsize_hint!=0){
		return cain_sip_object_to_alloc_string(obj,obj->vptr->tostring_bufsize_hint);
	}else{
		char buff[CAIN_SIP_MAX_TO_STRING_SIZE];
		size_t offset=0;
		cain_sip_error_code error = cain_sip_object_marshal(obj,buff,sizeof(buff),&offset);
		if (error==CAIN_SIP_BUFFER_OVERFLOW){
			cain_sip_message("cain_sip_object_to_string(): temporary buffer is too short while doing to_string() for %s, retrying", obj->vptr->type_name);
			return cain_sip_object_to_alloc_string(obj,get_hint_size(2*offset));
		}
		buff[offset]='\0';
		obj->vptr->tostring_bufsize_hint=get_hint_size(2*offset);
		return cain_sip_strdup(buff);
	}
}

char * _cain_sip_object_describe_type(cain_sip_object_vptr_t *vptr){
	const int maxbufsize=2048;
	char *ret=cain_sip_malloc(maxbufsize);
	cain_sip_object_vptr_t *it;
	unsigned int pos=0;
	cain_sip_list_t *l=NULL,*elem;
	cain_sip_snprintf(ret,maxbufsize,&pos,"Ownership:\n");
	cain_sip_snprintf(ret,maxbufsize,&pos,"\t%s is created initially %s\n",vptr->type_name,
	              vptr->initially_unowned ? "unowned" : "owned");
	cain_sip_snprintf(ret,maxbufsize,&pos,"\nInheritance diagram:\n");
	for(it=vptr;it!=NULL;it=it->parent){
		l=cain_sip_list_prepend(l,it);
	}
	for(elem=l;elem!=NULL;elem=elem->next){
		it=(cain_sip_object_vptr_t*)elem->data;
		cain_sip_snprintf(ret,maxbufsize,&pos,"\t%s\n",it->type_name);
		if (elem->next)
			cain_sip_snprintf(ret,maxbufsize,&pos,"\t        |\n");
	}
	cain_sip_list_free(l);
	cain_sip_snprintf(ret,maxbufsize,&pos,"\nImplemented interfaces:\n");
	for(it=vptr;it!=NULL;it=it->parent){
		cain_sip_interface_desc_t **desc=it->interfaces;
		if (desc!=NULL){
			for(;*desc!=NULL;desc++){
				cain_sip_snprintf(ret,maxbufsize,&pos,"\t* %s\n",(*desc)->ifname);
			}
		}
	}
	return ret;
}

char *cain_sip_object_describe(void *obj){
	cain_sip_object_t *o=CAIN_SIP_OBJECT(obj);
	return _cain_sip_object_describe_type(o->vptr);
}

#if !defined(WIN32)

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

#else

char *cain_sip_object_describe_type_from_name(const char *name){
	return cain_sip_strdup_printf("Sorry cain_sip_object_describe_type_from_name() is not implemented on this platform.");
}

#endif

struct cain_sip_object_pool{
	cain_sip_object_t base;
	cain_sip_list_t *objects;
	cain_sip_thread_t thread_id;
};

static void cain_sip_object_pool_destroy(cain_sip_object_pool_t *pool){
	cain_sip_object_pool_clean(pool);
	_cain_sip_object_pool_remove_from_stack(pool);
}

CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(cain_sip_object_pool_t);
CAIN_SIP_INSTANCIATE_VPTR(cain_sip_object_pool_t,cain_sip_object_t,cain_sip_object_pool_destroy,NULL,NULL,FALSE);

cain_sip_object_pool_t *cain_sip_object_pool_new(void){
	cain_sip_object_pool_t *pool=cain_sip_object_new(cain_sip_object_pool_t);
	pool->thread_id=cain_sip_thread_self();
	return pool;
}

void cain_sip_object_pool_add(cain_sip_object_pool_t *pool, cain_sip_object_t *obj){
	if (obj->pool!=NULL){
		cain_sip_fatal("It is not possible to add an object to multiple pools.");
	}
	pool->objects=cain_sip_list_prepend(pool->objects,obj);
	obj->pool_iterator=pool->objects;
	obj->pool=pool;
}

void cain_sip_object_pool_remove(cain_sip_object_pool_t *pool, cain_sip_object_t *obj){
	cain_sip_thread_t tid=cain_sip_thread_self();
	if (obj->pool!=pool){
		cain_sip_fatal("Attempting to remove object from an incorrect pool: obj->pool=%p, pool=%p",obj->pool,pool);
		return;
	}
	if (tid!=pool->thread_id){
		cain_sip_fatal("It is forbidden (and unsafe()) to ref()/unref() an unowned object outside of the thread that created it.");
		return;
	}
	pool->objects=cain_sip_list_delete_link(pool->objects,obj->pool_iterator);
	obj->pool_iterator=NULL;
	obj->pool=NULL;
}

int cain_sip_object_pool_cleanable(cain_sip_object_pool_t *pool){
	return pool->thread_id!=0 && cain_sip_thread_self()==pool->thread_id;
}

void cain_sip_object_pool_clean(cain_sip_object_pool_t *pool){
	cain_sip_list_t *elem,*next;
	
	if (!cain_sip_object_pool_cleanable(pool)){
		cain_sip_warning("Thread pool [%p] cannot be cleaned from thread [%lu] because it was created for thread [%lu]",
				 pool,(unsigned long)cain_sip_thread_self(),(unsigned long)pool->thread_id);
		return;
	}
	
	for(elem=pool->objects;elem!=NULL;elem=next){
		cain_sip_object_t *obj=(cain_sip_object_t*)elem->data;
		if (obj->ref==0){
			cain_sip_message("Garbage collecting unowned object of type %s",obj->vptr->type_name);
			obj->ref=-1;
			cain_sip_object_delete(obj);
			next=elem->next;
			cain_sip_free(elem);
		}else {
			cain_sip_fatal("Object %p is in unowned list but with ref count %i, bug.",obj,obj->ref);
			next=elem->next;
		}
	}
	pool->objects=NULL;
}

static void cain_sip_object_pool_detach_from_thread(cain_sip_object_pool_t *pool){
	cain_sip_object_pool_clean(pool);
	pool->thread_id=(cain_sip_thread_t)0;
}

static void cleanup_pool_stack(void *data){
	cain_sip_list_t **pool_stack=(cain_sip_list_t**)data;
	if (*pool_stack){
		/*
		 * We would expect the pool_stack to be empty when the thread terminates.
		 * Otherwise that means the management of object pool is not properly done by the application.
		 * Since the object pools might be still referenced by the application, we can't destroy them.
		 * Instead, we mark them as detached, so that when the thread that will attempt to destroy them will do it,
		 * we'll accept (since anyway these object pool are no longer needed.
		 */
		cain_sip_warning("There were still [%i] object pools for thread [%lu] while the thread exited. ",
				 cain_sip_list_size(*pool_stack),(unsigned long)cain_sip_thread_self());
		cain_sip_list_free_with_data(*pool_stack,(void (*)(void*)) cain_sip_object_pool_detach_from_thread);
	}
	*pool_stack=NULL;
	cain_sip_free(pool_stack);
}

static cain_sip_list_t** get_current_pool_stack(int *first_time){
	static cain_sip_thread_key_t pools_key;
	static int pools_key_created=0;
	cain_sip_list_t **pool_stack;
	
	if (first_time) *first_time=0;
	
	if (!pools_key_created){
		pools_key_created=1;
		if (cain_sip_thread_key_create(&pools_key, cleanup_pool_stack)!=0){
			return NULL;
		}
	}
	pool_stack=(cain_sip_list_t**)cain_sip_thread_getspecific(pools_key);
	if (pool_stack==NULL){
		pool_stack=cain_sip_new(cain_sip_list_t*);
		*pool_stack=NULL;
		cain_sip_thread_setspecific(pools_key,pool_stack);
		if (first_time) *first_time=1;
	}
	return pool_stack;
}

static void _cain_sip_object_pool_remove_from_stack(cain_sip_object_pool_t *pool){
	cain_sip_list_t **pools=get_current_pool_stack(NULL);
	cain_sip_thread_t tid=cain_sip_thread_self();
	
	if (tid!=pool->thread_id){
		cain_sip_fatal("It is forbidden to destroy a pool outside the thread that created it.");
		return;
	}
	
	if (pools==NULL) {
		cain_sip_fatal("Not possible to pop a pool.");
		return;
	}
	if (*pools==NULL){
		cain_sip_fatal("There is no current pool in stack.");
		return;
	}
	*pools=cain_sip_list_remove(*pools,pool);
}

cain_sip_object_pool_t * cain_sip_object_pool_push(void){
	cain_sip_list_t **pools=get_current_pool_stack(NULL);
	cain_sip_object_pool_t *pool;
	if (pools==NULL) {
		cain_sip_error("Not possible to create a pool.");
		return NULL;
	}
	pool=cain_sip_object_pool_new();
	*pools=cain_sip_list_prepend(*pools,pool);
	return pool;
}



cain_sip_object_pool_t *cain_sip_object_pool_get_current(void){
	int first_time;
	cain_sip_list_t **pools=get_current_pool_stack(&first_time);
	if (pools==NULL) return NULL;
	if (*pools==NULL ){
		if (first_time) {
			cain_sip_warning("There is no object pool created in thread [%lu]. "
			"Use cain_sip_object_pool_push() to create one. Unowned objects not unref'd will be leaked.",
			(unsigned long)cain_sip_thread_self());
		}
		return NULL;
	}
	return (cain_sip_object_pool_t*)(*pools)->data;
}



