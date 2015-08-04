#ifndef __MEM_CALC_H
#define __MEM_CALC_H
#include <map>
#include <stddef.h>
#include "link_tool.h"
#include "defs.h"

#define TQueueContain(x) container_of((x), ThreadNode, node)
#define TRACE_INF_LEN  512

class CClientInf;
typedef struct TraceInfoId
{
	base::pthread_t threadId;
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


class  ThreadQueue
{
public:
	ThreadQueue();
public:
	void initThreadNode(base::ThreadNode *queue_node);
	void initQueue();
	int insertQueue(base::ThreadNode *queue_node);
	int removeQueue(base::pthread_t thread_id);
	int removeNode(base::ThreadNode *queueNode, base::E_ENABLE_TYPE m_type);
	int getQueue(base::pthread_t thread_id, base::ThreadNode **ret_queue_node);
	void clearQueue();
	void dispQueue();
	void setEnable(bool enable);
	static bool getEnable();
	static void start();
	static void stop();
	void wrapMalloc(void* addr, size_t c);
	void wrapFree(void* addr);
	base::ThreadNode *getQueueNode(base::pthread_t thread_id);
	static ThreadQueue *instance();
private:
	static bool m_enable;
	static ThreadQueue *_instance;
private:
	base::ThreadNode head_node;
	base::ThreadNode *tail;
	int node_num;
	base::CPthreadMutex  m_mutex;
};

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
	base::pthread_t m_threadId;
};

class CGuardEnable
{
public:
	///\brief 构造函数
	CGuardEnable(base::E_ENABLE_TYPE type);

	///\brief 析构函数
	~CGuardEnable();

	bool needReturn();
private:
	base::E_ENABLE_TYPE m_type;
	base::ThreadNode *m_queueNode;
	bool m_needReturn;
};

#ifdef WRAP
//防止嵌套调用处理
#define threadQueueEnable(type)  \
	CGuardEnable guard(type);  \
	if (guard.needReturn())  \
	{  \
		return ;  \
	}

#else
#define threadQueueEnable(type)    
#endif


#endif


