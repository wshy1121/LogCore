#ifndef __LINK_TOOL_H
#define __LINK_TOOL_H
#include "Global.h"
#include <map>
#define container_of(ptr, type, member) ({  \
const typeof( ((type *)0)->member ) *__mptr = (ptr);   \
(type *)( (char *)__mptr - offsetof(type,member) );})

#define TQueueContain(x) container_of((x), ThreadNode, node)
#define each_link_node(head, node) for ((node)=(head)->next; (head) != (node); (node)=(node)->next)

struct node
{
    struct node *next;
	struct node *pre;
};

void init_node(struct node *node);
void insert_node(struct node *node, struct node *inser_node);
void remov_node(struct node *node);

typedef struct 
{
	pthread_t thread_id;
	bool enable;
	struct node node;
}ThreadNode;

class  ThreadQueue
{
public:
	ThreadQueue();
public:
	void initThreadNode(ThreadNode *queue_node, bool enable, pthread_t thread_id);
	void initQueue();
	int insertQueue(ThreadNode *queue_node);
	int removeQueue(pthread_t thread_id);
	int getQueue(pthread_t thread_id, ThreadNode **ret_queue_node);
	void clearQueue();
	void dispQueue();
	void setEnable(bool enable);
	bool getEnable();
	void start();
	void wrapMalloc(size_t c, void* addr);
	void wrapFree(void* addr);
	static ThreadQueue *instance();
private:
	static bool m_enable;
private:
	ThreadNode head_node;
	ThreadNode *tail;
	int node_num;
	pthread_mutex_t  m_mutex;
};

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
	pthread_mutex_t  m_mutex;
	///mallocSizeMap["local"][addr] = memSize;
	typedef std::map<void *, size_t>  MemNode;
	std::map<std::string, MemNode> m_mallocSizeMap;

	typedef std::map<std::string, MemNode>::iterator MemNodeIter;
	std::map<void *, MemNodeIter> m_memNodeMap;
};



#endif


