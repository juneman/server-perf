/*
 * File: poll_util.c
 *
 *      
 * author: db
 *  date: 2014-03-03
 *
 */
#ifndef __POLL_UTIL_H
#define __POLL_UTIL_H

int watch_fd(int efd, int fd);

int unwatch_fd(int efd, int fd);

int epoll_init();

#endif
