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


#ifndef CAIN_SIP_TRANSPORT_H
#define CAIN_SIP_TRANSPORT_H

CAIN_SIP_BEGIN_DECLS

CAINSIP_EXPORT const char *cain_sip_listening_point_get_ip_address(const cain_sip_listening_point_t *lp);
CAINSIP_EXPORT int cain_sip_listening_point_get_port(const cain_sip_listening_point_t *lp);
CAINSIP_EXPORT const char *cain_sip_listening_point_get_transport(const cain_sip_listening_point_t *lp);
CAINSIP_EXPORT const char *cain_sip_listening_point_get_ip_address(const  cain_sip_listening_point_t *lp);
/*
 * set keep alive frequency in ms
 * @param lp object
 * @param ms keep alive period in ms. Values <=0 disable keep alive
 * */
CAINSIP_EXPORT void cain_sip_listening_point_set_keep_alive(cain_sip_listening_point_t *lp,int ms);

/*
 * get keep alive frequency in ms
 * @param lp object
 * @return  keep alive period in ms. Values <=0 disable keep alive
 * */
CAINSIP_EXPORT int cain_sip_listening_point_get_keep_alive(const cain_sip_listening_point_t *lp);



/**
 * get the listening information as an URI
 * @return IP/port/transport as an URI
 */
CAINSIP_EXPORT const cain_sip_uri_t* cain_sip_listening_point_get_uri(const  cain_sip_listening_point_t *ip);
CAINSIP_EXPORT int cain_sip_listening_point_is_reliable(const cain_sip_listening_point_t *lp);
/**
 * Clean (close) all channels (connection) managed by this listening point.
**/
CAINSIP_EXPORT void cain_sip_listening_point_clean_channels(cain_sip_listening_point_t *lp);

/**
 * Get the number of channels managed by this listening point.
**/
CAINSIP_EXPORT int cain_sip_listening_point_get_channel_count(const cain_sip_listening_point_t *lp);
CAINSIP_EXPORT int cain_sip_listening_point_get_well_known_port(const char *transport);

CAINSIP_EXPORT int cain_sip_tls_listening_point_set_root_ca(cain_sip_tls_listening_point_t *s, const char *path);

#define CAIN_SIP_TLS_LISTENING_POINT_BADCERT_CN_MISMATCH 	(1)
#define CAIN_SIP_TLS_LISTENING_POINT_BADCERT_ANY_REASON 	(0xff)
CAINSIP_EXPORT int cain_sip_tls_listening_point_set_verify_exceptions(cain_sip_tls_listening_point_t *s, int flags);



#define CAIN_SIP_UDP_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_udp_listening_point_t)
#define CAIN_SIP_STREAM_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_stream_listening_point_t)
#define CAIN_SIP_TLS_LISTENING_POINT(obj) CAIN_SIP_CAST(obj,cain_sip_tls_listening_point_t)

CAIN_SIP_END_DECLS


#endif

