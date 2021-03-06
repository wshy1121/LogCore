#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#include <string>
#include <signal.h>
#include "mem_calc.h"
namespace base
{
#ifdef WIN32
	typedef unsigned long pthread_t;
#else
	typedef ::pthread_t  pthread_t;
#endif
}

#define debug_printf()  printf("WSHY DEBUG  %d  %s\n", __LINE__, __FILE__);

class CCandy
{
public:
	CCandy(int line=__LINE__, char *file_name=(char *)__FILE__, char *func_name=(char *)"__FUNCTION__", int display_level=100);
	~CCandy();
};


class CBugKiller
{
public:
	static void InsertTrace(int line, char *file_name, const char* fmt, ...);
	static void InsertHex(int line, char *file_name, char *psBuf, int nBufLen);
	static void DispAll();
	static void InsertTag(int line, char *file_name, const char* fmt, ...);
	static void InsertStrOnly(TraceInfoId &traceInfoId, const char* fmt, ...);
	static std::string& getBackTrace(std::string &backTrace);	
	static void printfMemInfMap();
	static void printfStackInfo(int line, char *file_name);
	static void getStackInfo(std::string &stackInf);
	static void startServer();
	static void start();
	static void stop();
private:
	CBugKiller();
};

#if !defined(NO_CTIME_CALC)
#define time_trace_level(level)  CCandy candy(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, level)
#define time_printf(format, ...)    CBugKiller::InsertTrace(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_all()    CBugKiller::DispAll()
#define time_tag(format, ...)  CBugKiller::InsertTag(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_str(str, len)    CBugKiller::InsertHex(__LINE__, (char *)__FILE__, str, len)
#define time_mem()   CBugKiller::printfMemInfMap()
#define time_stack()	CBugKiller::printfStackInfo(__LINE__, (char *)__FILE__)
#define time_start()    CBugKiller::start()
#define time_stop()    CBugKiller::stop()
#define time_num(num)	 time_printf("num:%d    %d", num, __LINE__)
#define time_err(num)       time_printf("ERRERRERRERRERRERRERR:%d    %d    %s", (num), __LINE__, __FILE__)
#define time_untrace()  time_trace_level(0)
#define time_trace()   time_trace_level(100)

#else
#define time_trace_level(level)    
#define time_printf(format, ...)      
#define time_all()    
#define time_tag(format, ...)   
#define time_str(str, len)    
#define time_mem()
#define time_stack()	
#define time_start()  
#define time_stop()  
#define time_num(num)	   
#define time_err(num)        
#define time_untrace()    
#define time_trace()   
#endif


#endif



