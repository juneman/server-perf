all: bldenv clean install

CC=gcc
NM_LIB=libnmio.so

CFLAGS += -Wall -Werror 
CFLAGS += -fPIC
nm_util.o: nm_util.h
nm_lock.o: nm_lock.h
dns_util.o: dns_util.h
iobase.o: nm_util.o dns_util.o nm_lock.o

bldenv:
	echo "#define __BUILD_TIME \"`date`\"" > build.h

$(NM_LIB):	nm_util.o dns_util.o nm_lock.o iobase.o
	gcc $(CFLAGS)  -shared -o $@ $^

install: $(NM_LIB)

clean:
	rm -f *.o $(NM_LIB)
	make clean -C test/
