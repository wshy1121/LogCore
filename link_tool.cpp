#include "Global.h"
#include "link_tool.h"


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

ThreadQueue::ThreadQueue()
{
	initQueue();
	return ;
}

void ThreadQueue::initQueue()
{
	init_node(&head_node.node);
	tail = &head_node;
	node_num = 0;
	return ;
}


int ThreadQueue::putQueue(ThreadNode *queue_node)
{
	insert_node(head_node.node.pre, &queue_node->node); 
	tail = queue_node;
	++(node_num);
	return 0;
}


int ThreadQueue::getQueue(pthread_t thread_id, ThreadNode **ret_queue_node)
{
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
			remov_node(&queue_node->node);
			tail = TQueueContain(head_node.node.pre);
			--(node_num);
			return 0;	
		}
	}

	*ret_queue_node = NULL;
	return -1;
}



