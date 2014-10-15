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

bool ThreadQueue::m_enable = false;
ThreadQueue *ThreadQueue::_instance = NULL;

ThreadQueue::ThreadQueue()
{
	pthread_mutex_init(&m_mutex, NULL);
	initQueue();
	return ;
}

ThreadQueue *ThreadQueue::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutex);
		if (NULL == _instance)
		{
			m_enable = false;
			_instance = new ThreadQueue;
			m_enable = true;
		}
	}
	return _instance;
}

void ThreadQueue::start()
{
	m_enable = true;
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
	pthread_mutex_lock(&m_mutex);
	init_node(&head_node.node);
	tail = &head_node;
	node_num = 0;
	pthread_mutex_unlock(&m_mutex);
	
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
	pthread_mutex_lock(&m_mutex);
	insert_node(head_node.node.pre, &queue_node->node); 
	tail = queue_node;
	++(node_num);
	pthread_mutex_unlock(&m_mutex);
	return 0;
}


int ThreadQueue::removeQueue(pthread_t thread_id)
{
	pthread_mutex_lock(&m_mutex);
	struct node *node = NULL;
	struct node *head = &head_node.node;
	ThreadNode *queue_node = NULL;

	if (node_num < 1) //无结点
	{
		pthread_mutex_unlock(&m_mutex);
		return -1;
	}
	if (&head_node == tail) //无结点
	{
		pthread_mutex_unlock(&m_mutex);
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
			pthread_mutex_unlock(&m_mutex);
			return 0;	
		}
	}

	pthread_mutex_unlock(&m_mutex);
	return -1;
}
int ThreadQueue::getQueue(pthread_t thread_id, ThreadNode **ret_queue_node)
{
	pthread_mutex_lock(&m_mutex);
	struct node *node = NULL;
	struct node *head = &head_node.node;
	ThreadNode *queue_node = NULL;

	if (node_num < 1) //无结点
	{
		*ret_queue_node = NULL;
		pthread_mutex_unlock(&m_mutex);
		return -1;
	}
	if (&head_node == tail) //无结点
	{
		*ret_queue_node = NULL;
		pthread_mutex_unlock(&m_mutex);
		return -1;
	}

	each_link_node(head, node)
	{
		queue_node = TQueueContain(node);
		if (queue_node->thread_id == thread_id)
		{
			*ret_queue_node = queue_node;
			tail = TQueueContain(head_node.node.pre);
			pthread_mutex_unlock(&m_mutex);
			return 0;	
		}
	}

	*ret_queue_node = NULL;
	pthread_mutex_unlock(&m_mutex);
	return -1;
}


void ThreadQueue::clearQueue()
{
	pthread_mutex_lock(&m_mutex);
	struct node *node = NULL;
	struct node *head = &head_node.node;
	ThreadNode *queue_node = NULL;

	if (node_num < 1) //无结点
	{
		pthread_mutex_unlock(&m_mutex);
		return ;
	}
	if (&head_node == tail) //无结点
	{
		pthread_mutex_unlock(&m_mutex);
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

	pthread_mutex_unlock(&m_mutex);
	return ;
}
void ThreadQueue::dispQueue()
{
	pthread_mutex_lock(&m_mutex);
	struct node *node = NULL;
	struct node *head = &head_node.node;
	ThreadNode *queue_node = NULL;

	if (node_num < 1) //无结点
	{
		pthread_mutex_unlock(&m_mutex);
		return ;
	}
	if (&head_node == tail) //无结点
	{
		pthread_mutex_unlock(&m_mutex);
		return ;
	}

	each_link_node(head, node)
	{
		queue_node = TQueueContain(node);
		printf("queue_node->thread_id  %ld\n", queue_node->thread_id);

	}

	pthread_mutex_unlock(&m_mutex);
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
	ThreadNode *queue_node = getQueueNode(pthread_self());

	if (queue_node->enable[e_Mem])
	{
		queue_node->enable[e_Mem] = false;
		CalcMem::instance()->wrapMalloc(c, addr);
		queue_node->enable[e_Mem] = true;	

	}

	return ;
}

void ThreadQueue::wrapFree(void* addr)
{
	ThreadNode *queue_node = getQueueNode(pthread_self());
	if (queue_node->enable[e_Mem])
	{
		queue_node->enable[e_Mem] = false;
		CalcMem::instance()->wrapFree(addr);
		queue_node->enable[e_Mem] = true;	

	}

	return ;
}

CalcMem *CalcMem::_instance = NULL;
CalcMem::CalcMem()
{
	pthread_mutex_init(&m_mutex, NULL);
}
CalcMem *CalcMem::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutex);
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
	
	pthread_mutex_lock(&m_mutex);
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
	
	pthread_mutex_unlock(&m_mutex);
	
}

void CalcMem::wrapFree(void* addr)
{

	pthread_mutex_lock(&m_mutex);
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
	
	pthread_mutex_unlock(&m_mutex);

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

