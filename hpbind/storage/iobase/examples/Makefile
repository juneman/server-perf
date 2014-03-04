
#
# author: db
#

PROGS = netmap_raw_test \
		cbuff_use_notify_test \
		cbuff_lock_use_notify_test \
		cbuff_no_notify_test \
		cbuff_lock_no_notify_test \
		sq_use_notify_test \
		sq_no_notify_test \
		lfq_use_notify_test \
		lfq_no_notify_test

CC=gcc
CLEANFILES=$(PROGS) *.o
CFLAGS += -Werror -Wall -g
CFLAGS += -I../ -I../include/ 

LDFLAGS +=  -lpthread 

all: prebuild clean $(PROGS)

netmap_raw_test: netmap_raw_test.o ./../lib/libionm.a 
	$(CC) $(CFLAGS)  -o $@ $^

cbuff_use_notify_test: use_notify_test.o ./../lib/libiobase_cb.a 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
cbuff_no_notify_test: no_notify_test.o  ./../lib/libiobase_cb.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
cbuff_lock_use_notify_test: use_notify_test.o ./../lib/libiobase_cb_lock.a 
	$(CC) $(CFLAGS) $(LDFLAGS) -DCBUFF_USE_LOCK -o $@ $^
cbuff_lock_no_notify_test: no_notify_test.o  ./../lib/libiobase_cb_lock.a
	$(CC) $(CFLAGS) $(LDFLAGS) -DCBUFF_USE_LOCK -o $@ $^

sq_use_notify_test: use_notify_test.o ./../lib/libiobase_sq.a 
	$(CC) $(CFLAGS) $(LDFLAGS) -DSQUEUE_USE_SLIST  -o $@ $^
sq_no_notify_test: no_notify_test.o  ./../lib/libiobase_sq.a
	$(CC) $(CFLAGS) $(LDFLAGS) -DSQUEUE_USE_SLIST -o $@ $^

lfq_use_notify_test: use_notify_test.o ./../lib/libiobase_lfq.a 
	$(CC) $(CFLAGS) $(LDFLAGS) -DSQUEUE_USE_LFDS -o $@ $^
lfq_no_notify_test: no_notify_test.o  ./../lib/libiobase_lfq.a
	$(CC) $(CFLAGS) $(LDFLAGS) -DSQUEUE_USE_LFDS -o $@ $^

prebuild:
	make -C ../
clean:
	rm -rf $(CLEANFILES)


