/*
 * cain_sip_list.h
 *
 *  Created on: 1 oct. 2010
 *      Author: jehanmonnier
 */

#ifndef CAIN_SIP_LIST_H_
#define CAIN_SIP_LIST_H_
typedef struct _cain_sip_list cain_sip_list;

cain_sip_list * cain_sip_list_append(cain_sip_list *elem, void * data);
cain_sip_list * cain_sip_list_prepend(cain_sip_list *elem, void * data);
cain_sip_list * cain_sip_list_free(cain_sip_list *elem);
cain_sip_list * cain_sip_list_concat(cain_sip_list *first, cain_sip_list *second);
cain_sip_list * cain_sip_list_remove(cain_sip_list *first, void *data);
int cain_sip_list_size(const cain_sip_list *first);
void cain_sip_list_for_each(const cain_sip_list *list, void (*func)(void *));
void cain_sip_list_for_each2(const cain_sip_list *list, void (*func)(void *, void *), void *user_data);
cain_sip_list *cain_sip_list_remove_link(cain_sip_list *list, cain_sip_list *elem);
cain_sip_list *cain_sip_list_find(cain_sip_list *list, void *data);
cain_sip_list *cain_sip_list_find_custom(cain_sip_list *list, int (*compare_func)(const void *, const void*), void *user_data);
void * cain_sip_list_nth_data(const cain_sip_list *list, int index);
int cain_sip_list_position(const cain_sip_list *list, cain_sip_list *elem);
int cain_sip_list_index(const cain_sip_list *list, void *data);
cain_sip_list *cain_sip_list_insert_sorted(cain_sip_list *list, void *data, int (*compare_func)(const void *, const void*));
cain_sip_list *cain_sip_list_insert(cain_sip_list *list, cain_sip_list *before, void *data);
cain_sip_list *cain_sip_list_copy(const cain_sip_list *list);


#endif /* CAIN_SIP_LIST_H_ */
