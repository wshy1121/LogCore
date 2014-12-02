#ifndef __INCLUDE_C_BT_COMMON_H__
#define __INCLUDE_C_BT_COMMON_H__

#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <memory.h>
#include <ucontext.h>
#include <time.h>
#include <string.h>
#include <sys/syscall.h>

#define gettid()	(int)syscall(__NR_gettid)

#define PLATFORM_X86			0
#define PLATFORM_ARM			1
#define PLATFORM_SH4			2
#define PLATFORM_ARM_V5T_LE	3
#define PLATFORM_POWERPC 4
#define PLATFORM_CSKY 5

#endif //__INCLUDE_C_BT_COMMON_H__

