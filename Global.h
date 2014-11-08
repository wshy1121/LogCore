#ifndef _GLOBAL_H_
#define _GLOBAL_H_
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

extern CPthreadMutex g_insMutexCalc;


class CCandy
{
public:
	CCandy(int line=__LINE__, char *file_name=(char *)__FILE__, char *func_name=(char *)__FUNCTION__, int display_level=100);
	~CCandy();
};


class CTimeCalcManager
{
public:
	static CTimeCalcManager *instance();
public:
	void printfMemInfMap();
	void printStack(int line, char *file_name, const char* fmt, ...);
	void getStackInfo(std::string &stackInf);
	void getStackInfo(FuncTraceInfo_t *TraceInfo, std::string &stackInf);
	void InsertTrace(int line, char *file_name, const char* fmt, ...);
	void InsertStrOnly(const char* fmt, ...);
	void InsertStrOnlyInfo(FuncTraceInfo_t *TraceInfo, char *pStr);
	void InsertTag(int line, char *file_name, const char* fmt, ...);
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
	void insertStackInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr);
	void DispTraces(int signo);
	bool needPrint(CTimeCalcList &calc_list);
	void insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr);
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


#if !defined(NO_CTIME_CALC)
#define time_trace_level(level)  CCandy timeCalc(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, level)
#define time_untrace()  time_trace_level(0)
#define time_trace()   time_trace_level(100)

#define time_printf(format, ...)    CTimeCalcManager::instance()->InsertTrace(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_num(num)	 time_printf("num:%d    %d", num, __LINE__)
#define time_err(num)       time_printf("ERRERRERRERRERRERRERR:%d    %d    %s", (num), __LINE__, __FILE__)
#define time_all()     CTimeCalcManager::instance()->DispAll()
#define time_str(str, len)  CTimeCalcManager::instance()->InsertHex(__LINE__, __FILE__, str, len)
#define time_tag(format, ...) CTimeCalcManager::instance()->InsertTag(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_stack()   CTimeCalcManager::instance()->printfMemInfMap()
#define get_stack(str)	CTimeCalcManager::instance()->getStackInfo(str);
#define time_start()    CTimeCalcManager::instance()->start();
#define time_stop()    CTimeCalcManager::instance()->stop();
#else
#define time_trace_level()    
#define time_untrace()    
#define time_trace()    
#define time_printf(format, ...)      
#define time_num(num)	   
#define time_err(num)        
#define time_all()     
#define time_str(str, len)    
#define time_tag(format, ...)   
#define time_stack()
#define get_stack(str)  
#define time_start()  
#define time_stop()  
#endif

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

