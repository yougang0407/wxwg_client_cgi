ARCH := x86
#ARCH := arm

ifeq (, $(ARCH))
CC := gcc
endif

ifeq (x86, $(ARCH))
CC := gcc
endif

CFLAGS := -g

SRCS := user.c
SRCS_TEST := $(wildcard test_user_main.c)
OBJS := $(patsubst %c, %o, $(SRCS))

INCLUDE := 

LIBSGXML := -lsg_xmldom
LIBPATH := -L../../../lib/xml/sg_xml/
LIBS := ../../../lib/xml/sg_xml/libsg_xmldom.so

.PHONY : all
all : $(OBJS) test_userd
%.o : $(SRCS)
	$(CC) $(CFLAGS) -o $@ -c $^ $(INCLUDE)
test_userd : $(SRCS_TEST)
	$(CC) $(CFLAGS) -o $@ $(SRCS_TEST) $(OBJS) $(INCLUDE)

.PHONY : clean
clean :
	rm -rf *.o test_userd

