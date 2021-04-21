ARCH := x86
#ARCH := arm

ifeq (, $(ARCH))
CC := gcc
endif

ifeq (x86, $(ARCH))
CC := gcc
endif

CFLAGS := -g -Wall -fno-strict-aliasing

SRCS := $(wildcard nacd_tcp_client.c ./soap/*.c) 
SRCS_TEST := $(wildcard test_nacd_tcp_client_main.c)
OBJS := $(patsubst %c, %o, $(SRCS))

INCLUDE := -I./include/ -I./soap/ -I../include/ -I../lib/libevent/libevent-2.0.21-stable/ \
		   -I../lib/libevent/libevent-2.0.21-stable/include/ -I../lib/openssl/openssl-1.0.2q/include/ \
		   -I../lib/xml/sg_xml/ -I../lib/xml/libxml2/include/libxml/ -I../lib/xml/libxml2/include/ \
		   -I../lib/xml/sg_xml/include/libxml/

LIBL := -levent -lssl -lcrypto -ldl -lpthread -lsg_xmldom -lxml2 -lgsoap
LIBPATH := -L../lib/libevent/libevent-2.0.21-stable/.libs/ -L../lib/openssl/openssl-1.0.2q/ \
		   -L../lib/xml/sg_xml/ -L../lib/xml/libxml2/.libs/ -L../lib/gsoap/usr/lib/
LIBS := ../lib/libevent/libevent-2.0.21-stable/.libs/libevent.so ../lib/openssl/openssl-1.0.2q/libssl.a \
		../lib/openssl/openssl-1.0.2q/libcrypto.a ../lib/xml/sg_xml/libsg_xmldom.so \
		../lib/xml/libxml2/.libs/libxml2.so ../lib/gsoap/usr/lib/libgsoap.a

.PHONY : all
all : $(OBJS) test_nacd_tcp_clientd
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE) $(LIBPATH) $(LIBL)
test_nacd_tcp_clientd : $(SRCS_TEST)
	$(CC) $(CFLAGS) -o $@ $(SRCS_TEST) $(OBJS) $(INCLUDE) $(LIBPATH) $(LIBL) $(LIBS)

.PHONY : clean
clean :
	rm -rf *.o ./soap/*.o test_nacd_tcp_clientd

