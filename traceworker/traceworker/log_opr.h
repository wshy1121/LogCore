#ifndef _LOG_OPR_H
#define _LOG_OPR_H
#include <stdio.h>
#include <string>
#include <map>

#include "IFile.h"
#include "trace_base.h"
#include "link_tool.h"
#include "mem_calc.h"

typedef struct TraceFileInf
{
	std::string m_fileName;
	size_t m_fileSize;
	IFileHander m_fileAddTime;
}TraceFileInf;
class CLogFile
{
public:
	friend class CLogOprManager;
	CLogFile(char *fileName, std::string &clientIpAddr);
	~CLogFile();
private:
	void initTraceFileInf(TraceFileInf *traceFileInf, char *fileName, std::string &clientIpAddr);
private:
	std::string m_fileName;
	std::string m_clientIpAddr;
	std::string m_content;
	TraceFileInf m_traceFileInf;
};

class CLogOprManager
{
public:
	typedef std::map<int, CLogFile *> LogFileMap;
	static CLogOprManager *instance();
	CLogOprManager();
public:
	TraceFileInf *openFile(int fileKey, char *fileName, std::string clientIpAddr);
	bool closeFile(int fileKey);
	void writeFile(TraceInfoId &traceInfoId, char *content);
private:
	static void *threadFunc(void *pArg);
	void threadProc();
	void toFile(CLogFile *logFile, std::string &content);
private:
	static CLogOprManager *_instance;
	CBase::pthread_t m_threadId;

	LogFileMap m_logFileMap;
	CPthreadMutex m_logFileMutex;
};
#endif
