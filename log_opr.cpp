#include "stdafx.h"
#include "string_base.h"
#include "log_opr.h"
#include "mem_calc.h"
#include "time_calc.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace base;
extern CPthreadMutex g_insMutexCalc;

CLogOprManager *CLogOprManager::_instance = NULL;
CLogOprManager::CLogOprManager() : m_logName("./Debug.cpp")
{
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
	const int sleepTime = 3* 1000 * 1000;
	LOG_FILE *pLogFile = NULL;
	while(1)
	{
		base::usleep(sleepTime);
		CGuardMutex guardMutex(m_logFileMutex);
		for (LogFileMap::iterator iter = m_logFileMap.begin(); iter != m_logFileMap.end(); ++iter)
		{
			pLogFile = iter->second;
			toFile(pLogFile->fileName, pLogFile->content);
		}
	}
}

void* CLogOprManager::threadFunc(void *pArg)
{
	CLogOprManager::instance()->threadProc();
	return NULL;
}

void CLogOprManager::dealLogData(LogDataInf *pLogData)
{
	CGuardMutex guardMutex(m_logFileMutex);
	LogDataInf::LogDataOpr &opr = pLogData->m_opr;
	int fileKey = pLogData->m_traceInfoId.clientId;
	char *content = pLogData->m_content;
	switch (opr)
	{
		case LogDataInf::e_writeFile:
			{
				writeFile(fileKey, content);
				break;
			}
		case LogDataInf::e_openFile:
			{
				openFile(fileKey, content);
				break;
			}
		case LogDataInf::e_closeFile:
			{
				closeFile(fileKey);
				break;
			}
		default:
			break;
	}
	return ;

}
bool CLogOprManager::openFile(int fileKey, char *fileName)
{
	LogFileMap::iterator iter = m_logFileMap.find(fileKey);
	if (iter != m_logFileMap.end())
	{
		closeFile(fileKey);
	}
	printf("openFile  fileKey, fileName  %d  %s\n", fileKey, fileName);
	LOG_FILE *pLogFile = createLogFile(fileName);
	m_logFileMap.insert(std::make_pair(fileKey, pLogFile));
	return true;
}

bool CLogOprManager::closeFile(int fileKey)
{
	printf("closeFile  fileKey  %d\n", fileKey);
	LogFileMap::iterator iter = m_logFileMap.find(fileKey);
	if (iter == m_logFileMap.end())
	{
		return false;
	}
	LOG_FILE *pLogFile = iter->second;
	toFile(pLogFile->fileName, pLogFile->content);
	m_logFileMap.erase(iter);
	
	destroyLogFile(pLogFile);
	return true;
}

void CLogOprManager::writeFile(int fileKey,char *content)
{
	if (!isAvailable())
	{
		return;
	}

	LogFileMap::iterator iter = m_logFileMap.find(fileKey);
	if (iter == m_logFileMap.end())
	{
		printf("writeFile failed! no file opened\n");
		return ;
	}
	LOG_FILE *pLogFile = m_logFileMap[fileKey];
	(pLogFile->content)->append(content);
}
void CLogOprManager::toFile(char *fileName, CString *pString)
{
	if (pString->size() == 0)
	{
		return ;
	}
	FILE *fp = NULL;
	fp = base::fopen (fileName, "a+");
	if (fp == NULL)
	{
		return ;
	}
	fwrite(pString->c_str(), pString->size(), 1, fp);
	pString->clear();
	fclose (fp);
	return ;
}

LOG_FILE *CLogOprManager::createLogFile(char *fileName)
{
	LOG_FILE *pLogFile = (LOG_FILE *)base::malloc(sizeof(LOG_FILE));
	pLogFile->fileName = (char *)base::malloc(strlen(fileName) + 1);
	pLogFile->content = CString::createCString();
	
	base::strcpy(pLogFile->fileName, fileName);
	return pLogFile;
}

void CLogOprManager::destroyLogFile(LOG_FILE *pLogFile)
{
	base::free(pLogFile->fileName);
	CString::destroyCString(pLogFile->content);
	base::free(pLogFile);
}

bool CLogOprManager::isAvailable()
{	trace_worker();
	bool bRet = CUserManager::instance()->isVerified();
	trace_printf("bRet  %d", bRet);
	return bRet;
}

