#ifndef __MEM_CHECK_H
#define __MEM_CHECK_H
#include <stdio.h>
#include <stdlib.h>

class CMemCheck
{
public:
	static CMemCheck *instance();
public:
	static void *malloc(size_t c);
	static void* realloc(void *p, size_t c);
	static void* calloc(size_t c);
	static void free(void*p);
private:
	CMemCheck(){}
private:
	static CMemCheck *_instance;
};


#endif

