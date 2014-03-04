/***
 *
 * file: cbuff.h
 *      implement a circular-buff 
 *
 * author: db
 * date: 2014-01-21
 */ 

#ifndef __CBUFF_H_
#define __CBUFF_H_

#include "config.h"
#include "types.h"
#include "slist.h"
#include "slock.h"
#include "sdata.h"

typedef struct __cbuff_t__
{
    char *__buff__;
    int capcity;
    int readp; // last read
    int writep; // next write
#ifdef CBUFF_USE_LOCK
    slock_t lock;
#endif
}cbuff_t;

#define cbuff_empty(buff) \
    ((((buff)->readp + 1) % ((buff)->capcity) == (buff)->writep) ? 1:0)

#define cbuff_full(buff) \
    ((((buff)->writep) == ((buff)->readp + (buff)->capcity ) % ((buff)->capcity)) ? 1: 0)

#define cbuff_pos(buff, pos) \
   (sdata_t *)((buff)->__buff__ + (sizeof(sdata_t) * (pos)))

#define cbuff_writeable_entry(buff) \
    cbuff_pos((buff), (buff)->writep)
#define cbuff_writeable_next(buff) do {\
    (buff)->writep ++; (buff)->writep %= (buff)->capcity;}while(0)

#define cbuff_readable_entry(buff) \
    cbuff_pos((buff), ((buff)->readp + 1)%((buff)->capcity))
#define cbuff_readable_next(buff) do {\
    (buff)->readp ++; (buff)->readp %= (buff)->capcity;}while(0)

inline int cbuff_init(cbuff_t *buff, int capcity);

inline int cbuff_read(cbuff_t *buff, char *dest, int len, char *extbuff, int extlen);

// return : bytes writed success
inline int cbuff_write(cbuff_t *buff, const char *src, int len, const char *extbuff, int extlen);

inline void cbuff_destroy(cbuff_t *buff);

#endif
