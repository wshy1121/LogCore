#ifndef _LOG_OPR_H_
#define _LOG_OPR_H_
#include "link_tool.h"
#include "mem_calc.h"

typedef struct TraceFileInf
{
	std::string m_fileName;
	size_t m_fileSize;
	int m_count;
	int m_candyCount;
	int m_traceCount;
	std::string m_lastCandy;
    std::string fileNameAddTime;
}TraceFileInf;

typedef struct LogDataInf
{
	typedef enum
	{
		e_none,
		e_writeFile,
		e_openFile,
		e_closeFile
	}LogDataOpr;
	LogDataOpr m_opr;
	TraceInfoId m_traceInfoId;
	char *m_content;
}LogDataInf;

typedef struct LOG_DATA
{
	LogDataInf logDataInf;
	struct node node;
}LOG_DATA;	

#define logDataContain(ptr)  container_of(ptr, LOG_DATA, node)

typedef struct LOG_FILE
{
	std::string fileName;
    std::string clientIpAddr;
	base::CString *content;
	TraceFileInf *traceFileInf;
	
}LOG_FILE;
class  CLogOprManager
{
public:
	typedef std::map<std::string, TraceFileInf *> TraceFileInfMap;
	typedef std::map<int, LOG_FILE*> LogFileMap;	
	static CLogOprManager *instance();
	TraceFileInf *openFile(int fileKey, char *fileName, std::string &clientIpAddr);
	bool closeFile(int fileKey);
	bool cleanFile(char *fileName);
	void writeFile(TraceInfoId &traceInfoId,char *content);	
	TraceFileInfMap &getTraceFileList();
private:
	CLogOprManager();
private:
	static void* threadFunc(void *pArg);
	void threadProc();	
	void toFile(LOG_FILE *logFile, base::CString *pString);
	LOG_FILE *createLogFile(char *fileName, std::string &clientIpAddr);
	void destroyLogFile(LOG_FILE *pLogFile);
	bool isAvailable();
	void initTraceFileInf(TraceFileInf *traceFileInf, char *fileName);
	TraceFileInf *addFile(char *fileName, std::string &clientIpAddr);
	void removeFile(char *fileName, std::string &clientIpAddr);	
    std::string nowTime();
    std::string &addAddrTime(std::string &fileName, std::string &clientIpAddr);
private:
	static CLogOprManager *_instance;
	base::CPthreadMutex m_logFileMutex;
	CBase::pthread_t m_threadId;
	const char *m_logName;
	LogFileMap m_logFileMap;
	static TraceFileInfMap m_traceFileInfMap;	
};

#endif


