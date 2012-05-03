/*
 * SipUri.h
 *
 *  Created on: 18 sept. 2010
 *      Author: jehanmonnier
 */

#ifndef CAIN_SIP_URI_H_
#define CAIN_SIP_URI_H_


#include "cain-sip/list.h"
/*inherite from cain_sip_parameters_t*/
typedef struct _cain_sip_uri cain_sip_uri_t;

	/**
	 *
	 */
	cain_sip_uri_t* cain_sip_uri_new();
	/**
	 *
	 */
	void cain_sip_uri_delete(cain_sip_uri_t* uri);
	/**
	 *
	 */
	cain_sip_uri_t* cain_sip_uri_parse (const char* uri) ;
	
	/**
	 *	Returns the value of the named header, or null if it is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_header(const cain_sip_uri_t* uri,const char* name);
			/**
			 * Returns an Iterator over the const char*names of all headers present in this SipURI.
			 *
			 */
	 const cain_sip_list_t*	cain_sip_uri_get_header_names(const cain_sip_uri_t* uri) ;
	/**
	 * 	          Returns the host part of this SipURI.
	 *
	 */
	 const char*	cain_sip_uri_get_host(const cain_sip_uri_t* uri) ;
	/**
	 * 	          Returns the value of the maddr parameter, or null if this is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_maddr_param(const cain_sip_uri_t* uri) ;
	/**
	 *	          Returns the value of the method parameter, or null if this is not set.
	 *
	 */
	const char*	cain_sip_uri_get_method_param(const cain_sip_uri_t* uri) ;
	/**
	 *	          Returns the port part of this SipURI.
	 *
	 */
	unsigned int	cain_sip_uri_get_port(const cain_sip_uri_t* uri) ;
	/**
 	 * Returns the port of the uri, if not specified in the uri returns the well known port according to the transport.
 	**/
	int cain_sip_uri_get_listening_port(const cain_sip_uri_t *uri);
	/**
	 * 	          Returns the value of the "transport" parameter, or null if this is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_transport_param(const cain_sip_uri_t* uri) ;
	/**
	 * 	          Returns the value of the "ttl" parameter, or -1 if this is not set.
	 *
	 */
	 int	cain_sip_uri_get_ttl_param(const cain_sip_uri_t* uri) ;
	/**
	 * 	          Returns the user part of this SipURI.
	 *
	 */
	 const char*	cain_sip_uri_get_user(const cain_sip_uri_t* uri) ;
	/**
	 * 	          Returns the value of the userParam, or null if this is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_user_param(const cain_sip_uri_t* uri) ;
	/**
	 * 	          Gets user password of SipURI, or null if it is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_user_password(const cain_sip_uri_t* uri) ;
	/**
	 *	          Returns whether the the lr parameter is set.
	 *
	 */
	 unsigned int	cain_sip_uri_has_lr_param(const cain_sip_uri_t* uri) ;
	/**
	 *
	 * 	          Returns true if this SipURI is secure i.e. if this SipURI represents a sips URI.
	 *
	 */
	 unsigned int	cain_sip_uri_is_secure(const cain_sip_uri_t* uri) ;
	/**
	 * 	          Removes the port part of this SipURI.
	 *
	 */
	 void	cain_sip_uri_remove_port(cain_sip_uri_t* uri) ;
	/**
	 * 	          Sets the value of the specified header fields to be included in a request constructed from the URI.
	 *
	 */
	 void	cain_sip_uri_set_header(cain_sip_uri_t* uri,const char*name, const char*value) ;
	/**
	 * 	          Set the host part of this SipURI to the newly supplied host parameter.
	 *
	 */
	 void	cain_sip_uri_set_host(cain_sip_uri_t* uri,const char*host) ;
	/**
	 * 	          Sets the value of the lr parameter of this SipURI.
	 *
	 */
	 void	cain_sip_uri_set_lr_param(cain_sip_uri_t* uri,unsigned int param) ;
	/**
	 *          Sets the value of the maddr parameter of this SipURI.
	 *
	 */
	 void	cain_sip_uri_set_maddr_param(cain_sip_uri_t* uri,const char*mAddr) ;
	/**
	 *	          Sets the value of the method parameter.
	 *
	 */
	 void	cain_sip_uri_set_method_param(cain_sip_uri_t* uri,const char*method) ;
	/**
	 * 	          Set the port part of this SipURI to the newly supplied port parameter.
	 *
	 */
	 void	cain_sip_uri_set_port(cain_sip_uri_t* uri,unsigned int port) ;
	/**
	 * 	          Sets the scheme of this URI to sip or sips depending on whether the argument is true or false.
	 *
	 */
	 void	cain_sip_uri_set_secure(cain_sip_uri_t* uri,unsigned int secure) ;
	/**
	 * 	          Sets the value of the "transport" parameter.
	 *
	 */
	 void	cain_sip_uri_set_transport_param(cain_sip_uri_t* uri,const char*transport) ;
	/**
	 *  	          Sets the value of the ttl parameter.
	 *
	 */
	 void	cain_sip_uri_set_ttl_param(cain_sip_uri_t* uri,int ttl) ;
	/**
	 *  	          Sets the user of SipURI.
	 *
	 */
	 void	cain_sip_uri_set_user(cain_sip_uri_t* uri,const char*user) ;
	/**
	 * 	          Sets the value of the user parameter.
	 *
	 */
	 void	cain_sip_uri_set_user_param(cain_sip_uri_t* uri,const char*userParam) ;
	/**
	 * 	          Sets the user password associated with the user of SipURI.
	 *
	 */
	 void	cain_sip_uri_set_user_password(cain_sip_uri_t* uri,const char*userPassword) ;
	/**
	 * 	          This method returns the URI as a string.
	 *
	 */
	 char*	cain_sip_uri_to_string(cain_sip_uri_t* uri) ;

	 int cain_sip_uri_marshal(const cain_sip_uri_t* uri, char* buff,unsigned int offset,unsigned int buff_size);

	#define CAIN_SIP_URI(obj) CAIN_SIP_CAST(obj,cain_sip_uri_t)

	 /**define URI equality as using comparison rules from RFC3261 section 19.1.4
	  * @param cain_sip_uri_t* uri_a
	  * @param cain_sip_uri_t* uri_a
	  * @return 0 if not matched.
	  *
	  * */
	 int cain_sip_uri_equals(const cain_sip_uri_t* uri_a,const cain_sip_uri_t* uri_b);

#endif  /*CAIN_SIP_URI_H_*/

