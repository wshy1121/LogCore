#include "mem_calc.h"
#include "Global.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "mem_check.h"
#include "mem_base.h"
#include "thread_base.h"

extern CPthreadMutex g_insMutexCalc;
extern "C" int backtrace(void **buffer, int size);
/******************************************************/


///\brief 构造函数
CGuardEnable::CGuardEnable(E_ENABLE_TYPE type)
	:m_type(type)
{
	if (!ThreadQueue::getEnable())
	{
		return ;
	}
	m_queueNode = ThreadQueue::instance()->getQueueNode(base::pthread_self());
	m_needReturn = !m_queueNode->enable[m_type];
}

CGuardEnable::~CGuardEnable()
{
	if (!ThreadQueue::getEnable())
	{
		return ;
	}
	
	if (m_needReturn) 
	{
		return ;
	}
	
	ThreadQueue::instance()->removeNode(m_queueNode, m_type);
}

bool CGuardEnable::needReturn()
{
	if (!ThreadQueue::getEnable())
	{
		return false;
	}
	
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
	queue_node->thread_id = base::pthread_self();
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
	if (queueNode->thread_id == base::pthread_self())
	{
		remov_node(&queueNode->node);
		tail = TQueueContain(head_node.node.pre);
		--(node_num);
		base::free(queueNode);
		return 0;	
	}

	return -1;
}

int ThreadQueue::removeQueue(base::pthread_t thread_id)
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
int ThreadQueue::getQueue(base::pthread_t thread_id, ThreadNode **ret_queue_node)
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

		base::free(queue_node);

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
ThreadNode *ThreadQueue::getQueueNode(base::pthread_t thread_id)
{
	ThreadNode *queue_node = NULL;
	getQueue(base::pthread_self(), &queue_node);
	if (!queue_node)
	{
		queue_node = (ThreadNode *)base::malloc(sizeof(ThreadNode));
		initThreadNode(queue_node);
		insertQueue(queue_node);
	}
	return queue_node;
}
void ThreadQueue::wrapMalloc(void* addr, size_t c)
{
	threadQueueEnable(e_Mem);
	std::string backtrace;
	CalcMem::instance()->getBackTrace(backtrace);
	CMemCheck::instance()->addMemInfo(addr, c, backtrace);
	
	MEM_DATA *pMemData =  CalcMem::instance()->createMemData(backtrace.size());
	CalcMemInf *pCalcMemInf = &pMemData->calcMemInf;
		
	pCalcMemInf->m_opr = CalcMemInf::e_wrapMalloc;
	pCalcMemInf->m_threadId = base::pthread_self();
	pCalcMemInf->m_memAddr = addr;
	pCalcMemInf->m_memSize= c;
	strcpy(pCalcMemInf->m_backTrace, backtrace.c_str());

	CalcMemManager::instance()->pushMemData(pMemData);
	return ;
}

void ThreadQueue::wrapFree(void* addr)
{
	threadQueueEnable(e_Mem);
	MemNodeInf nodeInf;
	if (CMemCheck::instance()->getMemNodeInf(addr, nodeInf) == false)
	{
		return ;
	}
	
	MEM_DATA *pMemData =  CalcMem::instance()->createMemData(nodeInf.path.size());
	CalcMemInf *pCalcMemInf = &pMemData->calcMemInf;

	pCalcMemInf->m_opr = CalcMemInf::e_wrapFree;
	pCalcMemInf->m_threadId = base::pthread_self();
	pCalcMemInf->m_memAddr = addr;
	pCalcMemInf->m_memSize= nodeInf.memSize;
	strcpy(pCalcMemInf->m_backTrace, nodeInf.path.c_str());
	
	CalcMemManager::instance()->pushMemData(pMemData);
	return ;
}
CalcMem *CalcMem::_instance = NULL;
CalcMem::CalcMem() : 	m_stackNum(32)					
{
	m_traceHead = "addr2line -e ./Challenge_Debug -f -C  ";
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


MEM_DATA *CalcMem::createMemData(int backTraceLen)
{
	MEM_DATA *pMemData = (MEM_DATA *)base::malloc(sizeof(MEM_DATA));
	CalcMemInf *pCalcMemInf = &pMemData->calcMemInf;

	pCalcMemInf->m_opr = CalcMemInf::e_wrapMalloc;
	pCalcMemInf->m_threadId = -1;
	pCalcMemInf->m_memAddr = NULL;
	pCalcMemInf->m_memSize= 0;

	pCalcMemInf->m_backTrace = NULL;
	if (backTraceLen > 0)
	{
		pCalcMemInf->m_backTrace = (char *)base::malloc(backTraceLen+1);
		((char *)pCalcMemInf->m_backTrace)[0] = '\0';
	}
	
	return pMemData;
	
}
void CalcMem::destroyMemData(MEM_DATA *pMemData)
{
	base::free(pMemData->calcMemInf.m_backTrace);
	base::free(pMemData);
}


void CalcMem::wrapMalloc(void* addr, size_t c, char *pBackTrace, base::pthread_t threadId)
{
	if (strlen(pBackTrace) <= 0)
	{
		return ;
	}
	CGuardMutex guardMutex(m_mutex);
	dealMemInf(pBackTrace, c, threadId);	
}

void CalcMem::wrapFree(void* addr, size_t c, char *pBackTrace, base::pthread_t threadId)
{
	CGuardMutex guardMutex(m_mutex);
	dealMemInf(pBackTrace, -c, threadId);
}
void CalcMem::printfMemInfMap(base::pthread_t threadId)
{
	threadId = base::pthread_self();
	CCandy candy(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, 0);

	std::string path;
	CGuardMutex guardMutex(m_mutex);
	for (MemInfMap::iterator iter = m_MemInfMap.begin(); iter != m_MemInfMap.end(); ++iter)
	{
		path = iter->first;
		MemInf *memInf = iter->second;

		size_t diffCount = memInf->mallocCount - memInf->freeCount;
		int itemSize = 0;
		if (diffCount > 0)
		{
			itemSize = memInf->memSize /diffCount;
		}
		CBugKiller::InsertStrOnly(threadId, "maxSize  itemSize  memSize  diffCount  mallocCount  freeCount  %016d  %08d  %d  %d  %d  %d %s", memInf->maxSize, itemSize, memInf->memSize, 
												diffCount, memInf->mallocCount, memInf->freeCount, path.c_str());
	}
	
	return ;	
}

void CalcMem::dealMemInf(const char *mallocPath, int size, base::pthread_t threadId)
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
		memInf->mallocCount = 0;
		memInf->freeCount = 0;
		
		m_MemInfMap.insert(std::make_pair(mallocPath, memInf));
	}
	
	memInf->memSize += size;
	if (size > 0)
	{
		memInf->mallocCount++;
	}
	else if (size < 0)
	{
		memInf->freeCount++;
	}
	if (memInf->memSize > memInf->maxSize)
	{
		memInf->maxSize = memInf->memSize;

		int count = memInf->mallocCount - memInf->freeCount;
		CBugKiller::InsertStrOnly(threadId, "%s  malloc size  %06d  %d  %d  %s", mallocPath, count, memInf->memSize, __LINE__, __FILE__);
	}
	return ;
}

std::string CalcMem::splitFilename (std::string &path)
{
	size_t found;
	found=path.find_last_of("/\\");
	return path.substr(found+1);
}

std::string &CalcMem::getBackTrace(std::string &backTrace)
{
       void *stack_addr[m_stackNum];
       int layer;
       int i;
	char tmp[256];
	backTrace = m_traceHead;

	m_mutex.Enter();
	layer = backtrace(stack_addr, m_stackNum);
	m_mutex.Leave();
	for(i = 3; i < layer; i++)
	{
		snprintf(tmp, sizeof(tmp), "%p  ", stack_addr[i]);
		backTrace += tmp;
	}
	return backTrace;
}


CalcMemManager *CalcMemManager::_instance = NULL;
CalcMemManager::CalcMemManager()
{
	m_recvList = CList::createCList();
	base::pthread_create(&m_threadId, NULL,threadFunc,NULL);
}
CalcMemManager *CalcMemManager::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CalcMemManager;
		}
	}
	return _instance;
}

void CalcMemManager::threadProc()
{	
	while(1)
	{

		if(m_recvList->empty())
		{
			usleep(10 * 1000);
			continue;
		}
		m_recvListMutex.Enter();
		struct node *pNode =  m_recvList->begin();
		MEM_DATA *pMemData = memDataContain(pNode);
		m_recvList->pop_front();	
		m_recvListMutex.Leave();
		
		dealRecvData(&pMemData->calcMemInf);
		CalcMem::instance()->destroyMemData(pMemData);		
	}
}

void* CalcMemManager::threadFunc(void *pArg)
{
	CalcMemManager::instance()->threadProc();
	return NULL;
}

void CalcMemManager::dealRecvData(CalcMemInf *pCalcMemInf)
{
	threadQueueEnable(e_Mem);	

	CalcMemInf::CalcMemOpr &opr = pCalcMemInf->m_opr;
	int threadId = pCalcMemInf->m_threadId;
	 
	void *memAddr = pCalcMemInf->m_memAddr;
	size_t memSize = pCalcMemInf->m_memSize;
	char *pBackTrace = pCalcMemInf->m_backTrace;
	
	switch (opr)
	{
		case CalcMemInf::e_wrapMalloc:
			{
				CalcMem::instance()->wrapMalloc(memAddr, memSize, pBackTrace, threadId);
 				break;
			}
		case CalcMemInf::e_wrapFree:
			{
				CalcMem::instance()->wrapFree(memAddr, memSize, pBackTrace, threadId);
				break;
			}
		default:
			break;
	}
	return ;

}

void CalcMemManager::pushMemData(MEM_DATA *pMemData)
{
	if (pMemData == NULL)
	{
		return ;
	}
	
	m_recvListMutex.Enter();
	m_recvList->push_back(&pMemData->node);
	m_recvListMutex.Leave();
	return ;
}



