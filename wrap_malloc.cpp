#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "Global.h"
#include "link_tool.h"
extern "C" void* __real_malloc(size_t);
extern "C" void* __real_realloc(size_t);
extern "C" void* __real_calloc(size_t);
extern "C" void __real_free(void* p);

static ThreadQueue threadQueue;

pid_t gettid()
{
	return syscall(SYS_gettid);
}

extern "C" void *__wrap_malloc(size_t c)
{
	void* p = __real_malloc(c);

	if (p && ThreadQueue::instance()->getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(c, p);
	}

	return p; 
}
extern "C" void* __wrap_realloc(size_t c)
{
	void *p = __real_realloc(c);
	if (p && ThreadQueue::instance()->getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(c, p);
	}
	
	return p;
}
extern "C" void* __wrap_calloc(size_t c)
{
	void *p = __real_calloc(c); 
	if (p && ThreadQueue::instance()->getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(c, p);
	}	
	return p;
}
extern "C" void __wrap_free(void*p)
{
	if (p && ThreadQueue::instance()->getEnable())
	{
		ThreadQueue::instance()->wrapFree(p);
	}
	__real_free(p);

}


