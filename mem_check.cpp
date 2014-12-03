#include "mem_check.h"
#include "link_tool.h"

extern CPthreadMutex g_insMutexCalc;
extern "C" void __real_free(void* p);
extern "C" void* __real_malloc(size_t);
extern "C" void *__real_realloc(void* c, int size);
extern "C" void* __real_calloc(size_t);

CMemCheck *CMemCheck::_instance = NULL;
void *CMemCheck::m_checkValue = (void *)&CMemCheck::instance;

CMemCheck *CMemCheck::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CMemCheck;
		}
	}
	return _instance;
}
void *CMemCheck::malloc(size_t c)
{	
	void* p = __real_malloc(c + sizeof(void *) * 3);
	return (void *)((char *)p + sizeof(void *) * 2);
}


void* CMemCheck::realloc(void *p, size_t c)
{
	p = (void *)((char *)p - sizeof(void *) * 2);
	p = __real_realloc(p, c + sizeof(void *) * 3);
	return (void *)((char *)p + sizeof(void *) * 2);
}


void* CMemCheck::calloc(size_t c)
{
	void* p = __real_calloc(c + sizeof(void *) * 3);
	return (void *)((char *)p + sizeof(void *) * 2);
}

void CMemCheck::free(void*p)
{
	if (p == NULL)
	{
		return ;
	}
	p = (void *)((char *)p - sizeof(void *) * 2);
	__real_free(p);
}

void CMemCheck::initMem(void *addr, int addrLen, std::string &backTrace)
{
	addr = (void *)((char *)addr - sizeof(void *) * 2);
	printf("initMem  addr  %p\n", addr);
	void *beginAddr = (void *)addr;
	void *argAddr = (void *)((char *)addr + sizeof(void *));
	void *endAddr = (void *)((char *)addr + sizeof(void *) * 2 + addrLen);
	
	beginAddr = NULL;
	argAddr = m_checkValue;
	endAddr = m_checkValue;
}

void CMemCheck::checkMem(void *addr, const char *errInfo)
{	
	addr = (void *)((char *)addr - sizeof(void *) * 2);

	printf("checkMem  addr  %p\n", addr);
	
	void *beginAddr = (void *)addr;
	void *argAddr = (void *)((char *)addr + sizeof(void *));
	void *endAddr = (void *)((char *)addr + sizeof(void *) * 2 + 123);

	if (beginAddr != NULL)
	{
		printf("checkMem  Err  errInfo  %s  %p\n", errInfo, beginAddr);
	}
	return ;
}



