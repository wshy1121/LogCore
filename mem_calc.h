#ifndef __MEM_CALC_H
#define __MEM_CALC_H
#include <map>
#include <stddef.h>
#include "link_tool.h"
#define container_of(ptr, type, member) ({  \
const typeof( ((type *)0)->member ) *__mptr = (ptr);   \
(type *)( (char *)__mptr - offsetof(type,member) );})

#define TQueueContain(x) container_of((x), ThreadNode, node)
#define each_link_node(head, node) for ((node)=(head)->next; (head) != (node); (node)=(node)->next)
#define TRACE_INF_LEN  512


class  ThreadQueue
{
public:
	ThreadQueue();
public:
	void initThreadNode(ThreadNode *queue_node);
	void initQueue();
	int insertQueue(ThreadNode *queue_node);
	int removeQueue(pthread_t thread_id);
	int removeNode(ThreadNode *queueNode, E_ENABLE_TYPE m_type);
	int getQueue(pthread_t thread_id, ThreadNode **ret_queue_node);
	void clearQueue();
	void dispQueue();
	void setEnable(bool enable);
	static bool getEnable();
	static void start();
	static void stop();
	void wrapMalloc(void* addr, size_t c);
	void wrapFree(void* addr);
	ThreadNode *getQueueNode(pthread_t thread_id);
	static ThreadQueue *instance();
private:
	static bool m_enable;
	static ThreadQueue *_instance;
private:
	ThreadNode head_node;
	ThreadNode *tail;
	int node_num;
	CPthreadMutex  m_mutex;
};
typedef struct MemNodeInf
{
	void *addr;
	std::string path;
	size_t memSize;
	//char memTest[16*1024];
}MemNodeInf;

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
	pthread_t m_threadId;
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
	void wrapMalloc(void* addr, size_t c, char *pBackTrace, pthread_t threadId);
	void wrapFree(void* addr, pthread_t threadId);
	void printfMemInfMap(pthread_t threadId);
private:
	CalcMem();
private:
	void dealMemInf(const char *mallocPath, int size, pthread_t threadId);
	inline std::string splitFilename (std::string &path);
private:
	static CalcMem *_instance;
	CPthreadMutex  m_mutex;

	typedef std::map<void *, MemNodeInf *> MemNodeMap;
	MemNodeMap m_memNodeMap;

	typedef std::map<std::string, MemInf *> MemInfMap;
	MemInfMap m_MemInfMap;
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
	CList m_recvList;
	CPthreadMutex m_recvListMutex;
	pthread_t m_threadId;
};

class CGuardEnable
{
public:
	///\brief 构造函数
	CGuardEnable(E_ENABLE_TYPE type);

	///\brief 析构函数
	~CGuardEnable();

	bool needReturn();
private:
	E_ENABLE_TYPE m_type;
	ThreadNode *m_queueNode;
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


