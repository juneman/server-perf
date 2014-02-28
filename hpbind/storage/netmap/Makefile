all: bldenv clean install

CC=gcc
AR=ar
RANLIB=ranlib
TOUCH=touch

IOLIB=CB

CFLAGS += -g -Wall -Werror 
CFLAGS += -fPIC -I./include/
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
TARGET_SRC = iobase-sq-p-lfds.o	
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
endif ## end CBEV

endif ## end LFQ

endif ##

OBJS = $(TARGET_SRC) $(LIBOBJS)

TARGET_LIB = ./lib/libiobase.a

$(TARGET_LIB) : $(OBJS) 
	$(AR) crv ./$@ $^
	$(RANLIB) ./$@
	$(TOUCH) timestamp
ifeq ($(IOLIB), SQ) 
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
endif

bldenv:
	echo "#define __BUILD_TIME \"`date`\"" > build.h
	echo "Link iolib: $(IOLIB)"

install: $(TARGET_LIB)  $(OBJS)

clean:
	rm -f *.o $(TARGET_LIB) 
	make clean -C test/
	make clean -C examples/
	make clean -C ./liblfds6.1.1/liblfds611/  -f makefile.linux
