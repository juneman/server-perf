
#
# author: db
#

PROGS = echo_threads echo_threads2

NM_LIB_PATH=../
NM_LIB=nmio
NM_INC=../iobase.h

CC=gcc
CLEANFILES=$(PROGS) *.o
CFLAGS += -Werror -Wall -g
CFLAGS += -I../ 

#LDFLAGS += -L$(NM_LIB_PATH) -lpthread -l$(NM_LIB)
LDFLAGS +=  -lpthread 

OBJS = ../nm_util.o ../dns_util.o ../sdata.o ../slist.o ../squeue.o ../iobase-sq-p.o 

all: prebuild clean $(PROGS)

echo_threads: echo_threads.o $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

echo_threads2: echo_threads2.o $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^


prebuild:
	make -C ../
clean:
	rm -rf $(CLEANFILES)
