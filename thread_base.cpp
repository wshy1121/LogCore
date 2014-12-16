#include "stdafx.h"
#include "thread_base.h"
#ifdef WIN32
#include <assert.h>
#include <process.h>
#else
#include <execinfo.h>
#endif

namespace base
{
#ifdef WIN32
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg)
{
	void (*routine)(void *) = (void (*) (void *))start_routine;
	return (int)_beginthread(routine, 0, arg);
}


pthread_t pthread_self(void)
{
	return GetCurrentThreadId();
}

int pthread_join(pthread_t thread, void **retval)
{
	assert(0);
	return 0;
}


int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	assert(0);
	return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	InitializeCriticalSection(mutex);
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	EnterCriticalSection(mutex);
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	assert(0);
	return 0;
}
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	LeaveCriticalSection(mutex);
	return 0;
}

int backtrace(void **buffer, int size)
{
	assert(0);
	return 0;
}

#else
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg)
{
	return ::pthread_create(thread, attr, start_routine, arg);
}


pthread_t pthread_self(void)
{
	return ::pthread_self();
}

int pthread_join(pthread_t thread, void **retval)
{
	return ::pthread_join(thread, retval);
}


int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	return ::pthread_mutex_destroy(mutex);
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	return ::pthread_mutex_init(mutex, attr);
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	return ::pthread_mutex_lock(mutex);
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	return ::pthread_mutex_trylock(mutex);
}
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	return ::pthread_mutex_unlock(mutex);
}

int backtrace(void **buffer, int size)
{
	return ::backtrace(buffer, size);
}
#endif
}//base



