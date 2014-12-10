#ifndef __LINK_TOOL_H
#define __LINK_TOOL_H
#include <map>
#include <stddef.h>
#include <string>
#include <stdlib.h>
#include <pthread.h>

#define container_of(ptr, type, member) ({  \
const typeof( ((type *)0)->member ) *__mptr = (ptr);   \
(type *)( (char *)__mptr - offsetof(type,member) );})

#define TQueueContain(x) container_of((x), ThreadNode, node)
#define each_link_node(head, node) for ((node)=(head)->next; (head) != (node); (node)=(node)->next)
#define TRACE_INF_LEN  512

typedef struct node
{
    struct node *next;
	struct node *pre;
}node;

void init_node(struct node *node);
void insert_node(struct node *node, struct node *inser_node);
void remov_node(struct node *node);

struct  CList
{
public:
	static CList *createCList();
	static void destroyClist(CList *pCList);
public:
	int push_back(node *pNode);
	node *begin();
	void pop_front();
	bool empty();
	void clear();
	int size();
private:
	void init();
	void exit();
private:
	node head_node;
	node *tail;
	int node_num;
};

class CPthreadMutex
{
public:
	///\brief 构造函数，默认为互斥锁
	CPthreadMutex()
	{
		m_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(m_mutex, NULL);
	}

	///\brief 析构函数
	~CPthreadMutex()
	{
		free(m_mutex);
	}

	///\brief 占用锁
	bool Enter()
	{
		pthread_mutex_lock(m_mutex);
		return true;
	}

	///\brief 释放锁
	bool Leave()
	{
		pthread_mutex_unlock(m_mutex);
		return true;
	}

private:
	pthread_mutex_t  *m_mutex;
};

class CGuardMutex
{
public:
	///\brief 构造函数
	inline CGuardMutex(CPthreadMutex& mutex)
		:m_mutex(mutex)
	{
		m_mutex.Enter();
	};

	///\brief 析构函数
	inline ~CGuardMutex()
	{
		m_mutex.Leave();
	};
private:
	CPthreadMutex &m_mutex;
};


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





#endif


