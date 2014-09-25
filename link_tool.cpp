#include "Global.h"
#include "link_tool.h"
#include <stdio.h>
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
void ThreadQueue::initThreadNode(ThreadNode *queue_node, bool enable, pthread_t thread_id)
{
	queue_node->enable = enable;
	queue_node->thread_id = thread_id;
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

