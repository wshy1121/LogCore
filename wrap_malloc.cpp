#ifdef WRAP

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
//	void *p = malloc(c);
	void* p = __real_malloc(c);
	//if (c >= 10 * 1024)
	{
		if (c > 64)
		{
			//printf("MALLOC called: %d, pid: %d, tid: %d, addresss: %p\n", c, getpid(), gettid(), p);
		}
			
	}

	if (threadQueue.getEnable())
	{
		ThreadNode *queue_node = NULL;
		threadQueue.getQueue(pthread_self(), &queue_node);
		if (!queue_node)
		{
			printf("%d  %s\n", __LINE__, __FILE__);
			queue_node = (ThreadNode *)__real_malloc(sizeof(ThreadNode));
			threadQueue.initThreadNode(queue_node, true, pthread_self());
			threadQueue.insertQueue(queue_node);
			threadQueue.dispQueue();
		}
		else
		{
		}

		
	}
	//threadQueue.putQueue(ThreadNode * queue_node);

	return p; 
}
extern "C" void* __wrap_realloc(size_t c)
{
	void *p = __real_realloc(c);
	//if (c >= 10 * 1024)
	{
		if (c > 64)
		{
			//printf("REALLOC called: %d, pid: %d, tid: %d, addresss: %p\n", c, getpid(), gettid(), p);
		}
			
	}
	
	return p;
}
extern "C" void* __wrap_calloc(size_t c)
{
	void *p = __real_calloc(c); 
	//if (c >= 10 * 1024)
	{
		if (c > 64)
		{
			//printf("CALLOC called: %d, pid: %d, tid: %d, addresss: %p\n", c, getpid(), gettid(), p);
		}
			
	}
	return p;
}
extern "C" void __wrap_free(void*p)
{
	//printf("free : %p\n", p);
	return __real_free(p);
}

#endif

