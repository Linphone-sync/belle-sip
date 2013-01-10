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
grammar cain_sip_message;

 
options {	
	language = C;
	
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
#include "cain-sip/cain-sip.h"
#include "cain_sip_internal.h"
}

@rulecatch 
{
    if (HASEXCEPTION())
    {

    // This is ugly.  We set the exception type to ANTLR3_RECOGNITION_EXCEPTION so we can always
    // catch them.
    //PREPORTERROR();
    EXCEPTION->type = ANTLR3_RECOGNITION_EXCEPTION;
    }
}

message returns [cain_sip_message_t* ret]
scope { size_t message_length; }
  : message_raw[&($message::message_length)] {$ret=$message_raw.ret;};
message_raw [size_t* length]  returns [cain_sip_message_t* ret]
scope { size_t* message_length; }
@init {$message_raw::message_length=length; }
	:	  request {$ret = CAIN_SIP_MESSAGE($request.ret);} 
	   | response {$ret = CAIN_SIP_MESSAGE($response.ret);} ;
request	returns [cain_sip_request_t* ret]
scope { cain_sip_request_t* current; }
@init {$request::current = cain_sip_request_new(); $ret=$request::current; }
	:	  request_line  message_header[CAIN_SIP_MESSAGE($request::current)]+ last_crlf=CRLF {*($message_raw::message_length)=$last_crlf->user1;} /*message_body ?*/ ;

request_line   
	:	  method {cain_sip_request_set_method($request::current,(const char*)($method.text->chars));} 
	    (SP)=>LWS 
	    uri {cain_sip_request_set_uri($request::current,$uri.ret);}
	    LWS 
	    sip_version 
	    CRLF ;

sip_version   
	:	word;// 'SIP/' DIGIT '.' DIGIT;

message_header [cain_sip_message_t* message] 

	:	           (/*accept
//                |  accept_encoding
//                |  accept_language
//                |  alert_info
//                |  allow
//                |  authentication_info
//                |  authorization
//                |  header_call_id {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_call_id.ret));}/*
//                |  call_info
//                |  header_contact {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_contact.ret));}
//                |  content_disposition
//                |  content_encoding
//                |  content_language*/
//                |  header_content_length  {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_content_length.ret));}
//                |  header_content_type  {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_content_type.ret));}
//                |  header_cseq  {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_cseq.ret));}/*
//                |  date
//                |  error_info
//                |  expires*/
//                |  header_from  {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_from.ret));}/*
//                |  in_reply_to
//                |  max_forwards
//                |  mime_version
//                |  min_expires
//                |  organization
//                |  priority
//                |  proxy_authenticate
//                |  proxy_authorization
//                |  proxy_require*/
//                |  header_record_route  {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_record_route.ret));}/*
//                |  reply_to
//                |  require
//                |  retry_after*/
//                |  header_route  {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_route.ret));}/*
//                |  server
//                |  subject
//                |  supported
//                |  timestamp*/
//                |  header_to  {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_to.ret));}/*
//                |  unsupported
//                |  user_agent*/
//                |  header_via  {cain_sip_message_add_header(message,CAIN_SIP_HEADER($header_via.ret));}/*
//                |  warning
//                |  www_authenticate*/
                  header_extension[TRUE] {
                    cain_sip_header_t* lheader = CAIN_SIP_HEADER($header_extension.ret);
                    do {
                      cain_sip_message_add_header(message,lheader);
                      }
                    while((lheader=cain_sip_header_get_next(lheader)) != NULL); } 
                ) CRLF 
               ;

       
/*
invitem           
	:	'INVITE' ; //INVITE in caps
ackm 	:	'ACK'; //ACK in caps
optionsm:	'OPTION'; //OPTIONS in caps
byem	:	'BYE' ; //BYE in caps
cancelm :	'CANCEL' ; //CANCEL in caps
registerm
	:	'REGISTER' ; //REGISTER in caps
optionm :	'OPTION';
*/
method 	:	          /* invitem | ackm | optionm | byem | cancelm | registerm |*/extension_method ;

extension_method  
	:	  token;

response  returns [cain_sip_response_t* ret]
scope { cain_sip_response_t* current; }
@init {$response::current = cain_sip_response_new(); $ret=$response::current; }         
	:	  status_line message_header[CAIN_SIP_MESSAGE($response::current)]+ last_crlf=CRLF {*($message_raw::message_length)=$last_crlf->user1;} /*message_body*/ ;

status_line     
	:	  sip_version 
	   LWS status_code {cain_sip_response_set_status_code($response::current,atoi((char*)$status_code.text->chars));}
	   LWS reason_phrase {cain_sip_response_set_reason_phrase($response::current,(char*)$reason_phrase.text->chars);}
	   CRLF ;
	
status_code     
	: extension_code;

extension_code  
	:	  DIGIT DIGIT DIGIT;
reason_phrase   
	:	  ~(CRLF)*;
/*
utf8cont
	:
	;

utf8_non_ascii
	:
	;


	
accept         
	:	  'Accept' HCOLON ( accept_range (COMMA accept_range)* )?;
accept_range   
	:	  media_range (SEMI accept_param)*;
*/
//media_range    
//	:	  ( '*/*'
//                  | ( m_type SLASH '*' )
//                  | ( m_type SLASH m_subtype )
//                  ) ( SEMI m_parameter )* ;

//accept_param   
//	:	  ('q' EQUAL qvalue) | generic_param;
/*
qvalue         
	:	  ( '0' ( '.' DIGIT+)? )
                  | ( '1'( '.'DIGIT+)? );
*/
generic_param [cain_sip_parameters_t* object]  returns [cain_sip_param_pair_t* ret]
scope{int is_value;} 
@init { $generic_param::is_value=0; }
	:	   token (  equal gen_value {$generic_param::is_value = 1;} )? {
	                                                   if (object == NULL) {
	                                                     $ret=cain_sip_param_pair_new((const char*)($token.text->chars)
	                                                                                 ,$generic_param::is_value?(const char*)($gen_value.text->chars):NULL);
	                                                   } else {
	                                                     cain_sip_parameters_set_parameter(object
	                                                                                       ,(const char*)($token.text->chars)
	                                                                                       ,$generic_param::is_value?(const char*)($gen_value.text->chars):NULL);
	                                                     $ret=NULL;
	                                                   }
	                                                   };
gen_value      
	:	  token |  quoted_string;

quoted_string 
options { greedy = false; }
	: DQUOTE unquoted_value=(.*) DQUOTE ;

/*
accept_encoding  
	:	  'Accept-Encoding' HCOLON ( encoding (COMMA encoding)* );
encoding         
	:	  codings (SEMI accept_param)*;
codings          
	:	  content_coding | '*';
content_coding   :  token;

accept_language  
	:	  'Accept-Language' HCOLON
                     ( language (COMMA language)* )?;
language         
	:	  language_range (SEMI accept_param)*;
language_range   
	:	  ( ( alpha alpha alpha alpha alpha alpha alpha alpha ( '-' alpha alpha alpha alpha alpha alpha alpha alpha )* ) | '*' );

alert_info   
	:	  'Alert-Info' HCOLON alert_param (COMMA alert_param)*;
	
alert_param  
	:	  LAQUOT absoluteURI RAQUOT ( SEMI generic_param )*;

absoluteURI    
	:	  token ':' token;
*/
header_allow returns [cain_sip_header_allow_t* ret]     
scope { cain_sip_header_allow_t* current; }
@init {$header_allow::current = cain_sip_header_allow_new(); $ret=$header_allow::current; }
    	
:	   {IS_TOKEN(Allow)}? token /*'Allow'*/ hcolon methods {cain_sip_header_allow_set_method($header_allow::current,(const char*)($methods.text->chars));} ;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_allow::current);
   $ret=NULL;
}
methods : LWS? method (comma method)*;
authorization_token: {IS_TOKEN(Authorization)}? token;
digest_token: {IS_TOKEN(Digest)}? token;


header_authorization  returns [cain_sip_header_authorization_t* ret]     
scope { cain_sip_header_authorization_t* current; }
@init {$header_authorization::current = cain_sip_header_authorization_new(); $ret=$header_authorization::current; }
	:	  authorization_token /*'Authorization'*/ hcolon credentials[$header_authorization::current];
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_authorization::current);
   $ret=NULL;
}

credentials  [cain_sip_header_authorization_t* header_authorization_base]     
	:	  (digest_token /*'Digest'*/ {cain_sip_header_authorization_set_scheme(header_authorization_base,"Digest");} 
	   LWS digest_response[header_authorization_base])
                     | other_response[header_authorization_base];
digest_response  [cain_sip_header_authorization_t* header_authorization_base]  
	:	  dig_resp[header_authorization_base] (comma dig_resp[header_authorization_base])*;
dig_resp  [cain_sip_header_authorization_t* header_authorization_base]         
	:	  username { cain_sip_header_authorization_set_username(header_authorization_base,$username.ret);
	               cain_sip_free($username.ret);
	             }
	| realm { cain_sip_header_authorization_set_realm(header_authorization_base,(char*)$realm.ret);
                 cain_sip_free($realm.ret);
           } 
	| nonce { cain_sip_header_authorization_set_nonce(header_authorization_base,(char*)$nonce.ret);
                 cain_sip_free($nonce.ret);
           }
	| digest_uri[header_authorization_base]
  | dresponse  { cain_sip_header_authorization_set_response(header_authorization_base,(char*)$dresponse.ret);
                 cain_sip_free($dresponse.ret);
               }
  | algorithm  {
                cain_sip_header_authorization_set_algorithm(header_authorization_base,(char*)$algorithm.ret);
               } 
  | cnonce{
            cain_sip_header_authorization_set_cnonce(header_authorization_base,(char*)$cnonce.ret);
            cain_sip_free($cnonce.ret);
           }
  | opaque {
            cain_sip_header_authorization_set_opaque(header_authorization_base,(char*)$opaque.ret);
            cain_sip_free($opaque.ret);
           }
  | message_qop{
            cain_sip_header_authorization_set_qop(header_authorization_base,$message_qop.ret);
           }
  | nonce_count{
            cain_sip_header_authorization_set_nonce_count(header_authorization_base,atoi((char*)$nonce_count.ret));
           } 
  | auth_param[header_authorization_base]
   ;
username_token: {IS_TOKEN(username)}? token;
username returns [char* ret]          
	:	  username_token /*'username'*/ equal username_value {
                      $ret = _cain_sip_str_dup_and_unquote_string((char*)$username_value.text->chars);
                       };

username_value    :  quoted_string;

uri_token: {IS_TOKEN(uri)}? token;
digest_uri [cain_sip_header_authorization_t* header_authorization_base]        
	:	  uri_token /*'uri'*/ equal DQUOTE uri DQUOTE  
	{cain_sip_header_authorization_set_uri(header_authorization_base,$uri.ret);
	 };
/*
digest_uri_value  :  rquest_uri ;
rquest_uri
	: uri;
*/	
// Equal to request-uri as specified by HTTP/1.1
message_qop  returns [const char* ret]     
	:	  {IS_TOKEN(qop)}? token/*'qop'*/ equal  qop_value {$ret = (const char*)$qop_value.text->chars;};

qop_value
	:  token;

cnonce returns [char* ret]            
	:	  {IS_TOKEN(cnonce)}? token /*'cnonce'*/ equal cnonce_value {
                                              $ret = _cain_sip_str_dup_and_unquote_string((char*)$cnonce_value.text->chars);
                                              };
cnonce_value      
	:	  nonce_value;
nonce_count returns [const char* ret]       
	:	  {IS_TOKEN(nc)}? token /*'nc'*/ equal nc_value {$ret=(char*)$nc_value.text->chars;};
nc_value          
	:	  huit_lhex; 
dresponse  returns [char* ret]       
	:	  {IS_TOKEN(response)}? token /*'response'*/ equal request_digest{
                      $ret = _cain_sip_str_dup_and_unquote_string((char*)$request_digest.text->chars);
                     };
request_digest    
	:	  quoted_string ;/*sp_laquot_sp huit_lhex huit_lhex huit_lhex huit_lhex sp_raquot_sp;
*/
huit_lhex
	: hexdigit+;

auth_param [cain_sip_header_authorization_t* header_authorization_base]        
	:	  auth_param_name equal
                     auth_param_value {cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS(header_authorization_base)
                                                                         ,(char*)$auth_param_name.text->chars
                                                                         ,(char*)$auth_param_value.text->chars);
                                       }
          ;
auth_param_value : token | quoted_string ;          
auth_param_name   
	:	  token;
other_response [cain_sip_header_authorization_t* header_authorization_base]    
	:	  auth_scheme {cain_sip_header_authorization_set_scheme(header_authorization_base,(const char*)$auth_scheme.text->chars);} 
	   LWS auth_param[header_authorization_base]
                     (comma auth_param[header_authorization_base])*;

auth_scheme       
	:	  token;
/*
authentication_info  :  'Authentication-Info' HCOLON ainfo
                        (COMMA ainfo)*;
ainfo                
	:	  nextnonce | message_qop
                         | response_auth | cnonce
                         | nonce_count;
nextnonce            
	:	  'nextnonce' EQUAL nonce_value;
	*/
nonce_value         :  quoted_string;
/*
response_auth        
	:	  'rspauth' EQUAL response_digest;
response_digest      
	:	  LDQUOT hexdigit* RDQUOT;
*/
/*callid header*/
call_id_token: {IS_TOKEN(Call-ID)}? token;

header_call_id  returns [cain_sip_header_call_id_t* ret]     
scope { cain_sip_header_call_id_t* current; }
@init {$header_call_id::current = cain_sip_header_call_id_new(); $ret=$header_call_id::current; }
  :  call_id_token /*( 'Call-ID' | 'i' )*/ hcolon call_id;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_call_id::current);
   $ret=NULL;
}  
call_id   
	:	  word ( '@' word )? {cain_sip_header_call_id_set_call_id($header_call_id::current,(const char*) $text->chars); };
/*
call_info   
	:	  'Call-Info' HCOLON info (COMMA info)*;
info        
	:	  LAQUOT absoluteURI RAQUOT ( SEMI info_param)*;
info_param  
	:	  ( 'purpose' EQUAL ( 'icon' | 'info'
               | 'card' | token ) ) | generic_param;
*/
/* contact header */
contact_token: {IS_TOKEN(Contact)}? token;

header_contact      returns [cain_sip_header_contact_t* ret]   
scope { cain_sip_header_contact_t* current; cain_sip_header_contact_t* first; }
@init { $header_contact::current =NULL; }
	:	  (contact_token /*'Contact'*/ /*| 'm'*/ ) hcolon
                  ( STAR  { $header_contact::current = cain_sip_header_contact_new();
                            cain_sip_header_contact_set_wildcard($header_contact::current,1);}
                  | (contact_param (comma contact_param)*)) {$ret = $header_contact::first; };
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   if ($header_contact::current) cain_sip_object_unref($header_contact::current);
   $ret=NULL;
} 
                  
contact_param 
scope { cain_sip_header_contact_t* prev;}
@init { if ($header_contact::current == NULL) {
            $header_contact::current = cain_sip_header_contact_new();
             $header_contact::first = $header_contact::current;
             $contact_param::prev==NULL; 
         } else {
            $contact_param::prev=$header_contact::current;
            cain_sip_header_set_next(CAIN_SIP_HEADER($header_contact::current),(cain_sip_header_t*)cain_sip_header_contact_new());
            $header_contact::current = (cain_sip_header_contact_t*)cain_sip_header_get_next(CAIN_SIP_HEADER($header_contact::current));
         } 
      }
	:	  (name_addr[CAIN_SIP_HEADER_ADDRESS($header_contact::current)] 
	   | paramless_addr_spec[CAIN_SIP_HEADER_ADDRESS($header_contact::current)]) (semi contact_params)*;

catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   if ( $contact_param::prev == NULL) {
      $header_contact::first==NULL;
   } else {
     cain_sip_header_set_next(CAIN_SIP_HEADER($contact_param::prev),NULL); 
   }
   cain_sip_object_unref($header_contact::current);
   $header_contact::current=$contact_param::prev;
}

header_address      returns [cain_sip_header_address_t* ret]   
scope { cain_sip_header_address_t* current;  }
@init { $header_address::current = cain_sip_header_address_new(); $ret=$header_address::current; }
	   : name_addr[CAIN_SIP_HEADER_ADDRESS($header_address::current)] 
	   | addr_spec[CAIN_SIP_HEADER_ADDRESS($header_address::current)];
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
  cain_sip_object_unref($header_address::current);
   $ret=NULL;
} 	   
name_addr[cain_sip_header_address_t* object]      
	:	  ( display_name[object] )? sp_laquot_sp addr_spec[object] sp_raquot_sp;
addr_spec[cain_sip_header_address_t* object]      
  :  uri {cain_sip_header_address_set_uri(object,$uri.ret);};//| absoluteURI;

paramless_addr_spec[cain_sip_header_address_t* object]      
  :  paramless_uri {cain_sip_header_address_set_uri(object,$paramless_uri.ret);};//| absoluteURI;
  
display_name[cain_sip_header_address_t* object]  
  :  token {cain_sip_header_address_set_displayname(object,(const char*)($token.text->chars));}
     | quoted_string {cain_sip_header_address_set_quoted_displayname(object,(const char*)($quoted_string.text->chars));}
     ;

contact_params     
	:	  /*c_p_q | c_p_expires
                      |*/ contact_extension;
/*c_p_q              
	:	  'q' EQUAL qvalue;
c_p_expires        
	:	  'expires' EQUAL delta_seconds;*/
contact_extension  
	:	  generic_param [CAIN_SIP_PARAMETERS($header_contact::current)];

delta_seconds      
	:	  DIGIT+;
/*

content_disposition   
	:	  'Content-Disposition' HCOLON
                         disp_type ( SEMI disp_param )*;
disp_type             :  'render' | 'session' | 'icon' | 'alert'
                         | disp_extension_token;

disp_param            
	:	  handling_param | generic_param;
handling_param        
	:	  'handling' EQUAL
                         ( 'optional' | 'required'
                         | other_handling );
other_handling        
	:	  token;
disp_extension_token  
	:	  token;

content_encoding  
	:	  ( 'Content-Encoding' | 'e' ) HCOLON
                     content_coding (COMMA content_coding)*;

content_language  
	:	  'Content-Language' HCOLON
                     language_tag (COMMA language_tag)*;
language_tag      
	:	  primary_tag ( '-' subtag )*;
primary_tag       
	:	  huit_alpha;
subtag            
	:	  huit_alpha;

huit_alpha
	: alpha+;
	;
*/
/*content_length_token :  {strcmp("Content-Length",(const char*)(INPUT->toStringTT(INPUT,LT(1),LT(14)))->chars) == 0}? token;*/
content_length_token :  {IS_TOKEN(Content-Length)}? token;
header_content_length  returns [cain_sip_header_content_length_t* ret]     
scope { cain_sip_header_content_length_t* current; }
@init {$header_content_length::current = cain_sip_header_content_length_new(); $ret=$header_content_length::current; }
	:	 content_length_token /*( 'Content-Length' | 'l' )*/ 
	   hcolon 
	   content_length {cain_sip_header_content_length_set_content_length($header_content_length::current,atoi((const char*)$content_length.text->chars));};
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_content_length::current);
   $ret=NULL;
}
content_length:DIGIT+;
 
content_type_token :  {IS_TOKEN(Content-Type)}? token;
header_content_type  returns [cain_sip_header_content_type_t* ret]   
scope { cain_sip_header_content_type_t* current;}
@init { $header_content_type::current = cain_sip_header_content_type_new();$ret=$header_content_type::current; }
	:	 content_type_token/* ( 'Content-Type' | 'c' )*/ hcolon media_type;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
  cain_sip_object_unref($header_content_type::current);
   $ret=NULL;
} 	
media_type       
	:	 m_type {cain_sip_header_content_type_set_type($header_content_type::current,(const char*)$m_type.text->chars);} 
	   slash 
	   m_subtype {cain_sip_header_content_type_set_subtype($header_content_type::current,(const char*)$m_subtype.text->chars);} 
	   (semi  generic_param [CAIN_SIP_PARAMETERS($header_content_type::current)])*;
m_type           
	:	token; /*  discrete_type | composite_type;
discrete_type    
	:	  'text' | 'image' | 'audio' | 'video'
                    | 'application' | extension_token;
composite_type   
	:	  'message' | 'multipart' | extension_token;
extension_token  
	:	  ietf_token | x_token;
ietf_token       
	:	  token;
x_token          
	:	  'x-' token;*/
m_subtype        : token ;/* extension_token | iana_token;
iana_token       
	:	  token;
m_parameter      
	:	  m_attribute EQUAL m_value;
m_attribute      
	:	  token;
m_value          
	:	  token | quoted_string;
*/

cseq_token :  {IS_TOKEN(CSeq)}? token;
header_cseq  returns [cain_sip_header_cseq_t* ret]   
scope { cain_sip_header_cseq_t* current; }
@init { $header_cseq::current = cain_sip_header_cseq_new();$ret = $header_cseq::current; }
  :	cseq_token
    hcolon 
    seq_number {cain_sip_header_cseq_set_seq_number($header_cseq::current,atoi((const char*)$seq_number.text->chars));} 
    LWS 
    method {cain_sip_header_cseq_set_method($header_cseq::current,(const char*)$method.text->chars);} ;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_cseq::current);
   $ret=NULL;
}     
seq_number:DIGIT+;
/*
date          
	:	  'Date' HCOLON sip_date;
sip_date      
	:	token;//  rfc1123-date
//rfc1123-date  =  wkday "," SP date1 SP time SP "GMT"
//date1         =  2DIGIT SP month SP 4DIGIT
//                 ; day month year (e.g., 02 Jun 1982)
//time          =  2DIGIT ":" 2DIGIT ":" 2DIGIT
//                 ; 00:00:00 - 23:59:59
//wkday         =  "Mon" / "Tue" / "Wed"
//                 / "Thu" / "Fri" / "Sat" / "Sun"
//month         =  "Jan" / "Feb" / "Mar" / "Apr"
//                 / "May" / "Jun" / "Jul" / "Aug"
//                 / "Sep" / "Oct" / "Nov" / "Dec"

error_info  
	:	  'Error-Info' HCOLON error_uri (COMMA error_uri)*;

error_uri   
	:	  LAQUOT absoluteURI RAQUOT ( SEMI generic_param )*;
*/
header_expires returns [cain_sip_header_expires_t* ret]   
scope { cain_sip_header_expires_t* current; }
@init { $header_expires::current = cain_sip_header_expires_new();$ret = $header_expires::current; }     
	:	  {IS_TOKEN(Expires)}? token /*'Expires'*/ hcolon delta_seconds {cain_sip_header_expires_set_expires($header_expires::current,atoi((const char *)$delta_seconds.text->chars));};
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_expires::current);
   $ret=NULL;
}   

from_token:  {IS_TOKEN(From)}? token;
header_from  returns [cain_sip_header_from_t* ret]   
scope { cain_sip_header_from_t* current; }
@init { $header_from::current = cain_sip_header_from_new();$ret = $header_from::current; }
        
	:	  from_token/* ( 'From' | 'f' )*/ hcolon from_spec ;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_from::current);
   $ret=NULL;
} 
	
from_spec   
	:	  ( name_addr[CAIN_SIP_HEADER_ADDRESS($header_from::current)] | paramless_addr_spec[CAIN_SIP_HEADER_ADDRESS($header_from::current)] )
               ( SEMI from_param )*;
from_param  
	:	  /*tag_param |*/ generic_param [CAIN_SIP_PARAMETERS($header_from::current)];
                       
/*
tag_param   
	:	  'tag' EQUAL token;
*/
/*
in_reply_to  
	:	  'In-Reply-To' HCOLON callid (COMMA callid);
*/
header_max_forwards  returns [cain_sip_header_max_forwards_t* ret]   
scope { cain_sip_header_max_forwards_t* current; }
@init { $header_max_forwards::current = cain_sip_header_max_forwards_new();$ret = $header_max_forwards::current; } 
  :   {IS_TOKEN(Max-Forwards)}? token /*'Max-Forwards'*/ hcolon 
    max_forwards {cain_sip_header_max_forwards_set_max_forwards($header_max_forwards::current,atoi((const char*)$max_forwards.text->chars));};
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_max_forwards::current);
   $ret=NULL;
} 
max_forwards:DIGIT+;
/*
mime_version  
	:	  'MIME-Version' HCOLON DIGIT+ '.' DIGIT+;

min_expires  
	:	  'Min-Expires' HCOLON delta_seconds;

organization  
	:	  'Organization' HCOLON (text_utf8_trim)?;

text_utf8_trim
	:
	;

priority        
	:	  'Priority' HCOLON priority_value;
priority_value  
	:	  'emergency' | 'urgent' | 'normal'
                   | 'non-urgent' | other_priority;
other_priority  
	:	  token;
*/
header_proxy_authenticate   returns [cain_sip_header_proxy_authenticate_t* ret]   
scope { cain_sip_header_proxy_authenticate_t* current; }
@init { $header_proxy_authenticate::current = cain_sip_header_proxy_authenticate_new();$ret = $header_proxy_authenticate::current; } 
	:	  {IS_TOKEN(Proxy-Authenticate)}? token /*'Proxy-Authenticate'*/ 
	hcolon challenge[CAIN_SIP_HEADER_WWW_AUTHENTICATE($header_proxy_authenticate::current)];
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_proxy_authenticate::current);
   $ret=NULL;
} 

challenge [cain_sip_header_www_authenticate_t* www_authenticate]           
	:	  ({IS_TOKEN(Digest)}? token /*'Digest'*/ {cain_sip_header_www_authenticate_set_scheme(www_authenticate,"Digest");} 
	   LWS digest_cln[www_authenticate] (comma digest_cln[www_authenticate])*)
                       | other_challenge [www_authenticate];
other_challenge [cain_sip_header_www_authenticate_t* www_authenticate]    
	:	  auth_scheme {cain_sip_header_www_authenticate_set_scheme(www_authenticate,(char*)$auth_scheme.text->chars);} 
	   LWS auth_param[NULL]
                       (comma auth_param[NULL])*;
digest_cln [cain_sip_header_www_authenticate_t* www_authenticate]         
	:	
  | realm {cain_sip_header_www_authenticate_set_realm(www_authenticate,(char*)$realm.ret);
           cain_sip_free($realm.ret);} 
  | nonce {cain_sip_header_www_authenticate_set_nonce(www_authenticate,(char*)$nonce.ret);
           cain_sip_free($nonce.ret);}
  | algorithm {cain_sip_header_www_authenticate_set_algorithm(www_authenticate,$algorithm.ret);} 
  | opaque  {cain_sip_header_www_authenticate_set_opaque(www_authenticate,$opaque.ret);
             cain_sip_free($opaque.ret);}
  | qop_opts {cain_sip_header_www_authenticate_set_qop(www_authenticate,$qop_opts.ret);
              /*cain_sip_free($qop_opts.ret);*/}
	| domain {cain_sip_header_www_authenticate_set_domain(www_authenticate,$domain.ret);
             cain_sip_free($domain.ret);} 
	| stale { if (strcmp("true",$stale.ret)==0) {
	             cain_sip_header_www_authenticate_set_stale(www_authenticate,1);
	           }
	        }
     | auth_param[(cain_sip_header_authorization_t*)www_authenticate];
/* the cast above is very BAD, but auth_param works on fields common to the two structures*/

realm returns [char* ret]              
	:	  {IS_TOKEN(realm)}? token /*'realm'*/ equal realm_value {
	                    $ret = _cain_sip_str_dup_and_unquote_string((char*)$realm_value.text->chars);
	                     };
realm_value         
	:	  quoted_string ;

domain returns [char* ret]              
	:	  {IS_TOKEN(domain)}? token /*'domain'*/ equal quoted_string {
                      $ret = _cain_sip_str_dup_and_unquote_string((char*)$quoted_string.text->chars);};
	/* LDQUOT uri
                       ( SP+ uri )* RDQUOT;
uri                 
	:	  absoluteURI | '/'.;
*/
nonce  returns [char* ret]             
	:	  {IS_TOKEN(nonce)}? token /*'nonce'*/ equal nonce_value{
                      $ret = _cain_sip_str_dup_and_unquote_string((char*)$nonce_value.text->chars);
                       };
opaque returns [char* ret]             
  :   {IS_TOKEN(opaque)}? token /*'opaque'*/ equal quoted_string{
                      $ret = _cain_sip_str_dup_and_unquote_string((char*)$quoted_string.text->chars);
                       };

stale  returns [const char* ret]             
	:	  {IS_TOKEN(stale)}? token /*'stale'*/ equal stale_value {$ret=(char*)$stale_value.text->chars;} /* ( 'true' | 'false' )*/;

stale_value:token;

algorithm returns [const char* ret]           
	:	  {IS_TOKEN(algorithm)}? token /*'algorithm'*/ equal /* ( 'MD5' | 'MD5-sess'
                       |*/ alg_value=token {$ret=(char*)$alg_value.text->chars;}/*)*/
  ;

qop_opts returns [cain_sip_list_t* ret]        
scope { cain_sip_list_t* list; }
@init{$qop_opts::list=NULL;}
	:	  {IS_TOKEN(qop)}? token /*'qop'*/ equal 
	ldquot 
	qop_opts_value 
  (COMMA qop_opts_value)* 
  rdquot {$ret=$qop_opts::list;} ;
  
qop_opts_value 	
: token {$qop_opts::list=cain_sip_list_append($qop_opts::list,cain_sip_strdup((const char*)$token.text->chars));};

header_proxy_authorization  returns [cain_sip_header_proxy_authorization_t* ret]
scope { cain_sip_header_proxy_authorization_t* current; }
@init { $header_proxy_authorization::current = cain_sip_header_proxy_authorization_new();$ret = $header_proxy_authorization::current; }
	:	  {IS_TOKEN(Proxy-Authorization)}? token /*'Proxy-Authorization'*/ hcolon credentials[(cain_sip_header_authorization_t*)$header_proxy_authorization::current];
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_proxy_authorization::current);
   $ret=NULL;
}

/*
proxy_require  
	:	  'Proxy-Require' HCOLON option_tag
                  (COMMA option_tag)*;
option_tag     
	:	  token;
*/
/*FIXME service-route = recorde-route = route, too many copy/past*/
service_route_token:  {IS_TOKEN(Service-Route)}? token;
header_service_route  returns [cain_sip_header_service_route_t* ret]   
scope { cain_sip_header_service_route_t* current; cain_sip_header_service_route_t* first;}
@init { $header_service_route::current = NULL;}
  :   service_route_token /*'Service-Route'*/ hcolon srv_route (comma srv_route)* {$ret = $header_service_route::first;};
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_service_route::current);
   $ret=NULL;
}
srv_route
scope { cain_sip_header_service_route_t* prev;}
@init { if ($header_service_route::current == NULL) {
            $header_service_route::first = $header_service_route::current = cain_sip_header_service_route_new();
            $srv_route::prev=NULL;
         } else {
            cain_sip_header_t* header = CAIN_SIP_HEADER($header_service_route::current); 
            $srv_route::prev=$header_service_route::current;
            cain_sip_header_set_next(header,(cain_sip_header_t*)($header_service_route::current = cain_sip_header_service_route_new()));
         } 
      }     
  :   name_addr[CAIN_SIP_HEADER_ADDRESS($header_service_route::current)] ( semi sr_param )*;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   if ( $srv_route::prev == NULL) {
      $header_service_route::first==NULL;
   } else {
     cain_sip_header_set_next(CAIN_SIP_HEADER($srv_route::prev),NULL); 
   }
   cain_sip_object_unref($header_service_route::current);
   $header_service_route::current=$srv_route::prev;
}
  
sr_param      
  :   generic_param[CAIN_SIP_PARAMETERS($header_service_route::current)];
  
record_route_token:  {IS_TOKEN(Record-Route)}? token;
header_record_route  returns [cain_sip_header_record_route_t* ret]   
scope { cain_sip_header_record_route_t* current; cain_sip_header_record_route_t* first;}
@init { $header_record_route::current = NULL;}
	:	  record_route_token /*'Record-Route'*/ hcolon rec_route (comma rec_route)* {$ret = $header_record_route::first;};
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_record_route::current);
   $ret=NULL;
}
rec_route
scope { cain_sip_header_record_route_t* prev;}
@init { if ($header_record_route::current == NULL) {
            $header_record_route::first = $header_record_route::current = cain_sip_header_record_route_new();
            $rec_route::prev=NULL;
         } else {
            cain_sip_header_t* header = CAIN_SIP_HEADER($header_record_route::current); 
            $rec_route::prev=$header_record_route::current;
            cain_sip_header_set_next(header,(cain_sip_header_t*)($header_record_route::current = cain_sip_header_record_route_new()));
         } 
      }     
	:	  name_addr[CAIN_SIP_HEADER_ADDRESS($header_record_route::current)] ( semi rr_param )*;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   if ( $rec_route::prev == NULL) {
      $header_record_route::first==NULL;
   } else {
     cain_sip_header_set_next(CAIN_SIP_HEADER($rec_route::prev),NULL); 
   }
   cain_sip_object_unref($header_record_route::current);
   $header_record_route::current=$rec_route::prev;
}
	
rr_param      
	:	  generic_param[CAIN_SIP_PARAMETERS($header_record_route::current)];
/*
reply_to      
	:	  'Reply-To' HCOLON rplyto_spec;
rplyto_spec   
	:	  ( name_addr | addr_spec )
                 ( SEMI rplyto_param )*;
rplyto_param  
	:	  generic_param;
require       
	:	  'Require' HCOLON option_tag (COMMA option_tag)*;

retry_after  
	:	  'Retry-After' HCOLON delta_seconds
                 comment? ( SEMI retry_param )*;
*/
comment	: '(' . ')';
	
/*
retry_param  
	:	  ('duration' EQUAL delta_seconds)
                | generic_param;
*/
route_token:  {IS_TOKEN(Route)}? token;
header_route  returns [cain_sip_header_route_t* ret]   
scope { cain_sip_header_route_t* current;cain_sip_header_route_t* first; }
@init { $header_route::current = NULL; }
  :   route_token /*'Route'*/ hcolon route_param (comma route_param)*{$ret = $header_route::first;};
route_param 
scope { cain_sip_header_route_t* prev;}
@init { if ($header_route::current == NULL) {
            $header_route::first = $header_route::current = cain_sip_header_route_new();
             $route_param::prev=NULL;
         } else {
            cain_sip_header_t* header = CAIN_SIP_HEADER($header_route::current); 
            $route_param::prev=$header_route::current;
            cain_sip_header_set_next(header,(cain_sip_header_t*)($header_route::current = cain_sip_header_route_new()));
         } 
      }      
  :   name_addr[CAIN_SIP_HEADER_ADDRESS($header_route::current)] ( semi r_param )*;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   if ( $route_param::prev == NULL) {
      $header_route::first==NULL;
   } else {
     cain_sip_header_set_next(CAIN_SIP_HEADER($route_param::prev),NULL); 
   }
   cain_sip_object_unref($header_record_route::current);
   $header_route::current=$route_param::prev;
}  
r_param      
  :   generic_param[CAIN_SIP_PARAMETERS($header_route::current)];
/*
server           
	:	  'Server' HCOLON server_val (LWS server_val)*;
*/

/*
subject  
	:	  ( 'Subject' | 's' ) HCOLON (text_utf_huit)?;

text_utf_huit
	:
	;

supported  
	:	  ( 'Supported' | 'k' ) HCOLON
              (option_tag (COMMA option_tag)*)?;

timestamp  
	:	  'Timestamp' HCOLON (DIGIT)+
               ( '.' (DIGIT)* )? ( LWS delay )?;
delay      
	:	  (DIGIT)* ( '.' (DIGIT)* )?;
*/
to_token:  {IS_TOKEN(To)}? token;
header_to  returns [cain_sip_header_to_t* ret]   
scope { cain_sip_header_to_t* current; }
@init { $header_to::current = cain_sip_header_to_new(); }
        
  :   to_token /*'To' ( 'To' | 't' )*/ hcolon to_spec {$ret = $header_to::current;};
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_to::current);
   $ret=NULL;
}  
to_spec   
  :   ( name_addr[CAIN_SIP_HEADER_ADDRESS($header_to::current)] | paramless_addr_spec[CAIN_SIP_HEADER_ADDRESS($header_to::current)] )
               ( semi to_param )*;
to_param  
  :   /*tag_param |*/ generic_param [CAIN_SIP_PARAMETERS($header_to::current)];
/*
unsupported  
	:	  'Unsupported' HCOLON option_tag (COMMA option_tag)*;
*/
header_user_agent  returns [cain_sip_header_user_agent_t* ret]   
scope { cain_sip_header_user_agent_t* current; }
@init { $header_user_agent::current = cain_sip_header_user_agent_new();$ret = $header_user_agent::current;}
	:	  {IS_TOKEN(User-Agent)}? token /*'User-Agent'*/ hcolon server_val (LWS server_val)*;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_user_agent::current);
   $ret=NULL;
} 

server_val    : word {cain_sip_header_user_agent_add_product($header_user_agent::current,(const char*)$word.text->chars); };
serval_item    
  :   product | comment  ;
product          
  :   token (SLASH product_version)?;
product_version  
  :   token;
  
via_token:  {IS_TOKEN(Via)}? token;
header_via  returns [cain_sip_header_via_t* ret]   
scope { cain_sip_header_via_t* current; cain_sip_header_via_t* first; }
@init { $header_via::current = NULL;}
        
  :   via_token/* ( 'via' | 'v' )*/ hcolon via_parm (comma via_parm)* {$ret = $header_via::first;};

via_parm
scope { cain_sip_header_via_t* prev;}
@init { if ($header_via::current == NULL) {
            $header_via::first = $header_via::current = cain_sip_header_via_new();
            $via_parm::prev=NULL;
         } else {
            $via_parm::prev=$header_via::current;
            cain_sip_header_t* header = CAIN_SIP_HEADER($header_via::current); 
            cain_sip_header_set_next(header,(cain_sip_header_t*)($header_via::current = cain_sip_header_via_new()));
         } 
      }          
	:	  sent_protocol  LWS sent_by ( semi via_params )*;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   if ( $via_parm::prev == NULL) {
      $header_via::first==NULL;
   } else {
     cain_sip_header_set_next(CAIN_SIP_HEADER($via_parm::prev),NULL); 
   }
   cain_sip_object_unref($header_via::current);
   $header_via::current=$via_parm::prev;
} 
	
via_params        
	:	  /*via_ttl | via_maddr
                     | via_received | via_branch
                     | via_extension */ generic_param [CAIN_SIP_PARAMETERS($header_via::current)];
/*via_ttl           
	:	  'ttl' EQUAL ttl;
via_maddr         
	:	  'maddr' EQUAL host;
via_received      
	:	  'received' EQUAL (ipv4address | ipv6address);
via_branch        
	:	  'branch' EQUAL token;
via_extension     
	:	  generic_param;*/
sent_protocol     
	:	  (protocol_name slash protocol_version) {cain_sip_header_via_set_protocol($header_via::current,(const char*)$text->chars);}
                     slash transport {cain_sip_header_via_set_transport($header_via::current,(const char*)$transport.text->chars);} ;
protocol_name     
	:	  /*'SIP' |*/ token;
protocol_version  
	:	  token;
transport         
	:	 /* 'UDP' | 'TCP' | 'TLS' | 'SCTP'
                     | */ other_transport;
other_transport
	: token;

sent_by           
	:	  host {cain_sip_header_via_set_host($header_via::current,(const char*)$host.text->chars);}
	   ( COLON port {cain_sip_header_via_set_port($header_via::current,$port.ret);} )? ;
/*
warning        
	:	  'Warning' HCOLON warning_value (COMMA warning_value)*;
warning_value  
	:	  warn_code SP warn_agent SP warn_text;
warn_code      
	:	  DIGIT DIGIT DIGIT;
warn_agent     
	:	  hostport | pseudonym;
                  //  the name or pseudonym of the server adding
                  //  the Warning header, for use in debugging
warn_text      
	:	  quoted_string;
pseudonym      
	:	  token;
*/
header_www_authenticate returns [cain_sip_header_www_authenticate_t* ret]   
scope { cain_sip_header_www_authenticate_t* current; }
@init { $header_www_authenticate::current = cain_sip_header_www_authenticate_new();$ret = $header_www_authenticate::current; } 
	:	  {IS_TOKEN(WWW-Authenticate)}? token /*'WWW-Authenticate'*/ hcolon challenge[$header_www_authenticate::current];
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_www_authenticate::current);
   $ret=NULL;
}
header_subscription_state  returns [cain_sip_header_subscription_state_t* ret] 
scope { cain_sip_header_subscription_state_t* current; }
@init { $header_subscription_state::current = cain_sip_header_subscription_state_new();$ret = $header_subscription_state::current; } 
 : {IS_TOKEN(Subscription-State)}? token /*"Subscription-State"*/ 
 hcolon state_value {cain_sip_header_subscription_state_set_state($header_subscription_state::current,(const char*)$state_value.text->chars);} 
 (semi  generic_param [CAIN_SIP_PARAMETERS($header_subscription_state::current)])* ;
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($header_subscription_state::current);
   $ret=NULL;
}
state_value: token ;

header_extension[ANTLR3_BOOLEAN check_for_known_header]  returns [cain_sip_header_t* ret]
	:	   header_name 
	     hcolon 
	     header_value {if (check_for_known_header && strcasecmp("Contact",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_contact_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("From",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_from_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("To",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_to_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Call-ID",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_call_id_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Content-Length",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_content_length_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Content-Type",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_content_type_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("CSeq",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_cseq_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Route",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_route_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Record-Route",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_record_route_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Via",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_via_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Authorization",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_authorization_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Proxy-Authorization",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_proxy_authorization_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("WWW-Authenticate",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_www_authenticate_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Proxy-Authenticate",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_proxy_authenticate_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Max-Forwards",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_max_forwards_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("User-Agent",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_user_agent_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Expires",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_expires_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp("Allow",(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_allow_parse((const char*)$header_extension.text->chars));
                    } else if (check_for_known_header && strcasecmp(CAIN_SIP_SUBSCRIPTION_STATE,(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_subscription_state_parse((const char*)$header_extension.text->chars));
                    }else if (check_for_known_header && strcasecmp(CAIN_SIP_SERVICE_ROUTE,(const char*)$header_name.text->chars) == 0) {
                     $ret = CAIN_SIP_HEADER(cain_sip_header_service_route_parse((const char*)$header_extension.text->chars));
                    }else {
                      $ret =  CAIN_SIP_HEADER(cain_sip_header_extension_new());
                      cain_sip_header_extension_set_value((cain_sip_header_extension_t*)$ret,(const char*)$header_value.text->chars);
                      cain_sip_header_set_name($ret,(const char*)$header_name.text->chars);
                     }
                   } ;
header_name       
	:	  token;

header_value      
	:	  ~(CRLF)* ;
	
message_body
options { greedy = false; }  
	:	  OCTET+;

paramless_uri  returns [cain_sip_uri_t* ret]    
scope { cain_sip_uri_t* current; }
@init { $paramless_uri::current = cain_sip_uri_new(); }
   :  sip_schema[$paramless_uri::current] ((userinfo[$paramless_uri::current] hostport[$paramless_uri::current]) | hostport[$paramless_uri::current] ) 
   headers[$paramless_uri::current]? {$ret = $paramless_uri::current;}; 
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($paramless_uri::current);
   $ret=NULL;
}

uri  returns [cain_sip_uri_t* ret]    
scope { cain_sip_uri_t* current; }
@init { $uri::current = cain_sip_uri_new(); }
   :  sip_schema[$uri::current] ((userinfo[$uri::current] hostport[$uri::current]) | hostport[$uri::current] ) 
   uri_parameters[$uri::current]? 
   headers[$uri::current]? {$ret = $uri::current;}; 
catch [ANTLR3_MISMATCHED_TOKEN_EXCEPTION]
{
   cain_sip_message("[\%s]  reason [\%s]",(const char*)EXCEPTION->name,(const char*)EXCEPTION->message);
   cain_sip_object_unref($uri::current);
   $ret=NULL;
}

sip_token:  {IS_TOKEN(sip)}? token;
sips_token:  {IS_TOKEN(sips)}? token;

sip_schema[cain_sip_uri_t* uri]  : (sips_token {cain_sip_uri_set_secure(uri,1);}
            | sip_token) COLON ;
userinfo[cain_sip_uri_t* uri] 
scope { cain_sip_uri_t* current; }
@init {$userinfo::current=uri;}
       :	user ( COLON password )? '@' ;
user            :	  ( unreserved  | escaped | user_unreserved )+ {cain_sip_uri_set_user($userinfo::current,(const char *)$text->chars);};
user_unreserved :  '&' | EQUAL | '+' | '$' | COMMA | SEMI | '?' | SLASH;
password        :	  ( unreserved  |'&' | EQUAL | '+' | '$' | COMMA )*;
hostport[cain_sip_uri_t* uri] 
scope { cain_sip_uri_t* current; }
@init {$hostport::current=uri;}
        :	  host ( COLON port {cain_sip_uri_set_port($hostport::current,$port.ret);})? {cain_sip_uri_set_host($hostport::current,(const char *)$host.text->chars);};
host            :	  (hostname | ipv4address | ipv6reference) ;
hostname        :	  ( domainlabel '.' )* toplabel '.'? ;
	
domainlabel     :	  alphanum | (alphanum ( alphanum | '-' )* alphanum) ;
toplabel        :	  alpha | (alpha ( alphanum | '-' )* alphanum) ;

ipv4address    :  three_digit '.' three_digit '.' three_digit '.' three_digit;
ipv6reference  :  '[' ipv6address ']';
ipv6address    :  hexpart ( COLON ipv4address )?;
hexpart        :  hexseq | hexseq '::' ( hexseq )? | '::' ( hexseq )?;
hexseq         :  hex4 ( COLON hex4)*;
hex4           :  hexdigit hexdigit hexdigit hexdigit ;

port	returns [int ret]:	DIGIT+ { $ret=atoi((const char *)$text->chars); };


uri_parameters[cain_sip_uri_t* uri] 
scope { cain_sip_uri_t* current; }
@init {$uri_parameters::current=uri;}    
	:	  ( semi uri_parameter )+;
uri_parameter  //all parameters are considered as other     
	:	   other_param ;
other_param       
:  pname {    cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS($uri_parameters::current)
                                      ,(const char *)$pname.text->chars
                                      ,NULL);}
  |
   (pname EQUAL pvalue)  {
    cain_sip_parameters_set_parameter(CAIN_SIP_PARAMETERS($uri_parameters::current)
                                      ,(const char *)$pname.text->chars
                                      ,(const char *)$pvalue.text->chars);}
   ;

pname             
	:	  paramchar+;
pvalue            
	:	  paramchar+;
paramchar         
	:	  param_unreserved | unreserved | escaped; 
param_unreserved  
	:	  '[' | ']' | SLASH | COLON | '&' | PLUS | '$' | '.';

headers[cain_sip_uri_t* uri] 
scope { cain_sip_uri_t* current; }
@init {$headers::current=uri;}
                :  '?' header ( '&' header )* ;
header          :  hname EQUAL hvalue? {cain_sip_uri_set_header($headers::current,(const char *)$hname.text->chars,(const char *)$hvalue.text->chars);};
hname           :  ( hnv_unreserved | unreserved | escaped )+;
hvalue          :  ( hnv_unreserved | unreserved | escaped )+;


hnv_unreserved  :  '[' | ']' | '|' | '?' | COLON | PLUS | '$' ;
escaped     :  '%' hexdigit hexdigit;
ttl : three_digit;
three_digit: DIGIT | (DIGIT DIGIT) | (DIGIT DIGIT DIGIT) ;	
token       
	:	  (alphanum | mark | '%' | PLUS | '`'  )+;

reserved    
	:	  SEMI | SLASH | '?' | COLON | '@' | '&' | EQUAL | PLUS
                     | '$' | COMMA;
                     
unreserved :	  alphanum |mark;  
alphanum :	   alpha | DIGIT ;                      
hexdigit 
	:	HEX_CHAR|DIGIT; 
alpha	: HEX_CHAR | COMMON_CHAR; 
 
word        
	:	  (alphanum | mark   | '%'  
                      | PLUS | '`' |
                     LPAREN | RPAREN | LAQUOT | RAQUOT |
                     COLON | '\\' | DQUOTE | SLASH | '[' | ']' | '?' | '{' | '}' )+;
                 

COMMON_CHAR
	:	'g'..'z' | 'G'..'Z' ;	
mark	:	         '-' | '_' | '.' | '!' | '~' | STAR | '\'' ; 


HEX_CHAR:	'a'..'f' |'A'..'F';
DIGIT	: '0'..'9' ;

CRLF	: '\r\n' { USER1 = (int)(ctx->pLexer->input->currentLine - ctx->pLexer->input->data); /*GETCHARINDEX()*/;};





hcolon	: ( LWS | HTAB )* COLON  LWS? //SWS;
	       ;//|( SP | HTAB )* COLON LWS+;
HTAB	: '	';




  
 
ldquot  :  LWS? DQUOTE ;
rdquot : DQUOTE LWS?;

DQUOTE	: '"';
// open double quotation mark
//SWS   :   LWS? ; 


LWS  	:	(SP* CRLF)? SP+ ; //linear whitespace





PLUS: '+';


COLON
	:	':'
	;
semi: LWS? SEMI LWS?;

SEMI
	:	';'
	;

comma : LWS? COMMA LWS?;
COMMA
	:	','
	;

sp_laquot_sp
	:	LWS? LAQUOT LWS?;
sp_raquot_sp
	:	LWS? RAQUOT LWS?;
LAQUOT
	:	'<'
	;

RAQUOT
	:	'>'
	;

RPAREN
	:	')'
	;

LPAREN
	:	'('
	;

equal:
   LWS? EQUAL LWS?;
EQUAL
	:	'='
	;

slash : LWS? SLASH LWS?;
SLASH
	:	'/'
	;

STAR
	:	'*'
	;
fragment 
SP
	:	' '
	;
OCTET	: . ;	
