#ifndef __MEM_CHECK_H
#define __MEM_CHECK_H
#include <stdio.h>
#include <stdlib.h>
#include <string>

extern "C" void __real_free(void* p);
extern "C" void* __real_malloc(size_t);
extern "C" void *__real_realloc(void* c, int size);
extern "C" void* __real_calloc(size_t nmemb, size_t size);

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
	static void* calloc(size_t nmemb, size_t size);
	static void free(void*p);
	static bool isMemCheck(void *addr);
public:
	void addMemInfo(void *addr, int addrLen, std::string &backTrace);
	MemNodeInf *getMemNodeInf(void *addr);
private:
	static void setFlag(void *addr, size_t size);
private:
	CMemCheck(){}	
private:
	static void *m_checkValue;
	static CMemCheck *_instance;
	static const size_t m_flagSize;
};


#endif

