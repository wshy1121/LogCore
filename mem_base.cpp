#include "stdafx.h"
#include "mem_base.h"
extern "C" void __real_free(void* p);
extern "C" void* __real_malloc(size_t);
extern "C" void *__real_realloc(void* c, int size);
extern "C" void* __real_calloc(size_t nmemb, size_t size);

namespace base
{


void* malloc(size_t size)
{
	return __real_malloc(size);
}


void free(void* p)
{
	__real_free(p);
}

void *realloc(void* c, size_t size)
{
	return __real_realloc(c, size);
}

void* calloc(size_t nmemb, size_t size)
{
	return __real_calloc(nmemb, size);
}

}//base


