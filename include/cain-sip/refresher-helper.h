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
void cain_sip_refresher_set_listener(cain_sip_refresher_t* refresher, cain_sip_refresher_listener_t listener,void* user_pointer);

/**
 * start the refresher
 */
int cain_sip_refresher_start(cain_sip_refresher_t* refresher);
/**
 * stop refresher
 */
void cain_sip_refresher_stop(cain_sip_refresher_t* refresher);


#endif /* REFRESHER_HELPER_H_ */
