/***
 *
 * Test for netmap raw recv/send in single thread 
 * 
 * author: db
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "sys/epoll.h"

#include "netmapio.h"
#include "dns_util.h"
#include "poll_util.h"
#include "log.h"

#define MAX_EVENTS 8

static int do_abort = 0;
static int recv_nums = 0;
static int send_nums = 0;
static int sig_nums = 0;

static void sigint_h(int sig)
{
    (void)sig;
    do_abort = 1;
    signal(SIGINT, SIG_DFL);
}

static void set_dns_response(char *buff)
{
    char *dns = buff;
    dns[2] |= 0x80;
}

void run(netmapio_ringobj *ringobj)
{
	struct epoll_event *events;
	int nfd, efd;
    int wait_link = 2;
 
    //----------------------------
	efd = epoll_init();
	if (efd == -1)
	{
		D("epoll init failed\n");
	    exit(1);	
	}
    
    watch_fd(efd, ringobj->fd);

	events = calloc(MAX_EVENTS, sizeof(struct epoll_event));
	
    D("Wait %d secs for link to come up...", wait_link);
    sleep(wait_link);
    D("Ready to go ....");
    
    char buff[SDATA_SIZE_MAX];
    netmap_address_t address;

    /* main loop */
    signal(SIGINT, sigint_h);
    while (!do_abort) {

        nfd = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfd != 1) continue;
        if (events[0].data.fd != ringobj->fd) continue;
        sig_nums ++; 

        if (events[0].events & EPOLLIN 
                || events[0].events & EPOLLOUT)
        {
            int recv_bytes = netmap_recv_raw(ringobj, buff, SDATA_SIZE_MAX, &address);
            if (recv_bytes <= 0) continue;
            
            ++recv_nums;
            set_dns_response(buff);
            int send_bytes = netmap_send_raw(ringobj, buff, recv_bytes, &address);
            if (send_bytes > 0)  ++send_nums;
        }
    }

    D("exiting");
   	return;
}

void print_stat()
{
    D(" Sig num: %d", sig_nums);
    D(" Total recv: %d", recv_nums);
    D(" Total send: %d", send_nums);
}

int main(int argc, char **argv)
{   
    netmapio_ringobj ringobj;
    memset(&ringobj, 0x0, sizeof(netmapio_ringobj));
    ringobj.ifname = argv[1];

    if (netmap_open(&ringobj, 0, 0) != NM_R_SUCCESS)
    {
        D("netmap open failed.");
        exit(1);
    }
    
    ringobj.filter = is_dns_query;
    ringobj.parse_header = dns_parse_header;
    ringobj.build_header = dns_build_header;
    
    run(&ringobj);

    print_stat();

    return (0);
}


