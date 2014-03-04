/***
 *
 * for testing multi-threads use CBUFF 
 * No notify. use busy-wait 
 *
 * author: db
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include "iobase.h"
#include "sys/epoll.h"
#include "pthread.h"

#define MAX_EVENTS 8
#define WORKER_NUM 2

#define RECV_BUFF_LEN 512

static int do_abort = 0;
static int g_fds[WORKER_NUM];
static int recv_nums[WORKER_NUM];
static int send_nums[WORKER_NUM];
static int sig_nums = 0;

typedef struct ___nm_args_t__
{
    int index;
    int fd;
}nm_args_t;

static nm_args_t g_args[WORKER_NUM];

#define TEST_MAX_FDS 512
static pthread_t pids[WORKER_NUM];

static void sigint_h(int sig)
{
    (void)sig;
    do_abort = 1;
    iobase_terminal();
    signal(SIGINT, SIG_DFL);
}

static void set_dns_response(char *buff)
{
    char *dns = buff;
    dns[2] |= 0x80;
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
    while(!do_abort)
    {
        int recv_bytes = iobase_recv(fd, buff, RECV_BUFF_LEN, &addr);
        if (recv_bytes <= 0) continue;

        recv_nums[index] ++;
        set_dns_response(buff);
        iobase_send(fd, buff, recv_bytes, &addr);
        send_nums[index] ++; 
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
    D(" Total recv: %d", recv_total);
    D(" Total send: %d", send_total);
}

int main(int argc, char **argv)
{   
    int i = 0;
    
    iobase_init();
    iobase_set_attr(IOBASE_ATTR_NOTIFY, IOBASE_NOTITY_OFF);
    
    signal(SIGINT, sigint_h);

    for (i = 0; i < WORKER_NUM; i++)
    {
        int nmfd  = iobase_openfd(argv[1]); 
        g_fds[i] = nmfd;
        D("open fd:%d on:%s", g_fds[i], argv[1]);
    }

    for (i = 0; i < WORKER_NUM; i++)
    {
        g_args[i].index = i;
        g_args[i].fd = g_fds[i];
        pthread_create(&pids[i], NULL, run, (void*)&g_args[i]);
        pthread_detach(pids[i]);
    }
    
    iobase_setup();

    D("Enter iobase loop...");
    iobase_loop();

    iobase_teardown();

    sleep(1);

    for (i = 0; i < WORKER_NUM; i++)
    {
        iobase_closefd(g_fds[i]);
    }
    
    iobase_destroy();

    print_stat();

    return (0);
}


