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

/* Magic: taken from sbin/dhclient/packet.c */
/* Compute the checksum of the given ip header. */
unsigned short checksum(const void *data, unsigned short len, unsigned int sum)
{
    const unsigned char *addr = data;
    unsigned int i;

    /* Checksum all the pairs of bytes first... */
    for (i = 0; i < (len & ~1U); i += 2) {
        sum += (u_int16_t)ntohs(*((u_int16_t *)(addr + i)));
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    /*
     *   * If there's a single byte left over, checksum it, too.
     *       * Network byte order is big-endian, so the remaining byte is
     *           * the high byte.
     *               */
    if (i < len) {
        sum += addr[i] << 8;
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    return sum;
}

unsigned short wrapsum(unsigned int sum)
{
    sum = ~sum & 0xFFFF;
    return (htons(sum));
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

