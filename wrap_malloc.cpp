#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <string.h>

#include "time_calc.h"
#include "link_tool.h"
const int stackNum = 24;
const char *pTraceHead = "addr2line -e ./Challenge_Debug -f -C  ";
int maxBackTraceLen = strlen(pTraceHead) + stackNum * 16;
static ThreadQueue threadQueue;
extern "C" void* __real_malloc(size_t);
extern "C" void *__real_realloc(void* c, int size);
extern "C" void* __real_calloc(size_t);
extern "C" void __real_free(void* p);


pid_t gettid()
{
	return syscall(SYS_gettid);
}

#ifdef UNW_LOCAL_ONLY
#include "unwind/include/libunwind.h"
void my_bt(int *list, int size)
{
	int i = 0;

	unw_cursor_t    cursor;
	unw_context_t   uc;
	//unw_proc_info_t pip;
	unw_word_t      ip;
	//unw_word_t      sp;
	//unw_word_t      off;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	while (unw_step(&cursor) > 0 && i < size) 
	{	tracepoint1();
		unw_get_reg(&cursor, UNW_REG_IP, &ip);

		//list[i++] = (int)ip;
		printf("%p  ", (void *)ip);

	}
	printf("\n");
}
#endif
char *__getBackTrace(char *pBackTrace, int backTraceLen)
{
	strcpy(pBackTrace, "123");
	return pBackTrace;
	
       void *stack_addr[stackNum];
       int layer;
       int i;
	char tmp[256];
	strcpy(pBackTrace, "");
	if (backTraceLen < maxBackTraceLen)
	{
		pBackTrace[0] = '\0';
		return NULL;
	}
	
	strcpy(pBackTrace, pTraceHead);
	
	layer = backtrace(stack_addr, stackNum);
	for(i = 3; i < layer; i++)
	{
		snprintf(tmp, sizeof(tmp), "%p  ", stack_addr[i]);
		strcat(pBackTrace, tmp);
	}
	return pBackTrace;
}

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




