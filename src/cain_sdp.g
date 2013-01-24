/*
    cain-sdp - SDP (RFC4566) library.
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
grammar cain_sdp;
 
 
options {	
	language = C;
	output=AST;
	
} 
@header {
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
}
@includes { 
#include "cain-sip/cain-sdp.h"
#include "cain_sip_internal.h"
}

session_description returns [cain_sdp_session_description_t* ret]     
scope { cain_sdp_session_description_t* current; }
@init {$session_description::current = cain_sdp_session_description_new(); $ret=$session_description::current; }
                    :    version CR LF
                         origin {cain_sdp_session_description_set_origin($session_description::current,$origin.ret);}CR LF
                         session_name CR LF
                         (info CR LF)?
                         (uri_field CR LF)?
                         (email CR LF)*
                         phone_field*
                         (connection {cain_sdp_session_description_set_connection($session_description::current,$connection.ret);} CR LF)?
                         (bandwidth {cain_sdp_session_description_add_bandwidth($session_description::current,$bandwidth.ret);} CR LF)*
                         time_field CR LF
                         (repeat_time CR LF)?
                         (zone_adjustments CR LF)?
                         (key_field CR LF)?
                         (attribute {cain_sdp_session_description_add_attribute($session_description::current,$attribute.ret);} CR LF)*
                         (media_description {cain_sdp_session_description_add_media_description($session_description::current,$media_description.ret);}) *;

version:       {IS_TOKEN(v)}?alpha_num EQUAL v=DIGIT+  {cain_sdp_version_t* version =cain_sdp_version_new();
                                                        cain_sdp_version_set_version(version,atoi((const char*)$v.text->chars));
                                                        cain_sdp_session_description_set_version($session_description::current,version);};
                       //  ;this memo describes version 0

origin returns [cain_sdp_origin_t* ret]     
scope { cain_sdp_origin_t* current; }
@init {$origin::current = cain_sdp_origin_new(); $ret=$origin::current; }
:        {IS_TOKEN(o)}?alpha_num EQUAL username {cain_sdp_origin_set_username($origin::current,(const char*)$username.text->chars);} 
                         SPACE sess_id {cain_sdp_origin_set_session_id($origin::current,atoi((const char*)$sess_id.text->chars));}
                         SPACE sess_version {cain_sdp_origin_set_session_version($origin::current,atoi((const char*)$sess_version.text->chars));}
                         SPACE nettype {cain_sdp_origin_set_network_type($origin::current,(const char*)$nettype.text->chars);} 
                         SPACE addrtype  {cain_sdp_origin_set_address_type($origin::current,(const char*)$addrtype.text->chars);} 
                         SPACE addr {cain_sdp_origin_set_address($origin::current,(const char*)$addr.text->chars);} ;

session_name:  {IS_TOKEN(s)}? alpha_num EQUAL text {cain_sdp_session_name_t* session_name =cain_sdp_session_name_new();
                                                        cain_sdp_session_name_set_value(session_name,(const char*)$text.text->chars);
                                                        cain_sdp_session_description_set_session_name($session_description::current,session_name);};

info returns [cain_sdp_info_t* ret]     
scope { cain_sdp_info_t* current; }
@init {$info::current = cain_sdp_info_new(); $ret=$info::current; }
:   {IS_TOKEN(i)}? alpha_num EQUAL info_value {cain_sdp_info_set_value($info::current,(const char*) $info_value.text->chars);} ;

info_value            options { greedy = false; }:        ~(CR|LF)*;

uri_field:           {IS_TOKEN(u)}?alpha_num EQUAL uri ;

email returns [cain_sdp_email_t* ret]     
scope { cain_sdp_email_t* current; }
@init {$email::current = cain_sdp_email_new(); $ret=$email::current; }
  :        {IS_TOKEN(e)}?alpha_num EQUAL email_address {cain_sdp_email_set_value($email::current,(const char*)$email_address.text->chars);};

phone_field:        {IS_TOKEN(p)}?alpha_num EQUAL phone_number CR LF;

connection returns [cain_sdp_connection_t* ret]     
scope { cain_sdp_connection_t* current; }
@init {$connection::current = cain_sdp_connection_new(); $ret=$connection::current; }
:    {IS_TOKEN(c)}?alpha_num EQUAL nettype { cain_sdp_connection_set_network_type($connection::current,(const char*)$nettype.text->chars);} 
                  SPACE addrtype{ cain_sdp_connection_set_address_type($connection::current,(const char*)$addrtype.text->chars);} 
                  SPACE connection_address {cain_sdp_connection_set_address($connection::current,(const char*)$connection_address.text->chars);}
                  ;
                         //;a connection field must be present
                         //;in every media description or at the
                         //;session-level
                         
bandwidth returns [cain_sdp_bandwidth_t* ret]     
scope { cain_sdp_bandwidth_t* current; }
@init {$bandwidth::current = cain_sdp_bandwidth_new(); $ret=$bandwidth::current; }
  :    {IS_TOKEN(b)}?alpha_num EQUAL bwtype {cain_sdp_bandwidth_set_type($bandwidth::current,(const char*)$bwtype.text->chars); } 
      COLON bandwidth_value {cain_sdp_bandwidth_set_value($bandwidth::current,atoi((const char*)$bandwidth_value.text->chars));};

time_field:   {IS_TOKEN(t)}?alpha_num EQUAL 
              start_time 
              SPACE 
              stop_time {cain_sdp_time_description_t* time_description =cain_sdp_time_description_new();
                         cain_sdp_time_t* time_value =cain_sdp_time_new();
						 cain_sip_list_t* time_description_list;
                         cain_sdp_time_set_start(time_value,atoi((const char*)$start_time.text->chars));
                         cain_sdp_time_set_stop(time_value,atoi((const char*)$stop_time.text->chars));
                         cain_sdp_time_description_set_time(time_description,time_value);
                         time_description_list = cain_sip_list_append(NULL,time_description);
                         cain_sdp_session_description_set_time_descriptions($session_description::current,time_description_list);};

repeat_time:       {IS_TOKEN(r)}?alpha_num EQUAL repeat_interval (SPACE typed_time)+;

zone_adjustments:    sdp_time SPACE '-'? typed_time
                         (SPACE sdp_time SPACE '-'? typed_time)*;

key_field:           {IS_TOKEN(k)}?alpha_num EQUAL key_value ;
key_value options { greedy = false; }:        (~(CR|LF))*;
//key_type:            {IS_TOKEN(prompt)}? alpha_num*  /*'prompt'*/ |
//                     {IS_TOKEN(clear)}? alpha_num* /*'clear'*/  COLON key_data |
//                     {IS_TOKEN(base64)}? alpha_num* /*'base64*/ COLON key_data |
//                     {IS_TOKEN(base64)}? alpha_num* /*'uri*/ COLON uri;
//
//key_data:            email_safe;


attribute returns [cain_sdp_attribute_t* ret]     
scope { cain_sdp_attribute_t* current; }
@init {$attribute::current = cain_sdp_attribute_new(); $ret=$attribute::current; }: {IS_TOKEN(a)}?alpha_num EQUAL attribute_value;
 
media_description  returns [cain_sdp_media_description_t* ret]     
scope { cain_sdp_media_description_t* current; }
@init {$media_description::current = cain_sdp_media_description_new(); $ret=$media_description::current; }
:                    media CR LF  {cain_sdp_media_description_set_media($media_description::current,$media.ret);}
                    (info {cain_sdp_media_description_set_info($media_description::current,$info.ret);} CR LF)?
                     (connection { cain_sdp_media_description_set_connection($media_description::current,$connection.ret);} CR LF)?
                     (bandwidth {cain_sdp_media_description_add_bandwidth($media_description::current,$bandwidth.ret);} CR LF)*
                     (key_field CR LF)?
                     (attribute {cain_sdp_media_description_add_attribute($media_description::current,$attribute.ret);} CR LF)*;
                       
 
media returns [cain_sdp_media_t* ret]     
scope { cain_sdp_media_t* current; }
@init {$media::current = cain_sdp_media_new(); $ret=$media::current; }
:         {IS_TOKEN(m)}?alpha_num EQUAL 
          media_value {cain_sdp_media_set_media_type($media::current,(const char*)$media_value.text->chars);} 
          SPACE port {cain_sdp_media_set_media_port($media::current,atoi((const char*)$port.text->chars));} 
          (SLASH integer{cain_sdp_media_set_port_count($media::current,atoi((const char*)$integer.text->chars));})? 
          SPACE proto {cain_sdp_media_set_protocol($media::current,(const char*)$proto.text->chars);}
          (SPACE fmt)*;

media_value:               alpha_num+;
                     //    ;typically "audio", "video", "application"
                     //    ;or "data"

fmt:                 DIGIT+ {cain_sdp_media_set_media_formats($media::current
                                                              ,cain_sip_list_append(cain_sdp_media_get_media_formats($media::current)
                                                              ,(void*)(long)atoi((const char*)$fmt.text->chars)));};
                     //;typically an RTP payload type for audio
                     //;and video media
proto              options { greedy = false; }:        ~(SPACE|CR|LF)*;
                     //;typically "RTP/AVP" or "udp" for IP4

port:                DIGIT+;
                     //    ;should in the range "1024" to "65535" inclusive
                     //    ;for UDP based media

attribute_value:           (att_field {cain_sdp_attribute_set_name($attribute::current,(const char*)$att_field.text->chars);} 
                            COLON att_value {cain_sdp_attribute_set_value($attribute::current,(const char*)$att_value.text->chars);}) 
                            | att_field {cain_sdp_attribute_set_name($attribute::current,(const char*)$att_field.text->chars);};

att_field:           token+;

att_value            options { greedy = false; }:        ~(CR|LF)*;

sess_id:             DIGIT*;
                        // ;should be unique for this originating username/host

sess_version:        DIGIT+;
                         //;0 is a new session

connection_address:  multicast_address
                         | addr;

multicast_address:   (decimal_uchar decimal_uchar decimal_uchar DOT) decimal_uchar '/' ttl  ( '/' integer )?;
                         //;multicast addresses may be in the range
                         //;224.0.0.0 to 239.255.255.255

ttl:                 decimal_uchar;

start_time:           DIGIT+  ;

stop_time:           DIGIT+  ;

sdp_time:                 DIGIT+;
                     //    ;sufficient for 2 more centuries

repeat_interval:     typed_time;

typed_time:          DIGIT* fixed_len_time_unit?;

fixed_len_time_unit: {IS_TOKEN(d)}? alpha_num
                      | {IS_TOKEN(h)}? alpha_num
                      | {IS_TOKEN(m)}? alpha_num
                      | {IS_TOKEN(s)}? alpha_num;

bwtype:              alpha_num+;

bandwidth_value:           DIGIT+;

username:            email_safe;
                         //;pretty wide definition, but doesn't include SPACE

email_address       options { greedy = false; }:        ~(CR|LF)* ;  //| email '(' email_safe ')' |
                         //email_safe '<' email '>';


uri:          text        ;//defined in RFC1630

phone_number:         phone;/*(phone '(') => (phone '(') email_safe ')'
                      | (phone) => phone 
                      | email_safe LQUOTE phone RQUOTE;*/

phone:               text;//'+' DIGIT*POS_DIGIT (SPACE | '-' | DIGIT)*;
                         //;there must be a SPACE or hyphen between the
                         //;international code and the rest of the number.

nettype:             alpha_num+;//'IN';
                        // ;list to be extended

addrtype:            alpha_num+; //'IP4' | 'IP6';
                         //;list to be extended

addr:                (fqdn)=>fqdn | unicast_address ;
  
fqdn:                (alpha_num |'-'|DOT)*;
                     //    ;fully qualified domain name as specified in RFC1035

unicast_address:     ipv4_address ;//| ipv6_address

ipv4_address :         b1 DOT decimal_uchar DOT decimal_uchar DOT b4;
b1:                  decimal_uchar;
                      //;less than "224"; not "0" or "127"
b4:                  decimal_uchar;
                      //;not "0"

//ip6_address :         ;//to be defined

text :                ~(CR|LF)*;
                      //default is to interpret this as IS0-10646 UTF8
                      //ISO 8859-1 requires a "a=charset:ISO-8859-1"
                      //session-level attribute to be used

byte_string options { greedy = false; }:        (.)* ; 
                         //any byte except NUL, CR or LF

decimal_uchar:        d+=DIGIT+ {$d->count<=3}?  ; 

integer:             POS_DIGIT DIGIT*;

email_safe : byte_string;

token : alpha_num | '!' | '#' | '$' |'&'| '%'| '\'' | '*' |'+' | '-' | '.' ;

alpha_num:       ALPHA | DIGIT;


DIGIT:           ZERO     | POS_DIGIT;
fragment ZERO: '0';
fragment POS_DIGIT :           '1'..'9';
ALPHA:               'a'..'z'|'A'..'Z';
SPACE: ' ';

//CRLF  : CR LF { USER1 = (int)(ctx->pLexer->input->currentLine - ctx->pLexer->input->data);};
LQUOTE: '<';
RQUOTE: '>';
CR:'\r';
LF:'\n';
DOT: '.';
EQUAL: '=';
COLON: ':';
SLASH: '/';
ANY_EXCEPT_CR_LF: ~(CR|LF);
