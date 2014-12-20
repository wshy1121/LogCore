#include "stdafx.h"
#include "string_base.h"
#include "log_opr.h"
#include "mem_calc.h"
#include "time_calc.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

extern CPthreadMutex g_insMutexCalc;

CLogOprManager *CLogOprManager::_instance = NULL;
CLogOprManager::CLogOprManager() : m_logName("./Debug.cpp")
{
	pString = CString::createCString();	
	base::pthread_create(&m_threadId, NULL,threadFunc,NULL);
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
	time_t startTime = time(NULL);
	time_t diff = 0;
	while(1)
	{
		if(pString->size() == 0)
		{
			base::usleep(10 * 1000);
			continue;
		}
		
		diff =  time(NULL) - startTime;
		if (diff < 3)
		{
			base::usleep(10 * 1000);
			continue;			
		}
		startTime = time(NULL);
		CGuardMutex guardMutex(m_logFileMutex);	
		toFile();
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
	
	switch (opr)
	{
		case LogDataInf::e_writeFile:
			{
 				break;
			}
		default:
			break;
	}
	return ;

}

void CLogOprManager::pushLogData(const char *logStr)
{
	CGuardMutex guardMutex(m_logFileMutex);
	pString->append(logStr);
	return ;
}

void CLogOprManager::toFile()
{
	if (pString->size() == 0)
	{
		return ;
	}
	FILE *fp = NULL;
	fp = fopen (m_logName, "a+");
	if (fp == NULL)
	{
		return ;
	}
	fwrite(pString->c_str(), pString->size(), 1, fp);
	pString->clear();
	fclose (fp);
	return ;
}

