#include "mem_check.h"
#include "link_tool.h"
#include "time_calc.h"

#include <string.h>

extern CPthreadMutex g_insMutexCalc;

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
	if (p != NULL)
	{
		p = (void *)((char *)p - sizeof(void *) * 2);
	}
	p = __real_realloc(p, c + sizeof(void *) * 3);
	initMem(p, c);
	return (void *)((char *)p + sizeof(void *) * 2);
}


void* CMemCheck::calloc(size_t nmemb, size_t size)
{
	size_t memNum = nmemb + 1;
	if (sizeof(void *) * 3 > size)
	{
		memNum = nmemb + sizeof(void *) * 3 / size + 1;
	}
	
	void* p = __real_calloc(memNum, size);
	initMem(p, nmemb * size);
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

	if (*beginAddr != m_checkValue || *endAddr != m_checkValue)
	{
		printf("checkMem  Err  errInfo  %s  (%p || %p)  != %p\n", errInfo, *beginAddr, *endAddr, m_checkValue);
		if (pNodeInf->path)
		{
			printf("mem path  %s\n", pNodeInf->path);
		}
	}

	if (pNodeInf->path)
	{
		__real_free(pNodeInf->path);	
	}
	__real_free(pNodeInf);		
	return ;
}

void CMemCheck::addMemInfo(void *addr, int addrLen, std::string &backTrace)
{
	addr = (void *)((char *)addr - sizeof(void *) * 2);

	void **argAddr = (void **)((char *)addr + sizeof(void *));
	MemNodeInf *pNodeInf = (MemNodeInf *)*argAddr;

	if (addrLen != (int)pNodeInf->memSize)
	{
		printf("addMemInfo  failed  addrLen != memSize  %d  %d\n", addrLen, (int)pNodeInf->memSize);
	}
	pNodeInf->path = (char *)__real_malloc(backTrace.size() + 1);
	strcpy(pNodeInf->path, backTrace.c_str());
}

MemNodeInf *CMemCheck::getMemNodeInf(void *addr)
{	
	addr = (void *)((char *)addr - sizeof(void *) * 2);
	void **argAddr = (void **)((char *)addr + sizeof(void *));
	MemNodeInf *pNodeInf = (MemNodeInf *)*argAddr;
	return pNodeInf;
}


bool CMemCheck::isMemCheck(void *addr)
{
	addr = (void *)((char *)addr - sizeof(void *) * 2);
	void **beginAddr = (void **)addr;
	return *beginAddr == m_checkValue;
}

