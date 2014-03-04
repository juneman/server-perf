/**
 * File: bind9_nm_io.c
 *
 * author: db
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <stdint.h>
#include <pthread.h>

static int g_eventfd = -1;
static int g_epollfd = -1;
static volatile int g_stoped = 0;

#define MAX_FDS 8

static int watch_fd(int efd, int fd)
{
	struct epoll_event event;
    
    memset(&event.data, 0, sizeof(event.data));

    event.data.fd = fd; 
    event.events = EPOLLIN | EPOLLOUT;
    if (-1 == epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event))
    {
        return -1;
    }

    return 0;
}

#if 0
static int unwatch_fd(int efd, int fd)
{
	struct epoll_event event;
    
    memset(&event.data, 0, sizeof(event.data));

    event.data.fd = fd; 
    event.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);

    return 0;
}
#endif

static int epoll_init()
{

    int epoll_fd = epoll_create(1);
	if (epoll_fd == -1)
	{
        exit(1);
	} 

    return epoll_fd;
}

void * worker(void*args)
{
	int nfd = 0;
	struct epoll_event *events = NULL;

    //----------------------------
    events = calloc(MAX_FDS, sizeof(struct epoll_event));
    if (events == NULL) 
    {
        exit(1);
    }
   	
	uint64_t val = 0;
	int times = 0;
    do {
        nfd = epoll_wait(g_epollfd, events, MAX_FDS, -1);
        if (nfd != 1) continue;
		
		if (events[0].data.fd == g_eventfd)
		{
			read(g_eventfd, &val, sizeof(uint64_t));
			times ++;
		}
    } while (!g_stoped);

    free(events);
	
	printf("times :%d. %ld.\n", times, val);

   	return NULL;
}

void *watcher(void *args)
{
	uint64_t val = 0;
	while(val < 100 * 100)
	{	
		write(g_eventfd, &val, sizeof(uint64_t));
		++val;
		usleep(1);
		printf("%ld\t", val);
		fflush(stdout);
	}
	printf("\nwait....\n");
	sleep(2);
	g_stoped = 1;
	write(g_eventfd, &val, sizeof(uint64_t));
	sleep(2);
	return NULL;
}

int main(int argc, char **argv)
{
	g_eventfd =  eventfd(0, 0);
	//g_eventfd =  eventfd(0, EFD_NONBLOCK);
	if (g_eventfd < 0) {
		printf("eventfd failed.\n");
		return -1;
	}
	
	g_epollfd = epoll_init();	
	
	watch_fd(g_epollfd, g_eventfd);		
	
	pthread_t pid;
	pthread_create(&pid, NULL, worker, NULL);
	
	watcher(NULL);
	
	pthread_join(pid, NULL);	
	return 0;
}

