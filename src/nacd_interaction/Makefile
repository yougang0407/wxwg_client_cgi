ARCH := x86
#ARCH := arm

ifeq (, $(ARCH))
CC := gcc
endif

ifeq (x86, $(ARCH))
CC := gcc
endif

CFLAGS := -g

SRCS := $(wildcard nacd_tcp_client.c)
SRCS_TEST := $(wildcard test_nacd_tcp_client_main.c)
OBJS := $(patsubst %c, %o, $(SRCS))

INCLUDE := -I./include/ -I../include/ -I../lib/libevent/libevent-2.0.21-stable/ \
		   -I../lib/libevent/libevent-2.0.21-stable/include/ -I../lib/openssl/openssl-1.0.2q/include/
LIBEVENT := -llibevent
LIBOPENSSL := -lssl -lcrypto -ldl -lpthread
LIBPATH := -L../lib/libevent/libevent-2.0.21-stable/.libs/ -L../lib/openssl/openssl-1.0.2q/
LIBS := ../lib/libevent/libevent-2.0.21-stable/.libs/libevent.so ../lib/openssl/openssl-1.0.2q/libssl.a \
		../lib/openssl/openssl-1.0.2q/libcrypto.a

.PHONY : all
all : $(OBJS) test_nacd_tcp_clientd
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE) $(LIBPATH) $(LIBEVENT) $(LIBOPENSSL)
test_nacd_tcp_clientd : $(SRCS_TEST)
	$(CC) $(CFLAGS) -o $@ $(SRCS_TEST) $(OBJS) $(INCLUDE) $(LIBPATH) $(LIBOPENSSL) $(LIBS)

.PHONY : clean
clean :
	rm -rf *.o test_nacd_tcp_clientd

