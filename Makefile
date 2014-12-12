############################################################################
#
# Makefile for DH-DVR_ARM2510
#
# arm-uclinux-elf-gcc version 3.4.3
#
############################################################################
#CROSS  = sh4-linux-uclibc-
#CROSS  = arm-none-linux-gnueabi-
#CROSS  = arm-linux-gnueabihf-
ifeq ($(OPT),WRAP)
OPT_CFLAGS += -DWRAP -Wl,-wrap,malloc -Wl,-wrap,realloc -Wl,-wrap,calloc -Wl,-wrap,free -static
OPT_CFLAGS +=  -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -lc
OPT_CFLAGS	+= -fstack-protector-all
CFLAGS += -DWRAP
LIB_OBJS += wrap_malloc.o mem_calc.o mem_check.o log_opr.o link_tool.o mem_base.o thread_base.o 
else
OPT_CFLAGS +=  -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -lc -static
endif


ifeq ($(CROSS),arm-linux-gnueabihf-)
#OPT_CFLAGS += -L backtrace/m64-gnueabihf-linux -lunwind -lbacktrace 
#CFLAGS += -D__arm__ -DUNW_LOCAL_ONLY 
endif


CPP	=	@echo " g++ $@"; $(CROSS)g++
CC	=	@echo " gcc $@"; $(CROSS)gcc
LD	=	@echo " ld  $@"; $(CROSS)ld
AR  = @echo " ar  $@"; $(CROSS)ar
STRIP	=	@echo " strip $@"; $(CROSS)strip
RANLIB = @echo " ranlib  $@"; $(CROSS)ranlib

CP	= cp -rf
RM	= rm

AFLAGS	+= -r   

LIB_OBJS += time_calc.o Global.o

LIB_TARGET=libwrapmalloc.a

all	:	$(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) $(AFLAGS) $@ $^
	$(RANLIB) $@
	$(CPP) -g -o deamon main.cpp  $(LIB_OBJS) $(OPT_CFLAGS)

.c.o:
	$(CC) -c $(CFLAGS) $^ -o $@

.cpp.o:
	$(CPP) -c -Wall $(CFLAGS) $^ -o $@

clean:
	$(RM) $(LIB_OBJS) deamon 

install:
	cp $(LIB_TARGET) ../Libs/libwrapmalloc.a 


	
