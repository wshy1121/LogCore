#include "stdafx.h"
#include "log_opr.h"
#include "mem_calc.h"
#include "time_calc.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

extern CPthreadMutex g_insMutexCalc;

CLogOprManager *CLogOprManager::_instance = NULL;
CLogOprManager::CLogOprManager() : m_maxFileDataLen(1024 *1024), m_logName("./Debug.cpp"), m_fileDataLen(0)
{
	m_fileData = (char *)base::malloc(m_maxFileDataLen + 16);
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
		if(m_fileDataLen == 0)
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
	int logStrLen = strlen(logStr);
	if ((logStrLen + m_fileDataLen) >= m_maxFileDataLen)
	{
		writeToFile();
	}
	snprintf(m_fileData + m_fileDataLen, m_maxFileDataLen - m_fileDataLen, "%s", logStr);
	m_fileDataLen += logStrLen;
	return ;
}


void CLogOprManager::writeToFile()
{
	FILE *fp = NULL;
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

