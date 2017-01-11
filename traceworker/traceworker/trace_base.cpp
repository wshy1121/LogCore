#include <time.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include "trace_base.h"

CBase::pthread_t CBase::pthread_self(void)
{
#ifdef WIN32
	return GetCurrentThreadId();
#else
	return ::pthread_self();
#endif
}

int CBase::pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
#ifdef WIN32
	InitializeCriticalSection(mutex);
	return 0;
#else
	return ::pthread_mutex_init(mutex, attr);
#endif

}

int CBase::pthread_mutex_lock(pthread_mutex_t *mutex)
{
#ifdef WIN32
	EnterCriticalSection(mutex);
	return 0;
#else
	return ::pthread_mutex_lock(mutex);
#endif
}

int CBase::pthread_mutex_unlock(pthread_mutex_t *mutex)
{
#ifdef WIN32
	LeaveCriticalSection(mutex);
	return 0;
#else
	return ::pthread_mutex_unlock(mutex);
#endif
}

int CBase::pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
#ifdef WIN32
	void (*routine)(void *) = (void(*)(void *))start_routine;
return (int)_beginthread(routine, 0, arg);
	return 0;
#else
	return ::pthread_create(thread, attr, start_routine, arg);
#endif

}

int CBase::ftime(TimeB *tp)
{
#ifdef WIN32
	DWORD tickCount = GetTickCount();
	tp->time = tickCount / 1000;
	tp->millitm = tickCount % 1000;
	tp->timezone = 0;
	tp->dstflag = 0;
	return 0;
#else
	struct timeb timeB;
	
	int ret = ::ftime(&timeB);
	
	tp->time = timeB.time;
	tp->millitm = timeB.millitm;
	tp->timezone = timeB.timezone;
	tp->dstflag = timeB.dstflag;
	return ret;
#endif

}


int CBase::snprintf(char *str, size_t size, const char *format, ...)
{
#ifdef WIN32
	va_list ap;
	va_start(ap, format);
	int ret = vsnprintf_s(str, size, _TRUNCATE, format, ap);
	va_end(ap);
	return ret;
#else
	va_list ap;
	va_start(ap, format);
	int ret = ::vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
#endif	
}

int CBase::vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
#ifdef WIN32
	return vsnprintf_s(str, size, _TRUNCATE, format, ap);
#else
	return ::vsnprintf(str, size, format, ap);
#endif
}

FILE *CBase::fopen(const char *path, const char *mode)
{
#ifdef WIN32
	FILE *fp = NULL;
	fopen_s(&fp, path, mode);
	return fp;
#else
	return ::fopen(path, mode);
#endif
}


int CBase::usleep(int micro_second)
{
#ifdef WIN32
	if (micro_second < 1000)
	{
		micro_second = 1000;
	}
	Sleep(micro_second / 1000);
	return 0;
#else
	return ::usleep(micro_second);
#endif

}

struct tm *CBase::localtime(const time_t *timep)
{
#ifdef WIN32
	static struct tm w;
	time_t now;
	time(&now);
	localtime_s(&w, &now);
	return &w;
#else
	time_t now;
	time(&now);
	return ::localtime(&now);
#endif
}

size_t CBase::filesize(const char *path)
{
#ifdef WIN32
	struct _stat statbuf;
	if (_stat(path, &statbuf) == 0)
	{
		return statbuf.st_size;
	}
#else
	struct stat statbuf;
	if (stat(path, &statbuf) == 0)
	{
		return statbuf.st_size;
	}
#endif
	return 0;
}

