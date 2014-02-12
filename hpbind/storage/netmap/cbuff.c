/***
 *
 * file: cbuff.c
 *      implement a circular-buff 
 *
 * author: db
 * date: 2014-01-21
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdata.h"
#include "cbuff.h"

inline int cbuff_init(cbuff_t *buff, int capcity)
{
    assert(buff != NULL);
    int index = 0;
    sdata_t *data = NULL;

    buff->capcity = capcity;
    buff->readp = -1;
    buff->writep = 0;
    
    if (capcity > 0)
    {
        buff->__buff__ = (char*)malloc(sizeof(sdata_t) * capcity);
        assert(buff->__buff__ != NULL);
        if (NULL == buff->__buff__) return 1;

        for (index = 0; index < capcity; index ++)
        {
            data = cbuff_pos(buff, index);
            sdata_init(data);
        }
    }
    
    slock_init(&(buff->lock));
    return 0;
}

// return: bytes write success
inline int cbuff_read(cbuff_t *buff, char *dest, int blen, char *extbuff, int extlen)
{
    assert(buff != NULL);
    assert(dest != NULL);
    assert(blen > 0 );
    
    sdata_t *data = NULL;
    int ret = -1;
    
    slock_lock(&(buff->lock));
	assert(buff != NULL);
    if (cbuff_empty(buff)) goto END_L;

    int readp = buff->readp;
    readp = (readp + 1) % buff->capcity;
    data = cbuff_pos(buff, readp); 
    buff->readp = readp;
    
    if (data->len > blen)
    {
        goto END_L;
    }

    memcpy(dest, data->buff, data->len);
    ret = data->len;

    if (NULL != extbuff)
    {
        assert(extlen > 0);
        assert(extlen  < SDATA_EXT_BUFF_SIZE);
        
        memcpy(extbuff, data->extbuff, extlen);
    }

END_L:
    slock_unlock(&(buff->lock));

    return (ret);
}

// return : bytes writed success
inline int cbuff_write(cbuff_t *buff, const char *src, int len, const char *extbuff, int extlen)
{
    assert(buff != NULL);
    assert(src  != NULL);
    assert(len  > 0);
    assert(len  < SDATA_SIZE_MAX);
    

    slock_lock(&(buff->lock));
    if (cbuff_full(buff)) goto END_L;

    int writep = buff->writep;
    sdata_t *data = cbuff_pos(buff, writep);
    memcpy(data->buff, src, len);
    data->len = len;
    
    if (NULL != extbuff)
    {
        assert(extlen > 0 );
        assert(extlen  < SDATA_EXT_BUFF_SIZE);
        memcpy(data->extbuff, extbuff, extlen);
    }
    
    writep = (writep + 1) % buff->capcity;
    buff->writep = writep;

END_L:
    slock_unlock(&(buff->lock));

    return len;
}

inline void cbuff_destroy(cbuff_t *buff)
{
    assert(buff != NULL);
    
    if (buff->capcity > 0)
        free(buff->__buff__);

    buff->__buff__ = NULL;
    buff->capcity = 0;
    buff->readp = -1;
    buff->writep = 0;
}
