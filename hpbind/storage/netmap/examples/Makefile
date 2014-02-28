
#
# author: db
#

PROGS =echo echo_threads echo_threads2 echo_threads3

CC=gcc
CLEANFILES=$(PROGS) *.o
CFLAGS += -Werror -Wall -g
CFLAGS += -I../ -I../include/ 

LDFLAGS +=  -lpthread 

OBJS = ./../lib/libiobase.a 

all: prebuild clean $(PROGS)

echo: echo.o $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

echo_threads: echo_threads.o $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

echo_threads2: echo_threads2.o $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

echo_threads3: echo_threads3.o $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^


prebuild:
	make -C ../
clean:
	rm -rf $(CLEANFILES)
