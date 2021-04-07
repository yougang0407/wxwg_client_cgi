ARCH := x86
#ARCH := arm

ifeq (,$(ARCH))
CC := gcc
endif

ifeq (x86,$(ARCH))
CC := gcc
endif

CFLAGS := -g

SRCS := $(wildcard test_libevent_main.c)
OBJS := $(patsubst %c, %o, $(SRCS))

INCLUDE := -I../../include/ -I./libevent-2.0.21-stable/ -I./libevent-2.0.21-stable/include/

LIBEVENT := -llibevent
LIBPATH := -L./libevent-2.0.21-stable/.libs/
LIBS := ./libevent-2.0.21-stable/.libs/libevent.so

.PHONY:all
all:$(OBJS) test_libeventd
%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE) $(LIBPATH) $(LIBEVENT)
test_libeventd:$(SRCS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(INCLUDE) $(LIBS)

.PHONY:clean
clean:
	rm -rf *.o test_libeventd
