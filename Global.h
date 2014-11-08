#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#include <string>


class CCandy
{
public:
	CCandy(int line=__LINE__, char *file_name=(char *)__FILE__, char *func_name=(char *)__FUNCTION__, int display_level=100);
	~CCandy();
};


class CBugKiller
{
public:
	static void InsertTrace(int line, char *file_name, const char* fmt, ...);
	static void DispAll();
	static void InsertTag(int line, char *file_name, const char* fmt, ...);
	static void printfMemInfMap();
	static void getStackInfo(std::string &stackInf);
	static void start();
	static void stop();
private:
	CBugKiller();
};

#if !defined(NO_CTIME_CALC)
#define time_trace_level(level)  CCandy candy(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, level)
#define time_untrace()  time_trace_level(0)
#define time_trace()   time_trace_level(100)

#define time_printf(format, ...)    CBugKiller::InsertTrace(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define time_num(num)	 time_printf("num:%d    %d", num, __LINE__)
#define time_err(num)       time_printf("ERRERRERRERRERRERRERR:%d    %d    %s", (num), __LINE__, __FILE__)

#define time_all()    CBugKiller::DispAll();

#define time_tag(format, ...)  CBugKiller::InsertTag(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__);


#define time_stack()   CBugKiller::printfMemInfMap();
#define get_stack(str)	CBugKiller::getStackInfo(str);
#define time_start()    CBugKiller::start()
#define time_stop()    CBugKiller::stop()

#else
#define time_trace_level()    
#define time_untrace()    
#define time_trace()   

#define time_printf(format, ...)      
#define time_num(num)	   
#define time_err(num)        

#define time_all()     

#define time_tag(format, ...)   


#define time_str(str, len)    
#define time_stack()
#define get_stack(str)  
#define time_start()  
#define time_stop()  
#endif


#endif



