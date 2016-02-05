#ifndef __MEM_CALC_H
#define __MEM_CALC_H
#include <map>
#include <stddef.h>
#include "link_tool.h"
#include "defs.h"
#include "trace_base.h"

#define TQueueContain(x) container_of((x), ThreadNode, node)
#define TRACE_INF_LEN  512

class CClientInf;
typedef struct TraceInfoId
{
	CBase::pthread_t threadId;
	int clientId;
	int socket;
	CClientInf *clientInf;
	bool operator < (const struct TraceInfoId &key) const
	{
		if (key.clientId < clientId)
		{
			return true;
		}
		else if (key.clientId == clientId)
		{
			return key.socket < socket;
		}
		return false;
	}

}TraceInfoId;

typedef struct MemInf
{
	size_t memSize;
	size_t maxSize;
	int mallocCount;
	int freeCount;
}MemInf;



typedef struct CalcMemInf
{
	typedef enum
	{
		e_none,
		e_wrapMalloc, 
		e_wrapFree, 			
	}CalcMemOpr;
	CalcMemOpr m_opr;
	TraceInfoId m_traceInfoId;
 	void *m_memAddr;
	size_t m_memSize;
	char *m_backTrace;
 }CalcMemInf;

typedef struct MEM_DATA
{
	CalcMemInf calcMemInf;
	struct node node;
}MEM_DATA;	
#define memDataContain(ptr)  container_of(ptr, MEM_DATA, node)

class  CalcMem
{
public:
	static CalcMem *instance();
	MEM_DATA *createMemData(int backTraceLen = 0);
	void destroyMemData(MEM_DATA *pMemData);
	void wrapMalloc(void* addr, size_t c, char *pBackTrace, TraceInfoId &traceInfoId);
	void wrapFree(void* addr, size_t c, char *pBackTrace, TraceInfoId &traceInfoId);
	void printfMemInfMap(TraceInfoId &traceInfoId);
	std::string &getBackTrace(std::string &backTrace);
private:
	CalcMem();
private:
	void dealMemInf(const char *mallocPath, int size, TraceInfoId &traceInfoId);
	inline std::string splitFilename (std::string &path);
private:
	static CalcMem *_instance;
	static const int m_stackNum;	
	base::CPthreadMutex  m_mutex;

	typedef std::map<std::string, MemInf *> MemInfMap;
	MemInfMap m_MemInfMap;
	const char *m_traceHead;
};




class  CalcMemManager
{
public:
	static CalcMemManager *instance();
	void pushMemData(MEM_DATA *pCalcInf);
private:
	CalcMemManager();
private:
	static void* threadFunc(void *pArg);
	void threadProc();
	void dealRecvData(CalcMemInf *pCalcMemInf);
private:
	static CalcMemManager *_instance;
	base::CList *m_recvList;
	base::CPthreadMutex m_recvListMutex;
	CBase::pthread_t m_threadId;
};





#endif


