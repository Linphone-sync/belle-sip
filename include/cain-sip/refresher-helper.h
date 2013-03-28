/*
	cain-sip - SIP (RFC3261) library.
    Copyright (C) 2012  Belledonne Communications SARL, Grenoble, France

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

#ifndef REFRESHER_HELPER_H_
#define REFRESHER_HELPER_H_

typedef struct cain_sip_refresher cain_sip_refresher_t;
/**
 * Refresher listener invoked every time a refresh action is performed
 * @param refresher corresponding refresher object.
 * @param user_pointer user pointer
 * @param status_code status code for the last refresh action
 * @param reason_phrase
 * */
typedef void (*cain_sip_refresher_listener_t) ( const cain_sip_refresher_t* refresher
												,void* user_pointer
												,unsigned int status_code
												,const char* reason_phrase);

/**
 * add a refresher listener
 */
CAINSIP_EXPORT void cain_sip_refresher_set_listener(cain_sip_refresher_t* refresher, cain_sip_refresher_listener_t listener,void* user_pointer);

/**
 * start the refresher
 */
int cain_sip_refresher_start(cain_sip_refresher_t* refresher);
/**
 * stop refresher
 */
CAINSIP_EXPORT void cain_sip_refresher_stop(cain_sip_refresher_t* refresher);

/**
 * Manually initiate a new transaction .
 * @param refresher object
 * @param expires -1 means value extrated from the transaction
 * @return 0 if succeed
 */
CAINSIP_EXPORT int cain_sip_refresher_refresh(cain_sip_refresher_t* refresher,int expires);
/**
 * returns current expires value;
 */
CAINSIP_EXPORT int cain_sip_refresher_get_expires(const cain_sip_refresher_t* refresher);

/**
 * returns  delay in ms after which the refresher will retry in case of recoverable error (I.E 408, 480, 503, 504, io error);
 */
CAINSIP_EXPORT int cain_sip_refresher_get_retry_after(const cain_sip_refresher_t* refresher);

/**
 * Delay in ms after which the refresher will retry in case of recoverable error (I.E 408, 480, 503, 504, io error);
 */
CAINSIP_EXPORT void cain_sip_refresher_set_retry_after(cain_sip_refresher_t* refresher, int delay_ms);

#endif /* REFRESHER_HELPER_H_ */
