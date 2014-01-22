#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>


int main(int argc, char *argv[])
{
    int fds[2];

    if (pipe(fds) < 0) 
    {
        printf("pip failed.\n");
        return 0;
    }

    int flags = fcntl(fds[0], F_GETFL, 0); 
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(fds[1], F_GETFL, 0); 
    fcntl(fds[1], F_SETFL, flags | O_NONBLOCK);

    char buff[512];
    
    int count = 0;
    while(1)
    {
        count ++;
        printf("start write...\n");
        int ret = write(fds[1], buff, sizeof(buff));
        if (ret <= 0) 
        {
            printf("ret :%d, count:%d, total bytes:%d\n", 
                    ret, count, (count * sizeof(buff)) / 1024 );
            return 0;
        }
        printf("end write...\n");
        //sleep(1);
    }

    return 0;
}

