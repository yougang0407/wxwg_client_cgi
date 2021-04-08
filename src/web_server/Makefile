ARCH := x86
#ARCH := arm

ifeq (, $(ARCH))
CC := gcc
endif

ifeq (x86, $(ARCH))
CC := gcc
endif

CFLAGS := -g

SRCS := $(wildcard *.c)
OBJS := $(patsubst %c, %o, $(SRCS))

INCLUDE := -I./ -I../include/ -I../lib/libevent/libevent-2.0.21-stable/ \
		   -I../lib/libevent/libevent-2.0.21-stable/include/

.PHONY : all
all : $(OBJS)
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE)

.PHONY : clean
clean :
	rm -rf *.o


