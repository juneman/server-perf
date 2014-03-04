/**
 * File: iobase-sq.c
 *
 * author: db
 *
 */
#include <unistd.h>
#include <sys/epoll.h>
#include "build.h"
#include "slock.h"
#include "slist.h"
#include "squeue.h"

#include "cs_util.h"
#include "dns_util.h"
#include "poll_util.h"

#include "iobase.h"

#ifdef SQUEUE_USE_LFDS
#define NM_LIB "LFQ-LIB"
#else
#define NM_LIB "SQ-LIB"
#endif

typedef struct __iobase_ifnode_t__ {
    netmapio_ringobj ring;
    char ifname[NM_IFNAME_SIZE + 1];
    int refcount; 

    slist_node_t *pipelines;
    slist_node_t *next_line;

    int recv_count;
    int send_count;
}iobase_ifnode_t;

typedef struct __iobase_pipeline_t__ {
    int magic;
    int pipe_infd;
    int pipe_outfd;
    int nmfd;
    int actived;
    
    slist_node_t node;

    squeue_t send_queue;
    squeue_t send_idle_queue;
    squeue_t recv_queue;
    squeue_t recv_idle_queue;

    int enq_nums;
    int deq_nums;
}iobase_pipeline_t;

typedef struct __iobase_manager_t__ 
{
    volatile NM_BOOL stopped;

#ifdef IOBASE_HAS_NOTIFY    
    NM_BOOL has_notify;
#endif

#ifdef IOBASE_IOSYNC_USE_CAS
	volatile int _io_sync;
#else
	slock_t iolock;
#endif

#ifdef IOBASE_HAS_REPORT
    int interupt_count[MAX_FDS];
    int signal_count[MAX_FDS];
#endif

    pthread_t run_pid;
    pthread_t send_pid;
    int epoll_fd;

    slock_t mgtlock;

    volatile int avail;
    iobase_ifnode_t ifnodes[MAX_INTERFACE_NUMS];
    
    // from pipe fd to netmap (dup) fd
    iobase_pipeline_t  *pipeline_map[MAX_FDS];
    
    // from netmap fd to ifnode
    iobase_ifnode_t *ifnode_map[MAX_FDS];
}iobase_manager_t;

static iobase_manager_t g_iomgr;

/**
 *
 */
#ifdef IOBASE_HAS_NOTIFY
static int iobase_notify(int nmfd);
static int iobase_clear_notify(int userfd);
#endif

static void iobase_init_pipeline(iobase_pipeline_t *line, int capcity)
{
    assert(line != NULL);
    assert(capcity > 0);

    line->magic = NM_PIPELINE_MAGIC;
    line->pipe_infd = line->pipe_outfd = line->nmfd = -1;
    line->actived = 0;
    line->enq_nums = 0;
    line->deq_nums = 0;

    squeue_init(&(line->recv_queue), capcity, 0);
    squeue_init(&(line->send_queue), capcity, 0);
    squeue_init(&(line->recv_idle_queue), capcity, capcity);
    squeue_init(&(line->send_idle_queue), capcity, capcity);
    slist_init(&(line->node));
}

static void iobase_destroy_pipeline(iobase_pipeline_t *line)
{
    assert(line != NULL);
    assert(NM_PIPELINE_VALID(line));

    line->pipe_infd = line->pipe_outfd = line->nmfd = -1;
    line->actived = 0;

    squeue_destroy(&(line->recv_queue));
    squeue_destroy(&(line->send_queue));
    squeue_destroy(&(line->recv_idle_queue));
    squeue_destroy(&(line->send_idle_queue));
}

static void iobase_init_ifnode(iobase_ifnode_t *node, int capcity)
{
    assert(node != NULL);
    assert(capcity > 0);

    memset(&(node->ring), 0x0, sizeof(netmapio_ringobj));
    memset(node->ifname, 0x0, NM_IFNAME_SIZE + 1);
    
    node->ring.filter = is_dns_query;
    node->ring.parse_header = dns_parse_header;
    node->ring.build_header = dns_build_header;

    node->pipelines = NULL;
    node->next_line = NULL;

    node->refcount = 0;
    node->recv_count = 0;
    node->send_count = 0;
}

static void iobase_destroy_ifnode(iobase_ifnode_t *node)
{
    assert(node != NULL);

    memset(&(node->ring), 0x0, sizeof(netmapio_ringobj));
    memset(node->ifname, 0x0, NM_IFNAME_SIZE + 1);

    iobase_pipeline_t *line, *n;
    if (NULL != node->pipelines)
    {
        slist_foreach_entry_safe(line, n, node->pipelines, iobase_pipeline_t, node)
        {
            assert(NM_PIPELINE_VALID(line));
            slist_delete(&(line->node));
            free(line);
        }

        line = slist_entry(node->pipelines, iobase_pipeline_t, node);
        assert(NM_PIPELINE_VALID(line));
        free(line);
    }
    
    node->pipelines = NULL;
    node->next_line = NULL;

    node->refcount = 0;
    node->recv_count = 0;
    node->send_count = 0;
}

static void iobase_manager_init(iobase_manager_t *mgr) 
{
    int index = 0;
    for (index = 0; index < MAX_INTERFACE_NUMS; index ++ )
    {
        iobase_ifnode_t *node = &(mgr->ifnodes[index]);
        iobase_init_ifnode(node, NM_QUEUE_CAPCITY);
    }
 
    memset(&(mgr->ifnode_map), 0x0, sizeof(iobase_ifnode_t*) * (MAX_FDS));
    memset(&(mgr->pipeline_map), 0x0, sizeof(iobase_pipeline_t*) * (MAX_FDS));
   
    mgr->epoll_fd = -1;
    mgr->stopped = NM_FALSE;
    mgr->avail = 0;
#ifdef IOBASE_HAS_NOTIFY
    mgr->has_notify = NM_TRUE;    
#endif

#ifdef IOBASE_IOSYNC_USE_CAS
    mgr->_io_sync = 0;
#else
    slock_init(&(mgr->iolock));
#endif
    slock_init(&(mgr->mgtlock));
}
// must locked by mgr mgtlock by caller
static iobase_ifnode_t * 
get_available_ifnode(iobase_manager_t *mgr, const char *ifname)
{
    assert(mgr != NULL);
    assert(ifname != NULL);

    iobase_ifnode_t * node = NULL; 
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

static iobase_pipeline_t *
get_available_pipeline(iobase_manager_t *mgr)
{
    (void)mgr;

    iobase_pipeline_t *line = NULL;
    line = (iobase_pipeline_t *) malloc(sizeof(iobase_pipeline_t));
    if (NULL == line) return NULL;
    
    iobase_init_pipeline(line, NM_QUEUE_CAPCITY);

    return line;
}

static int 
add_pipeline_to_ifnode(iobase_ifnode_t *ifnode, iobase_pipeline_t *line)
{   
    assert(ifnode != NULL);
    assert(line != NULL);

    if (NULL == ifnode->pipelines) 
    {
        ifnode->pipelines = &(line->node);
        ifnode->next_line = ifnode->pipelines;
    }
    else
    {
        slist_add_tail(ifnode->pipelines, &(line->node));
    }

    return NM_R_SUCCESS;
}

// TODO: need to code it.
//static int netmap_ifnode_del_pipeline(iobase_ifnode_t *ifnode, iobase_pipeline_t *line)
//{
//
//}

static iobase_ifnode_t* 
ifnode_mapfrom_nmfd(iobase_manager_t *mgr, int nmfd)
{
    assert(nmfd< MAX_FDS);
    iobase_ifnode_t *node = NULL;
    if (nmfd < MAX_FDS)
    {
        node = mgr->ifnode_map[nmfd];
    }

    return node;
}

static iobase_pipeline_t* 
next_pipeline_mapfrom_nmfd(iobase_manager_t *mgr, int nmfd)
{
    assert(nmfd< MAX_FDS);
    iobase_pipeline_t *line = NULL;
    iobase_ifnode_t *node = NULL;
    
    if (nmfd >= MAX_FDS) return NULL;
    node = ifnode_mapfrom_nmfd(mgr, nmfd);
    if (NULL == node) return NULL;
    if (NULL == node->next_line ) return NULL;

    line = slist_entry(node->next_line, iobase_pipeline_t, node);
    CHECK_PIPELINE_AND_EXIT(line);
    if (!line->actived) return NULL;

    return line;
}

static int move_to_next_pipeline_by_nmfd(iobase_manager_t *mgr, int nmfd)
{
    assert(nmfd< MAX_FDS);
    iobase_ifnode_t *node = NULL;
    iobase_pipeline_t *line = NULL;

    if (nmfd >= MAX_FDS) return NM_R_FD_OUTOFBOUND;
    node = ifnode_mapfrom_nmfd(mgr, nmfd);
    if (NULL == node) return NM_R_FAILED;

    if (NULL == node->next_line->next) return NM_R_FAILED;

    line = slist_entry(node->next_line->next, iobase_pipeline_t, node);
    CHECK_PIPELINE_AND_EXIT(line);
    node->next_line = node->next_line->next;

    return NM_R_SUCCESS;
}

static iobase_ifnode_t* 
ifnode_mapfrom_userfd(iobase_manager_t *mgr, int pipe_fd)
{
    assert(pipe_fd < MAX_FDS);
    iobase_ifnode_t *node = NULL;
    if (pipe_fd < MAX_FDS)
    {
        iobase_pipeline_t *p = mgr->pipeline_map[pipe_fd];
        if (NULL != p && p->nmfd < MAX_FDS)
            node = mgr->ifnode_map[p->nmfd];
    }

    return node;
}

static iobase_pipeline_t *
pipeline_mapfrom_userfd(iobase_manager_t *mgr, int pipe_fd)
{
    assert(pipe_fd < MAX_FDS);
    iobase_pipeline_t *line = NULL;

    if (pipe_fd < MAX_FDS)
    {
        line = mgr->pipeline_map[pipe_fd]; 
    }
    
    if (NULL == line) return NULL;
    CHECK_PIPELINE_AND_EXIT(line);
    if (!line->actived) return NULL;
    return line;
}

int iobase_openfd(const char *ifname)
{
    int fd = -1, nmfd = -1, err = NM_R_FAILED;
    int pipe_fds[2];
    iobase_ifnode_t *node = NULL;
    iobase_pipeline_t *line = NULL;

    slock_lock(&(g_iomgr.mgtlock));

    if (NULL == (
            node = get_available_ifnode(&g_iomgr, ifname)))
        goto END_L;
    
    if (node->ring.fd >= 0)
    {
        nmfd = node->ring.fd;
        line = get_available_pipeline(&g_iomgr);
        if (NULL == line) goto END_L;
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
 
    line = get_available_pipeline(&g_iomgr);
    if (NULL == line)
    {
        err = NM_R_FD_OUTOFBOUND;
        goto CLOSE_NETMAP_RING;
    }
     
    g_iomgr.ifnode_map[nmfd] = node;
    watch_fd(g_iomgr.epoll_fd, nmfd);

    D("On %s watch fd:%d", node->ifname, node->ring.fd);

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
    line->nmfd = nmfd;
    line->actived = 1;
    g_iomgr.pipeline_map[line->pipe_infd] = line;
    add_pipeline_to_ifnode(node, line);
        
    fd = line->pipe_infd;
    goto END_L;

CLOSE_NETMAP_RING:
    if (node->refcount == 0)
    {
        netmap_close(&(node->ring));
        g_iomgr.ifnode_map[nmfd] = NULL;
        unwatch_fd(g_iomgr.epoll_fd, nmfd);
        close(nmfd);
    }

END_L:
    slock_unlock(&(g_iomgr.mgtlock));

    if (err != NM_R_SUCCESS) return err;
    return fd;
}

int iobase_closefd(int fd)
{
    int ret = NM_R_FAILED;
    iobase_ifnode_t *node = NULL;
    iobase_pipeline_t *line = NULL;

    slock_lock(&(g_iomgr.mgtlock));

    node = ifnode_mapfrom_userfd(&g_iomgr, fd);
    if (NULL == node) goto END_L;
    
    line = pipeline_mapfrom_userfd(&g_iomgr, fd); 
    if (NULL != line)
    {
        close(line->pipe_infd);
        close(line->pipe_outfd);
// 
// TODO: Need fix it. by db.
// need to delete this line form ifnode. 
// and at __iobase_run__ runtime, need to avoid access this line.
//
        g_iomgr.pipeline_map[line->pipe_infd] = NULL;
        iobase_destroy_pipeline(line); 
    }
     
    assert(node->refcount > 0);
    node->refcount --;
    D("%s refcount:%d", node->ifname, node->refcount);
    if (node->refcount <= 0)
    {
        netmap_close(&(node->ring));
        close(node->ring.fd);

        unwatch_fd(g_iomgr.epoll_fd, node->ring.fd);
        g_iomgr.ifnode_map[line->nmfd] = NULL;
        iobase_destroy_ifnode(node);
    }

END_L:
    slock_unlock(&(g_iomgr.mgtlock));

    return ret;
}

int iobase_recv(int fd, char *buff, int buff_len, iobase_address_t *addr) 
{
    iobase_pipeline_t *line = NULL; 

    if (buff == NULL || buff_len <= 0) return NM_R_ARGS_NULL;

    line = pipeline_mapfrom_userfd(&g_iomgr, fd);
    assert(line != NULL);
    if (line == NULL) return NM_R_FD_OUTOFBOUND;

#ifdef IOBASE_HAS_NOTIFY
    iobase_clear_notify(line->pipe_infd);
#endif

    sdata_t *data = squeue_deq(&(line->recv_queue));
    if (NULL == data) return NM_R_SUCCESS;
    
    assert(data->len > 0);
    assert(data->len < buff_len);
    
    memcpy(buff, data->buff, data->len);
    memcpy(addr, data->extbuff, sizeof(netmap_address_t));
    
    squeue_enq(&(line->recv_idle_queue), data);

    return data->len;
}

int iobase_send(int fd, const char *buff, int data_len, const iobase_address_t *addr)
{
    iobase_pipeline_t *line = NULL; 
    
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

    line = pipeline_mapfrom_userfd(&g_iomgr, fd);
    assert(line != NULL);
    if (line == NULL) return NM_R_FD_OUTOFBOUND;

    sdata_t *data = squeue_deq(&(line->send_idle_queue));
    if (NULL == data) return NM_R_QUEUE_EMPTY;
    
    data->len = data_len;
    memcpy(data->buff, buff, data_len);
    memcpy(data->extbuff, addr, sizeof(netmap_address_t));
    squeue_enq(&(line->send_queue), data);
    
    return data_len;
}

// only in main thread call ONCE time.
int iobase_init(void)
{
#ifdef __BUILD_TIME 
    printf(" BUILD AT: %s, USE: %s\n", __BUILD_TIME, NM_LIB);
#endif

    iobase_manager_init(&g_iomgr);
    if ( (g_iomgr.epoll_fd = epoll_init()) < 0 )
    {
        return NM_R_FAILED;
    }

    return NM_R_SUCCESS; 
}

static int __iobase_recvfrom__(int nmfd)
{
    iobase_pipeline_t *line = NULL;
    iobase_ifnode_t *node = NULL;
	int recv_count = 0;
	int total_count = 0;

    line = next_pipeline_mapfrom_nmfd(&g_iomgr, nmfd);
    if (NULL == line) return NM_R_FD_OUTOFBOUND;
    node = ifnode_mapfrom_nmfd(&g_iomgr, nmfd);
    if (NULL == node) return NM_R_FD_OUTOFBOUND;

	sdata_t *data = NULL;
	
LOOP:
    data = squeue_deq(&(line->recv_idle_queue));
    if (NULL == data) goto END_L;
    CHECK_SDATA_AND_EXIT(data);

    if ( ( recv_count = netmap_recv_raw(&(node->ring), 
                    data->buff, SDATA_SIZE_MAX, (netmap_address_t*)data->extbuff)) > 0)
    { 
        data->len = recv_count;
        squeue_enq(&(line->recv_queue),data); 

#ifdef IOBASE_HAS_NOTIFY
        iobase_notify(nmfd);
#endif
        ++ node->recv_count;
        ++ line->enq_nums;
		total_count += recv_count;
		goto LOOP;
    }
    else
    {
        squeue_enq(&(line->recv_idle_queue), data);
    }
	
END_L:
	if (total_count > 0)
	{
	    move_to_next_pipeline_by_nmfd(&g_iomgr, nmfd);
	}

    return NM_R_SUCCESS;
}

static int __iobase_sendto__(iobase_ifnode_t *node, iobase_pipeline_t *line)
{
    assert(node != NULL);
    assert(line != NULL);

	sdata_t *data = NULL;
	int send_bytes = -1;	

LOOP:
    data = squeue_deq(&(line->send_queue));
    if (NULL == data) goto END_L;
    CHECK_SDATA_AND_EXIT(data);
    assert(data->len > 0);

#ifdef IOBASE_IOSYNC_USE_CAS	
	while(!__sync_bool_compare_and_swap(&(g_iomgr._io_sync), 0, 1)){}
#else
	slock_lock(&(g_iomgr.iolock));
#endif

    send_bytes = netmap_send_raw(&(node->ring), data->buff, data->len, (netmap_address_t*)data->extbuff);

#ifdef IOBASE_IOSYNC_USE_CAS
	__sync_bool_compare_and_swap(&(g_iomgr._io_sync), 1, 0);
#else
	slock_unlock(&(g_iomgr.iolock));
#endif

    squeue_enq(&(line->send_idle_queue), data);
    if (send_bytes > 0)
    {
        ++ node->send_count;
        ++ line->deq_nums;
        goto LOOP;
	}
	
END_L:
    return NM_R_SUCCESS;
}

#ifdef IOBASE_HAS_NOTIFY
static int iobase_notify(int nmfd)
{
	assert(nmfd >= 0);
    
    if (g_iomgr.has_notify == NM_FALSE) return NM_R_SUCCESS;

    iobase_pipeline_t * line = NULL;
    static char _null_buff_[1024 * 64] = {'a'};
    static char _buff_[1] = {'a'};
	
    line = next_pipeline_mapfrom_nmfd(&g_iomgr, nmfd);
    if (NULL != line)
    {
        if (write(line->pipe_outfd, _buff_, 1) <= 0)
        {
            read(line->pipe_infd, _null_buff_, sizeof(_null_buff_));
            write(line->pipe_outfd, _buff_, 1);
        }

        g_iomgr.signal_count[nmfd] ++;
        return NM_R_SUCCESS;
    }

    return NM_R_FAILED;
}

static int iobase_clear_notify(int userfd)
{
    static char _buff_[1];

    if (g_iomgr.has_notify == NM_FALSE) return NM_R_SUCCESS;

    read(userfd, _buff_, sizeof(_buff_));
    return NM_R_SUCCESS;
}
#endif

void * __iobase_run__(void*args)
{
	int nfd = 0;
    int index = 0;
	struct epoll_event *events = NULL;

    iobase_manager_t *iomgr = (iobase_manager_t*)(args);

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
#ifdef IOBASE_IOSYNC_USE_CAS
		while(!__sync_bool_compare_and_swap(&(iomgr->_io_sync), 0, 1)){}
#else
		slock_lock(&(iomgr->iolock));
#endif

	    nfd = epoll_wait(iomgr->epoll_fd, events, MAX_FDS, -1);

#ifdef IOBASE_IOSYNC_USE_CAS
		__sync_bool_compare_and_swap(&(iomgr->_io_sync), 1, 0);
#else
		slock_unlock(&(iomgr->iolock));
#endif

        if (nfd < 0) continue;
        for (index = 0; index < nfd; index ++)
        {
            int ppfd = events[index].data.fd;
            if ( (events[index].events & EPOLLIN 
                    || events[index].events & EPOLLOUT) && !iomgr->stopped )
            {
                iomgr->interupt_count[ppfd] ++;
                __iobase_recvfrom__(ppfd);
            }

            if (iomgr->stopped) goto END;
        }
       	sleep(0);
    }

END:
    D("exiting");
    free(events);

   	return NULL;
}

void *__iobase_flush__(void *args)
{
    iobase_manager_t *iomgr = (iobase_manager_t*)(args);
    int index = 0;

    while (!iomgr->stopped) 
    {
        for (index = 0; index < iomgr->avail; index ++)
        {
            iobase_ifnode_t *node = &(iomgr->ifnodes[index]);
            if (NULL == node) continue;

            slist_node_t *n = node->pipelines;
			if (node->ring.fd >= 0 && n != NULL)
            {
                do {
                    iobase_pipeline_t *line = slist_entry(n, iobase_pipeline_t, node);
                    CHECK_PIPELINE_AND_EXIT(line);
					__iobase_sendto__(node, line);

                    n = n->next;
                }while( n != node->pipelines && n != NULL && !iomgr->stopped);
            }

            if (iomgr->stopped) goto END;
        }
        usleep(5);
    }

END:
    D("exit");
    return NULL;
}

void iobase_setup(void)
{
    int index = 0;
    for (index = 0; index < MAX_FDS; index ++)
    {
        g_iomgr.interupt_count[index] = -1;
        g_iomgr.signal_count[index] = -1;
    }

#ifdef IOBASE_HAS_NOTIFY
    if (g_iomgr.has_notify == NM_TRUE)
        pthread_create(&(g_iomgr.run_pid), NULL, __iobase_run__, (void*)(&g_iomgr));
#endif
    pthread_create(&(g_iomgr.send_pid), NULL, __iobase_flush__, (void*)(&g_iomgr));
}

void iobase_loop(void)
{
    __iobase_run__((void*)&g_iomgr);
}

void iobase_report(void)
{
    int index = 0;
    int i = 0;

    printf("Statistics ... \n");
    for (index = 0; index < g_iomgr.avail; index ++)
    {
        iobase_ifnode_t *node = &(g_iomgr.ifnodes[index]);
        if (NULL == node) continue;

        printf(" On %s recv:%d, send:%d, droped:%d\n", 
                node->ifname, 
                node->recv_count, node->send_count,
                node->recv_count - node->send_count);

        slist_node_t *n = node->pipelines;
        i = 0;
        if (node->ring.fd >= 0 && n != NULL)
        {
            do {
                iobase_pipeline_t *line = slist_entry(n, iobase_pipeline_t, node);
                CHECK_PIPELINE_AND_EXIT(line);
                
                printf("    On line[%d] enq:%d, deq:%d, droped:%d\n", i,
                        line->enq_nums, line->deq_nums,
                        line->enq_nums - line->deq_nums);
                ++i;
                n = n->next;
            }while( n != node->pipelines && n != NULL);
        }
    }
    printf("\n");

    for (index = 0; index < MAX_FDS; index++)
    {
        if (g_iomgr.interupt_count[index] > -1) D("fd:%d, interupts:%d", index, g_iomgr.interupt_count[index]);
        if (g_iomgr.signal_count[index] > -1)   D("fd:%d,    signal:%d", index, g_iomgr.signal_count[index]);
    }

    printf("\n");
    fflush(stdout);
}

void iobase_set_attr(int what, int cmd)
{
#ifdef IOBASE_HAS_NOTIFY
    if (what == IOBASE_ATTR_NOTIFY) 
        g_iomgr.has_notify = cmd;
#endif
}

void iobase_terminal(void)
{
    g_iomgr.stopped = NM_TRUE;
}

void iobase_teardown(void)
{
    iobase_terminal();

#ifdef IOBASE_HAS_NOTIFY
    if (g_iomgr.has_notify == NM_TRUE)
        pthread_join(g_iomgr.run_pid, NULL);
#endif
    pthread_join(g_iomgr.send_pid, NULL);

    iobase_report();
}

int iobase_destroy(void)
{
    return NM_R_SUCCESS; 
}

