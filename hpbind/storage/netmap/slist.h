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
    struct __slist_node_t__ *prev, *next;\
}slist_node_t;

#define slist_init(head) do {(head)->prev = (head)->next = (head);}while(0)

#define slist_entry(node, type, member) \
            (type *)( ((char *)node) - (char*)(&((type *)0)->member))

inline int slist_add(slist_node_t *list, slist_node_t *node);
inline int slist_add_tail(slist_node_t *list, slist_node_t *node);
inline int slist_delete(slist_node_t *node);
inline int slist_empty(slist_node_t *list);


#endif // __LIST_H_


