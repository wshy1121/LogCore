#ifndef _TIME_CALC_H_
#define _TIME_CALC_H_
#include <time.h>
#include <string>
#include <map>
#include <list>
#include <signal.h>
#include <stdlib.h>
#include "link_tool.h"
#include "mem_calc.h"

void NextStep(const char *function, const char *fileName, int line);
#define nextStep()  NextStep(__FUNCTION__, __FILE__, __LINE__)
#define tracepoint1()  printf("%d  %s  \t\t%ld\n", __LINE__, __FILE__, base::pthread_self());


typedef struct FuncTraceInfo_t
{
	TimeB EndTime;
	int deep;
	CString *pUpString;
	CList *pCalcList;
	TraceInfoId traceInfoId;
	node Node;
} FuncTraceInfo_t;
#define TNodeContain(x) container_of((x), FuncTraceInfo_t, Node)

class CTimeCalcManager;



typedef struct CTimeCalc
{
	friend class CTimeCalcManager;
private:
	void DealFuncEnter();
	void DealFuncExit();
	void insertEnterInfo(FuncTraceInfo_t *TraceInfo);
	void insertExitInfo(FuncTraceInfo_t *TraceInfo);
public:
	static CTimeCalc * createCTimeCalc(int line, char *file_name, char *func_name, int display_level, TraceInfoId &traceInfoId);
	static void destroyCTimeCalc(CTimeCalc *pTimeCalc);
	void init(int line, char *file_name, char *func_name, int display_level, TraceInfoId &traceInfoId);
	void exit();
private:
	void initTimeCalc(CList *pCalcList);
	void exitTimeCalc(CList *pCalcList);
	CTimeCalc *getLastTimeCalc(CList *pCalcList);
	void setDisplayFlag(CTimeCalc *timeCalc);
public:
	bool m_displayFlag;
	int m_DisplayLevel;
	TraceInfoId m_traceInfoId;
	 //使当前TimeCale不能显示的等级
	int m_noDisplayLevel;  

	int m_Line;
	char *m_FileName;
	char *m_FuncName;
	TimeB m_StartTime;
	struct node m_node;
	
}CTimeCalc;
#define CTimeCalcContain(x) container_of((x), CTimeCalc, m_node)


class CTimeCalcManager
{
public:
	static CTimeCalcManager *instance();
public:
	void printfMemInfMap(TraceInfoId &traceInfoId);
	void printStack(int line, char *file_name, const char* fmt, ...);
	void getStackInfo(std::string &stackInf);
	void InsertTrace(int line, char *file_name, TraceInfoId &traceInfoId, const char* content);
	void InsertStrOnly(TraceInfoId &traceInfoId, const char* fmt, ...);
	void InsertTag(TraceInfoId &traceInfoId, int line, char *file_name, const char* content);
	void DispAll(int clientId, const char* content);
	void cleanAll(int clientId);
	void InsertHex(TraceInfoId &traceInfoId, int line, char *file_name, char *psBuf, int nBufLen);
	bool openFile(int fileKey, char *fileName);
	bool closeFile(int fileKey);
	void start();
	void stop();
public:
	FuncTraceInfo_t *CreatTraceInf(TraceInfoId &traceInfoId);
	void DestroyTraceInf(FuncTraceInfo_t *TraceInfo, TraceInfoId &traceInfoId);
	FuncTraceInfo_t *GetTraceInf(TraceInfoId &traceInfoId);
	void printLog(TraceInfoId &traceInfoId, char *sFmt, ...);
	void printStrLog(TraceInfoId &traceInfoId, const char *logStr);
	bool openFile(TraceInfoId &traceInfoId, char *fileName);
	bool closeFile(TraceInfoId &traceInfoId);
private:	
	void getStackInfo(FuncTraceInfo_t *TraceInfo, std::string &stackInf);
	void InsertStrOnlyInfo(FuncTraceInfo_t *TraceInfo, char *pStr);
	void insertStackInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr);
	bool needPrint(CList *pCalcList);
	void insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, TraceInfoId &traceInfoId, const char *pStr);
	FILE *openLog(const char *sLogName);
private:
	CTimeCalcManager();
	~CTimeCalcManager();
private:
	CPthreadMutex  m_threadListMutex;
	CList *m_pThreadList;
	std::map<std::string, int > m_stack_inf_map;
	FILE *m_fp;
	const char *m_logName;
	CPthreadMutex  m_logFileMutex;
	static CTimeCalcManager *_instance;
};


typedef struct TimeCalcInf
{
	typedef enum
	{
		e_none,
		e_createCandy, 
		e_destroyCandy, 			
		e_insertTrace, 
		e_InsertStrOnly, 
		e_dispAll, 
		e_cleanAll, 
		e_insertTag, 
		e_insertHex, 
		e_getBackTrace, 
		e_printfMemInfMap, 
		e_openFile, 
		e_closeFile, 
		e_printfStackInfo, 
		e_getStackInfo, 
	}TimeCalcOpr;
	TimeCalcOpr m_opr;
	TraceInfoId m_traceInfoId;
	int m_line;
	char * m_fileName;
	char * m_funcName;
	int m_displayLevel;
	char *m_pContent;
	int m_contentLen;
}TimeCalcInf;

typedef struct RECV_DATA
{
	TimeCalcInf calcInf;
	struct node node;
}RECV_DATA;	
#define recvDataContain(ptr)  container_of(ptr, RECV_DATA, node)

class CTimeCalcInfManager
{
public:
	static CTimeCalcInfManager *instance();
	RECV_DATA *createRecvData(int contentLen = 0);
	void destroyRecvData(RECV_DATA *pRecvData);
	void *calcMalloc(int size);
	void calcFree(void *pMem);
	void pushRecvData(RECV_DATA *pRecvData);	
	void dealRecvData(TimeCalcInf *pCalcInf);
private:
	CTimeCalcInfManager();
	static void* threadFunc(void *pArg);
	void threadProc();
private:
	static CTimeCalcInfManager *_instance;
	const int  m_maxListSize;
	CList *m_recvList;
	CPthreadMutex m_recvListMutex;
	base::pthread_t m_threadId;
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

