#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdata.h"


inline int sdata_init(sdata_t *data)
{
    assert(data != NULL);
    memset(data, 0x0, sizeof(sdata_t));
    slist_init(&(data->node));
    return 0;
}
