#include <stdio.h>
#include <stdlib.h>

#include "squeue.h"

static int g_total_expect = 0;
static int g_pass_expect = 0;

void expect_t(int c, const char *fc, int line)
{
    g_total_expect ++; 
    if (c) 
    { 
        g_pass_expect ++;
        printf("[%s][%d] PASS\n", fc, line); 
    }
    else 
    {
        printf("[%s][%d] FAILED\n", fc, line);
    }
}

void  report()
{
    printf("\n Report:\n");
    printf(" Total case: %d, Passed: %d.\n", g_total_expect, g_pass_expect);
}


