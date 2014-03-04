#include <stdio.h>
#include <stdlib.h>

#define EXPECT_T(c) do {\
        const char *func = __FUNCTION__; \
        int line = __LINE__; \
        expect_t(c, func, line); \
}while(0)

void expect_t(int c, const char *fun, int line);

void  report();
