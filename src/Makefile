ARCH := x86
#ARCH := arm

ifeq (,$(ARCH))
CC := gcc
endif

ifeq (x86,$(ARCH))
CC := gcc
endif

TOP = $(CURDIR)
SRC_DIR = $(TOP)
SRC = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/intertest/test/user/user.c)
SRC_SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
NONE_SRC_OBJ = $(patsubst %.c, %.o, $(wildcard $(SRC_DIR)/intertest/test/user/user.c))

INCLUDE = -I./ -I./include/ -I./intertest/test/user/

SUBDIRS := $(SRC_DIR)/intertest/test/user $(SRC_DIR)/nacd_interaction

MAKE = make

.PHONY:all
all:wxwg_clientd

	cd ./ && $(MAKE) -f $(TOP)/Makefile wxwg_clientd CURDIR=$(TOP)


wxwg_clientd:$(OBJ)
	for dir in $(SUBDIRS); \
	do $(MAKE) -C $$dir all CC=$(CC) || exit 1; \
	done

	$(CC) -g -o $@ $(SRC_SRC) $(NONE_SRC_OBJ) $(INCLUDE) -lpthread


.PHONY:clean
clean:
	@for dir in $(SUBDIRS); do make -C $$dir clean|| exit 1; done
	rm -rf *.o wxwg_clientd output

.PHONY:output
output:all
	mkdir output
	cp -f wxwg_clientd output/
	cp -rf log output/
	cp -rf conf output/
	cp -rf webui output/
	cp -rf webui/cgi_bin/ output/
	cp -r http_ctl.sh output/
