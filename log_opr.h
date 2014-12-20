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
	base::pthread_t m_threadId;
	
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
	void pushLogData(const char *logStr);
private:
	CLogOprManager();
private:
	static void* threadFunc(void *pArg);
	void threadProc();
	void dealRecvData(LogDataInf *pCalcMemInf);
	void toFile();
private:
	static CLogOprManager *_instance;
	CPthreadMutex m_logFileMutex;
	base::pthread_t m_threadId;
	const char *m_logName;
	CString *pString;
};

#endif


