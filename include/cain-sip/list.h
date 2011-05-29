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

#ifndef CAIN_SIP_LIST_H_
#define CAIN_SIP_LIST_H_
typedef struct _cain_sip_list cain_sip_list_t;

cain_sip_list_t * cain_sip_list_append(cain_sip_list_t * elem, void * data);
cain_sip_list_t * cain_sip_list_prepend(cain_sip_list_t * elem, void * data);
cain_sip_list_t * cain_sip_list_free(cain_sip_list_t * elem);
cain_sip_list_t * cain_sip_list_concat(cain_sip_list_t * first, cain_sip_list_t * second);
cain_sip_list_t * cain_sip_list_remove(cain_sip_list_t * first, void *data);
int cain_sip_list_size(const cain_sip_list_t * first);
void cain_sip_list_for_each(const cain_sip_list_t * list, void (*func)(void *));
void cain_sip_list_for_each2(const cain_sip_list_t * list, void (*func)(void *, void *), void *user_data);
cain_sip_list_t * cain_sip_list_remove_link(cain_sip_list_t * list, cain_sip_list_t * elem);
cain_sip_list_t * cain_sip_list_delete_link(cain_sip_list_t * list, cain_sip_list_t * elem);
cain_sip_list_t * cain_sip_list_find(cain_sip_list_t * list, void *data);

typedef  int (*cain_sip_compare_func)(const void *, const void*);
cain_sip_list_t * cain_sip_list_find_custom(cain_sip_list_t * list, cain_sip_compare_func cmp, const void *user_data);
void * cain_sip_list_nth_data(const cain_sip_list_t * list, int index);
int cain_sip_list_position(const cain_sip_list_t * list, cain_sip_list_t * elem);
int cain_sip_list_index(const cain_sip_list_t * list, void *data);
cain_sip_list_t * cain_sip_list_insert_sorted(cain_sip_list_t * list, void *data, cain_sip_compare_func cmp);
cain_sip_list_t * cain_sip_list_insert(cain_sip_list_t * list, cain_sip_list_t * before, void *data);
cain_sip_list_t * cain_sip_list_copy(const cain_sip_list_t * list);


#endif /* CAIN_SIP_LIST_H_ */
