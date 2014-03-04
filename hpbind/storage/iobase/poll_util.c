/*
 * File: poll_util.c
 *
 *      
 * author: db
 *  date: 2014-03-03
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <assert.h>
#include <string.h>
#include "types.h"
#include "log.h"
#include "poll_util.h"

int watch_fd(int efd, int fd)
{
	struct epoll_event event;
    
    assert(efd >= 0);
    memset(&event.data, 0, sizeof(event.data));

    event.data.fd = fd; 
    event.events = EPOLLIN | EPOLLOUT;
    if (-1 == epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event))
    {
        D("epoll_ctl error");
        return NM_R_FAILED;
    }

    return NM_R_SUCCESS;
}

int unwatch_fd(int efd, int fd)
{
	struct epoll_event event;
    
    assert(efd >= 0);
    memset(&event.data, 0, sizeof(event.data));

    event.data.fd = fd; 
    event.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(efd, EPOLL_CTL_DEL, fd, &event);

    return NM_R_SUCCESS;
}

int epoll_init()
{

	int epoll_fd = epoll_create(1);
	if (epoll_fd == -1)
	{
		D("epoll_create failed");
        exit(1);
	} 
    
    return epoll_fd;
}


