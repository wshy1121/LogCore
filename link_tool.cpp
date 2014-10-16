#include "Global.h"
#include "link_tool.h"
#include <stdio.h>
#include <string.h>
#include <execinfo.h>
#include <assert.h>

extern "C" void __real_free(void* p);
extern "C" void* __real_malloc(size_t);

/******************************************************/
void init_node(struct node *node)
{
	node->next = node;
	node->pre = node;
	return ;
}

//后插法
void insert_node(struct node *node, struct node *inser_node)
{
	struct node *temp_node = NULL;
	temp_node = node->next;

	node->next = inser_node;
	inser_node->pre = node;

	inser_node->next= temp_node;
	temp_node->pre = inser_node;
}

void remov_node(struct node *node)
{
	struct node *pre = NULL;
	struct node *next = NULL;
	pre = node->pre;
	next = node->next;

	pre->next = next;
	next->pre = pre;
	return ;
}


///\brief 构造函数
CGuardEnable::CGuardEnable(E_ENABLE_TYPE type)
	:m_type(type)
{
	m_queueNode = ThreadQueue::instance()->getQueueNode(pthread_self());
	m_needReturn = !m_queueNode->enable[m_type];
}

CGuardEnable::~CGuardEnable()
{
	if (m_needReturn) 
	{
		return ;
	}
	
	ThreadQueue::instance()->removeNode(m_queueNode, m_type);
}

bool CGuardEnable::needReturn()
{
	if (m_needReturn) 
	{
		return true;
	}

	m_queueNode->enable[m_type] = false;
	return false;
}
	

bool ThreadQueue::m_enable = false;
ThreadQueue *ThreadQueue::_instance = NULL;

ThreadQueue::ThreadQueue()
{
	initQueue();
	return ;
}

ThreadQueue *ThreadQueue::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new ThreadQueue;
		}
	}
	return _instance;
}

void ThreadQueue::start()
{
	m_enable = false;
	CalcMem::instance();	
	ThreadQueue::instance();
	CTimeCalcManager::instance();
	m_enable = true;
}

void ThreadQueue::stop()
{
	m_enable = false;
}

void ThreadQueue::setEnable(bool enable)
{
	m_enable = enable;
}

bool ThreadQueue::getEnable()
{
	return m_enable;
}


void ThreadQueue::initQueue()
{
	CGuardMutex guardMutex(m_mutex);
	init_node(&head_node.node);
	tail = &head_node;
	node_num = 0;
	
	return ;
}
void ThreadQueue::initThreadNode(ThreadNode *queue_node)
{
	for (int i=0; i<e_ThreadEnableNum; ++i)
	{
		queue_node->enable[i] = true;
	}
	queue_node->thread_id = pthread_self();
	return ;
}

int ThreadQueue::insertQueue(ThreadNode *queue_node)
{
	CGuardMutex guardMutex(m_mutex);
	insert_node(head_node.node.pre, &queue_node->node); 
	tail = queue_node;
	++(node_num);
	return 0;
}
int ThreadQueue::removeNode(ThreadNode *queueNode, E_ENABLE_TYPE type)
{
	CGuardMutex guardMutex(m_mutex);
	queueNode->enable[type] = true;
	
	for (int i=0; i<e_ThreadEnableNum; ++i)
	{
		if (queueNode->enable[i] == false)
		{
			return -1;
		}
	}
	if (queueNode->thread_id == pthread_self())
	{
		remov_node(&queueNode->node);
		tail = TQueueContain(head_node.node.pre);
		--(node_num);
		__real_free(queueNode);
		return 0;	
	}

	return -1;
}

int ThreadQueue::removeQueue(pthread_t thread_id)
{
	CGuardMutex guardMutex(m_mutex);
	struct node *node = NULL;
	struct node *head = &head_node.node;
	ThreadNode *queue_node = NULL;

	if (node_num < 1) //无结点
	{
		return -1;
	}
	if (&head_node == tail) //无结点
	{
		return -1;
	}

	each_link_node(head, node)
	{
		queue_node = TQueueContain(node);
		if (queue_node->thread_id == thread_id)
		{
			remov_node(&queue_node->node);
			tail = TQueueContain(head_node.node.pre);
			--(node_num);
			return 0;	
		}
	}

	return -1;
}
int ThreadQueue::getQueue(pthread_t thread_id, ThreadNode **ret_queue_node)
{
	CGuardMutex guardMutex(m_mutex);
	struct node *node = NULL;
	struct node *head = &head_node.node;
	ThreadNode *queue_node = NULL;

	if (node_num < 1) //无结点
	{
		*ret_queue_node = NULL;
		return -1;
	}
	if (&head_node == tail) //无结点
	{
		*ret_queue_node = NULL;
		return -1;
	}

	each_link_node(head, node)
	{
		queue_node = TQueueContain(node);
		if (queue_node->thread_id == thread_id)
		{
			*ret_queue_node = queue_node;
			tail = TQueueContain(head_node.node.pre);
			return 0;	
		}
	}

	*ret_queue_node = NULL;
	return -1;
}


void ThreadQueue::clearQueue()
{
	CGuardMutex guardMutex(m_mutex);
	struct node *node = NULL;
	struct node *head = &head_node.node;
	ThreadNode *queue_node = NULL;

	if (node_num < 1) //无结点
	{
		return ;
	}
	if (&head_node == tail) //无结点
	{
		return ;
	}

	for ((node)=(head)->next; (head) != (node); )
	{
		queue_node = TQueueContain(node);
		(node)=(node)->next;
		//printf("queue_node->thread_id  %ld\n", queue_node->thread_id);

		__real_free(queue_node);

	}


	init_node(&head_node.node);
	tail = &head_node;
	node_num = 0;

	return ;
}
void ThreadQueue::dispQueue()
{
	CGuardMutex guardMutex(m_mutex);
	struct node *node = NULL;
	struct node *head = &head_node.node;
	ThreadNode *queue_node = NULL;

	if (node_num < 1) //无结点
	{
		return ;
	}
	if (&head_node == tail) //无结点
	{
		return ;
	}

	each_link_node(head, node)
	{
		queue_node = TQueueContain(node);
		printf("queue_node->thread_id  %ld\n", queue_node->thread_id);

	}

	return ;

}
ThreadNode *ThreadQueue::getQueueNode(pthread_t thread_id)
{
	ThreadNode *queue_node = NULL;
	getQueue(pthread_self(), &queue_node);
	if (!queue_node)
	{
		queue_node = (ThreadNode *)__real_malloc(sizeof(ThreadNode));
		initThreadNode(queue_node);
		insertQueue(queue_node);
	}
	return queue_node;
}
void ThreadQueue::wrapMalloc(size_t c, void* addr)
{
	threadQueueEnable(e_Mem);
	CalcMem::instance()->wrapMalloc(c, addr);
	return ;
}

void ThreadQueue::wrapFree(void* addr)
{
	threadQueueEnable(e_Mem);
	CalcMem::instance()->wrapFree(addr);
	return ;
}

CalcMem *CalcMem::_instance = NULL;
CalcMem::CalcMem()
{
}
CalcMem *CalcMem::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CalcMem;
		}
	}
	return _instance;
}

void CalcMem::wrapMalloc(size_t c, void* addr)
{
	std::string insertTrace;
	CTimeCalcManager::instance()->getInsertTrace(insertTrace);
	if (!insertTrace.size())
	{
		return ;
	}
	
	CGuardMutex guardMutex(m_mutex);
	MemNodeMap::iterator iter = m_memNodeMap.find(addr);
	if (iter == m_memNodeMap.end())
	{
		MemNodeInf *pNodeInf = new MemNodeInf;
		if (pNodeInf == NULL)
		{
			printf("pNodeInf new failed\n");
		}
		pNodeInf->addr = addr;
		pNodeInf->path = insertTrace;
		pNodeInf->memSize = c;

		m_memNodeMap.insert( std::make_pair(addr, pNodeInf) );
		dealMemInf(pNodeInf->path.c_str(), pNodeInf->memSize);
	}
	else
	{
		//printf("traceInf  %s\n", traceInf);
	}
	
	
}

void CalcMem::wrapFree(void* addr)
{

	CGuardMutex guardMutex(m_mutex);
	MemNodeMap::iterator iter = m_memNodeMap.find(addr);
	if (iter != m_memNodeMap.end())
	{
		MemNodeInf *pNodeInf = iter->second;

		dealMemInf(pNodeInf->path.c_str(), -pNodeInf->memSize);
		delete pNodeInf;
		m_memNodeMap.erase(iter);
	}
	else
	{
		//printf("traceInf  %s\n", traceInf);
	}
	

}
void CalcMem::printfMallocMap()
{
	return ;	
}

void CalcMem::dealMemInf(const char *mallocPath, size_t size)
{
	MemInf *memInf = NULL;
	MemInfMap::iterator memInfMapIter = m_MemInfMap.find(mallocPath);
	if (memInfMapIter != m_MemInfMap.end())
	{
		memInf = memInfMapIter->second;
	}
	else
	{
		memInf = new MemInf;
		assert(memInf != NULL);
		
		memInf->memSize = 0;
		memInf->maxSize = 0;
		
		m_MemInfMap.insert(std::make_pair(mallocPath, memInf));
	}
	
	memInf->memSize += size;
	if (memInf->memSize > memInf->maxSize)
	{
		memInf->maxSize = memInf->memSize;
		CTimeCalcManager::instance()->InsertTrace(0, (char *)"", "mallocPath, size  %s %d", mallocPath, memInf->memSize);
	}
	return ;
}

