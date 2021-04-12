ARCH = x86
#ARCH = arm

ifeq (, $(ARCH))
CC = gcc
endif

ifeq (x86, $(ARCH))
CC = gcc
endif

CFLAGS = -g

TOP = $(CURDIR)
SRC_DIR = $(TOP)
SRC = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/web_server/*.c) \
	  $(wildcard $(SRC_DIR)/intertest/test/user/user.c) \
	  $(wildcard $(SRC_DIR)/nacd_interaction/nacd_tcp_client.c)
SRC_SRC = $(wildcard $(SRC_DIR)/*.c)
NONE_SRC_SRC = $(wildcard $(SRC_DIR)/web_server/*.c) \
			   $(wildcard $(SRC_DIR)/intertest/test/user/user.c) \
			   $(wildcard $(SRC_DIR)/nacd_interaction/nacd_tcp_client.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
NONE_SRC_OBJ = $(patsubst %.c, %.o, $(NONE_SRC_SRC))

INCLUDE = -I./include/ -I./web_server/ -I./intertest/test/user/ -I./lib/libevent/libevent-2.0.21-stable/ \
		  -I./lib/libevent/libevent-2.0.21-stable/include/
LIBEVENT = -llibevent
LIBOPENSSL = -lssl -lcrypto -ldl -lpthread
LIBPATH = -L./lib/libevent/libevent-2.0.21-stable/.libs/ -L./lib/openssl/openssl-1.0.2q/
LIBS = ./lib/libevent/libevent-2.0.21-stable/.libs/libevent.so ./lib/openssl/openssl-1.0.2q/libssl.a \
		./lib/openssl/openssl-1.0.2q/libcrypto.a

SUBDIRS = $(SRC_DIR)/web_server $(SRC_DIR)/intertest/test/user $(SRC_DIR)/nacd_interaction

MAKE = make

.PHONY : all
all : wxwg_clientd

	cd ./ && $(MAKE) -f $(TOP)/Makefile wxwg_clientd CURDIR=$(TOP)

%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDE) $(LIBPATH) $(LIBEVENT) $(LIBOPENSSL)

wxwg_clientd : $(OBJ)
	for dir in $(SUBDIRS); \
	do $(MAKE) -C $$dir all CC=$(CC) || exit 1; \
	done

	$(CC) $(CFLAGS) -o $@ $(SRC_SRC) $(NONE_SRC_OBJ) $(INCLUDE) $(LIBPATH) $(LIBOPENSSL) $(LIBS)


.PHONY : clean
clean :
	@for dir in $(SUBDIRS); do make -C $$dir clean|| exit 1; done
	rm -rf *.o wxwg_clientd output

.PHONY : output
output : all
	mkdir output
	cp -f wxwg_clientd output/
	cp -rf log output/
	cp -rf conf output/
	cp -rf webui output/
	cp -rf webui/cgi_bin/ output/
	cp -r http_ctl.sh output/
