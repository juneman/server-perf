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

#define SDATA_MAGIC ((int)('a' << 24 | 'b' << 16 | 'c' << 8 | 'd'))
#define SDATA_VALID(p) ( ((p)->magic == SDATA_MAGIC) ? 1 : 0 )

#define CHECK_SDATA_AND_EXIT(p) do { \
    assert(SDATA_VALID((p))); \
    if (!SDATA_VALID((p))) { \
        printf("%s,%s,%d: sdata is valid.\n", __FILE__, __FUNCTION__, __LINE__) ; \
            exit(1);\
    } \
    }while(0)

typedef struct __sdata_t__
{
    int magic;
#define SDATA_SIZE_MAX (511)
    char buff[SDATA_SIZE_MAX + 1]; 
    int len;

#define SDATA_EXT_BUFF_SIZE (511)
    char extbuff[SDATA_EXT_BUFF_SIZE + 1];
    slist_node_t node;
}sdata_t;

inline int sdata_init(sdata_t *data);

#endif // end of __SDATA_H_

