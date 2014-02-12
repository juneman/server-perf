/***
 *
 * File: slist.h
 * 
 * author: db
 * date: 2014-01-17
 *
 */

#ifndef __LIST_H_
#define __LIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct __slist_node_t__
{
    struct __slist_node_t__ *prev;
    struct __slist_node_t__ *next;
}slist_node_t;

#define slist_init(head) do {(head)->prev = (head)->next = (head);}while(0)

#define slist_entry(node, type, member) \
            ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))

#define slist_foreach(pos, head) \
    for(pos = (head)->next; pos != (head); pos = pos->next)

#define slist_foreach_entry(pos, head, type, member) \
    for (pos = slist_entry((head)->next, type, member); \
            &(pos->member) != (head); \
                pos = slist_entry((pos)->member.next, type, member))

#define slist_foreach_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos =n ; n = pos->next)

#define slist_foreach_entry_safe(pos, n, head, type, member) \
    for (pos = slist_entry((head)->next, type, member), \
        n = slist_entry(pos->member.next, type, member); \
            &(pos->member) != (head); \
              pos = n, n = slist_entry((n)->member.next, type, member))

inline int slist_add(slist_node_t *list, slist_node_t *node);
inline int slist_add_tail(slist_node_t *list, slist_node_t *node);
inline int slist_delete(slist_node_t *node);
inline int slist_empty(slist_node_t *list);


#endif // __LIST_H_


