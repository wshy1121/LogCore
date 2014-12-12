#ifndef __MEM_BASE_H
#define __MEM_BASE_H
#include <stdlib.h>

namespace base
{
void* malloc(size_t size);
void free(void* p);
void *realloc(void* c, int size);
void* calloc(size_t nmemb, size_t size);
}//base



#endif


