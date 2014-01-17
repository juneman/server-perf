
#
# author: db
#

PROGS	=	echo echo_threads echo_sq

NM_LIB_PATH=../
NM_LIB=nmio
NM_INC=../iobase.h

CC=gcc
CLEANFILES=$(PROGS) *.o
CFLAGS= -g 
CFLAGS += -Werror -Wall
CFLAGS += -I ../ 

LDFLAGS += -L$(NM_LIB_PATH) -lpthread -l$(NM_LIB)

all: prebuild clean $(PROGS)

echo: echo.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

echo_threads: echo_threads.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

echo_sq: echo_sq.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

echo.o: $(NM_INC) 
echo_threads.o: $(NM_INC)


prebuild:
	make -C ../
clean:
	rm -rf $(CLEANFILES)
