/***
 *
 * for testing multi-threads 
 * 
 * author: db
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "iobase.h"
#include "sys/epoll.h"
#include "pthread.h"
#include "squeue.h"


#define WORKER_NUM 6
static int do_abort_watcher = 0;
static int do_abort_worker = 0;
static int recv_nums[WORKER_NUM];
static int send_nums[WORKER_NUM];
static int sig_nums = 0;
static int watcher_recv_nums = 0;
static int watcher_send_nums = 0;

static int g_fd = -1;

static pthread_t pids[WORKER_NUM];

static squeue_t g_recv_queue;
static squeue_t g_send_queue;
static squeue_t g_idle_queue;

typedef struct nm_args_t_
{
    int index;
}nm_args_t;
static nm_args_t g_args[WORKER_NUM];

static void sigint_h(int sig)
{
    (void)sig;
    do_abort_watcher = 1;
    signal(SIGINT, SIG_DFL);
}

static void set_dns_response(char *buff)
{
    char *dns = buff;
    dns[2] |= 0x80;
}

#define MAX_EVENTS 4
void *watcher(void *arg)
{
	struct epoll_event event, *events;
	int nfd, efd, s;
    int wait_link = 2;
   
    //----------------------------
	efd = epoll_create(MAX_EVENTS);
	if (efd == -1)
	{
		D("epoll_create failed");
        exit(1);
	} 

    event.data.fd = g_fd; 
    event.events = EPOLLIN | EPOLLOUT;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, g_fd, &event);
    if (s == -1) {
        D("epoll_ctl error");
        exit(1);
    }

	events = calloc(MAX_EVENTS, sizeof(struct epoll_event));
    if (events == NULL) 
    {
        D("calloc failed");
        exit(1);
    }
	
    D("Wait %d secs for link to come up...", wait_link);
    sleep(wait_link);
    D("Ready to go ....");

    /* main loop */
    signal(SIGINT, sigint_h);
    sqmsg_t *msg = NULL; 
    while (!do_abort_watcher) 
    {
        nfd = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfd < 0) continue;
        
        sig_nums ++;
        int i = 0;
        for (i = 0; i < nfd; i ++)
        {
            if (events[i].data.fd != g_fd)
            {
                continue;
            }

            if ( events[i].events & EPOLLIN)
            {
                goto TRY_RX;
            }
                
            if (events[i].events & EPOLLOUT)
            {
                goto TRY_TX;
            }
        }
        
        continue;

TRY_RX:
        msg = squeue_pop(&g_idle_queue); 
        if (NULL != msg) 
        {
            // handle recv
            netmap_address_t *addr = (netmap_address_t *)(&msg->extbuff);
            int recv_bytes = netmap_recv(g_fd, msg->buff, SQMSG_SIZE_MAX, addr);
            if (recv_bytes > 0) 
            { 
                set_dns_response(msg->buff);
                msg->len = recv_bytes;
                squeue_push(&g_recv_queue, msg);
                watcher_recv_nums ++;
            }
            else
            {
                squeue_push(&g_idle_queue, msg);
            }
        }

TRY_TX:
        // handle tx
        msg = squeue_pop(&g_send_queue);
        if (NULL != msg) 
        {
            int send_bytes = netmap_send(g_fd, msg->buff, msg->len, (netmap_address_t*)(msg->extbuff));
            squeue_push(&g_idle_queue, msg);
            if (send_bytes > 0) 
            {
                watcher_send_nums ++;
            }
            else
            {
                D("SEND ret:%d", send_bytes);
            }
        }
    }

    do_abort_worker = 1;

    D("exiting");
   	return NULL;
}

void *run(void *arg)
{
    nm_args_t *args = (nm_args_t*)arg;
    int index = args->index;

    recv_nums[index] = 0;
    send_nums[index] = 0;

    D("start thread<index:%d>", index);
    while(!do_abort_worker)
    {
        squeue_wait(&g_recv_queue);

        {
            do {
                sqmsg_t *msg = squeue_pop(&g_recv_queue);
                if (NULL == msg) break;
                
                recv_nums[index] ++;
                set_dns_response(msg->buff);
                
                squeue_push(&g_send_queue, msg);
                send_nums[index] ++;
            }while(1);
        }
    }

    D("exit thread<index:%d>", index);
    return NULL;
}

void stat()
{
    int i = 0;
    int recv_total = 0;
    int send_total = 0;
    for (i = 0; i < WORKER_NUM; i++)
    {
       D("thread %d recv nums:%d, send nums:%d", i, recv_nums[i], send_nums[i]); 
       recv_total += recv_nums[i];
       send_total += send_nums[i];
    }
    
    D(" Sig num: %d", sig_nums);
    D(" Total watcher recv: %d", watcher_recv_nums);
    D(" Total watcher send: %d", watcher_send_nums);
    D(" Total recv: %d", recv_total);
    D(" Total send: %d", send_total);
}

int main(int argc, char **argv)
{   
    int i = 0;
    
    squeue_init(&g_recv_queue);
    squeue_init(&g_send_queue);
    squeue_prealloc(&g_idle_queue, 2048);

    netmap_init();
	
    g_fd = netmap_openfd("eth1"); 
    if (g_fd <= 0 ) 
    {
        D("netmap open fd failed.");
        exit(1);
    }

    for (i = 0; i < WORKER_NUM; i++)
    {
        g_args[i].index = i;
        pthread_create(&pids[i], NULL, run, &g_args[i]);
        pthread_detach(pids[i]);
    }
   
    watcher(NULL);
    
    sleep(1);
    netmap_closefd(g_fd);

    netmap_destroy();

    stat();

    return (0);
}


