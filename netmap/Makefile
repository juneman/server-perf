all: bldenv clean install

CC=gcc
NM_LIB=libnmio.so

CFLAGS += -g -Wall -Werror 
CFLAGS += -fPIC
nm_util.o: nm_util.h
slist.o: slist.h
sdata.o: sdata.h
squeue.o: squeue.h
dns_util.o: dns_util.h
iobase.o: nm_util.o dns_util.o slist.o squeue.o sdata.o

bldenv:
	echo "#define __BUILD_TIME \"`date`\"" > build.h

$(NM_LIB):	nm_util.o dns_util.o iobase.o slist.o squeue.o sdata.o
	gcc $(CFLAGS)  -shared -o $@ $^

install: $(NM_LIB)

clean:
	rm -f *.o $(NM_LIB)
	make clean -C test/
	make clean -C examples/
