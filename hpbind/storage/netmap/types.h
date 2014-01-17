/**
 * File: types.h
 * define marco for netmap io lib use
 * 
 * author : db
 */ 

#ifndef __TYPES_H
#define __TYPES_H

#define PROTO_LEN (14 + 20 + 8)

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
#define NM_R_FD_OUTOFBIND (-100)
// end of define return code

#endif
