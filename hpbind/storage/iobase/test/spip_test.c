#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>

typedef struct __args_t__
{
    int fd;
}args_t;

#define WORKER_NUMS 8
static args_t g_args[WORKER_NUMS];
static pthread_t g_pid[WORKER_NUMS];

#define MAX_EVENTS 1
void *run(void*args)
{
    args_t *ag = (args_t *)args;

    int fd = ag->fd;
    
    struct epoll_event event, events[MAX_EVENTS];
    int nfd, efd, s;

    efd = epoll_create(MAX_EVENTS);
    if (efd == -1) 
    {
        printf("epoll create failed.\n");
        exit(1);
    }
    
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLOUT;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);
    if (s == -1)
    {
        printf("epoll ctl error.\n");
        exit(1);
    }
    
        printf("epoll ctl add:%d.\n", fd);
    while(1) 
    {
        nfd = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfd < 0) continue;
        {
            printf("epoll ...\n");
            int buf[2];
            int cc = read(fd, buf, sizeof(buf));
            if (cc < 0)
            {
                printf("read msg error\n");
                continue;
            }
            
            printf("fd:%d, %d-%d\n", fd, buf[0], buf[1]);
        }
    }
}

int main(int argc, char *argv[])
{
    int fds[2];
    int i = 0;

    if (pipe(fds) < 0) 
    {
        printf("pip failed.\n");
        return 0;
    }
#if 0 
    int flags = fcntl(fds[0], F_GETFL, 0); 
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(fds[1], F_GETFL, 0); 
    fcntl(fds[1], F_SETFL, flags | O_NONBLOCK);

#endif
    for (i = 0; i < WORKER_NUMS; i++)
    {
        g_args[i].fd = dup(fds[0]);
        pthread_create(&g_pid[i], NULL, run,&g_args[i]);
        pthread_detach(g_pid[i]);
    }

    int buf[2];
    int j =0;
    i = 0;
    
    while(1)
    {
        buf[0] = i;
        buf[1] = j;
        write(fds[1], buf, sizeof(buf));
        //write(fds[1], buf, 0);
        printf("write...\n"); 
        i+=1;
        j+=2;
        sleep(1);
    }

    return 0;
}

