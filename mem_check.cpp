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

	setFlag(p, nmemb * size);
	return p;
}

void CMemCheck::free(void*p)
{
	__real_free(p);
}


void CMemCheck::addMemInfo(void *addr, int addrLen, std::string &backTrace)
{
	MemNodeInf nodeInf;
	nodeInf.path = backTrace;
	nodeInf.memSize = addrLen;

	CGuardMutex guardMutex(m_memNodeInfMapMutex);
	m_memNodeInfMap.insert((std::make_pair(addr, nodeInf)));
}

bool CMemCheck::getMemNodeInf(void *addr, MemNodeInf &nodeInf)
{
	nodeInf.memSize = 0;
	CGuardMutex guardMutex(m_memNodeInfMapMutex);

	MemNodeInfMap::iterator iter = m_memNodeInfMap.find(addr);
	if (iter == m_memNodeInfMap.end())
	{
		return false;
	}
	nodeInf = iter->second;
	m_memNodeInfMap.erase(iter);
	return true;	
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

