#ifndef _LOG_OPR_H_
#define _LOG_OPR_H_
#include "link_tool.h"

typedef struct LogDataInf
{
	typedef enum
	{
		e_none,
		e_writeFile, 			
	}LogDataOpr;
	LogDataOpr m_opr;
	pthread_t m_threadId;
	
	char *m_backTrace;
 }LogDataInf;

typedef struct LOG_DATA
{
	LogDataInf logDataInf;
	struct node node;
}LOG_DATA;	

#define logDataContain(ptr)  container_of(ptr, LOG_DATA, node)


class  CLogOprManager
{
public:
	static CLogOprManager *instance();
	void pushLogData(char *sFmt, ...);
private:
	CLogOprManager();
private:
	static void* threadFunc(void *pArg);
	void threadProc();
	void dealRecvData(LogDataInf *pCalcMemInf);
	void writeToFile();
private:
	static CLogOprManager *_instance;
	CPthreadMutex m_logFileMutex;
	pthread_t m_threadId;
	const int m_maxFileDataLen;
	const char *m_logName;
	char *m_fileData;
	int m_fileDataLen;
};

#endif


