all: clean bldenv  nm_util.o dns_util.o

nm_util.o: nm_util.h
dns_util.o: dns_util.h


bldenv:
	echo "#define __BUILD_TIME \"`date`\"" > build.h

install:
clean:
	rm -f *.o
