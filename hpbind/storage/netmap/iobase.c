/**
 * File: iobase.c
 *
 * author: db
 *
 */
#include <unistd.h>
#include <sys/epoll.h>
#include "iobase.h"
#include "lfqueue.h"

#define O_NOATIME 01000000

static int verbose = -11;

// for list netmap slot limited. 
static int g_limit = 256;

static pthread_mutex_t g_nm_inited_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_nm_inited = 0;

static unsigned short g_ip_id_next = 0x11;

#define NM_IFNAME_SIZE (31)
typedef struct __netmap_ifnode_t__ {
    struct my_ring ring;
    char ifname[NM_IFNAME_SIZE + 1];
    int refcount; 

#define NM_QUEUE_CAPCITY (2048)
    lfqueue_t send_queue;
    lfqueue_t recv_queue;

    int recv_count;
    int send_count;
}netmap_ifnode_t;

#define MAX_FDS (32)
typedef struct __netmap_pipeline_t__ {
    int pipe_infd;
    int pipe_outfd;
    int nmfd;    
}netmap_pipeline_t;

#define MAX_INTERFACE_NUMS 4
typedef struct __netmap_io_manager_t__ 
{
    NM_BOOL stopped;
    pthread_t run_pid;
    int epoll_fd;

    slock_t mgtlock;

    int avail;
    netmap_ifnode_t ifnodes[MAX_INTERFACE_NUMS];
    
    // from netmap dup fd to pipe fd
    netmap_pipeline_t pipe_lines[MAX_FDS];

    // from pipe fd to netmap (dup) fd
    netmap_pipeline_t  *pipeline_map[MAX_FDS];
    
    // from netmap fd to ifnode
    netmap_ifnode_t *ifnode_map[MAX_FDS];
}netmap_io_manager_t;

static netmap_io_manager_t g_iomgr;

static int watch_fd(int efd, int fd)
{
	struct epoll_event event;
    
    assert(efd >= 0);

    event.data.fd = fd; 
    event.events = EPOLLIN | EPOLLOUT;
    if (-1 == epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event))
    {
        D("epoll_ctl error");
        return NM_R_FAILED;
    }

    return NM_R_SUCCESS;
}

static int epoll_init(netmap_io_manager_t * mgr)
{
    assert(mgr != NULL);

	mgr->epoll_fd = epoll_create(1);
	if (mgr->epoll_fd == -1)
	{
		D("epoll_create failed");
        exit(1);
	} 

    return NM_R_SUCCESS;
}

static void netmap_init_ifnode(netmap_ifnode_t *node, int capcity)
{
    assert(node != NULL);
    assert(capcity > 0);

    memset(&(node->ring), 0x0, sizeof(struct my_ring));
    memset(node->ifname, 0x0, NM_IFNAME_SIZE + 1);
    
    lfqueue_init(&(node->send_queue), capcity);
    lfqueue_init(&(node->recv_queue), capcity);

    node->refcount = 0;
    node->recv_count = 0;
    node->send_count = 0;
}

static void netmap_destroy_ifnode(netmap_ifnode_t *node)
{
    assert(node != NULL);

    memset(&(node->ring), 0x0, sizeof(struct my_ring));
    memset(node->ifname, 0x0, NM_IFNAME_SIZE + 1);

    lfqueue_destroy(&(node->send_queue));
    lfqueue_destroy(&(node->recv_queue));

    node->refcount = 0;
    node->recv_count = 0;
    node->send_count = 0;
}

static void netmap_init_pipeline(netmap_pipeline_t *line)
{
    assert(line != NULL);
    line->pipe_infd = line->pipe_outfd = line->nmfd = -1;
}

static void netmap_destroy_pipeline(netmap_pipeline_t *line)
{
    assert(line != NULL);
    line->pipe_infd = line->pipe_outfd = line->nmfd = -1;
}

static void init_io_manager(netmap_io_manager_t *mgr) 
{
    int index = 0;
    for (index = 0; index < MAX_INTERFACE_NUMS; index ++ )
    {
        netmap_ifnode_t *node = &(mgr->ifnodes[index]);
        netmap_init_ifnode(node, NM_QUEUE_CAPCITY);
    }
    
    for (index = 0; index < MAX_FDS; index ++)
    {
        netmap_pipeline_t *line = &(mgr->pipe_lines[index]);
        netmap_init_pipeline(line);
    }

    mgr->epoll_fd = -1;

    mgr->stopped = NM_FALSE;
    mgr->avail = 0;

    memset(&(mgr->ifnode_map), 0x0, sizeof(netmap_ifnode_t*) * (MAX_FDS));
    memset(&(mgr->pipeline_map), 0x0, sizeof(netmap_pipeline_t*) * (MAX_FDS));

    slock_init(&(mgr->mgtlock));
}
// must locked by mgr mgtlock by caller
static netmap_ifnode_t * 
netmap_get_available_ifnode(netmap_io_manager_t *mgr, const char *ifname)
{
    assert(mgr != NULL);
    assert(ifname != NULL);

    netmap_ifnode_t * node = NULL; 
    int index = 0;

    if (mgr->avail >= MAX_INTERFACE_NUMS)
    {
        return NULL;
    }
    
    for (index = 0; index < mgr->avail; index ++)
    {
        node = &(mgr->ifnodes[index]);
        if (strncmp(ifname, node->ifname, NM_IFNAME_SIZE) == 0)
        {
            return node;
        }
    }

    node = &(mgr->ifnodes[mgr->avail]);
    mgr->avail ++;
    node->ring.mem = NULL;
    node->ring.fd = -1;

    return node;
}

static netmap_ifnode_t* 
netmap_ifnode_by_nmfd(netmap_io_manager_t *mgr, int nmfd)
{
    assert(nmfd< MAX_FDS);
    netmap_ifnode_t *node = NULL;
    if (nmfd < MAX_FDS)
    {
        node = mgr->ifnode_map[nmfd];
    }

    return node;
}

static netmap_pipeline_t* 
netmap_pipeline_by_nmfd(netmap_io_manager_t *mgr, int nmfd)
{
    assert(nmfd< MAX_FDS);
    netmap_pipeline_t *line = NULL;
    if (nmfd < MAX_FDS)
    {
        line = &(mgr->pipe_lines[nmfd]);
    }

    return line;
}

static netmap_ifnode_t* 
netmap_ifnode_by_pipefd(netmap_io_manager_t *mgr, int pipe_fd)
{
    assert(pipe_fd < MAX_FDS);
    netmap_ifnode_t *node = NULL;
    if (pipe_fd < MAX_FDS)
    {
        netmap_pipeline_t *p = mgr->pipeline_map[pipe_fd];
        if (NULL != p && p->nmfd < MAX_FDS)
            node = mgr->ifnode_map[p->nmfd];
    }

    return node;
}

static netmap_pipeline_t *
netmap_pipeline_by_pipefd(netmap_io_manager_t *mgr, int pipe_fd)
{
    assert(pipe_fd < MAX_FDS);
    netmap_pipeline_t *line = NULL;

    if (pipe_fd < MAX_FDS)
    {
        line = mgr->pipeline_map[pipe_fd]; 
    }

    return line;
}

int netmap_openfd(const char *ifname)
{
    int fd = -1, nmfd = -1, err = NM_R_FAILED;
    int pipe_fds[2];
    netmap_ifnode_t *node = NULL;
    netmap_pipeline_t *line = NULL;

    slock_lock(&(g_iomgr.mgtlock));

    if (NULL == (
            node = netmap_get_available_ifnode(&g_iomgr, ifname)))
        goto END_L;
    
    if (node->ring.fd >= 0)
    {
        nmfd = dup(node->ring.fd);
        if (nmfd >= MAX_FDS) 
        {
            close(nmfd);
            err = NM_R_FD_OUTOFBOUND;
            goto END_L;
        }
        line = &(g_iomgr.pipe_lines[nmfd]);
        err = NM_R_SUCCESS;
        goto CREATE_PIPE_LINE;
    }

    strncpy(node->ifname, ifname, NM_IFNAME_SIZE);
    node->ring.ifname = node->ifname;
    if (NM_R_SUCCESS != (err = netmap_open(&node->ring, 0, 0)))
        goto END_L;
    
    if ((nmfd = node->ring.fd) >= MAX_FDS)
    {
        err = NM_R_FD_OUTOFBOUND;
        goto CLOSE_NETMAP_RING;
    }
 
    line = &(g_iomgr.pipe_lines[nmfd]);
    
CREATE_PIPE_LINE:    
    if (pipe(pipe_fds) < 0 )
    {
        D("pipe failed.");
        err = NM_R_FD_OUTOFBOUND;
        goto CLOSE_NETMAP_RING;
    } else {
        if (pipe_fds[0] >= MAX_FDS 
                || pipe_fds[1] >= MAX_FDS)
        {
            close(pipe_fds[0]);
            close(pipe_fds[1]);
            err = NM_R_FD_OUTOFBOUND;
            goto CLOSE_NETMAP_RING;
        }
        line->pipe_infd = pipe_fds[0];
        line->pipe_outfd = pipe_fds[1];

        int flags = fcntl(line->pipe_infd, F_GETFL, 0); 
        fcntl(line->pipe_infd, F_SETFL, flags | O_NONBLOCK | O_NOATIME);

        flags = fcntl(line->pipe_outfd, F_GETFL, 0); 
        fcntl(line->pipe_outfd, F_SETFL, flags | O_NONBLOCK);
    }
    
    node->refcount ++;
    g_iomgr.ifnode_map[nmfd] = node;
    g_iomgr.pipeline_map[line->pipe_infd] = line;
    
    watch_fd(g_iomgr.epoll_fd, nmfd);
    
    line->nmfd = nmfd;
    fd = line->pipe_infd;

    goto END_L;

CLOSE_NETMAP_RING:
    if (node->refcount == 0)
    {
        netmap_close(&(node->ring));
        close(nmfd);
    }

END_L:
    slock_unlock(&(g_iomgr.mgtlock));

    if (err != NM_R_SUCCESS) return err;
    return fd;
}

int netmap_closefd(int fd)
{
    int ret = NM_R_FAILED;
    netmap_ifnode_t *node = NULL;
    netmap_pipeline_t *line = NULL;

    slock_lock(&(g_iomgr.mgtlock));

    node = netmap_ifnode_by_pipefd(&g_iomgr, fd);
    if (NULL == node) goto END_L;
    
    line = netmap_pipeline_by_pipefd(&g_iomgr, fd); 
    if (NULL != line)
    {
        close(line->pipe_infd);
        close(line->pipe_outfd);
        close(line->nmfd);

        g_iomgr.ifnode_map[line->nmfd] = NULL;
        g_iomgr.pipeline_map[line->pipe_infd] = NULL;
        netmap_destroy_pipeline(line); 
    }
    else
    {
        close(node->ring.fd);
    }
     
    assert(node->refcount > 0);
    node->refcount --;
    if (node->refcount <= 0)
    {
        netmap_close(&(node->ring));
        netmap_destroy_ifnode(node);
    }

END_L:
    slock_unlock(&(g_iomgr.mgtlock));

    return ret;
}

static int parse_addr(const char *buff, int len, netmap_address_t *addr)
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

    if (verbose > -1)
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

    return NM_R_SUCCESS;
}

static int build_pkt_header(char *buff, int len, const netmap_address_t *addr)
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

    if (verbose > -1)
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

static int process_recv_ring(struct netmap_ring *rxring, 
        char *buff, int buff_len, netmap_address_t *addr, int limit) 
{
    u_int j, m = 0;
    int recv_bytes = 0;

    j = rxring->cur; /* RX */
    if (rxring->avail < limit)
        limit = rxring->avail;
    while (m < limit) {
        struct netmap_slot *rs = &rxring->slot[j];
        char *rxbuf = NETMAP_BUF(rxring, rs->buf_idx);

        if ( rs->len >= NM_PKT_BUFF_SIZE_MAX
                || rs->len >= buff_len + PROTO_LEN
                || rs->len <= PROTO_LEN)
        {
            goto NEXT_L;
        }

        if (NM_TRUE != is_dns_query(rxbuf, rs->len)) 
        {
            goto NEXT_L;
        }

        if (addr != NULL) parse_addr(rxbuf, rs->len, addr);

        recv_bytes = rs->len - PROTO_LEN;
        memcpy(buff, rxbuf + PROTO_LEN , recv_bytes);

NEXT_L:	
        j = NETMAP_RING_NEXT(rxring, j);
        m++;

        rs->flags |= NS_BUF_CHANGED;

        if (recv_bytes > 0) 
        {
            break;
        }
    }

    rxring->avail -= m;
    rxring->cur = j;

    if (recv_bytes <= 0) 
    {
        return NM_R_FAILED;
    }

    return (recv_bytes);
}

static int process_send_ring(struct netmap_ring *txring, 
        const char *buff, int data_len, const netmap_address_t *addr)
{
    u_int k = txring->cur; /* TX */
    struct netmap_slot *ts = &txring->slot[k];
    char *txbuf = NETMAP_BUF(txring, ts->buf_idx);

    assert(data_len > 0);

    ts->len = data_len + PROTO_LEN; 
    memcpy(txbuf + PROTO_LEN, buff, data_len);

    build_pkt_header(txbuf, ts->len, addr);

    k = NETMAP_RING_NEXT(txring, k);
    ts->flags |= NS_BUF_CHANGED;

    txring->avail -= 1;
    txring->cur = k;

    return (data_len);
}

static int netmap_recv_from_ring(struct my_ring *src, 
        char *buff, int buff_len, netmap_address_t *addr, int limit)
{
    struct netmap_ring *rxring;
    u_int ri = src->begin;
    int ret = NM_R_FAILED;

    while (ri < src->end) 
    {
        rxring = NETMAP_RXRING(src->nifp, ri);
        if (rxring->avail == 0) 
        {
            ri ++;
            continue;
        }

        ret = process_recv_ring(rxring, buff, buff_len, addr, limit);
        if (ret > 0) 
        {
            break;
        }

        ri ++;
    }

    return ret;
}

static int netmap_send_to_ring(struct my_ring *src, 
        const char *buff, int data_len, const netmap_address_t *addr)
{
    struct netmap_ring *txring;
    u_int ti=src->begin;
    int ret = NM_R_FAILED;
    
    ti=src->begin;
    while (ti < src->end) {
        txring = NETMAP_TXRING(src->nifp, ti);

        if (txring->avail == 0) 
        {
            ti++;
            continue;
        }   

        ret = process_send_ring(txring, buff, data_len, addr);
        if (ret > 0) 
        {
            return ret;
        }

        ti ++;
    }
    
    return NM_R_FAILED;
}


int netmap_recv(int fd, char *buff, int buff_len, netmap_address_t *addr) 
{
    netmap_pipeline_t *line = NULL; 
    netmap_ifnode_t *node = NULL; 

    if (buff == NULL || buff_len <= 0) return NM_R_ARGS_NULL;

    line = netmap_pipeline_by_pipefd(&g_iomgr, fd);
    assert(line != NULL);
    if (line == NULL) return NM_R_FD_OUTOFBOUND;
    
    node = netmap_ifnode_by_pipefd(&g_iomgr, fd);
    assert(node != NULL);
    if (node == NULL) return NM_R_FD_OUTOFBOUND;
    
    char buf[1];
    read(line->pipe_infd, buf, sizeof(buf));

    int n = lfqueue_dequeue(&(node->recv_queue), buff, buff_len, (char*)addr, sizeof(netmap_address_t));
    if (n <= 0) 
    {
        return NM_R_QUEUE_EMPTY;
    }

    return n;
}

int netmap_send(int fd, const char *buff, int data_len, const netmap_address_t *addr)
{
    netmap_ifnode_t *node = NULL; 
    
    assert(buff != NULL);
    assert(data_len > 0 && data_len < NM_PKT_BUFF_SIZE_MAX);
    assert(addr != NULL);

    if (buff == NULL 
            || data_len <= 0 
            || data_len >= NM_PKT_BUFF_SIZE_MAX
            || addr == NULL )
    {
        return NM_R_ARGS_NULL;
    }

    node = netmap_ifnode_by_pipefd(&g_iomgr, fd);
    assert(node != NULL);
    if (node == NULL) return NM_R_FD_OUTOFBOUND;
    
    int n = lfqueue_enqueue(&(node->send_queue), buff, data_len, (char*)addr, sizeof(netmap_address_t));
    if (n <= 0)
    {
        return NM_R_FAILED;
    }

    return n;
}

// only in main thread call ONCE time.
int netmap_init(void)
{
    if (pthread_mutex_trylock(&g_nm_inited_lock) == EBUSY)
    {
        return NM_R_SUCCESS; 
    }

    if (g_nm_inited == 1)
    {
        return NM_R_SUCCESS; 
    }

    g_nm_inited = 1;

#ifdef __BUILD_TIME 
    printf(" BUILD AT: %s\n", __BUILD_TIME);
#endif

    init_io_manager(&g_iomgr);
    epoll_init(&g_iomgr);

    pthread_mutex_unlock(&g_nm_inited_lock);

    return NM_R_SUCCESS; 
}

static int __netmap_recvfrom__(int nmfd)
{
    int have_data = NM_R_FAILED;
    netmap_ifnode_t *node = NULL;
 
    node = netmap_ifnode_by_nmfd(&g_iomgr, nmfd);
    if (NULL == node) return NM_R_FD_OUTOFBOUND;
    
    sdata_t data;
LOOP:
    {
        int recv_bytes = netmap_recv_from_ring(&(node->ring), 
                              data.buff, SDATA_SIZE_MAX, 
                             (netmap_address_t *)&(data.extbuff), g_limit);
        if (recv_bytes > 0) 
        { 
            data.len = recv_bytes;
            lfqueue_enqueue(&(node->recv_queue), 
                    data.buff, data.len, data.extbuff, sizeof(netmap_address_t)); 
            have_data = NM_R_SUCCESS;
            node->recv_count ++;

			goto LOOP;
        }
    }

    return have_data;
}

static int __netmap_sendto__(int nmfd)
{
    netmap_ifnode_t *node = NULL;
    
    node = netmap_ifnode_by_nmfd(&g_iomgr, nmfd);
    if (NULL == node) return NM_R_FD_OUTOFBOUND;
    
    sdata_t data;
LOOP:
    {
        int n = lfqueue_dequeue(&(node->send_queue), 
                    data.buff, SDATA_SIZE_MAX, data.extbuff, sizeof(netmap_address_t));
        if (n <= 0) return NM_R_QUEUE_EMPTY;
        data.len = n;

        n = netmap_send_to_ring(&(node->ring), data.buff, data.len, 
                                        (netmap_address_t*)(data.extbuff));
        
		if (n > 0 ) {
            node->send_count ++;
            goto LOOP;
        }
    }

    return NM_R_SUCCESS;
}

static int __netmap_signal__(int nmfd)
{
    netmap_pipeline_t * line = NULL;
    static char _null_buff_[1024 * 64];

    line = netmap_pipeline_by_nmfd(&g_iomgr, nmfd);
    if (NULL != line)
    {
        char buff[1] = {'a'};
        if (write(line->pipe_outfd, buff, sizeof(buff)) <= 0)
        {
            read(line->pipe_infd, _null_buff_, sizeof(_null_buff_));
            write(line->pipe_outfd, buff, sizeof(buff));
        }
        return NM_R_SUCCESS;
    }

    return NM_R_FAILED;
}

void * __netmap_run__(void*args)
{
	int nfd = 0;
    int index = 0;
	struct epoll_event *events = NULL;

    netmap_io_manager_t *iomgr = (netmap_io_manager_t*)(args);

    //----------------------------
    assert(iomgr != NULL);
    assert(iomgr->epoll_fd >= 0);
 
    events = calloc(MAX_FDS, sizeof(struct epoll_event));
    if (events == NULL) 
    {
        D("calloc failed");
        exit(1);
    }
   
    while (!iomgr->stopped) 
    {
        nfd = epoll_wait(iomgr->epoll_fd, events, MAX_FDS, -1);
        if (nfd < 0) continue;
        for (index = 0; index < nfd; index ++)
        {
            int ppfd = events[index].data.fd;
            if ( events[index].events & EPOLLIN 
                    || events[index].events & EPOLLOUT)
            {
                int have_data =  __netmap_recvfrom__(ppfd);
                if (NM_R_SUCCESS == have_data) __netmap_signal__(ppfd);
            }
        }
    }

    D("exiting");
    free(events);

   	return NULL;
}

void *__netmap_flush__(void *args)
{
    netmap_io_manager_t *iomgr = (netmap_io_manager_t*)(args);
    int index = 0;

    while (!iomgr->stopped) 
    {
        for (index = 0; index < iomgr->avail; index ++)
        {
            netmap_ifnode_t *node = &(iomgr->ifnodes[index]);
            __netmap_sendto__(node->ring.fd);
        }
        usleep(1);
    }

    return NULL;
}


void netmap_setup(void)
{
    pthread_create(&(g_iomgr.run_pid), NULL, __netmap_run__, (void*)(&g_iomgr));
    pthread_detach(g_iomgr.run_pid);
    
    pthread_t pid;
    pthread_create(&pid, NULL, __netmap_flush__, (void*)(&g_iomgr));
    pthread_detach(pid);
}

void netmap_teardown(void)
{
    int index = 0;

    g_iomgr.stopped = NM_TRUE;
    
    for (index = 0; index < g_iomgr.avail; index ++)
    {
        netmap_ifnode_t *node = &(g_iomgr.ifnodes[index]);
        printf(" On %s recv:%d, send:%d\n", node->ifname, node->recv_count, node->send_count);
        fflush(stdout);
    }
}

int netmap_destroy(void)
{
   
    return NM_R_SUCCESS; 
}


