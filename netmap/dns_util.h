/**
 * File: dns_util.h
 *
 * author: db
 */

#ifndef __DNS_UTIL_H
#define __DNS_UTIL_H

#include "config.h"
#include "types.h"

unsigned short checksum(const void *data, unsigned short len, unsigned int sum);
unsigned short wrapsum(unsigned int sum);

NM_BOOL is_dns_query(const char *buff, int len);
    
#endif // end of __DNS_UTIL_H
