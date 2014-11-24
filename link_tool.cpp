#include "link_tool.h"
#include <stdio.h>
#include <string.h>
#include <execinfo.h>
#include <assert.h>
#include <unistd.h>

extern "C" void __real_free(void* p);
extern "C" void* __real_malloc(size_t);
/******************************************************/
void init_node(struct node *node)
{
	node->next = node;
	node->pre = node;
	return ;
}

//ºó²å·¨
void insert_node(struct node *node, struct node *inser_node)
{
	struct node *temp_node = NULL;
	temp_node = node->next;

	node->next = inser_node;
	inser_node->pre = node;

	inser_node->next= temp_node;
	temp_node->pre = inser_node;
}
extern CPthreadMutex g_insMutexCalc;

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




CList::CList()
{
	init_node(&head_node);
	tail = &head_node;
	node_num = 0;
}



int CList::push_back(node *pNode)
{
	insert_node(head_node.pre, pNode); 
	tail = pNode;
	++(node_num);
	return 0;

}

node *CList::begin()
{
	return head_node.next;
}

void CList::pop_front()
{
	node *pNode = head_node.next;
	remov_node(pNode);
	tail = head_node.pre;
	--(node_num);
}


bool CList::empty()
{
	if (node_num == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CList::clear()
{
	node *pNode = NULL;
	#define each_link_node(head, node) for ((node)=(head)->next; (head) != (node); (node)=(node)->next)
	
	each_link_node(&head_node, pNode)
	{
	}		
}

int CList::size()
{
	return node_num;
}

