ARCH := x86
#ARCH := arm

ifeq (,$(ARCH))
CC := gcc
endif

ifeq (x86,$(ARCH))
CC := gcc
endif

CFLAGS := -g

SRCS := $(wildcard nacd_tcp_client.c)
SRCS_TEST := $(wildcard test_nacd_tcp_client_main.c)
OBJS := $(patsubst %c, %o, $(SRCS))

INCLUDE := -I../include/

.PHONY:all
all:$(OBJS) test_nacd_tcp_clientd
%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)
test_nacd_tcp_clientd:$(SRCS_TEST)
	$(CC) $(CFLAGS) -o $@ $(SRCS_TEST) $(OBJS) $(INCLUDE)

.PHONY:clean
clean:
	rm -rf *.o test_nacd_tcp_clientd

