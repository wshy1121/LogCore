#ifndef _TRACE_BASE_H
#define _TRACE_BASE_H

#ifdef WIN32
#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <assert.h>
#else
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>
#endif
#include "platform_base.h"

class CBase
{
public:
#ifdef WIN32
	typedef unsigned long pthread_t;
	typedef CRITICAL_SECTION pthread_mutex_t;
	typedef int pthread_mutexattr_t;
	typedef int pthread_attr_t;
	//typedef struct _stat struct stat
#else
	typedef ::pthread_t pthread_t;
	typedef ::pthread_mutex_t pthread_mutex_t;
	typedef ::pthread_mutexattr_t pthread_mutexattr_t;
	typedef ::pthread_attr_t pthread_attr_t;
#endif

public:
	static pthread_t pthread_self(void);
	static int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
	static int pthread_mutex_lock(pthread_mutex_t *mutex);
	static int pthread_mutex_unlock(pthread_mutex_t *mutex);
	static int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
	static int ftime(TimeB *tp);
	static int snprintf(char *str, size_t size, const char *format, ...);
	static int vsnprintf(char *str, size_t size, const char *format, va_list ap);
	static FILE *fopen(const char *path, const char *mode);
	static int usleep(int micro_second);
	static struct tm *localtime(const time_t *timep);
	static size_t filesize(const char *path);
};
#endif
