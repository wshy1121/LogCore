#include "mem_check.h"
#include "link_tool.h"

typedef struct MemNodeInf
{
	char *path;
	size_t memSize;
}MemNodeInf;

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
	initMem(p, c);
	return (void *)((char *)p + sizeof(void *) * 2);
}


void* CMemCheck::realloc(void *p, size_t c)
{
	p = (void *)((char *)p - sizeof(void *) * 2);
	p = __real_realloc(p, c + sizeof(void *) * 3);
	initMem(p, c);
	return (void *)((char *)p + sizeof(void *) * 2);
}


void* CMemCheck::calloc(size_t c)
{
	void* p = __real_calloc(c + sizeof(void *) * 3);
	initMem(p, c);
	return (void *)((char *)p + sizeof(void *) * 2);
}

void CMemCheck::free(void*p)
{
	if (p == NULL)
	{
		return ;
	}
	p = (void *)((char *)p - sizeof(void *) * 2);
	exitMem(p, "123");
	__real_free(p);
}

void CMemCheck::initMem(void *addr, int addrLen)
{
	MemNodeInf *pNodeInf = (MemNodeInf *)__real_malloc(sizeof(MemNodeInf));
	pNodeInf->path = NULL;
	pNodeInf->memSize = addrLen;
	
	void **beginAddr = (void **)addr;
	void **argAddr = (void **)((char *)addr + sizeof(void *));
	void **endAddr = (void **)((char *)addr + sizeof(void *) * 2 + addrLen);

	*beginAddr = m_checkValue;
	*argAddr = pNodeInf;
	*endAddr = m_checkValue;
}

void CMemCheck::exitMem(void *addr, const char *errInfo)
{
	void **argAddr = (void **)((char *)addr + sizeof(void *));
	MemNodeInf *pNodeInf = (MemNodeInf *)*argAddr;

	void **beginAddr = (void **)addr;
	void **endAddr = (void **)((char *)addr + sizeof(void *) * 2 + pNodeInf->memSize);

	
	__real_free(pNodeInf);	
	if (*beginAddr != m_checkValue || *endAddr != m_checkValue)
	{
		printf("checkMem  Err  errInfo  %s  %p\n", errInfo, beginAddr);
	}
	return ;
}

void CMemCheck::addMemInfo(void *addr, int addrLen, std::string &backTrace)
{

}

