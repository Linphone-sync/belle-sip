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
	extern CAIN_SIP_OBJECT_VPTR_TYPE(object_type) CAIN_SIP_OBJECT_VPTR_NAME(object_type);

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

extern cain_sip_object_vptr_t cain_sip_object_t_vptr;		

struct _cain_sip_object{
	cain_sip_object_vptr_t *vptr;
	size_t size;
	int ref;
	char* name;
	struct weak_ref *weak_refs;
};


CAIN_SIP_BEGIN_DECLS


cain_sip_object_t * _cain_sip_object_new(size_t objsize, cain_sip_object_vptr_t *vptr);

#define cain_sip_object_new(_type) (_type*)_cain_sip_object_new(sizeof(_type),(cain_sip_object_vptr_t*)&CAIN_SIP_OBJECT_VPTR_NAME(_type))


int cain_sip_object_is_unowed(const cain_sip_object_t *obj);

/**
 * Increments reference counter, which prevents the object from being destroyed.
 * If the object is initially unowed, this acquires the first reference.
**/
cain_sip_object_t * cain_sip_object_ref(void *obj);

/**
 * Decrements the reference counter. When it drops to zero, the object is destroyed.
**/
void cain_sip_object_unref(void *obj);


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

cain_sip_object_t *cain_sip_object_clone(const cain_sip_object_t *obj);

/**
 * Delete the object: this function is intended for unowed object, that is objects
 * that were created with a 0 reference count. For all others, use cain_sip_object_unref().
**/
void cain_sip_object_delete(void *obj);

/**
 * Returns a string describing the inheritance diagram and implemented interfaces of object obj.
**/
char *cain_sip_object_describe(void *obj);

/**
 * Returns a string describing the inheritance diagram and implemented interfaces of an object given its type name.
**/
char *cain_sip_object_describe_type_from_name(const char *name);

void *cain_sip_object_cast(cain_sip_object_t *obj, cain_sip_type_id_t id, const char *castname, const char *file, int fileno);

char* cain_sip_object_to_string(cain_sip_object_t* obj);

int cain_sip_object_marshal(cain_sip_object_t* obj, char* buff,unsigned int offset,size_t buff_size);

int cain_sip_object_is_instance_of(cain_sip_object_t * obj,cain_sip_type_id_t id);

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

void *cain_sip_object_interface_cast(cain_sip_object_t *obj, cain_sip_interface_id_t id, const char *castname, const char *file, int fileno);

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




#endif

