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

CPP	=	@echo " g++ $@"; $(CROSS)g++
CC	=	@echo " gcc $@"; $(CROSS)gcc
LD	=	@echo " ld  $@"; $(CROSS)ld
AR  = @echo " ar  $@"; $(CROSS)ar
STRIP	=	@echo " strip $@"; $(CROSS)strip
RANLIB = @echo " ranlib  $@"; $(CROSS)ranlib

CP	= cp -rf
RM	= rm

AFLAGS	+= -r   

LIB_OBJS = wrap_malloc.o Global.o

LIB_TARGET=libwrapmalloc.a

all	:	$(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) $(AFLAGS) $@ $^
	$(RANLIB) $@
	g++ -o test Global.cpp main.cpp -lpthread

.c.o:
	$(CC) -c $(CFLAGS) $^ -o $@

.cpp.o:
	$(CPP) -c -Wall $(CFLAGS) $^ -o $@

clean:
	$(RM) $(LIB_OBJS) test

install:
	cp $(LIB_TARGET) ../Libs/libwrapmalloc.a 


	
