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
#ifndef CAIN_SIP_H
#define CAIN_SIP_H

#include <stdlib.h>

#define CAIN_SIP_MAGIC(_type) _type##_magic

typedef enum cain_sip_magic{
	cain_sip_magic_first=0x32445191,
	CAIN_SIP_MAGIC(cain_sip_transaction_t),
	CAIN_SIP_MAGIC(cain_sip_server_transaction_t),
	CAIN_SIP_MAGIC(cain_sip_client_transaction_t),
	cain_sip_magic_end
}cain_sip_magic_t;


#define CAIN_SIP_IMPLEMENT_CAST(_type) \
_type *_type##_cast(void *obj, const char *file, int line){ \
	if (((_type*)obj)->magic==CAIN_SIP_MAGIC(_type)) return (_type*)obj; \
	cain_sip_fatal("Bad cast to "#_type " at %s:%i", file, line);\
	return NULL; \
}

#define CAIN_SIP_DECLARE_CAST(_type)\
_type *_type##_cast(void *obj, const char *file, int line);

#define CAIN_SIP_CAST(obj,_type) _type##_cast(obj, __FILE__, __LINE__)

#include "cain-sip/list.h"
#include "cain-sip/mainloop.h"
#include "cain-sip/uri.h"
#include "cain-sip/message.h"
#include "cain-sip/transaction.h"

#undef TRUE
#define TRUE 1


#undef FALSE
#define FALSE 0

#endif
