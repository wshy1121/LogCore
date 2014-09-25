#ifndef __LINK_TOOL_H
#define __LINK_TOOL_H

#define container_of(ptr, type, member) ({  \
const typeof( ((type *)0)->member ) *__mptr = (ptr);   \
(type *)( (char *)__mptr - offsetof(type,member) );})

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
	void initQueue();
	int putQueue(ThreadNode *queue_node);
	int getQueue(pthread_t thread_id, ThreadNode **ret_queue_node);

private:
	ThreadNode head_node;
	ThreadNode *tail;
	int node_num;
};


#define TQueueContain(x) container_of((x), ThreadNode, node)

#endif


