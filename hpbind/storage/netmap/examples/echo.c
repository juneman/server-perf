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


int main(int argc, char **argv)
{   
    
    netmap_init();

    int nmfd  = netmap_openfd("eth1"); 

    D("open fd:%d on:%s", nmfd, "eth1");

    netmap_closefd(nmfd);

    return (0);
}


