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
#define WORKER_NUM 2

static int do_abort_watcher = 0;
static int do_abort_worker = 0;
static int fds[WORKER_NUM];
static int recv_nums[WORKER_NUM];
static int send_nums[WORKER_NUM];
static int sig_nums = 0;
static int watcher_recv_nums = 0;
static int watcher_send_nums = 0;

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

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

void *watcher(void *arg)
{
	struct epoll_event event[WORKER_NUM], *events;
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
		event[i].data.fd = fds[i]; 
		event[i].events = EPOLLIN | EPOLLOUT;
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

    char buff[512]; 
    int data_len = 0;
    netmap_address_t addr;

    /* main loop */
    signal(SIGINT, sigint_h);
    while (!do_abort_watcher) {

        nfd = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfd < 0) continue;

        // process fd
        pthread_mutex_lock(&g_lock);
        sig_nums ++;
        pthread_cond_broadcast(&g_cond);
        pthread_mutex_unlock(&g_lock);

        {
            int ret = netmap_recv(fds[0], buff, &data_len, &addr);
            if (ret != NM_SUCCESS) continue;

            watcher_recv_nums ++;
            set_dns_response(buff);
            netmap_send(fds[0], buff, data_len, &addr);
            watcher_send_nums ++;
        }
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
    int id = ((int)arg);
    
    recv_nums[id] = 0;
    send_nums[id] = 0;


    char buff[512]; 
    int data_len = 0;
    netmap_address_t addr;

    D("start run ...: id:%d", id);
    while(!do_abort_worker)
    {
        pthread_mutex_lock(&g_lock);
        pthread_cond_wait(&g_cond, &g_lock);
        pthread_mutex_unlock(&g_lock);
        {
            do {
                int ret = netmap_recv(fds[id], buff, &data_len, &addr);
                if (ret != NM_SUCCESS) break;

                recv_nums[id] ++;
                set_dns_response(buff);
                netmap_send(fds[id], buff, data_len, &addr);
                send_nums[id] ++; 
            }while(0);
        }
    }
    
    D("exit run ...:id:%d", id);
    return NULL;
}

int
main(int argc, char **argv)
{   
    int i = 0;
    
    netmap_init();

    for (i = 0; i < WORKER_NUM; i++)
    {
        pthread_create(&pids[i], NULL, run, (void*)i);
        pthread_detach(pids[i]);
    }
  
    for (i = 0; i < WORKER_NUM; i++)
    {
        fds[i] = netmap_openfd("eth1"); 
        printf("open fd:%d\n", fds[i]);
    }
   
    watcher(NULL);
    
    sleep(1);
    for (i = 0; i < WORKER_NUM; i++)
       	netmap_closefd(fds[i]);
    
    netmap_destroy();

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

    D(" Total recv: %d", recv_total + watcher_recv_nums);
    D(" Total send: %d", send_total + watcher_send_nums);

    return (0);
}


