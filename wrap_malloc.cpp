#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <string.h>

#include "time_calc.h"
#include "mem_calc.h"
#include "mem_check.h"

extern "C" void *__wrap_malloc(size_t c)
{
	void* p = CMemCheck::malloc(c);

	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(p, c);
	}

	return p;
}
extern "C" void* __wrap_realloc(void *p, size_t c)
{
	p = CMemCheck::realloc(p, c);
	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(p, c);
	}
	
	return p;
}
extern "C" void* __wrap_calloc(size_t nmemb, size_t size)
{
	size_t c = nmemb * size;
	void *p = CMemCheck::calloc(c); 
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
	CMemCheck::free(p);

}




