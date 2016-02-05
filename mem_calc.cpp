#include "stdafx.h"
#include "string_base.h"
#include "mem_calc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mem_check.h"
#include "thread_queue.h"


using namespace base;
extern CPthreadMutex g_insMutexCalc;
extern "C" int backtrace(void **buffer, int size);
/******************************************************/


CalcMem *CalcMem::_instance = NULL;
const int CalcMem::m_stackNum = 32;
CalcMem::CalcMem()
{
	m_traceHead = "addr2line -e ./Challenge_Debug -f -C  ";
}
CalcMem *CalcMem::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CalcMem;
		}
	}
	return _instance;
}


MEM_DATA *CalcMem::createMemData(int backTraceLen)
{
	MEM_DATA *pMemData = (MEM_DATA *)base::malloc(sizeof(MEM_DATA));
	CalcMemInf *pCalcMemInf = &pMemData->calcMemInf;

	pCalcMemInf->m_opr = CalcMemInf::e_wrapMalloc;
	pCalcMemInf->m_traceInfoId.threadId = -1;
	pCalcMemInf->m_traceInfoId.clientId = -1;
	pCalcMemInf->m_memAddr = NULL;
	pCalcMemInf->m_memSize= 0;

	pCalcMemInf->m_backTrace = NULL;
	if (backTraceLen > 0)
	{
		pCalcMemInf->m_backTrace = (char *)base::malloc(backTraceLen+1);
		((char *)pCalcMemInf->m_backTrace)[0] = '\0';
	}
	
	return pMemData;
	
}
void CalcMem::destroyMemData(MEM_DATA *pMemData)
{
	base::free(pMemData->calcMemInf.m_backTrace);
	base::free(pMemData);
}


void CalcMem::wrapMalloc(void* addr, size_t c, char *pBackTrace, TraceInfoId &traceInfoId)
{
	if (strlen(pBackTrace) <= 0)
	{
		return ;
	}
	CGuardMutex guardMutex(m_mutex);
	dealMemInf(pBackTrace, (int)c, traceInfoId);	
}

void CalcMem::wrapFree(void* addr, size_t c, char *pBackTrace, TraceInfoId &traceInfoId)
{
	CGuardMutex guardMutex(m_mutex);
	dealMemInf(pBackTrace, -(int)c, traceInfoId);
}
void CalcMem::printfMemInfMap(TraceInfoId &traceInfoId)
{
	CCandy candy(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, 0);

	std::string path;
	CGuardMutex guardMutex(m_mutex);
	for (MemInfMap::iterator iter = m_MemInfMap.begin(); iter != m_MemInfMap.end(); ++iter)
	{
		path = iter->first;
		MemInf *memInf = iter->second;

		size_t diffCount = memInf->mallocCount - memInf->freeCount;
		size_t itemSize = 0;
		if (diffCount > 0)
		{
			itemSize = memInf->memSize /diffCount;
		}
		//CBugKiller::InsertStrOnly(traceInfoId, "maxSize  itemSize  memSize  diffCount  mallocCount  freeCount  %016d  %08d  %d  %d  %d  %d %s", memInf->maxSize, itemSize, memInf->memSize, diffCount, memInf->mallocCount, memInf->freeCount, path.c_str());
	}
	
	return ;	
}

void CalcMem::dealMemInf(const char *mallocPath, int size, TraceInfoId &traceInfoId)
{ 
	MemInf *memInf = NULL;
	MemInfMap::iterator memInfMapIter = m_MemInfMap.find(mallocPath);
	if (memInfMapIter != m_MemInfMap.end())
	{
		memInf = memInfMapIter->second;
	}
	else
	{
		memInf = new MemInf;
		assert(memInf != NULL);
		
		memInf->memSize = 0;
		memInf->maxSize = 0;
		memInf->mallocCount = 0;
		memInf->freeCount = 0;
		
		m_MemInfMap.insert(std::make_pair(mallocPath, memInf));
	}
	
	memInf->memSize += size;
	if (size > 0)
	{
		memInf->mallocCount++;
	}
	else if (size < 0)
	{
		memInf->freeCount++;
	}
	if (memInf->memSize > memInf->maxSize)
	{
		memInf->maxSize = memInf->memSize;

		//int count = memInf->mallocCount - memInf->freeCount;
		//CBugKiller::InsertStrOnly(traceInfoId, "%s  malloc size  %06d  %d  %d  %s", mallocPath, count, memInf->memSize, __LINE__, __FILE__);
	}
	return ;
}

std::string CalcMem::splitFilename (std::string &path)
{
	size_t found;
	found=path.find_last_of("/\\");
	return path.substr(found+1);
}

std::string &CalcMem::getBackTrace(std::string &backTrace)
{
	void *stack_addr[m_stackNum];
	int layer;
	int i;
	char tmp[256];
	backTrace = m_traceHead;

	m_mutex.Enter();
	layer = CBase::backtrace(stack_addr, m_stackNum);
	m_mutex.Leave();
	for(i = 3; i < layer; i++)
	{
		base::snprintf(tmp, sizeof(tmp), "%p  ", stack_addr[i]);
		backTrace += tmp;
	}
	return backTrace;
}


CalcMemManager *CalcMemManager::_instance = NULL;
CalcMemManager::CalcMemManager()
{
	m_recvList = CList::createCList();
	CBase::pthread_create(&m_threadId, NULL,threadFunc,NULL);
}
CalcMemManager *CalcMemManager::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CalcMemManager;
		}
	}
	return _instance;
}

void CalcMemManager::threadProc()
{	
	while(1)
	{

		if(m_recvList->empty())
		{
			base::usleep(1000);
			continue;
		}
		m_recvListMutex.Enter();
		struct node *pNode =  m_recvList->begin();
		MEM_DATA *pMemData = memDataContain(pNode);
		m_recvList->pop_front();	
		m_recvListMutex.Leave();
		
		dealRecvData(&pMemData->calcMemInf);
		CalcMem::instance()->destroyMemData(pMemData);		
	}
}

void* CalcMemManager::threadFunc(void *pArg)
{
	CalcMemManager::instance()->threadProc();
	return NULL;
}

void CalcMemManager::dealRecvData(CalcMemInf *pCalcMemInf)
{
	threadQueueEnable(e_Mem);	

	CalcMemInf::CalcMemOpr &opr = pCalcMemInf->m_opr;
	TraceInfoId &traceInfoId = pCalcMemInf->m_traceInfoId;
	 
	void *memAddr = pCalcMemInf->m_memAddr;
	size_t memSize = pCalcMemInf->m_memSize;
	char *pBackTrace = pCalcMemInf->m_backTrace;
	
	switch (opr)
	{
		case CalcMemInf::e_wrapMalloc:
			{
				CalcMem::instance()->wrapMalloc(memAddr, memSize, pBackTrace, traceInfoId);
 				break;
			}
		case CalcMemInf::e_wrapFree:
			{
				CalcMem::instance()->wrapFree(memAddr, memSize, pBackTrace, traceInfoId);
				break;
			}
		default:
			break;
	}
	return ;

}

void CalcMemManager::pushMemData(MEM_DATA *pMemData)
{
	if (pMemData == NULL)
	{
		return ;
	}
	
	m_recvListMutex.Enter();
	m_recvList->push_back(&pMemData->node);
	m_recvListMutex.Leave();
	return ;
}



