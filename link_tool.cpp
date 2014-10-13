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

ThreadQueue::ThreadQueue()
{
	pthread_mutex_init(&m_mutex, NULL);
	initQueue();
	return ;
}

ThreadQueue *ThreadQueue::instance()
{
	static ThreadQueue _instance;
	return &_instance; 
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


CalcMem::CalcMem()
{
	pthread_mutex_init(&m_mutex, NULL);
}
CalcMem *CalcMem::instance()
{
	static CalcMem _instance;
	return &_instance; 
}


void CalcMem::wrapMalloc(size_t c, void* addr)
{
	std::string tmp;
	CTimeCalcManager::instance()->getStackInfo(tmp);
	if (tmp.size())
	{
		printf("tmp.c_str() %s\n", tmp.c_str());
	}
	
	const int stackSize = 15;
	void *stack_addr[stackSize];
	int layer = 0;
	int i;
	char traceInf[TRACE_INF_LEN] = "addr2line -e ./Challenge_Debug -f -C  ";
	int infPos = strlen(traceInf);;
	
	pthread_mutex_lock(&m_mutex);	
	/* 通过调用libc函数实现 */
	layer = backtrace(stack_addr, stackSize);

	for(i = 2; i < layer; i++)
	{
		snprintf(traceInf+infPos, sizeof(traceInf)-infPos, "%p  ", stack_addr[i]);
		infPos = strlen(traceInf);
	}

	MemNodeMap::iterator iter = m_memNodeMap.find(addr);
	if (iter == m_memNodeMap.end())
	{
		MemNodeInf *pNodeInf = new MemNodeInf;
		if (pNodeInf == NULL)
		{
			printf("pNodeInf new failed\n");
		}
		pNodeInf->addr = addr;
		pNodeInf->path = traceInf;
		pNodeInf->memSize = c;

		m_memNodeMap.insert( std::make_pair(addr, pNodeInf) );
		dealMemInf(pNodeInf->path.c_str(), "Malloc", pNodeInf->memSize);
	}
	else
	{
		//printf("traceInf  %s\n", traceInf);
	}
	
	pthread_mutex_unlock(&m_mutex);
	
}

void CalcMem::wrapFree(void* addr)
{
	const int stackSize = 15;
	void *stack_addr[stackSize];
	int layer = 0;
	int i;
	char traceInf[TRACE_INF_LEN] = "addr2line -e ./Challenge_Debug -f -C  ";
	int infPos = strlen(traceInf);;
	
	pthread_mutex_lock(&m_mutex);	
	/* 通过调用libc函数实现 */
	layer = backtrace(stack_addr, stackSize);

	for(i = 2; i < layer; i++)
	{
		snprintf(traceInf+infPos, sizeof(traceInf)-infPos, "%p  ", stack_addr[i]);
		infPos = strlen(traceInf);
	}
	
	MemNodeMap::iterator iter = m_memNodeMap.find(addr);
	if (iter != m_memNodeMap.end())
	{
		MemNodeInf *pNodeInf = iter->second;

		dealMemInf(pNodeInf->path.c_str(), traceInf, pNodeInf->memSize);
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
	printf("printfMallocMap()---------------------------->\n");
	pthread_mutex_lock(&m_mutex);

	MemInfType *memInfType = NULL;
	MemInf *memInf = NULL;

	MemInfMap::iterator memInfMapIter;
	MemInfType::iterator memInfTypeIter;

	size_t totolMallocSize = 0;
	size_t totolFreeSize = 0;
	for (memInfMapIter = m_MemInfMap.begin(); memInfMapIter != m_MemInfMap.end(); ++memInfMapIter)
	{
		std::string mallocPath = memInfMapIter->first;
		memInfType = memInfMapIter->second;

		//printf("mallocPath %s\n", mallocPath.c_str());
		for (memInfTypeIter = memInfType->begin(); memInfTypeIter != memInfType->end(); ++memInfTypeIter)
		{
			std::string freePath = memInfTypeIter->first;
			memInf = memInfTypeIter->second;

			if (freePath.size() < 16)
			{
				totolMallocSize += memInf->memSize;
			}
			else
			{
				totolFreeSize += memInf->memSize;
			}
		}

	}
	
	if (totolMallocSize  > totolFreeSize)
	{
		size_t diffSize =  totolMallocSize-totolFreeSize;
		printf("diffSize  %d  ---------------totolMallocSize, totolFreeSize  %d  %d\n", diffSize, totolMallocSize, totolFreeSize);
		//printf("%s\n", mallocPath.c_str());
	}

	printf("\n\n\n");	
	
	pthread_mutex_unlock(&m_mutex);
	
}

void CalcMem::dealMemInf(const char *mallocPath, const char *freePath, size_t size)
{
	MemInfType *memInfType = NULL;
	MemInfMap::iterator memInfMapIter = m_MemInfMap.find(mallocPath);
	if (memInfMapIter != m_MemInfMap.end())
	{
		memInfType = memInfMapIter->second;
	}
	else
	{
		memInfType = new MemInfType;
		assert(memInfType != NULL);
		m_MemInfMap.insert(std::make_pair(mallocPath, memInfType));
	}
	
	MemInf *memInf = NULL;
	MemInfType::iterator memInfTypeIter = memInfType->find(freePath);
	if (memInfTypeIter != memInfType->end())
	{
		memInf = memInfTypeIter->second;
	}
	else
	{
		memInf = new MemInf;
		assert(memInf != NULL);

		memInf->memSize = 0;
		memInfType->insert(std::make_pair(freePath, memInf));
	}

	memInf->memSize += size;

	return ;
}

