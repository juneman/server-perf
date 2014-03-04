/***
 *
 * File: slist.c
 * 
 * author: db
 * date: 2014-01-17
 *
 */

#include "slist.h" 


inline int slist_add(slist_node_t *list, slist_node_t *node)
{
    assert(list != NULL);
    assert(node != NULL);

    node->next = list->next;
    node->prev = list;

    list->next->prev = node;
    list->next = node;

    return 0;
}

inline int slist_add_tail(slist_node_t *list, slist_node_t *node)
{
    return slist_add((slist_node_t*)list->prev, (slist_node_t*)node);
}

inline int slist_delete(slist_node_t *node)
{
    assert(node != NULL);
    
    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->prev = node->next = node;

    return 0;
}

inline int slist_empty(slist_node_t *list)
{
    assert(list != NULL);

    return ((list->prev == list && list->next == list)? 1 : 0) ;
}

