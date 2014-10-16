#ifndef __LINK_TOOL_H
#define __LINK_TOOL_H
#include "Global.h"
#include <map>
#define container_of(ptr, type, member) ({  \
const typeof( ((type *)0)->member ) *__mptr = (ptr);   \
(type *)( (char *)__mptr - offsetof(type,member) );})

#define TQueueContain(x) container_of((x), ThreadNode, node)
#define each_link_node(head, node) for ((node)=(head)->next; (head) != (node); (node)=(node)->next)
#define TRACE_INF_LEN  512

struct node
{
    struct node *next;
	struct node *pre;
};

void init_node(struct node *node);
void insert_node(struct node *node, struct node *inser_node);
void remov_node(struct node *node);

typedef enum
{
	e_Mem, 
	e_TimeCalc, 
	e_ThreadEnableNum,
}E_ENABLE_TYPE;

typedef struct 
{
	pthread_t thread_id;
	bool enable[e_ThreadEnableNum];
	struct node node;
}ThreadNode;

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

class  ThreadQueue
{
public:
	ThreadQueue();
public:
	void initThreadNode(ThreadNode *queue_node);
	void initQueue();
	int insertQueue(ThreadNode *queue_node);
	int removeQueue(pthread_t thread_id);
	int getQueue(pthread_t thread_id, ThreadNode **ret_queue_node);
	void clearQueue();
	void dispQueue();
	void setEnable(bool enable);
	static bool getEnable();
	static void start();
	static void stop();
	void wrapMalloc(size_t c, void* addr);
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
}MemInf;

class  CalcMem
{
public:
	static CalcMem *instance();
	void wrapMalloc(size_t c, void* addr);
	void wrapFree(void* addr);
	void printfMallocMap();
private:
	CalcMem();
private:
	void dealMemInf(const char *mallocPath, size_t size);
private:
	static CalcMem *_instance;
	CPthreadMutex  m_mutex;

	typedef std::map<void *, MemNodeInf *> MemNodeMap;
	MemNodeMap m_memNodeMap;

	typedef std::map<std::string, MemInf *> MemInfMap;
	MemInfMap m_MemInfMap;
};



#endif


