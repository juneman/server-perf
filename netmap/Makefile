all: bldenv clean install

CC=gcc
AR=ar cvr
RANLIB=ranlib
TOUCH=touch

IOLIB=LFQ

CFLAGS += -g -Wall -Werror 
CFLAGS += -fPIC
nm_util.o: nm_util.h
slist.o: slist.h
sdata.o: sdata.h
cbuff.o: cbuff.h
lfqueue.o: lfqueue.h
squeue.o: squeue.h
dns_util.o: dns_util.h
LIBOBJS = nm_util.o dns_util.o slist.o sdata.o 


iobase-cb-p.o: $(LIBOBJS)
iobase-sq-p.o: $(LIBOBJS)
iobase-lfq-pn.o: $(LIBOBJS)

ifeq ($(IOLIB), SQ)
TARGET_SRC = iobase-sq-p.o	
LIBOBJS += squeue.o 
else
ifeq ($(IOLIB), LFQ)
TARGET_SRC = iobase-lfq-pn.o	
LIBOBJS += lfqueue.o
else

ifeq ($(IOLIB), CBEV)
TARGET_SRC = iobase-cb-p-evf.o	
LIBOBJS += cbuff.o
else
TARGET_SRC = iobase-cb-p.o	
LIBOBJS += cbuff.o
endif

endif
endif

OBJS = $(TARGET_SRC) $(LIBOBJS)

TARGET_LIB = libiobase.a

$(TARGET_LIB) : $(OBJS)
	$(AR) $@ $^
	$(RANLIB) $@
	$(TOUCH) timestamp

bldenv:
	echo "#define __BUILD_TIME \"`date`\"" > build.h
	echo "Link iolib: $(IOLIB)"

install: $(TARGET_LIB)  $(OBJS)

clean:
	rm -f *.o $(TARGET_LIB)
	make clean -C test/
	make clean -C examples/
