
#
# author: db
#

TESTSUIT = squeue_test slist_test spip_test

NM_LIB_PATH=../
NM_LIB=nmio
NM_INC=../iobase.h

CLEANFILES=$(TESTSUIT) *.o
CFLAGS= -g 
CFLAGS += -Werror -Wall
CFLAGS += -I ../ 

LDFLAGS += -L$(NM_LIB_PATH) -lpthread -l$(NM_LIB)

all: prebuild clean $(TESTSUIT)  

# testcase
squeue_test: squeue_test.o test.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

slist_test: slist_test.o test.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

spip_test: spip_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^



prebuild:
	make -C ../
clean:
	rm -rf $(CLEANFILES)
