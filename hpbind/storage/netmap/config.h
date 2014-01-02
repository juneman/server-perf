/*
 * File: config.h
 * Date: 2013-12-25
 * 
 * author: db
 * 
 *
 */
#ifndef _CONFIG_H
#define _CONFIG_H
// for enable/disable debug
#define NM_DEBUG

//#define NM_DBG_RECV_ECHO
//#define NM_DBG_RECV_ECHO_SINGLE
//#define NM_DBG_SEND_ECHO
//#define NM_DBG_SEND_ECHO_STEP 3

// lock lib
#define NM_USE_MUTEX_LOCK
//#define NM_USE_SPIN_LOCK

// lock scope
#define NM_HAVE_G_LOCK 

#endif /* _CONFIG_H*/
