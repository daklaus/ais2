CC=gcc
CFLAGS=-m32 -g -fno-stack-protector -D_FORTIFY_SOURCE=0
LDFLAGS=-m32
CXXFLAGS=-m32
CPPFLAGS=-m32

all: bankROPery

libinetsec.so: libinetsec.c
	gcc $(CFLAGS) -c -Wall -Werror -fpic libinetsec.c
	gcc $(CFLAGS) -shared -o libinetsec.so  libinetsec.o 

clean:
	rm -f *.o 
clean-all:	clean
	rm -f *.so bankROPery

bankROPery: bankROPery.c libinetsec.so
	gcc $(CFLAGS) -L./ -Wl,-rpath=./ -Wall -o bankROPery bankROPery.c -linetsec

run: bankROPery
	LD_LIBRARY_PATH=./ ./bankROPery 2000


