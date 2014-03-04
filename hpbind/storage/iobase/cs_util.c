/**
 * File: cs_util.c
 *      checksum 
 *
 * author: db
 *
 */
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/ethtool.h>
#include "cs_util.h" 

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

