#ifndef __LINK_TOOL_H
#define __LINK_TOOL_H

#define container_of(ptr, type, member) ({  \
const typeof( ((type *)0)->member ) *__mptr = (ptr);   \
(type *)( (char *)__mptr - offsetof(type,member) );})

struct node
{
    struct node *next;
	struct node *pre;
};


struct queue_node
{
	int type;
	struct node node;
};
struct queue
{
	struct queue_node head_node;
	struct queue_node *tail;
	int node_num;
};


void init_node(struct node *node);
void insert_node(struct node *node, struct node *inser_node);
void remov_node(struct node *node);

struct stack
{
  struct node head;
  struct node *top;
  int stacksize;
};

int initStack(struct stack *stack);
int isStackEmpty(struct stack *stack);
int getStackTop(struct stack *stack, struct node **top);
int pushStack(struct stack *stack, struct node *node);
int popStack(struct stack *stack, struct node **node);
void dispStack(struct stack *stack, void dip_func(struct node *node));

#define each_link_node(head, node) for ((node)=(head)->next; (head) != (node); (node)=(node)->next)

void initQueue(struct queue *queue);
int putQueue(struct queue *queue, struct queue_node *queue_node);
int getQueue(struct queue *queue, int type, struct queue_node **ret_queue_node);
int rmQueueNode(struct queue *queue, struct queue_node *queue_node);
int getQueueHead(struct queue *queue, struct queue_node **ret_queue_node);
int getQueueNodeNum(struct queue *queue);
void dispQueue(struct queue *queue);
int eachQueue(struct queue *queue, int (*queue_node_deal)(struct queue_node *, void *arg), void *arg);

typedef struct 
{
	pthread_t thread_id;
	bool enable;
	struct node node;
}ThreadNode;

typedef struct
{
	ThreadNode head_node;
	ThreadNode *tail;
	int node_num;
}ThreadQueue;
#define TQueueContain(x) container_of((x), struct ThreadNode, node)

#endif


