/**
 * File: dns_util.h
 *
 * author: db
 */

#ifndef __DNS_UTIL_H
#define __DNS_UTIL_H

#include "config.h"
#include "types.h"
#include "netmapio.h"

int dns_parse_header(const char *buff, int len, netmap_address_t *addr);
int dns_build_header(char *buff, int len, const netmap_address_t *addr);
NM_BOOL is_dns_query(const char *buff, int len);
    
#endif // end of __DNS_UTIL_H
