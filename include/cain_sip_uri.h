/*
 * SipUri.h
 *
 *  Created on: 18 sept. 2010
 *      Author: jehanmonnier
 */

#ifndef CAIN_SIP_URI_H_
#define CAIN_SIP_URI_H_



typedef struct _cain_sip_uri cain_sip_uri;

	cain_sip_uri* cain_sip_uri_new();

	void cain_sip_uri_delete(cain_sip_uri* uri);
	cain_sip_uri* cain_sip_uri_parse (const char* uri) ;

	/**
	 *	Returns the value of the named header, or null if it is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_header(cain_sip_uri* uri,const char* name);
			/**
			 * Returns an Iterator over the const char*names of all headers present in this SipURI.
			 *
			 */
	 /*list<const char*>::iterator	cain_sip_uri_get_header_names(SipUri* uri) ;*/
	/**
	 * 	          Returns the host part of this SipURI.
	 *
	 */
	 const char*	cain_sip_uri_get_host(cain_sip_uri* uri) ;
	/**
	 * 	          Returns the value of the maddr parameter, or null if this is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_maddr_param(cain_sip_uri* uri) ;
	/**
	 *	          Returns the value of the method parameter, or null if this is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_method_param(cain_sip_uri* uri) ;
	/**
	 *	          Returns the port part of this SipURI.
	 *
	 */
	 unsigned int	cain_sip_uri_get_port(cain_sip_uri* uri) ;
	/**
	 * 	          Returns the value of the "transport" parameter, or null if this is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_transport_param(cain_sip_uri* uri) ;
	/**
	 * 	          Returns the value of the "ttl" parameter, or -1 if this is not set.
	 *
	 */
	 int	cain_sip_uri_get_ttl_param(cain_sip_uri* uri) ;
	/**
	 * 	          Returns the user part of this SipURI.
	 *
	 */
	 const char*	cain_sip_uri_get_user(cain_sip_uri* uri) ;
	/**
	 * 	          Returns the value of the userParam, or null if this is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_user_param(cain_sip_uri* uri) ;
	/**
	 * 	          Gets user password of SipURI, or null if it is not set.
	 *
	 */
	 const char*	cain_sip_uri_get_user_password(cain_sip_uri* uri) ;
	/**
	 *	          Returns whether the the lr parameter is set.
	 *
	 */
	 unsigned int	cain_sip_uri_has_lr_param(cain_sip_uri* uri) ;
	/**
	 *
	 * 	          Returns true if this SipURI is secure i.e. if this SipURI represents a sips URI.
	 *
	 */
	 unsigned int	cain_sip_uri_is_secure(cain_sip_uri* uri) ;
	/**
	 * 	          Removes the port part of this SipURI.
	 *
	 */
	 void	cain_sip_uri_remove_port(cain_sip_uri* uri) ;
	/**
	 * 	          Sets the value of the specified header fields to be included in a request constructed from the URI.
	 *
	 */
	 void	cain_sip_uri_set_header(cain_sip_uri* uri,const char*name, const char*value) ;
	/**
	 * 	          Set the host part of this SipURI to the newly supplied host parameter.
	 *
	 */
	 void	cain_sip_uri_set_host(cain_sip_uri* uri,const char*host) ;
	/**
	 * 	          Sets the value of the lr parameter of this SipURI.
	 *
	 */
	 void	cain_sip_uri_set_lr_param(cain_sip_uri* uri,unsigned int param) ;
	/**
	 *          Sets the value of the maddr parameter of this SipURI.
	 *
	 */
	 void	cain_sip_uri_set_maddr_param(cain_sip_uri* uri,const char*mAddr) ;
	/**
	 *	          Sets the value of the method parameter.
	 *
	 */
	 void	cain_sip_uri_set_method_param(cain_sip_uri* uri,const char*method) ;
	/**
	 * 	          Set the port part of this SipURI to the newly supplied port parameter.
	 *
	 */
	 void	cain_sip_uri_set_port(cain_sip_uri* uri,unsigned int port) ;
	/**
	 * 	          Sets the scheme of this URI to sip or sips depending on whether the argument is true or false.
	 *
	 */
	 void	cain_sip_uri_set_secure(cain_sip_uri* uri,unsigned int secure) ;
	/**
	 * 	          Sets the value of the "transport" parameter.
	 *
	 */
	 void	cain_sip_uri_set_transport_param(cain_sip_uri* uri,const char*transport) ;
	/**
	 *  	          Sets the value of the ttl parameter.
	 *
	 */
	 void	cain_sip_uri_set_ttl_param(cain_sip_uri* uri,int ttl) ;
	/**
	 *  	          Sets the user of SipURI.
	 *
	 */
	 void	cain_sip_uri_set_user(cain_sip_uri* uri,const char*user) ;
	/**
	 * 	          Sets the value of the user parameter.
	 *
	 */
	 void	cain_sip_uri_set_user_param(cain_sip_uri* uri,const char*userParam) ;
	/**
	 * 	          Sets the user password associated with the user of SipURI.
	 *
	 */
	 void	cain_sip_uri_set_user_password(cain_sip_uri* uri,const char*userPassword) ;
	/**
	 * 	          This method returns the URI as a string.
	 *
	 */
	 char*	cain_sip_uri_to_string(cain_sip_uri* uri) ;


#endif  /*CAIN_SIP_URI_H_*/

