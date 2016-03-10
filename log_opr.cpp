#include "stdafx.h"
#include "string_base.h"
#include "log_opr.h"
#include "mem_calc.h"
#include "time_calc.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <time.h>

using namespace base;
extern CPthreadMutex g_insMutexCalc;

CLogOprManager::TraceFileInfMap CLogOprManager::m_traceFileInfMap;
CLogOprManager *CLogOprManager::_instance = NULL;
CLogOprManager::CLogOprManager() : m_logName("./Debug.cpp")
{
	CBase::pthread_create(&m_threadId, NULL,threadFunc,NULL);
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
			toFile(pLogFile, pLogFile->content);
		}
	}
}

void* CLogOprManager::threadFunc(void *pArg)
{
	CLogOprManager::instance()->threadProc();
	return NULL;
}

TraceFileInf *CLogOprManager::openFile(int fileKey, char *fileName)
{
	CGuardMutex guardMutex(m_logFileMutex);
	LogFileMap::iterator iter = m_logFileMap.find(fileKey);
	if (iter != m_logFileMap.end())
	{
		m_logFileMutex.Leave();
		closeFile(fileKey);
		m_logFileMutex.Enter();
	}
	printf("openFile  fileKey, fileName  %d  %s\n", fileKey, fileName);
	LOG_FILE *pLogFile = createLogFile(fileName);
	m_logFileMap.insert(std::make_pair(fileKey, pLogFile));
	
	pLogFile->traceFileInf = addFile(fileName);
	return pLogFile->traceFileInf;
}

bool CLogOprManager::closeFile(int fileKey)
{
	CGuardMutex guardMutex(m_logFileMutex);
	printf("closeFile  fileKey  %d\n", fileKey);
	LogFileMap::iterator iter = m_logFileMap.find(fileKey);
	if (iter == m_logFileMap.end())
	{
		return false;
	}
	LOG_FILE *pLogFile = iter->second;
	toFile(pLogFile, pLogFile->content);

	removeFile((char*)pLogFile->fileName.c_str());
	m_logFileMap.erase(iter);
	
	destroyLogFile(pLogFile);
	return true;
}

bool CLogOprManager::cleanFile(char *fileName)
{	trace_worker();
	FILE *fp = NULL;
	trace_printf("fileName  %s", fileName);
	fp = base::fopen(fileName, "w");
	if (fp == NULL)
	{	trace_printf("NULL");
		return true;
	}
	fclose(fp);
	trace_printf("NULL");	
	return true;
}


void CLogOprManager::writeFile(TraceInfoId &traceInfoId, char *content)
{
	if (!isAvailable())
	{
		return;
	}

	CGuardMutex guardMutex(m_logFileMutex);
	LogFileMap::iterator iter = m_logFileMap.find(traceInfoId.clientId);
	if (iter == m_logFileMap.end())
	{
		printf("writeFile failed! no file opened\n");
		return ;
	}
	LOG_FILE *pLogFile = iter->second;
	(pLogFile->content)->append(content);
	TraceFileInf *&traceFileInf = traceInfoId.clientInf->m_traceFileInf;
	traceFileInf->m_fileSize += strlen(content);
}
void CLogOprManager::toFile(LOG_FILE *logFile, CString *pString)
{
	const char *fileName = logFile->fileNameAddTime.c_str();
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

	if (!logFile->traceFileInf)
	{
		return ;
	}
	TraceFileInf *&traceFileInf = logFile->traceFileInf;
	struct stat statbuf; 
	if (stat(fileName,&statbuf) == 0)
	{
		traceFileInf->m_fileSize = statbuf.st_size;
	}
    if (traceFileInf->m_fileSize > 67108864) //large than 64M
    {
        logFile->fileNameAddTime = logFile->fileName;
        std::string &fileNameAddTime = logFile->fileNameAddTime;
        
        std::string::size_type nameIndex = fileNameAddTime.find_last_of('.');
        fileNameAddTime = fileNameAddTime.insert(nameIndex, nowTime());  
    }

	return ;
}

LOG_FILE *CLogOprManager::createLogFile(char *fileName)
{
	LOG_FILE *pLogFile = new LOG_FILE;
	pLogFile->content = CString::createCString();

    pLogFile->fileName = fileName;   
    pLogFile->fileNameAddTime = fileName;   

    std::string &fileNameAddTime = pLogFile->fileNameAddTime;
    
    std::string::size_type nameIndex = fileNameAddTime.find_last_of('.');
    fileNameAddTime = fileNameAddTime.insert(nameIndex, nowTime());  

	return pLogFile;
}

void CLogOprManager::destroyLogFile(LOG_FILE *pLogFile)
{
	CString::destroyCString(pLogFile->content);
	delete pLogFile;
}

bool CLogOprManager::isAvailable()
{	trace_worker();
	bool bRet = CUserManager::instance()->isVerified();
	trace_printf("bRet  %d", bRet);
	return bRet;
}

void CLogOprManager::initTraceFileInf(TraceFileInf *traceFileInf, char *fileName)
{	trace_worker();
	traceFileInf->m_fileName = fileName;
	traceFileInf->m_count = 0;
	traceFileInf->m_fileSize = 0;
	traceFileInf->m_candyCount = 0;
	traceFileInf->m_traceCount = 0;
	
	struct stat statbuf; 
	if (stat(fileName,&statbuf) == 0)
	{
		traceFileInf->m_fileSize = statbuf.st_size;
	}
	trace_printf("traceFileInf->m_fileSize	%d", traceFileInf->m_fileSize); 
	return ;	
}

TraceFileInf *CLogOprManager::addFile(char *fileName)
{	trace_worker();
	trace_printf("fileName  %s", fileName);
	TraceFileInf *traceFileInf = NULL;
	TraceFileInfMap::iterator iter = m_traceFileInfMap.find(fileName);
	
	if (iter == m_traceFileInfMap.end())
	{	trace_printf("NULL");
		traceFileInf = new TraceFileInf;
		initTraceFileInf(traceFileInf, fileName);
		m_traceFileInfMap.insert(std::make_pair(fileName, traceFileInf));
	}
	else
	{	trace_printf("NULL");
		traceFileInf = iter->second;
	}
	traceFileInf->m_count++;
	trace_printf("traceFileInf->m_count  %d", traceFileInf->m_count);
	return traceFileInf;
}

void CLogOprManager::removeFile(char *fileName)
{	trace_worker();
	trace_printf("fileName  %s", fileName);
	TraceFileInfMap::iterator iter = m_traceFileInfMap.find(fileName);
	if (iter == m_traceFileInfMap.end())
	{	trace_printf("NULL");
		return ;
	}
	TraceFileInf *traceFileInf = iter->second;
	traceFileInf->m_count--;
	trace_printf("traceFileInf->m_count  %d", traceFileInf->m_count);		
	if (traceFileInf->m_count == 0)
	{	trace_printf("NULL");
		m_traceFileInfMap.erase(iter);
		delete traceFileInf;
	}
}

CLogOprManager::TraceFileInfMap &CLogOprManager::getTraceFileList()
{
	return m_traceFileInfMap;
}

std::string CLogOprManager::nowTime()
{
    char nowTime[64];
    time_t now;
    struct tm  *w;
    time(&now);  
    w=localtime(&now);
    CBase::snprintf(nowTime, sizeof(nowTime), "%04d%02d%02d-%02d%02d%02d",w->tm_year+1900,w->tm_mon+1,w->tm_mday,w->tm_hour,w->tm_min,w->tm_sec);
    return nowTime;
}

