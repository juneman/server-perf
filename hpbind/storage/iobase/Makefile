.PHONY: all bldenv clean install

all: bldenv clean install

CC=gcc
AR=ar
RANLIB=ranlib
TOUCH=touch

CFLAGS += -g -Wall -Werror 
CFLAGS += -fPIC -I./include/

########################################################################
OBJS = netmapio.o cs_util.o dns_util.o poll_util.o slist.o sdata.o  

#####################################################
CBUFF_TARGET_LIB = ./lib/libiobase_cb.a
LOCK_CBUFF_TARGET_LIB = ./lib/libiobase_cb_lock.a
SQ_TARGET_LIB = ./lib/libiobase_sq.a
LFQ_TARGET_LIB = ./lib/libiobase_lfq.a
IONM_LIB = ./lib/libionm.a
TARGET_LIB = $(CBUFF_TARGET_LIB) $(LOCK_CBUFF_TARGET_LIB) $(SQ_TARGET_LIB)  $(IONM_LIB)
ALL_TARGET_LIBS = $(TARGET_LIB) $(LFQ_TARGET_LIB)
############################################################################
cbuff-lock.o : cbuff.c					  
	$(CC) -g -c $(CFLAGS) -DCBUFF_USE_LOCK -o $@  $^ 
iobase-cb-lock.o : iobase-cb.c 
	$(CC) -g -c $(CFLAGS) -DCBUFF_USE_LOCK -o $@  $^ 
squeue-sq.o : squeue.c 
	$(CC) -g -c $(CFLAGS) -DSQUEUE_USE_SLIST -o $@  $^ 
iobase-sq.o : iobase-sq.c
	$(CC) -g -c $(CFLAGS) -DSQUEUE_USE_SLIST -o $@  $^ 
squeue-lfq.o : squeue.c 
	$(CC) -g -c $(CFLAGS) -DSQUEUE_USE_LFDS $^  -o $@  
iobase-lfq.o : iobase-sq.c 
	$(CC) -g -c $(CFLAGS) -DSQUEUE_USE_LFDS $^ -o $@  

################################################################
$(TARGET_LIB) : %.a :  
	$(AR) crv ./$@ $^
	$(RANLIB) ./$@
	$(TOUCH) timestamp

$(IONM_LIB) : $(OBJS) 
$(CBUFF_TARGET_LIB) : $(OBJS) cbuff.o iobase-cb.o 
$(LOCK_CBUFF_TARGET_LIB) : $(OBJS) cbuff-lock.o iobase-cb-lock.o 
$(SQ_TARGET_LIB) : $(OBJS) squeue-sq.o iobase-sq.o 
$(LFQ_TARGET_LIB) : $(OBJS) squeue-lfq.o iobase-lfq.o 
	$(AR) crv ./$@ $^
	$(RANLIB) ./$@
	$(TOUCH) timestamp
	/bin/rm -f ./lib/liblfds611.a
	make clean -C ./liblfds6.1.1/liblfds611/  -f makefile.linux
	make -C ./liblfds6.1.1/liblfds611/  -f makefile.linux
	cp -f ./liblfds6.1.1/liblfds611/bin/liblfds611.a ./lib/liblfds611.a
	echo OPEN ./$@ > ar.tmp
	echo ADDLIB ./lib/liblfds611.a >> ar.tmp
	echo SAVE >> ar.tmp
	echo END >> ar.tmp
	$(AR) -M < ar.tmp
	/bin/rm -f ar.tmp
	$(RANLIB) ./$@
	$(TOUCH) timestamp

bldenv:
	echo "#define __BUILD_TIME \"`date`\"" > build.h
	echo "Link iolib: $(IOLIB)"

install: $(ALL_TARGET_LIBS) 
	echo ""
	ls ./lib/

clean:
	rm -f *.o $(ALL_TARGET_LIBS) 
	make clean -C test/
	make clean -C examples/
	make clean -C ./liblfds6.1.1/liblfds611/  -f makefile.linux

