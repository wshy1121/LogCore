#ifndef _TIME_CALC_H_
#define _TIME_CALC_H_
#include <time.h>
#include <string>
#include <map>
#include <list>
#include <pthread.h>
#include <signal.h>
#include <sys/timeb.h>
#include <stdlib.h>


void NextStep(const char *function, const char *fileName, int line);
#define nextStep()  NextStep(__FUNCTION__, __FILE__, __LINE__)
#define tracepoint1()  printf("%d  %s  \t\t%ld\n", __LINE__, __FILE__, pthread_self());
class CTimeCalc;

typedef std::list<CTimeCalc *> CTimeCalcList;
typedef struct FuncTraceInfo_t
{
	struct timeb EndTime;
	int deep;
	std::string up_string;
	CTimeCalcList calc_list;
} FuncTraceInfo_t;

class CTimeCalcManager;
class CPthreadMutex
{
public:
	///\brief 构造函数，默认为互斥锁
	CPthreadMutex()
	{
		m_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(m_mutex, NULL);
	}

	///\brief 析构函数
	~CPthreadMutex()
	{
		free(m_mutex);
	}

	///\brief 占用锁
	bool Enter()
	{
		pthread_mutex_lock(m_mutex);
		return true;
	}

	///\brief 释放锁
	bool Leave()
	{
		pthread_mutex_unlock(m_mutex);
		return true;
	}

private:
	pthread_mutex_t  *m_mutex;
};



class CGuardMutex
{
public:
	///\brief 构造函数
	inline CGuardMutex(CPthreadMutex& mutex)
		:m_mutex(mutex)
	{
		m_mutex.Enter();
	};

	///\brief 析构函数
	inline ~CGuardMutex()
	{
		m_mutex.Leave();
	};
private:
	CPthreadMutex &m_mutex;
};



class CTimeCalc
{
	friend class CTimeCalcManager;
private:
	void DealFuncEnter();
	void DealFuncExit();
	void insertEnterInfo(FuncTraceInfo_t *TraceInfo);
	void insertExitInfo(FuncTraceInfo_t *TraceInfo);
public:
	CTimeCalc(int line=__LINE__, char *file_name=(char *)__FILE__, char *func_name=(char *)__FUNCTION__, int display_level=100);
	~CTimeCalc();
private:
	void initTimeCalc(CTimeCalcList &calc_list);
	void exitTimeCalc(CTimeCalcList &calc_list);
	CTimeCalc *getLastTimeCalc(CTimeCalcList &calc_list);
	void setDisplayFlag(CTimeCalc *timeCalc);
private:
	bool m_displayFlag;
	int m_DisplayLevel;
	 //使当前TimeCale不能显示的等级
	int m_noDisplayLevel;  

	int m_Line;
	std::string m_FileName;
	std::string m_FuncName;

	struct timeb m_StartTime;
};


class CTimeCalcManager
{
public:
	static CTimeCalcManager *instance();
public:
	void printfMemInfMap();
	void printStack(int line, char *file_name, const char* fmt, ...);
	void getStackInfo(std::string &stackInf);
	void InsertTrace(int line, char *file_name, const char* content);
	void InsertStrOnly(const char* fmt, ...);
	void InsertTag(int line, char *file_name, const char* content);
	void DispAll();
	void InsertHex(int line, char *file_name, char *psBuf, int nBufLen);
	void start();
	void stop();
public:
	FuncTraceInfo_t *CreatTraceInf();
	void DestroyTraceInf(FuncTraceInfo_t *TraceInfo);
	FuncTraceInfo_t *GetTraceInf();
	void printLog(char *sFmt, ...);
private:	
	void getStackInfo(FuncTraceInfo_t *TraceInfo, std::string &stackInf);
	void InsertStrOnlyInfo(FuncTraceInfo_t *TraceInfo, char *pStr);
	void insertStackInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr);
	void DispTraces(int signo);
	bool needPrint(CTimeCalcList &calc_list);
	void insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, const char *pStr);
	FILE *openLog(const char *sLogName);
private:
	CTimeCalcManager();

private:
	CPthreadMutex  m_thread_map_mutex;
	std::map<pthread_t, FuncTraceInfo_t *> m_thread_map; 
	std::map<std::string, int > m_stack_inf_map;
	FILE *m_fp;
	const char *m_logName;
	CPthreadMutex  m_logFileMutex;
	static CTimeCalcManager *_instance;
};

class CTimeCalcInf
{
public:
	CTimeCalcInf();
public:
	typedef enum
	{
		e_node,
		e_createCandy, 
		e_destroyCandy, 			
		e_insertTrace, 
		e_dispAll, 
		e_insertTag, 
		e_getBackTrace, 
		e_printfMemInfMap, 
		e_printfStackInfo, 
		e_getStackInfo, 
	}TimeCalcOpr;
	TimeCalcOpr m_opr;
	int m_threadId;
	int m_line;
	char * m_fileName;
	char * m_funcName;
	int m_displayLevel;
};

class CTimeCalcInfManager
{
public:
	static CTimeCalcInfManager *instance();
	void pushRecvData(CTimeCalcInf *pRecvData);	
private:
	CTimeCalcInfManager();
	static void* threadFunc(void *pArg);
	void threadProc();
	void dealRecvData(CTimeCalcInf *pRecvData);
private:
	static CTimeCalcInfManager *_instance;
	std::list<CTimeCalcInf *> m_recvList;
	CPthreadMutex m_recvListMutex;
	pthread_t m_threadId;
	bool m_isLocked;
};



#define VOUT16
#ifdef VOUT16
#define cutLine  // 
#define debug_return  return ;
#else
#define cutLine  
#define debug_return  
#endif


#endif


#ifdef hysw
-rdynamic

#include <signal.h>
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

static void wait_child_exit(int sig)
{
	printf("SIGSEGV  SIGSEGV\n");
	time_all();
	exit(sig);
}
int main(int argc, char **argv)
{
    signal(SIGSEGV, &wait_child_exit);   //捕捉SIGSEGV信号
    //signal(SIGINT, &ctrl_c_func);  //捕捉SIGINT信号
}

#endif

