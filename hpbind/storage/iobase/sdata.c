/**
 * File : sdata.h
 *
 * author: db
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sdata.h"


inline int sdata_init(sdata_t *data)
{
    assert(data != NULL);
    memset(data, 0x0, sizeof(sdata_t));
#ifdef SQUEUE_USE_LIST
    slist_init(&(data->node));
#endif
    data->magic = SDATA_MAGIC;
    return 0;
}
