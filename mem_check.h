#ifndef __MEM_CHECK_H
#define __MEM_CHECK_H
#include <stdio.h>
#include <stdlib.h>
#include <string>

typedef struct MemNodeInf
{
	char *path;
	size_t memSize;
}MemNodeInf;

class CMemCheck
{
public:
	static CMemCheck *instance();
public:
	static void *malloc(size_t c);
	static void* realloc(void *p, size_t c);
	static void* calloc(size_t c);
	static void free(void*p);
	static bool isMemCheck(void *addr);
public:
	void addMemInfo(void *addr, int addrLen, std::string &backTrace);
	MemNodeInf *getMemNodeInf(void *addr);
private:
	static void initMem(void *addr, int addrLen);
	static void exitMem(void *addr, const char *errInfo);
private:
	CMemCheck(){}	
private:
	static void *m_checkValue;
	static CMemCheck *_instance;
};


#endif

