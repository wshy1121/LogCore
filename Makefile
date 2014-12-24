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

LIB_OBJS += wrap_malloc.o mem_calc.o mem_check.o log_opr.o 
LIB_OBJS += link_tool.o mem_base.o thread_base.o platform_base.o string_base.o 
LIB_OBJS += net_server.o defs.o net_client.o 
LIB_OBJS += time_calc.o Global.o

CPP	=	@echo " g++ $@"; $(CROSS)g++
CC	=	@echo " gcc $@"; $(CROSS)gcc
LD	=	@echo " ld  $@"; $(CROSS)ld
AR  = @echo " ar  $@"; $(CROSS)ar
STRIP	=	@echo " strip $@"; $(CROSS)strip
RANLIB = @echo " ranlib  $@"; $(CROSS)ranlib

CP	= cp -rf
RM	= rm

AFLAGS	+= -r   


LIB_TARGET=libLogCore.a

all	:	$(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) $(AFLAGS) $@ $^
	$(RANLIB) $@

.c.o:
	$(CC) -c $(CFLAGS) $^ -o $@

.cpp.o:
	$(CPP) -c -Wall $(CFLAGS) $^ -o $@

clean:
	$(RM) $(LIB_OBJS)

install:
	cp $(LIB_TARGET) ../Libs/libLogCore.a 


	
