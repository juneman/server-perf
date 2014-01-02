all: clean bldenv  nm_util.o dns_util.o bind9_nm_io.o

nm_util.o: nm_util.h
dns_util.o: dns_util.h
bind9_nm_io.o: bind9_nm_io.h


bldenv:
	echo "#define __BUILD_TIME \"`date`\"" > build.h

install:
clean:
	rm -f *.o
