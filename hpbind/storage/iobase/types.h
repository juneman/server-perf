/**
 * File: types.h
 * define marco for netmap io lib use
 * 
 * author : db
 */ 

#ifndef __TYPES_H
#define __TYPES_H

#define PROTO_LEN (14 + 20 + 8)

#define SDATA_SIZE_MAX (511)
#define SDATA_EXT_BUFF_SIZE (511)

#define NM_PKT_BUFF_SIZE_MAX (SDATA_SIZE_MAX)

/**
 *
 */
typedef unsigned char NM_BOOL;
#define NM_TRUE (1)
#define NM_FALSE (0)


/*
 * return code
 *
 */
#define NM_R_SUCCESS (0)

/// define error macro 
#define NM_R_FAILED  (-1)
#define NM_R_ARGS_NULL (-2)
#define NM_R_BUFF_NOT_ENOUGH (-3)
#define NM_R_FD_FAILED (-100)
#define NM_R_FD_OUTOFBOUND (-101)

#define NM_R_QUEUE_EMPTY (-200)
#define NM_R_QUEUE_FULL  (-201)
// end of define return code

#endif