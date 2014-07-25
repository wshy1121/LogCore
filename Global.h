#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#include <time.h>
#include <string>
#include <map>
#include <pthread.h>
#include <signal.h>
#include <sys/timeb.h>
#include <stdlib.h>
int Debug_print(char *sLogName, int nLogMode, char *sFmt, ...);

#define DEBUG_MOD 1
#if DEBUG_MOD == 1
#define debug_print(str)   trace("DEBUG		%d %d:%s		%d		%s %s\n", (int)pthread_self(), (int)time(NULL), (char *)str, __LINE__, __FILE__, __FUNCTION__)
#define num_print(num)    trace("DENUM		%d %d:%d		%d		%s %s\n",  (int)pthread_self(), (int)time(NULL), (int)num, __LINE__, __FILE__, __FUNCTION__)
#define err_print(num)      trace("ERRERRERRERRERRERRERRERRERRERRERRERR		%d %d:%d		%d		%s %s\n",  (int)pthread_self(), (int)time(NULL), (int)num, __LINE__, __FILE__, __FUNCTION__) 
#elif DEBUG_MOD == 2
#define debug_print(str)   printf("%s:%d\n", __FUNCTION__, __LINE__);
#define num_print(num) 
#define err_print(num)  
#elif DEBUG_MOD == 3
#define debug_print(str)   Debug_print((char *)"Debug.txt", 3, (char *)"DENUM		%d %d:%s		%d		%s %s\n",  (int)pthread_self(), (int)time(NULL), str, __LINE__, __FILE__, __FUNCTION__);
#define num_print(num)    Debug_print((char *)"Debug.txt", 3, (char *)"DENUM		%d %d:%d		%d		%s %s\n",  (int)pthread_self(), (int)time(NULL), (int)num, __LINE__, __FILE__, __FUNCTION__)
#define err_print(num)      Debug_print((char *)"Debug.txt", 3, (char *)"ERRERRERRERRERRERRERRERRERRERRERRERR		%d %d:%d		%d		%s %s\n",  (int)pthread_self(), (int)time(NULL), (int)num, __LINE__, __FILE__, __FUNCTION__) 
#else
#define debug_print(str)    
#define num_print(num)  
#define err_print(num)  
#endif

void NextStep(const char *function, const char *fileName, int line);
#define nextStep()  NextStep(__FUNCTION__, __FILE__, __LINE__)


typedef struct FuncTraceInfo_t
{
	struct timeb EndTime;
	int deep;
	std::string up_string;
} FuncTraceInfo_t;

class CTimeCalc
{
private:
	void calcStartMem();
	void calcEndMem();
	static void InitMutex();
	void DealFuncEnter();
	void DealFuncExit();
	FuncTraceInfo_t *GreatTraceInf();
	void insertEnterInfo(FuncTraceInfo_t *TraceInfo);
	void insertExitInfo(FuncTraceInfo_t *TraceInfo);
	static void insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr);
	static FuncTraceInfo_t *GetTraceInf();

public:
	static void InsertTag(int line, char *file_name, const char* fmt, ...);
	static void InsertTrace(int line, char *file_name, const char* fmt, ...);
	static void DispTraces(int signo);
	static void DispAll();
	static void BackTrace();
	static void InsertHex(int line, char *file_name, char *psBuf, int nBufLen);
	CTimeCalc(int line=__LINE__, char *file_name=(char *)__FILE__, char *func_name=(char *)__FUNCTION__, int display_level=100);
	~CTimeCalc();
private:
	int m_Line;
	std::string m_FileName;
	std::string m_FuncName;
	int m_DisplayLevel;

	struct timeb m_StartTime;
	static pthread_mutex_t  *m_thread_map_mutex;
	static std::map<pthread_t, FuncTraceInfo_t *> m_thread_map; 
	
	//用于记录内存情况
	int m_startMem[64];
	int m_endMem[64];
};



#if !defined(NO_CTIME_CALC)
#define time_untrace()  CTimeCalc timeCalc(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, 0)
#define time_trace()  CTimeCalc timeCalc(__LINE__, (char *)__FILE__, (char *)__FUNCTION__)
#define time_printf(format, ...)    CTimeCalc::InsertTrace(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_num(num)	 time_printf("num:%d    %d", num, __LINE__)
#define time_err(num)       time_printf("ERRERRERRERRERRERRERR:%d    %d    %s", (num), __LINE__, __FILE__)
#define time_all()     CTimeCalc::DispAll()
#define time_str(str, len)  CTimeCalc::InsertHex(__LINE__, __FILE__, str, len)
#define time_tag(format, ...) CTimeCalc::InsertTag(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_stack()   CTimeCalc::BackTrace()
#else
#define time_untrace()    
#define time_trace()    
#define time_printf(format, ...)      
#define time_num(num)	   
#define time_err(num)        
#define time_all()     
#define time_str(str, len)    
#define time_tag(format, ...)   
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

