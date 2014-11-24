#include "log_opr.h"
#include <unistd.h>

extern CPthreadMutex g_insMutexCalc;

CLogOprManager *CLogOprManager::_instance = NULL;
CLogOprManager::CLogOprManager()
{
	pthread_create(&m_threadId, NULL,threadFunc,NULL);
}
CLogOprManager *CLogOprManager::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CLogOprManager;
		}
	}
	return _instance;
}

void CLogOprManager::threadProc()
{	
	while(1)
	{

		if(m_recvList.empty())
		{
			usleep(10 * 1000);
			continue;
		}
		m_recvListMutex.Enter();
		struct node *pNode =  m_recvList.begin();
		LOG_DATA *pLogData = logDataContain(pNode);
		m_recvList.pop_front();	
		m_recvListMutex.Leave();
		
		dealRecvData(&pLogData->logDataInf);
		//CalcMem::instance()->destroyMemData(pMemData);		
	}
}

void* CLogOprManager::threadFunc(void *pArg)
{
	CLogOprManager::instance()->threadProc();
	return NULL;
}

void CLogOprManager::dealRecvData(LogDataInf *pLogDataInf)
{
	threadQueueEnable(e_Mem);	

	LogDataInf::LogDataOpr &opr = pLogDataInf->m_opr;
	//int threadId = pLogDataInf->m_threadId;
	 
	
	switch (opr)
	{
		case LogDataInf::e_writeFile:
			{
				//CalcMem::instance()->wrapMalloc(memAddr, memSize, pBackTrace, threadId);
 				break;
			}
		default:
			break;
	}
	return ;

}

void CLogOprManager::pushMemData(LOG_DATA *pMemData)
{
	if (pMemData == NULL)
	{
		return ;
	}
	
	m_recvListMutex.Enter();
	m_recvList.push_back(&pMemData->node);
	m_recvListMutex.Leave();
	return ;
}



