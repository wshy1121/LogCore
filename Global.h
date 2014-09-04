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

class CTimeCalc;

typedef std::list<CTimeCalc *> CTimeCalcList;
typedef struct FuncTraceInfo_t
{
	struct timeb EndTime;
	int deep;
	std::string up_string;
	CTimeCalcList calc_list;
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
	static void insertStackInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr);
	static void getStackInfo(FuncTraceInfo_t *TraceInfo, std::string &stackInf);
	static FuncTraceInfo_t *GetTraceInf();

public:
	static bool isInitFinished();
	static void InsertTag(int line, char *file_name, const char* fmt, ...);
	static void InsertTrace(int line, char *file_name, const char* fmt, ...);
	static void DispTraces(int signo);
	static void DispAll();
	static void BackTrace();
	static void InsertHex(int line, char *file_name, char *psBuf, int nBufLen);
	static void printStack(int line, char *file_name, const char* fmt, ...);
	static bool getStackInfo(std::string &stackInf);
	CTimeCalc(int line=__LINE__, char *file_name=(char *)__FILE__, char *func_name=(char *)__FUNCTION__, int display_level=100);
	~CTimeCalc();
private:
	static bool needPrint(CTimeCalcList &calc_list);
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
	static pthread_mutex_t  *m_thread_map_mutex;
	static std::map<pthread_t, FuncTraceInfo_t *> m_thread_map; 
	static std::map<std::string, int > m_stack_inf_map;
	//用于记录内存情况
	int m_startMem[64];
	int m_endMem[64];
};



#if !defined(NO_CTIME_CALC)
#define time_trace_level(level)  CTimeCalc timeCalc(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, level)
#define time_untrace()  time_trace_level(0)
#define time_trace()   time_trace_level(100)

#define time_printf(format, ...)    CTimeCalc::InsertTrace(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_num(num)	 time_printf("num:%d    %d", num, __LINE__)
#define time_err(num)       time_printf("ERRERRERRERRERRERRERR:%d    %d    %s", (num), __LINE__, __FILE__)
#define time_all()     CTimeCalc::DispAll()
#define time_str(str, len)  CTimeCalc::InsertHex(__LINE__, __FILE__, str, len)
#define time_tag(format, ...) CTimeCalc::InsertTag(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_stack(format, ...)   CTimeCalc::printStack(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define get_stack(str)	CTimeCalc::getStackInfo(str);
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
#define time_stack(format, ...)  
#define get_stack(str)  
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

