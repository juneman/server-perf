/***
 *
 * file: sdata.h
 *      define metadata for data exchange
 *
 * author: db
 * date: 2014-01-21
 */ 

#ifndef __SDATA_H_
#define __SDATA_H_

#include "slist.h"

typedef struct __sdata_t__
{
#define SDATA_SIZE_MAX (511)
    char buff[SDATA_SIZE_MAX + 1]; 
    int len;

#define SDATA_EXT_BUFF_SIZE (511)
    char extbuff[SDATA_EXT_BUFF_SIZE + 1];
    slist_node_t node;
}sdata_t;

inline int sdata_init(sdata_t *data);

#endif // end of __SDATA_H_

