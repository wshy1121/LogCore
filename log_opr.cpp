#include "log_opr.h"
#include "mem_calc.h"
#include "time_calc.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

extern CPthreadMutex g_insMutexCalc;

CLogOprManager *CLogOprManager::_instance = NULL;
CLogOprManager::CLogOprManager() : m_maxFileDataLen(1024 *1024), m_logName("./Debug.cpp"), m_fileDataLen(0)
{
	m_fileData = (char *)malloc(m_maxFileDataLen + 16);
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
	time_t startTime = time(NULL);
	time_t diff = 0;
	while(1)
	{
		diff =  time(NULL) - startTime;
		if(m_fileDataLen == 0 && diff < 2)
		{
			usleep(10 * 1000);
			continue;
		}
		startTime = time(NULL);
		writeToFile();
		//dealRecvData(&pLogData->logDataInf);
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

void CLogOprManager::pushLogData(const char *logStr)
{
	CGuardMutex guardMutex(m_logFileMutex);
	strcat(m_fileData, logStr);
	m_fileDataLen += strlen(m_fileData + m_fileDataLen);
	return ;
}


void CLogOprManager::writeToFile()
{
	FILE *fp = NULL;
	CGuardMutex guardMutex(m_logFileMutex);
	fp = fopen (m_logName, "a+");
	if (fp == NULL)
	{
		return ;
	}
	fprintf(fp, "%s", m_fileData);
	fclose (fp);
	m_fileDataLen = 0;
	return ;
}

