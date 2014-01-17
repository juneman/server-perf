/**
 * for testing sigle thread 
 * 
 * author : db
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "iobase.h"

#define MAX_EVENTS 8
#define WORKER_NUM 1

#define RECV_BUFF_LEN 512

static int do_abort_watcher = 0;
static int fds[WORKER_NUM];
static int sig_nums = 0;
static int watcher_recv_nums = 0;
static int watcher_send_nums = 0;

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

    char buff[RECV_BUFF_LEN];
    netmap_address_t addr;

    /* main loop */
    signal(SIGINT, sigint_h);
    while (!do_abort_watcher) {

        nfd = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfd < 0) continue;
        {
            int recv_bytes = netmap_recv(fds[0], buff, RECV_BUFF_LEN, &addr);
            if (recv_bytes <= 0) continue;

            watcher_recv_nums ++;
            set_dns_response(buff);
            netmap_send(fds[0], buff, recv_bytes, &addr);
            watcher_send_nums ++; 
        }
    }
    
    D("exiting");
   	return NULL;
}

int
main(int argc, char **argv)
{   
    int i = 0;
    
    netmap_init();
  
    for (i = 0; i < WORKER_NUM; i++)
    {
        fds[i] = netmap_openfd("eth1"); 
        D("open fd:%d", fds[i]);
    }
   
    netmap_setup();

    watcher(NULL);
    
    for (i = 0; i < WORKER_NUM; i++)
       	netmap_closefd(fds[i]);
    
    netmap_destroy();

    D(" Sig num: %d", sig_nums);

    D(" Total watcher recv: %d", watcher_recv_nums);
    D(" Total watcher send: %d", watcher_send_nums);

    D(" Total recv: %d", watcher_recv_nums);
    D(" Total send: %d", watcher_send_nums);

    return (0);
}


