#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <string.h>
#include "time_calc.h"
#include "link_tool.h"
extern "C" void* __real_malloc(size_t);
extern "C" void *__real_realloc(void* c, int size);
extern "C" void* __real_calloc(size_t);
extern "C" void __real_free(void* p);

static ThreadQueue threadQueue;

pid_t gettid()
{
	return syscall(SYS_gettid);
}

char *__getBackTrace()
{
	const int stackNum = 24;
       void *stack_addr[stackNum];
       int layer;
       int i;
	char tmp[256];
	const char *pTraceHead = "addr2line -e ./Challenge_Debug -f -C  ";
	int backTraceLen = strlen(pTraceHead) + stackNum * 8;
	char* pBackTrace = (char*)__real_malloc(backTraceLen);
	strcpy(pBackTrace, pTraceHead);
	
	layer = backtrace(stack_addr, stackNum);
	for(i = 3; i < layer; i++)
	{
		snprintf(tmp, sizeof(tmp), "%p  ", stack_addr[i]);
		strcat(pBackTrace, tmp);
	}
	return pBackTrace;
}
void __realaseBackTrace(char *backTrace)
{
	__real_free(backTrace);
}

extern "C" void *__wrap_malloc(size_t c)
{
	void* p = __real_malloc(c);

	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(c, p);
	}

	return p; 
}
extern "C" void* __wrap_realloc(void *p, size_t c)
{
	p = __real_realloc(p, c);
	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(c, p);
	}
	
	return p;
}
extern "C" void* __wrap_calloc(size_t c)
{
	void *p = __real_calloc(c); 
	if (p && ThreadQueue::getEnable())
	{
		ThreadQueue::instance()->wrapMalloc(c, p);
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




