/**
 * File: dns_util.h
 *
 * author: db
 */

#ifndef __DNS_UTIL_H
#define __DNS_UTIL_H

#include "config.h"

struct pesudo_udphdr { 
    unsigned int saddr, daddr; 
    unsigned char unused; 
    unsigned char protocol; 
    unsigned short udplen; 
}; 

unsigned short in_cksum(unsigned short *addr, int len); 
int is_dns_query(char *buff, int len);
    
#endif // end of __DNS_UTIL_H
