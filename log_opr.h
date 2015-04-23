#ifndef _LOG_OPR_H_
#define _LOG_OPR_H_
#include "link_tool.h"
#include "mem_calc.h"

typedef struct TraceFileInf
{
	std::string m_fileName;
	int m_count;
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
	char *fileName;
	base::CString *content;
	
}LOG_FILE;
class  CLogOprManager
{
public:
	typedef std::map<std::string, TraceFileInf *> TraceFileInfMap;
	typedef std::map<int, LOG_FILE*> LogFileMap;	
	static CLogOprManager *instance();
	void dealLogData(LogDataInf *pLogData);	
	TraceFileInfMap &getTraceFileInfs();
private:
	CLogOprManager();
private:
	static void* threadFunc(void *pArg);
	void threadProc();	
	bool openFile(int fileKey, char *fileName);
	bool closeFile(int fileKey);
	void writeFile(int fileKey,char *content);
	void toFile(char *fileName, base::CString *pString);
	LOG_FILE *createLogFile(char *fileName);
	void destroyLogFile(LOG_FILE *pLogFile);
	bool isAvailable();
	void addFile(char *fileName);
	void removeFile(char *fileName);	
private:
	static CLogOprManager *_instance;
	base::CPthreadMutex m_logFileMutex;
	base::pthread_t m_threadId;
	const char *m_logName;
	LogFileMap m_logFileMap;
	static TraceFileInfMap m_traceFileInfMap;	
};

#endif


