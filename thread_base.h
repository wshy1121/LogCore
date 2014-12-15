#ifndef __THREAD_BASE_H
#define __THREAD_BASE_H
#include <pthread.h>

namespace base
{
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);

 pthread_t pthread_self(void);

 int pthread_join(pthread_t thread, void **retval);
 
}//base



#endif //__THREAD_BASE_H

