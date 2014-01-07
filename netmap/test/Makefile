# For multiple programs using a single source file each,
# we can just define 'progs' and create custom targets.
PROGS	=	echo

CLEANFILES = $(PROGS) *.o
CFLAGS = -g -O2 
CFLAGS += -Werror -Wall
CFLAGS += -I ../ 

LDFLAGS += -lpthread

all: prebuild clean $(PROGS)


echo: echo.o ../nm_util.o ../dns_util.o ../iobase.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
echo.o: ../nm_util.h ../dns_util.h ../iobase.h 

prebuild:
	make -C ../
clean:
	rm -rf $(CLEANFILES)
