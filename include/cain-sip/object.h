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

#ifndef cain_sip_object_h
#define cain_sip_object_h

#include "cain-sip/defs.h"

/*
 * typedefs, macros and functions for object definition and manipulation.
 */

#define CAIN_SIP_TYPE_ID(_type) _type##_id

typedef unsigned int cain_sip_type_id_t;

#define CAIN_SIP_DECLARE_TYPES_BEGIN(namezpace,unique_namespace_id) \
	enum namezpace##type_ids{\
		namezpace##type_id_first=unique_namespace_id,

#define CAIN_SIP_DECLARE_TYPES_END };

#define CAIN_SIP_OBJECT_VPTR_NAME(object_type)	object_type##_vptr

#define CAIN_SIP_OBJECT_VPTR_TYPE(object_type)	object_type##_vptr_t

#define CAIN_SIP_DECLARE_VPTR(object_type) \
	typedef cain_sip_object_vptr_t CAIN_SIP_OBJECT_VPTR_TYPE(object_type);\
	CAINSIP_VAR_EXPORT CAIN_SIP_OBJECT_VPTR_TYPE(object_type) CAIN_SIP_OBJECT_VPTR_NAME(object_type);

#define CAIN_SIP_DECLARE_CUSTOM_VPTR_BEGIN(object_type, parent_type) \
	typedef struct object_type##_vptr_struct CAIN_SIP_OBJECT_VPTR_TYPE(object_type);\
	extern CAIN_SIP_OBJECT_VPTR_TYPE(object_type) CAIN_SIP_OBJECT_VPTR_NAME(object_type);\
	struct object_type##_vptr_struct{\
		CAIN_SIP_OBJECT_VPTR_TYPE(parent_type) base;

#define CAIN_SIP_DECLARE_CUSTOM_VPTR_END };

#define CAIN_SIP_INSTANCIATE_CUSTOM_VPTR(object_type) \
	CAIN_SIP_OBJECT_VPTR_TYPE(object_type) CAIN_SIP_OBJECT_VPTR_NAME(object_type)


#define CAIN_SIP_VPTR_INIT(object_type,parent_type,unowned) \
		CAIN_SIP_TYPE_ID(object_type), \
		#object_type,\
		unowned,\
		(cain_sip_object_vptr_t*)&CAIN_SIP_OBJECT_VPTR_NAME(parent_type), \
		(cain_sip_interface_desc_t**)object_type##interfaces_table


#define CAIN_SIP_INSTANCIATE_VPTR(object_type,parent_type,destroy,clone,marshal,unowned) \
		CAIN_SIP_OBJECT_VPTR_TYPE(object_type) CAIN_SIP_OBJECT_VPTR_NAME(object_type)={ \
		CAIN_SIP_VPTR_INIT(object_type,parent_type,unowned), \
		(cain_sip_object_destroy_t)destroy,	\
		(cain_sip_object_clone_t)clone,	\
		(cain_sip_object_marshal_t)marshal\
		}

/**
 * cain_sip_object_t is the base object.
 * It is the base class for all cain sip non trivial objects.
 * It owns a reference count which allows to trigger the destruction of the object when the last
 * user of it calls cain_sip_object_unref().
 * 
 * About object lifecycle<br>
 * In cain-sip, objects can be, depending on their types, initially owned, that there are created with a ref count of 1, or
 * initially unowned, that is with reference count of 0. Such objets are also referred as "floating object". They are automatically destroyed
 * by the main loop iteration, so a floating object can be seen as a temporary object, until someones calls cain_sip_object_ref() on it.
 * 
 * In order to know whether a kind of object is initially owned or initially unowned, you can use the test program tester/cain_sip_object_destribe.
 * This tool gives the hierarchy and properties of the object type whose name is supplied in argument. For example:
 * 
 * <pre>./tester/cain_sip_object_describe cain_sip_request_t</pre>
 * 
 * The object memory management depends slightly on whether an object type is created initially owned or not.
 * In order not to be lost and make memory fault or leaks, consider the following rules:
 * 
 * When an object is of type initially unowned:
 * * call cain_sip_object_ref() on it only if you need a pointer to this object to be used outside the scope of the current function.
 * * call cain_sip_object_unref() on it only if you previously called cain_sip_object_ref().
 * 
 * When an object is of type initially owned:
 * * you can safely store its pointer.
 * * use cain_sip_object_unref() when you no longer need it.
 * 
 * Also, keep in mind that most objects of cain-sip are initially unowned, especially 
 * * all objects who are usually required to be used inside another object (for example: an URI is part of a from header, a contact header is part of a message)
 * * all objects whose lifecyle is maintained by the stack: transactions, dialogs.
 * 
 * On the contrary, top level objects whose lifecyle belongs only to the application are initially owned:
 * * cain_sip_provider_t, cain_sip_stack_t, cain_sip_source_t.
 * 
 * Internally, cain-sip objects containing pointers to other objects must take a reference count on the other objects they hold; and leave this reference
 * when they no longer need it. This rule must be strictly followed by developers doing things inside cain-sip.
**/
typedef struct _cain_sip_object cain_sip_object_t;
		

typedef void (*cain_sip_object_destroy_t)(cain_sip_object_t*);
typedef void (*cain_sip_object_clone_t)(cain_sip_object_t* obj, const cain_sip_object_t *orig);
typedef int (*cain_sip_object_marshal_t)(cain_sip_object_t* obj, char* buff,unsigned int offset,size_t buff_size);

struct _cain_sip_object_vptr{
	cain_sip_type_id_t id;
	const char *type_name;
	int initially_unowned;
	struct _cain_sip_object_vptr *parent;
	struct cain_sip_interface_desc **interfaces; /*NULL terminated table of */
	cain_sip_object_destroy_t destroy;
	cain_sip_object_clone_t clone;
	cain_sip_object_marshal_t marshal;
};

typedef struct _cain_sip_object_vptr cain_sip_object_vptr_t;

struct _cain_sip_object{
	cain_sip_object_vptr_t *vptr;
	size_t size;
	int ref;
	char* name;
	struct weak_ref *weak_refs;
	struct cain_sip_object_pool *pool;
	struct _cain_sip_list *pool_iterator;
};


CAIN_SIP_BEGIN_DECLS

CAINSIP_VAR_EXPORT cain_sip_object_vptr_t cain_sip_object_t_vptr;		


CAINSIP_EXPORT cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_object_vptr_t *vptr);

#define cain_sip_object_new(_type) (_type*)_cain_sip_object_new(sizeof(_type),(cain_sip_object_vptr_t*)&CAIN_SIP_OBJECT_VPTR_NAME(_type))


int cain_sip_object_is_unowed(const cain_sip_object_t *obj);

/**
 * Increments reference counter, which prevents the object from being destroyed.
 * If the object is initially unowed, this acquires the first reference.
 * 
**/
CAINSIP_EXPORT cain_sip_object_t * cain_sip_object_ref(void *obj);

/*#define CAIN_SIP_REF(object,type) (type*)cain_sip_object_ref(object);*/
/**
 * Decrements the reference counter. When it drops to zero, the object is destroyed.
**/
CAINSIP_EXPORT void cain_sip_object_unref(void *obj);


typedef void (*cain_sip_object_destroy_notify_t)(void *userpointer, cain_sip_object_t *obj_being_destroyed);
/**
 * Add a weak reference to object.
 * When object will be destroyed, then the destroy_notify callback will be called.
 * This allows another object to be informed when object is destroyed, and then possibly
 * cleanups pointer it holds to this object.
**/
cain_sip_object_t *cain_sip_object_weak_ref(void *obj, cain_sip_object_destroy_notify_t destroy_notify, void *userpointer);

/**
 * Remove a weak reference to object.
**/
void cain_sip_object_weak_unref(void *obj, cain_sip_object_destroy_notify_t destroy_notify, void *userpointer);

/**
 * Set object name.
**/
void cain_sip_object_set_name(cain_sip_object_t *obj,const char* name);
/**
 * Get object name.
**/
const char* cain_sip_object_get_name(cain_sip_object_t *obj);

/*copy the content of ref object to new object, for the part they have in common in their inheritence diagram*/
void _cain_sip_object_copy(cain_sip_object_t *newobj, const cain_sip_object_t *ref);

/**
 * Clone an object.
 * 
 * This clone function makes a deep copy of all object internal structure, so that the new object and the reference object have no dependencies at all.
 * 
**/
CAINSIP_EXPORT cain_sip_object_t *cain_sip_object_clone(const cain_sip_object_t *obj);

/**
 * Same as #cain_sip_object_clone but with ref count set to 1
 *
**/
cain_sip_object_t *cain_sip_object_clone_and_ref(const cain_sip_object_t *obj);


/**
 * Returns a string describing the inheritance diagram and implemented interfaces of object obj.
**/
char *cain_sip_object_describe(void *obj);

/**
 * Returns a string describing the inheritance diagram and implemented interfaces of an object given its type name.
**/
char *cain_sip_object_describe_type_from_name(const char *name);

CAINSIP_EXPORT void *cain_sip_object_cast(cain_sip_object_t *obj, cain_sip_type_id_t id, const char *castname, const char *file, int fileno);

/**
 * Returns a newly allocated string representing the object.
 * WHen the object is a sip header, uri or message, this is the textual representation of the header, uri or message.
 * This function internally calls cain_sip_object_marshal().
**/
CAINSIP_EXPORT char* cain_sip_object_to_string(cain_sip_object_t* obj);

/**
 * Writes a string representation of the object into the supplied buffer.
 * Same as cain_sip_object_to_string(), but without allocating space for the output string.
**/
CAINSIP_EXPORT int cain_sip_object_marshal(cain_sip_object_t* obj, char* buff,unsigned int offset,size_t buff_size);

CAINSIP_EXPORT int cain_sip_object_is_instance_of(cain_sip_object_t * obj,cain_sip_type_id_t id);

CAIN_SIP_END_DECLS

#define CAIN_SIP_CAST(obj,_type) 		((_type*)cain_sip_object_cast((cain_sip_object_t *)(obj), _type##_id, #_type, __FILE__, __LINE__))

#define CAIN_SIP_OBJECT(obj) CAIN_SIP_CAST(obj,cain_sip_object_t)
#define CAIN_SIP_IS_INSTANCE_OF(obj,_type) cain_sip_object_is_instance_of((cain_sip_object_t*)obj,_type##_id)
#define CAIN_SIP_OBJECT_IS_INSTANCE_OF(obj,_type)	CAIN_SIP_IS_INSTANCE_OF(obj,_type)
#define cain_sip_object_describe_type(type) \
	cain_sip_object_describe_type_from_name(#type)

/*
 * typedefs, macros and functions for interface definition and manipulation.
 */

#define CAIN_SIP_INTERFACE_ID(_interface) _interface##_id

typedef unsigned int cain_sip_interface_id_t;

CAIN_SIP_BEGIN_DECLS

CAINSIP_EXPORT void *cain_sip_object_interface_cast(cain_sip_object_t *obj, cain_sip_interface_id_t id, const char *castname, const char *file, int fileno);

int cain_sip_object_implements(cain_sip_object_t *obj, cain_sip_interface_id_t id);

CAIN_SIP_END_DECLS


#define CAIN_SIP_INTERFACE_METHODS_TYPE(interface_name) methods_##interface_name

#define CAIN_SIP_INTERFACE_CAST(obj,_iface) ((_iface*)cain_sip_object_interface_cast((cain_sip_object_t*)(obj),_iface##_id,#_iface,__FILE__,__LINE__))

#define CAIN_SIP_IMPLEMENTS(obj,_iface)		cain_sip_object_implements((cain_sip_object_t*)obj,_iface##_id)


typedef struct cain_sip_interface_desc{
	cain_sip_interface_id_t id;
	const char *ifname;
}cain_sip_interface_desc_t;

#define CAIN_SIP_DECLARE_INTERFACE_BEGIN(interface_name) \
	typedef struct struct##interface_name interface_name;\
	typedef struct struct_methods_##interface_name CAIN_SIP_INTERFACE_METHODS_TYPE(interface_name);\
	struct struct_methods_##interface_name {\
		cain_sip_interface_desc_t desc;\
		

#define CAIN_SIP_DECLARE_INTERFACE_END };

#define CAIN_SIP_IMPLEMENT_INTERFACE_BEGIN(object_type,interface_name) \
	static CAIN_SIP_INTERFACE_METHODS_TYPE(interface_name)  methods_##object_type##_##interface_name={\
		{ CAIN_SIP_INTERFACE_ID(interface_name),\
		#interface_name },

#define CAIN_SIP_IMPLEMENT_INTERFACE_END };

#define CAIN_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(object_type)\
	static cain_sip_interface_desc_t * object_type##interfaces_table[]={\
		NULL \
	}

#define CAIN_SIP_DECLARE_IMPLEMENTED_INTERFACES_1(object_type,iface1) \
	static cain_sip_interface_desc_t * object_type##interfaces_table[]={\
		(cain_sip_interface_desc_t*)&methods_##object_type##_##iface1, \
		NULL \
	}

#define CAIN_SIP_DECLARE_IMPLEMENTED_INTERFACES_2(object_type,iface1,iface2) \
	static cain_sip_interface_desc_t * object_type##interfaces_table[]={\
		(cain_sip_interface_desc_t*)&methods_##object_type##_##iface1, \
		(cain_sip_interface_desc_t*)&methods_##object_type##_##iface2, \
		NULL \
	}


/**
 * Object holding unowned objects - used as a kind of garbage collector for temporary objects.
**/
typedef struct cain_sip_object_pool cain_sip_object_pool_t;

CAIN_SIP_BEGIN_DECLS

/**
 * Push a new object pool for use for creation of new objects.
 * When no longer needed, this pool can be destroyed with cain_sip_object_unref().
**/
CAINSIP_EXPORT cain_sip_object_pool_t * cain_sip_object_pool_push(void);

cain_sip_object_pool_t * cain_sip_object_pool_get_current();
int cain_sip_object_pool_cleanable(cain_sip_object_pool_t *pool);
void cain_sip_object_pool_clean(cain_sip_object_pool_t *obj);

CAIN_SIP_END_DECLS

#endif

