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

//��巨
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

CList *CList::createCList()
{
	CList *pCList = (CList *)__real_malloc(sizeof(CList));
	if (pCList)
	{
		pCList->init();
	}
	return pCList;
}

void CList::destroyClist(CList *pCList)
{
	pCList->exit();
	__real_free(pCList);
}



void CList::init()
{
	init_node(&head_node);
	tail = &head_node;
	node_num = 0;
}
void CList::exit()
{
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

node *CList::back()
{
	return head_node.pre;
}

void CList::pop_front()
{
	node *pNode = head_node.next;
	remov_node(pNode);
	tail = head_node.pre;
	--(node_num);
}

void CList::pop_back()
{
	node *pNode = head_node.pre;
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

node *CList::getHead()
{
	return &head_node;
}

void CStrNode::init(int maxStrLen)
{
	m_strLen = 0;
	m_remainMem = 0;
	if (maxStrLen < 1)
	{
		m_str = NULL;
		return ;
	}
	init_node(&m_node);
	m_str = (char *)__real_malloc(maxStrLen + 1);
	m_remainMem = maxStrLen;
	return ;
}

void CStrNode::exit()
{
	__real_free(m_str);
	m_str = NULL;
	m_strLen = 0;
}
CStrNode *CStrNode::createCStrNode(int maxStrLen)
{
	CStrNode *pNode = (CStrNode *)__real_malloc(sizeof(CStrNode));
	if (pNode)
	{
		pNode->init(maxStrLen);
	}
	return pNode;
}



void CStrNode::destroyCStrNode(CStrNode *pNode)
{
	if (!pNode)
	{
		return ;
	}
	pNode->exit();
	__real_free(pNode);
}

int CStrNode::writeStr(char *str)
{
	int strLen = strlen(str);
	int writeLen = m_remainMem > strLen ? strLen:m_remainMem;

	memcpy(m_str + m_strLen, str, writeLen);
	m_strLen += writeLen;
	m_remainMem -= writeLen;
	m_str[m_strLen] = '\0';
	return writeLen;
}

node *CStrNode::getNode()
{
	return &m_node;
}
 int CStrNode::size()
{
	return m_strLen;
}

void CStrNode::setStr(char *str, int strLen)
{
	if (strLen == -1)
	{
		strLen = strlen(str);
	}
	m_strLen = strLen;	
	m_str = str;
}

char *CStrNode::getStr()
{
	return m_str;
}


void CString::init()
{
	m_pStrList = CList::createCList();
	m_strLen = 0;
	m_lastStrNode = NULL;
	m_maxStrNodeLen = 32 * 1024;
}

void CString::exit()
{
	CStrNode *pStrNode = NULL; 
	struct node *pNode = NULL;
	while (m_pStrList->size())
	{
		pNode =  m_pStrList->begin();
		pStrNode = TStrNodeContain(pNode);
		m_pStrList->pop_front();
		CStrNode::destroyCStrNode(pStrNode);
	}

	CList::destroyClist(m_pStrList);
	m_pStrList = NULL;
	m_strLen = 0;	
}

 CString* CString::createCString()
{
	CString *pCString = (CString *)__real_malloc(sizeof(CString));
	if (pCString)
	{
		pCString->init();
	}
	return pCString;
}
 
void CString::destroyCString(CString *pCString)
{
	if (pCString == NULL)
	{
		return ;
	}

	pCString->exit();
	__real_free(pCString);
}
 int CString::size()
{
	return m_strLen;
}
 
void CString::append(char *str)
{
	if (str == NULL)
	{
		return ;
	}
	if (!m_lastStrNode)
	{
		m_lastStrNode = CStrNode::createCStrNode(m_maxStrNodeLen);
		m_pStrList->push_back(m_lastStrNode->getNode());
	}
	
	int remainLen = strlen(str);
	int pos = 0;
	int writeLen = 0;

	m_strLen += remainLen;

	while (1)
	{
		writeLen = m_lastStrNode->writeStr(str + pos);
		pos += writeLen;
		remainLen -= writeLen;
		if (remainLen == 0)
		{
			break;
		}
		else
		{
			m_lastStrNode = CStrNode::createCStrNode(m_maxStrNodeLen);
			m_pStrList->push_back(m_lastStrNode->getNode());
		}
	}
}

void CString::append(const char *str)
{
	append((char *)str);
}
char *CString::c_str()
{
	CStrNode *pStrNode = NULL;
	CStrNode *newStrNode = NULL;
	struct node *pNode = NULL;

	int newStrLen = m_maxStrNodeLen > m_strLen ? m_maxStrNodeLen : m_strLen;
	newStrNode = CStrNode::createCStrNode(newStrLen);
	
	while (m_pStrList->size())
	{
		pNode =  m_pStrList->begin();
		pStrNode = TStrNodeContain(pNode);
		m_pStrList->pop_front();

		newStrNode->writeStr(pStrNode->getStr());
		
		CStrNode::destroyCStrNode(pStrNode);
	}


	m_pStrList->push_back(newStrNode->getNode());
	m_lastStrNode = pStrNode;
	return newStrNode->getStr();
}

