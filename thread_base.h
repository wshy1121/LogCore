#ifndef __THREAD_BASE_H
#define __THREAD_BASE_H
#include "stdafx.h"


namespace base
{
#ifdef WIN32
typedef CRITICAL_SECTION  pthread_mutex_t;
typedef unsigned long pthread_t;
typedef int  pthread_attr_t;
typedef int  pthread_mutexattr_t;
#else
typedef ::pthread_mutex_t  pthread_mutex_t;
typedef ::pthread_t  pthread_t;
typedef ::pthread_attr_t  pthread_attr_t;
typedef ::pthread_mutexattr_t  pthread_mutexattr_t;
#endif


int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
pthread_t pthread_self(void);
int pthread_join(pthread_t thread, void **retval);
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int backtrace(void **buffer, int size);
}//base



#endif //__THREAD_BASE_H

