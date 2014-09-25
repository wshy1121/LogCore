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




int initStack(struct stack *stack)
{
	init_node(&stack->head);
	stack->top = &stack->head;
	stack->stacksize = 0;
	return 0;
}

int isStackEmpty(struct stack *stack)
{
	if ((&stack->head == stack->top) && (stack->stacksize == 0))
	{
		return 1;
	}
	return 0;
}

int getStackTop(struct stack *stack, struct node **top)
{
	int ret;
	ret = isStackEmpty(stack);
	if (ret)
	{
		*top = NULL;
		return -1;
	}
	*top = stack->top;
	return 0;
}

int pushStack(struct stack *stack, struct node *node)
{
	insert_node(&stack->head, node);
	stack->top = node;
	++(stack->stacksize);
	return 0;
}

int popStack(struct stack *stack, struct node **node)
{
	int ret;
	ret = isStackEmpty(stack);
	if (ret)
	{
		*node = NULL;
		return -1;
	}
	*node = stack->top;
	stack->top = stack->top->next;
	remov_node(*node);
	--(stack->stacksize);
	if (stack->stacksize < 0)
	{
		return -1;
	}
    return 0;
}

void dispStack(struct stack *stack, void dip_func(struct node *node))
{
	struct node *head = NULL;
	struct node *node = NULL;
	dip_func(stack->top);
	head = &stack->head;
	each_link_node(head, node)
	{
	    dip_func(node);
	}
	return ;
}



//链式队列接口
#define queueContain(x) container_of((x), struct queue_node, node)
void initQueue(struct queue *queue)
{
	init_node(&queue->head_node.node);
	queue->head_node.type = -1;
	queue->tail = &queue->head_node;
	queue->node_num = 0;
	return ;
}
//同一节点不能插入两次，否则会出现队列的混乱
//消息类型不能为0
int putQueue(struct queue *queue, struct queue_node *queue_node)
{
	if (queue_node->type <= 0)
	{
		return -1;
	}
	insert_node(queue->head_node.node.pre, &queue_node->node); 
	queue->tail = queue_node;
	++(queue->node_num);
	return 0;
}

int getQueue(struct queue *queue, int type, struct queue_node **ret_queue_node)
{
	struct node *node = NULL;
	struct node *head = &queue->head_node.node;
	struct queue_node *queue_node = NULL;
	if (type < 0)
	{
		*ret_queue_node = NULL;
		return -1;
	}
	if (queue->node_num < 1) //无结点
	{
		*ret_queue_node = NULL;
		return -1;
	}
	if (&queue->head_node == queue->tail) //无结点
	{
		*ret_queue_node = NULL;
		return -1;
	}

	if (type == 0) //如果type为0，则从队列头部取出结点
	{
		
		*ret_queue_node = queueContain(queue->head_node.node.next); 
		remov_node(&(*ret_queue_node)->node);
		--(queue->node_num);
		queue->tail = container_of(queue->head_node.node.pre, struct queue_node, node);
		return 0;
	}
	each_link_node(head, node)
	{
		queue_node = container_of(node, struct queue_node, node);
		if (queue_node->type == type)
		{
			*ret_queue_node = queue_node;
			remov_node(&queue_node->node);
			queue->tail = container_of(queue->head_node.node.pre, struct queue_node, node);
			--(queue->node_num);
			return 0;	
		}
	}

	*ret_queue_node = NULL;
	return -1;
}

int eachQueue(struct queue *queue, int (*queue_node_deal)(struct queue_node *, void *arg), void *arg)
{
	int ret;
	struct node *next_node = NULL;
	struct node *head = &queue->head_node.node;
	struct node *node = NULL;
	struct queue_node *queue_node_ptr = NULL;
	
	for ((node)=(head)->next; (head) != (node); node = next_node)
	{
		next_node = node->next;
		queue_node_ptr = queueContain(node);

		if (queue_node_deal != NULL)
		{
			ret = queue_node_deal(queue_node_ptr, arg);
		}
	}

	return 0;
}
int rmQueueNode(struct queue *queue, struct queue_node *queue_node)
{
	if (queue->node_num < 1)
	{
		return -1;
	}

	remov_node(&queue_node->node); 
	queue->tail = container_of(queue->head_node.node.pre, struct queue_node, node);
	--(queue->node_num);
	return 0;
}
int getQueueHead(struct queue *queue, struct queue_node **ret_queue_node)
{
	*ret_queue_node = queueContain(queue->head_node.node.next);
	return 0;
}

int getQueueNodeNum(struct queue *queue)
{
	return queue->node_num;
}
void dispQueue(struct queue *queue)
{
	struct node *head = &queue->head_node.node;
	struct node *node = NULL;
	struct queue_node *queue_node_ptr = NULL;
	each_link_node(head, node)
	{
		queue_node_ptr = queueContain(node);
	}
	return ;
}


void initTQueue(ThreadQueue *queue)
{
	init_node(&queue->head_node.node);
	queue->tail = &queue->head_node;
	queue->node_num = 0;
	return ;
}


int putTQueue(ThreadQueue *queue, ThreadNode *queue_node)
{
	insert_node(queue->head_node.node.pre, &queue_node->node); 
	queue->tail = queue_node;
	++(queue->node_num);
	return 0;
}


int getTQueue(ThreadQueue *queue, pthread_t thread_id, ThreadNode **ret_queue_node)
{
	struct node *node = NULL;
	struct node *head = &queue->head_node.node;
	ThreadNode *queue_node = NULL;

	if (queue->node_num < 1) //无结点
	{
		*ret_queue_node = NULL;
		return -1;
	}
	if (&queue->head_node == queue->tail) //无结点
	{
		*ret_queue_node = NULL;
		return -1;
	}

	each_link_node(head, node)
	{
		queue_node = container_of(node, ThreadNode, node);
		if (queue_node->thread_id == thread_id)
		{
			*ret_queue_node = queue_node;
			remov_node(&queue_node->node);
			queue->tail = container_of(queue->head_node.node.pre, ThreadNode, node);
			--(queue->node_num);
			return 0;	
		}
	}

	*ret_queue_node = NULL;
	return -1;
}



