#ifndef __STRING_BASE_H
#define __STRING_BASE_H

#include <stdarg.h>

namespace base
{
char *strcpy(char *dest, const char *src);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

int snprintf(char *str, size_t size, const char *format, ...);

}//base



#endif //__THREAD_BASE_H

