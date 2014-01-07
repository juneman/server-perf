
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "iobase.h"
#include "sys/epoll.h"
#include "pthread.h"

#define MAX_EVENTS 8
#define WORKER_NUM 1

static int do_abort = 0;
static int fds[WORKER_NUM];
static int recv_nums[WORKER_NUM];
static int send_nums[WORKER_NUM];
static int sig_nums = 0;

pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

static void sigint_h(int sig)
{
    (void)sig;
    do_abort = 1;
    signal(SIGINT, SIG_DFL);
}

void *watcher(void *arg)
{
	struct epoll_event event[WORKER_NUM], *events;
	int nfd, efd, s;
    int wait_link = 2;
    int i = 0;
    
    netmap_init(NULL);

    for (i = 0; i < WORKER_NUM; i++)
    {
        fds[i] = netmap_getfd("eth1"); 
    }

    //----------------------------
	efd = epoll_create(MAX_EVENTS);
	if (efd == -1)
	{
		printf("epoll_create failed\n");
		return 0;
	}
   	
	for (i = 0; i < WORKER_NUM; i++) 
	{ 
		event[i].data.fd = fds[i]; 
		event[i].events = EPOLLIN | EPOLLOUT | EPOLLET;
		s = epoll_ctl(efd, EPOLL_CTL_ADD, fds[i], &event[i]);
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
    while (!do_abort) {

        nfd = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfd < 0) continue;
        
        // process fd
        pthread_mutex_lock(&g_lock);
        sig_nums ++;
        pthread_cond_broadcast(&g_cond);
        pthread_cond_signal(&g_cond);
        pthread_mutex_unlock(&g_lock);
    }

    D("exiting");
   	return NULL;
}

void *run(void *arg)
{
    int id = ((int)arg);
    
    recv_nums[id] = 0;
    send_nums[id] = 0;

    io_block_t block[32];
    io_cache_t cache;
    memset(&cache, 0x0, sizeof(io_cache_t));

    cache.blocks = block;
    cache.capcity = 8;

    D("start run ...: id:%d", id);
    while(!do_abort)
    {
        pthread_mutex_lock(&g_lock);
        D(" wait....:<%d>", id);
        pthread_cond_wait(&g_cond, &g_lock);
        pthread_mutex_unlock(&g_lock);
        {
            do {
             ///   D("1");
                memset(&block, 0x0, sizeof(io_block_t) * 32);
                int n = netmap_recv(fds[id], &cache);
                if (n == 0) break;

                recv_nums[id] += n;

                send_nums[id] += netmap_send(fds[id], &cache);
            //    D("3");
            }while(1);
        }
    }
    
    D("exit run ...:id:%d", id);
    return NULL;
}

int
main(int argc, char **argv)
{   
    pthread_t pids[WORKER_NUM];
    int i = 0;

    for (i = 0; i < WORKER_NUM; i++)
    {
        pthread_create(&pids[i], NULL, run, (void*)i);
        pthread_detach(pids[i]);
    }
    
    watcher(NULL);
    
    for (i = 0; i < WORKER_NUM; i++)
       	netmap_closefd(fds[i]);
    
    int recv_total = 0;
    int send_total = 0;
    for (i = 0; i < WORKER_NUM; i++)
    {
       D("thread %d recv nums:%d, send nums:%d", i, recv_nums[i], send_nums[i]); 
       recv_total += recv_nums[i];
       send_total += send_nums[i];
    }
    
    D(" Sig num: %d", sig_nums);
    D(" Total recv: %d", recv_total);
    D(" Total send: %d", send_total);

    return (0);
}


