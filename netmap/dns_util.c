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

unsigned short in_cksum(unsigned short *addr, int len) 
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

int is_dns_query(char *buff, int len)
{
    struct ethhdr *eh;
    struct iphdr *ip;
    struct udphdr *udp;

    char *ip_buff = buff + 14;

    eh = (struct ethhdr*)buff;
    ip = (struct iphdr*) (ip_buff);
    udp = (struct udphdr *) (ip_buff + sizeof(struct iphdr));

    if (len < 14 + 20 + 8 + 8) 
    {
        return 1;
    }

    if (eh->h_proto != ntohs(0x0800))
    {
        return 2;
    }

    if (ip->protocol != IPPROTO_UDP )
    {
        return 3;
    }

    if (udp->dest != ntohs(53))
    {
        return 4;
    }

    return 0;
}

