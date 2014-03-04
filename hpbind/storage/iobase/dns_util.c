/**
 * File: dns_util.c
 *
 * author: db
 *
 */

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/ethtool.h>
#include "cs_util.h"
#include "dns_util.h"

static volatile unsigned short g_ip_id_next = 0x11;

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

int dns_parse_header(const char *buff, int len, netmap_address_t *addr)
{
    struct ethhdr *eh = NULL; 
    struct iphdr *ip = NULL; 
    struct udphdr *udp = NULL; 
    char *ip_buff = NULL;

    eh = (struct ethhdr *)buff;
    ip_buff =(char*) (((char*)eh) + sizeof(struct ethhdr));
    
    ip = (struct iphdr*) (ip_buff);
    udp = (struct udphdr *) (ip_buff + sizeof(struct iphdr));

    {
        addr->remote_addr = ip->saddr;
        addr->local_addr = ip->daddr;
        addr->remote_port = udp->source;
        addr->local_port = udp->dest;
        
        memcpy(addr->local_macaddr, buff, 6);
        memcpy(addr->remote_macaddr, buff + 6, 6);
    }

#ifdef __DEBUG_VERB__
    {
        char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));
        unsigned short qid = ((unsigned short*) query)[0];
        printf("parse addr : remote addr:%d, local addr:%d, remote port:%d, local port:%d, "
                " remote mac:-%x-%x-%x, local mac:-%x-%x-%x, "
                "qid:0x%x\n",
                (unsigned int )addr->remote_addr, 
                (unsigned int)addr->local_addr, 
                htons(addr->remote_port), htons(addr->local_port), 
                addr->remote_macaddr[3],
                addr->remote_macaddr[4],
                addr->remote_macaddr[5],
                addr->local_macaddr[3],
                addr->local_macaddr[4],
                addr->local_macaddr[5],
                qid);
    }
#endif

    return NM_R_SUCCESS;
}

int dns_build_header(char *buff, int len, const netmap_address_t *addr)
{
    char *ip_buff = buff + sizeof(struct ethhdr);

    struct iphdr* ip = (struct iphdr*)ip_buff; 
    struct udphdr * udp = (struct udphdr*) (ip_buff + sizeof(struct iphdr ));

    char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));

    // Filling Ethnet header
    memcpy(buff, addr->remote_macaddr, 6);
    memcpy(buff + 6, addr->local_macaddr, 6);

    // mark it is IP packet
    buff[12] = 0x8;
    buff[13] = 0x0;

#ifdef __DEBUG_VERB__
    {
        struct ethhdr *eh = (struct ethhdr *)buff;
        unsigned short qid = ((unsigned short*) query)[0];
        printf("build local mac:%x-%x-%x-%x-%x-%x," 
                "remote mac:%x-%x-%x-%x-%x-%x,"
                "local addr:%d,remote addr:%d, "
                "local port:%d, remote port:%d, qid:0x%x\n",
                eh->h_source[0],eh->h_source[1],eh->h_source[2],
                eh->h_source[3],eh->h_source[4],eh->h_source[5],
                eh->h_dest[0],eh->h_dest[1],eh->h_dest[2],
                eh->h_dest[3],eh->h_dest[4],eh->h_dest[5],
                (unsigned int )addr->local_addr,
                (unsigned int )addr->remote_addr,
                htons(addr->local_port), 
                htons(addr->remote_port), 
                qid);
    }
#endif

    // Filling ip header
    ip->daddr = addr->remote_addr;
    ip->saddr = addr->local_addr;
    ip->tot_len = htons(len - 14);
    ip->id = (++g_ip_id_next & 0xFFFF);
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->check = 0;
    ip->check = wrapsum(checksum(ip, sizeof(struct iphdr), 0)); 

    // Filling UDP header
    udp->len = htons(len - sizeof(struct iphdr ) - sizeof(struct ethhdr));
    udp->dest = addr->remote_port;
    udp->source = addr->local_port;
    udp->check = 0;
    udp->check = wrapsum(checksum(udp, sizeof(struct udphdr),
                checksum(query,
                    len - PROTO_LEN,
                    checksum(&ip->saddr, 2 * sizeof(ip->saddr),
                        IPPROTO_UDP + (u_int32_t)ntohs(udp->len)
                        )
                    )
                ));

    return NM_R_SUCCESS;
}

