#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <string.h>

#include "time_calc.h"
#include "mem_calc.h"
static ThreadQueue threadQueue;
extern "C" void* __real_malloc(size_t);
extern "C" void *__real_realloc(void* c, int size);
extern "C" void* __real_calloc(size_t);
extern "C" void __real_free(void* p);




extern "C" void *__wrap_malloc(size_t c)
{
	void* p = __real_malloc(c);

	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(p, c);
	}

	return p; 
}
extern "C" void* __wrap_realloc(void *p, size_t c)
{
	p = __real_realloc(p, c);
	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(p, c);
	}
	
	return p;
}
extern "C" void* __wrap_calloc(size_t c)
{
	void *p = __real_calloc(c); 
	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(p, c);
	}	
	return p;
}
extern "C" void __wrap_free(void*p)
{
	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapFree(p);
	}
	__real_free(p);

}




