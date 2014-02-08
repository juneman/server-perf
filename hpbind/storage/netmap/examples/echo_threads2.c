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

#define MAX_EVENTS 8
#define WORKER_NUM 4

#define RECV_BUFF_LEN 512

static int do_abort_watcher = 0;
static int do_abort_worker = 0;
static int g_fds[WORKER_NUM];
static int recv_nums[WORKER_NUM];
static int send_nums[WORKER_NUM];
static int sig_nums = 0;
static int watcher_recv_nums = 0;
static int watcher_send_nums = 0;

typedef struct ___nm_args_t__
{
    int index;
    int fd;
}nm_args_t;

static nm_args_t g_args[WORKER_NUM];

typedef struct ___nm_cond_t__ 
{
    pthread_mutex_t lock;
    pthread_cond_t cond;
}nm_cond_t;

#define TEST_MAX_FDS 512
static nm_cond_t g_conds[TEST_MAX_FDS];

static pthread_t pids[WORKER_NUM];

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

static void signal_worker(int fd)
{
    nm_cond_t *c = &g_conds[fd];

    pthread_mutex_lock(&(c->lock));
    //pthread_cond_broadcast(&(c->cond));
    pthread_cond_signal(&(c->cond));
    pthread_mutex_unlock(&(c->lock));
}

static void wait_worker(int fd)
{
    nm_cond_t *c = &g_conds[fd];

    pthread_mutex_lock(&(c->lock));
    pthread_cond_wait(&(c->cond), &(c->lock));
    pthread_mutex_unlock(&(c->lock));
}


void *watcher(void *arg)
{
	struct epoll_event event, *events;
	int nfd, efd, s;
    int wait_link = 2;
    int i = 0;
   
    //----------------------------
	efd = epoll_create(MAX_EVENTS);
	if (efd == -1)
	{
		printf("epoll_create failed\n");
		return 0;
	}
    
    for (i = 0; i < WORKER_NUM; i++)
	{ 
		event.data.fd = g_fds[i]; 
		event.events = EPOLLIN | EPOLLOUT;
		s = epoll_ctl(efd, EPOLL_CTL_ADD, g_fds[i], &event);
		if (s == -1) {
			printf("epoll_ctl error \n");
			return 0;
		}
	}

	events = calloc(MAX_EVENTS, sizeof(struct epoll_event));
	
    D("Wait %d secs for link to come up...", wait_link);
    sleep(wait_link);
    D("Ready to go ....");

    /* main loop */
    signal(SIGINT, sigint_h);
    while (!do_abort_watcher) {

        nfd = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfd < 0) continue;
        sig_nums ++; 
        for (i = 0; i < nfd; i++)
        {
            if (events[i].events & EPOLLIN 
                    || events[i].events & EPOLLOUT)
            {
                signal_worker(events[i].data.fd);
            }
        }
        
        continue;
     }
    
    for (i = 0; i < WORKER_NUM; i++)
    {
        pthread_cancel(pids[i]);
    }
    do_abort_worker = 1;

    D("exiting");
   	return NULL;
}

void *run(void *arg)
{
    nm_args_t *args = (nm_args_t*)arg;
    int index = args->index;
    int fd = args->fd;

    recv_nums[index] = 0;
    send_nums[index] = 0;

    char buff[RECV_BUFF_LEN]; 
    netmap_address_t addr;

    D("start thread<index:%d><listening fd:%d>", index, fd);
    while(!do_abort_worker)
    {
        wait_worker(fd);
        {
            do {
                int recv_bytes = netmap_recv(fd, buff, RECV_BUFF_LEN, &addr);
                if (recv_bytes <= 0) break;

                recv_nums[index] ++;
                set_dns_response(buff);
                netmap_send(fd, buff, recv_bytes, &addr);
                send_nums[index] ++; 
            }while(0);
        }
    }

    D("exit thread<index:%d><listening fd:%d>", index, fd);
    return NULL;
}

void print_stat()
{
    int recv_total = 0;
    int send_total = 0;
    int i = 0;

    for (i = 0; i < WORKER_NUM; i++)
    {
       D("thread %d recv nums:%d, send nums:%d", i, recv_nums[i], send_nums[i]); 
       recv_total += recv_nums[i];
       send_total += send_nums[i];
    }
    
    D(" Sig num: %d", sig_nums);
    D(" Total watcher recv: %d", watcher_recv_nums);
    D(" Total watcher send: %d", watcher_send_nums);
    D(" Total recv: %d", recv_total + watcher_recv_nums);
    D(" Total send: %d", send_total + watcher_send_nums);
}

int main(int argc, char **argv)
{   
    int i = 0;
    
    netmap_init();
    
    for (i = 0; i < WORKER_NUM; i++)
    {
        int nmfd  = netmap_openfd(argv[1]); 
        g_fds[i] = nmfd;

        D("open fd:%d on:%s", g_fds[i], argv[1]);

        pthread_mutex_init(&(g_conds[nmfd].lock), NULL);
        pthread_cond_init(&(g_conds[nmfd].cond), NULL);
    }

    for (i = 0; i < WORKER_NUM; i++)
    {
        g_args[i].index = i;
        g_args[i].fd = g_fds[i];

        pthread_create(&pids[i], NULL, run, (void*)&g_args[i]);
        pthread_detach(pids[i]);
    }
    
    netmap_setup();

    watcher(NULL);
    
    netmap_teardown();

    sleep(1);

    for (i = 0; i < WORKER_NUM; i++)
    {
        netmap_closefd(g_fds[i]);
    }
    
    netmap_destroy();

    print_stat();

    return (0);
}


