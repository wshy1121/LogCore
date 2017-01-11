#include "log_opr.h"
#include "file_manager.h"

static CPthreadMutex g_insMutexCalc;

CLogOprManager *CLogOprManager::_instance = NULL;


CLogOprManager *CLogOprManager::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guarMutex(g_insMutexCalc);
		if (_instance == NULL)
		{
			_instance = new CLogOprManager;
		}
	}
	
	return _instance;
}

CLogOprManager::CLogOprManager()
{
	CBase::pthread_create(&m_threadId, NULL, threadFunc, NULL);
}

void *CLogOprManager::threadFunc(void *pArg)
{
	CLogOprManager::instance()->threadProc();
	return NULL;
}

void CLogOprManager::threadProc()
{
	const int sleepTime = 1 * 1000 * 1000;
	CLogFile *pLogFile = NULL;
	while (1)
	{
		CBase::usleep(sleepTime);
		CGuardMutex guarMutex(m_logFileMutex);
		for (LogFileMap::iterator iter = m_logFileMap.begin(); iter != m_logFileMap.end(); ++iter)
		{
			pLogFile = iter->second;
			toFile(pLogFile, pLogFile->m_content);
		}
	}

}

void CLogOprManager::writeFile(TraceInfoId &traceInfoId, char *content)
{	
	CGuardMutex guarMutex(m_logFileMutex);

	LogFileMap::iterator iter = m_logFileMap.find(traceInfoId.clientId);
	if (iter == m_logFileMap.end())
	{
		printf("writeFile failed! no file opened\n");
		return ;
	}

	CLogFile *pLogFile = iter->second;
	
	pLogFile->m_content.append(content);
}


void CLogOprManager::toFile(CLogFile *logFile, std::string &content)
{
	if (content.size() == 0)
	{
		return ;
	}

	IFileHander fileAddTime = logFile->m_traceFileInf.m_fileAddTime;

	if (fileAddTime == NULL || fileAddTime->open() == false)
	{
		printf("toFile fopen  %s  failed\n", fileAddTime->getPath().c_str());
		content.clear();
		return ;
	}

	fileAddTime->write(content.c_str(), content.size());
	content.clear();
	fileAddTime->close();

	
	TraceFileInf &traceFileInf = logFile->m_traceFileInf;
	traceFileInf.m_fileSize = fileAddTime->size();
	if (traceFileInf.m_fileSize > 67108864)
	{
		traceFileInf.m_fileAddTime = CFileManager::instance()->getFileHander(logFile->m_fileName, logFile->m_clientIpAddr);
	}
	return ;
}


TraceFileInf *CLogOprManager::openFile(int fileKey, char *fileName, std::string clientIpAddr)
{
	CGuardMutex guarMutex(m_logFileMutex);
	LogFileMap::iterator iter = m_logFileMap.find(fileKey);
	if (iter != m_logFileMap.end())
	{
		m_logFileMutex.Leave();
		closeFile(fileKey);
		m_logFileMutex.Enter();
	}
	printf("openFile fileKey, fileNmae  %d  %s\n", fileKey, fileName);

	CLogFile *pLogFile = new CLogFile(fileName, clientIpAddr);
	m_logFileMap.insert(std::make_pair(fileKey, pLogFile));

	
	return &pLogFile->m_traceFileInf;
}

bool CLogOprManager::closeFile(int fileKey)
{
	CGuardMutex guarMutex(m_logFileMutex);
	printf("closeFile fileKey  %d\n", fileKey);

	LogFileMap::iterator iter = m_logFileMap.find(fileKey);
	if (iter == m_logFileMap.end())
	{
		return false;
	}

	CLogFile *pLogFile = iter->second;
	toFile(pLogFile, pLogFile->m_content);

	m_logFileMap.erase(iter);
	delete pLogFile;
		
	return true;
}


CLogFile::CLogFile(char *fileName, std::string &clientIpAddr)
{
	m_fileName = fileName;
	m_clientIpAddr = clientIpAddr;

	TraceFileInf &traceFileInf = m_traceFileInf;

	initTraceFileInf(&traceFileInf, fileName, clientIpAddr);
}

CLogFile::~CLogFile()
{
	
}


void CLogFile::initTraceFileInf(TraceFileInf *traceFileInf, char *fileName, std::string &clientIpAddr)
{
	traceFileInf->m_fileName = fileName;
	traceFileInf->m_fileSize = 0;

	traceFileInf->m_fileSize = CBase::filesize(fileName);

	traceFileInf->m_fileAddTime = CFileManager::instance()->getFileHander(fileName, clientIpAddr);
	
}
