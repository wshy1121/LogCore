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

CXXFLAGS += -std=c++0x -I ../CrossPlat -I../TraceWorker -I../NetApp -I../LogCore -I../Encrypt -I../CosApp 
LIB_OBJS += wrap_malloc.o mem_calc.o mem_check.o log_opr.o time_calc.o user_manager.o 

CPP	=	@echo " g++ $@"; $(CROSS)g++
CC	=	@echo " gcc $@"; $(CROSS)gcc
LD	=	@echo " ld  $@"; $(CROSS)ld
AR  = @echo " ar  $@"; $(CROSS)ar
STRIP	=	@echo " strip $@"; $(CROSS)strip
RANLIB = @echo " ranlib  $@"; $(CROSS)ranlib

CP	= cp -rf
RM	= rm

AFLAGS	+= -r   

LIB_TARGET=../CosApp/lib/libLogCore.a

all	:	$(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) $(AFLAGS) $@ $^
	$(RANLIB) $@

.c.o:
	$(CC) -c $(CFLAGS) $^ -o $@

.cpp.o:
	$(CPP) -c -Wall $(CXXFLAGS) $^ -o $@

clean:
	$(RM) $(LIB_OBJS)

install:
	cp $(LIB_TARGET) ../Build/Libs/libLogCore.a 


	
