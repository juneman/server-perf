/**
 * File: dns_util.c
 *
 * author: db
 *
 */

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/ethtool.h>
#include "dns_util.h"

unsigned short in_cksum(const unsigned short *addr, int len) 
{ 
    int sum=0; 
    unsigned short res=0; 
    while( len > 1)  { 
        sum += *addr++; 
        len -=2; 
    } 
    if( len == 1) { 
        *((unsigned char *)(&res))=*((unsigned char *)addr); 
        sum += res; 
    } 
    sum = (sum >>16) + (sum & 0xffff); 
    sum += (sum >>16) ; 
    res = ~sum; 
    return res; 
}

NM_BOOL is_dns_query(const char *buff, int len)
{
    struct ethhdr *eh;
    struct iphdr *ip;
    struct udphdr *udp;
    char *ip_buff;

    eh = (struct ethhdr*)buff;
    ip_buff = (char *)((char*)eh + sizeof(struct ethhdr));
    ip = (struct iphdr*) (ip_buff);
    udp = (struct udphdr *) (ip_buff + sizeof(struct iphdr));

    if (len < PROTO_LEN + 8
         || eh->h_proto != ntohs(0x0800) 
         || ip->protocol != IPPROTO_UDP 
         || udp->dest != ntohs(53)) 
        return NM_FALSE;

    return NM_TRUE;
}

