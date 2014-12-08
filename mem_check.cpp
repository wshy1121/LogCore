#include "mem_check.h"
#include "link_tool.h"
#include "time_calc.h"

#include <string.h>

extern CPthreadMutex g_insMutexCalc;

CMemCheck *CMemCheck::_instance = NULL;
void *CMemCheck::m_checkValue = (void *)&CMemCheck::instance;
const size_t CMemCheck::m_flagSize = sizeof(void *);

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
	void* p = __real_malloc(c + m_flagSize);
	setFlag(p, c);
	return p;
}


void* CMemCheck::realloc(void *p, size_t c)
{
	p = __real_realloc(p, c + m_flagSize);

	setFlag(p, c);	
	return p;
}


void* CMemCheck::calloc(size_t nmemb, size_t size)
{
	const size_t endFlagSize = m_flagSize;
	
	size_t memNum = nmemb + 1;
	if (endFlagSize  > size)
	{
		memNum = nmemb + endFlagSize / size + 1;
	}
	void* p = __real_calloc(memNum, size);

	setFlag(p, memNum * size);
	return p;
}

void CMemCheck::free(void*p)
{
	__real_free(p);
}


void CMemCheck::addMemInfo(void *addr, int addrLen, std::string &backTrace)
{
	addr = (void *)((char *)addr - m_flagSize * 2);

	void **argAddr = (void **)((char *)addr + m_flagSize);
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
	addr = (void *)((char *)addr - m_flagSize * 2);
	void **argAddr = (void **)((char *)addr + m_flagSize);
	MemNodeInf *pNodeInf = (MemNodeInf *)*argAddr;
	return pNodeInf;
}


void CMemCheck::setFlag(void *addr, size_t size)
{
	void **flagAddr = (void **)((char *)addr + size);
	*flagAddr = m_checkValue;	
}

bool CMemCheck::isMemCheck(void *addr)
{
	addr = (void *)((char *)addr - m_flagSize * 2);
	void **beginAddr = (void **)addr;
	return *beginAddr == m_checkValue;
}

