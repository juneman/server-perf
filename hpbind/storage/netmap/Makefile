all: bldenv clean install

CC=gcc
NM_LIB=libnmio.so

#CFLAGS += -g -Wall -Werror 
CFLAGS += -fPIC
nm_util.o: nm_util.h
slist.o: slist.h
sdata.o: sdata.h
cbuff.o: cbuff.h
lfqueue.o: lfqueue.h
squeue.o: squeue.h
dns_util.o: dns_util.h
OBJS = nm_util.o dns_util.o slist.o squeue.o sdata.o cbuff.o lfqueue.o

iobase.o: $(OBJS)

bldenv:
	echo "#define __BUILD_TIME \"`date`\"" > build.h

$(NM_LIB):	iobase.o $(OBJS) 
	gcc $(CFLAGS)  -shared -o $@ $^

install: $(NM_LIB)

clean:
	rm -f *.o $(NM_LIB)
	make clean -C test/
	make clean -C examples/
