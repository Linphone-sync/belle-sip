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

#ifndef HEADERS_H_
#define HEADERS_H_

#include "cain-sip/uri.h"
/***************************************************************************************
 * header address
 *
 **************************************************************************************/

typedef struct _cain_sip_header_address cain_sip_header_address_t;

cain_sip_header_address_t* cain_sip_header_address_new();

void cain_sip_header_address_delete(cain_sip_header_address_t* contact);

cain_sip_header_address_t* cain_sip_header_address_parse (const char* address) ;

cain_sip_header_address_t* cain_sip_header_address_ref (cain_sip_header_address_t* address) ;
void cain_sip_header_address_unref (cain_sip_header_address_t* address) ;

/**
 *
 */
cain_sip_uri_t* cain_sip_header_address_get_uri(cain_sip_header_address_t* address);
/**
 *
 */
void cain_sip_header_address_set_uri(cain_sip_header_address_t* address, cain_sip_uri_t* uri);

/**
 *
 */
const char* cain_sip_header_address_get_displayname(cain_sip_header_address_t* address);
/**
 *
 */
void cain_sip_header_address_set_displayname(cain_sip_header_address_t* address, const char* uri);


/***************************************************************************************
 * header common
 *
 **************************************************************************************/

typedef struct _cain_sip_header cain_sip_header_t;

/**
 * Contact header object
 */
typedef struct _cain_sip_header_contact cain_sip_header_contact_t;

cain_sip_header_contact_t* cain_sip_header_contact_new();

void cain_sip_header_contact_delete(cain_sip_header_contact_t* contact);

cain_sip_header_contact_t* cain_sip_header_contact_parse (const char* contact) ;


/**
* Returns the value of the expires parameter or -1 if no expires parameter was specified or if the parameter value cannot be parsed as an int.
*@returns value of the expires parameter measured in delta-seconds, O implies removal of Registration specified in Contact Header.
*
*/
 int	cain_sip_header_contact_get_expires(cain_sip_header_contact_t* contact);
/**
 * Returns the value of the q-value parameter of this ContactHeader. The q-value parameter indicates the relative preference amongst a set of locations. q-values are decimal numbers from 0 to 1, with higher values indicating higher preference.
 * @return the q-value parameter of this ContactHeader, -1 if the q-value is not set.
 */
 float	cain_sip_header_contact_get_qvalue(cain_sip_header_contact_t* contact);
 /**
  * Returns a boolean value that indicates if the contact header has the format of Contact: *.
  * @return true if this is a wildcard address, false otherwise.
  */
 unsigned int cain_sip_header_contact_is_wildcard(cain_sip_header_contact_t* contact);
 /**
 *
 */
 int cain_sip_header_contact_set_expires(cain_sip_header_contact_t* contact, int expires);
/**
 *  Sets the qValue value of the Name Address.
 */
 int cain_sip_header_contact_set_qvalue(cain_sip_header_contact_t* contact, float qvalue);
/**
 * Sets a wildcard on this contact address that is "*" is assigned to the contact header so that the header will have the format of Contact: *.
 *
 */
 void cain_sip_header_contact_set_wildcard(cain_sip_header_contact_t* contact,unsigned int is_wildcard);

 /**
 * From header object inherent from header_address
 *
 */
 typedef struct _cain_sip_header_from cain_sip_header_from_t;

 cain_sip_header_from_t* cain_sip_header_from_new();

 void cain_sip_header_from_delete(cain_sip_header_from_t* from);

 cain_sip_header_from_t* cain_sip_header_from_parse (const char* from) ;

 cain_sip_header_from_t* cain_sip_header_from_ref (cain_sip_header_from_t* from) ;

 void cain_sip_header_from_unref (cain_sip_header_from_t* from) ;

 void cain_sip_header_from_set_tag(cain_sip_header_from_t* from, const char* tag);

 const char* cain_sip_header_from_get_tag(cain_sip_header_from_t* from);




#endif /* HEADERS_H_ */
