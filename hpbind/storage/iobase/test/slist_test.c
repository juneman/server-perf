#include <stdio.h>
#include <stdlib.h>

#include "slist.h"
#include "test.h"

int testcase_1()
{
    slist_node_t head;

    slist_init(&head);

    int e = slist_empty(&head);
    EXPECT_T(e);

    slist_node_t node;
    slist_add(&head, &node);

    e = slist_empty(&head);
    EXPECT_T(!e);

    return 0;
}

int main(int ac, char *av[])
{
    testcase_1();
    
    report();
    return 0;
}

