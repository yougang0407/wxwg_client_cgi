ARCH := x86
#ARCH := arm

ifeq (, $(ARCH))
CC := gcc
endif

ifeq (x86, $(ARCH))
CC := gcc
endif

CFLAGS := -g -fno-strict-aliasing

TOP := $(CURDIR)
SRC_DIR := $(TOP)
SRCS := ./dtd/dtd_chk.c ./expat-lite/xmltok.c ./expat-lite/xmlrole.c \
		./expat-lite/xmlparse.c ./lib/domlib.c ./lib/domlog.c \
		./dom-tree/domtree.c ./dom-tree/domapi.c ./dom-tree/gb.c sg_xmllib.c
OBJS := $(patsubst %c, %o, $(SRCS))

TARGET_STATIC_LIB := libsg_xmldom.a
TARGET_DYNAMIC_LIB := libsg_xmldom.so
AR := ar rcs
CFLAGS_LIB := -shared -fPIC

INCLUDE := -I./ -I./dom-tree/ -I./dtd/ -I./expat-lite/ -I./lib/ -I../libxml2/include/ \
		   -I./include/ -I./include/libxml/

LIBXML := -lxml2
LIBPATH := -L../libxml2/.libs/
LIBS := ../libxml2/.libs/libxml2.so


MAKE := make

.PHONY : all
all : $(OBJS) $(TARGET_STATIC_LIB) $(TARGET_DYNAMIC_LIB)
$(OBJS) : %.o : %.c
	$(CC) $(CFLAGS) -o $@ -c -fPIC $< $(INCLUDE) $(LIBPATH) $(LIBXML)
$(TARGET_STATIC_LIB) : $(OBJS)
	$(AR) -o $@ $(OBJS) $(LIBS)
$(TARGET_DYNAMIC_LIB) : $(OBJS)
	$(CC) $(CFLAGS_LIB) -o $@ $(OBJS) $(LIBS)

.PHONY : clean
clean :
	rm -rf $(OBJS) $(TARGET_STATIC_LIB) $(TARGET_DYNAMIC_LIB)

