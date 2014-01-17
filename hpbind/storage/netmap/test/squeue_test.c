#include <stdio.h>
#include <stdlib.h>

#include "squeue.h"
#include "test.h"

int testcase_1()
{
    squeue_t queue;
    sqmsg_t msg;

    squeue_init(&queue);
    sqmsg_init(&msg);
    
    msg.buff[0] = 'a';
    msg.len = 1;

    int e = squeue_empty(&queue);
    EXPECT_T(e);
    
    sqmsg_t *tm = squeue_pop(&queue);
    EXPECT_T(tm == NULL);

    squeue_push(&queue, &msg);
    e = squeue_empty(&queue);
    EXPECT_T(!e);
    
    tm = squeue_pop(&queue);
    EXPECT_T(tm != NULL);
    EXPECT_T(tm->len == 1);
    EXPECT_T(tm->buff[0] == 'a');


    e = squeue_empty(&queue);
    EXPECT_T(e);
    
    return 0;
}



int main(int ac, char *av[])
{
    testcase_1();
    
    report();
    return 0;
}

