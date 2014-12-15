#ifndef __THREAD_BASE_H
#define __THREAD_BASE_H
#include <pthread.h>

namespace base
{
typedef ::pthread_mutex_t  pthread_mutex_t;
typedef ::pthread_t  pthread_t;


int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
 pthread_t pthread_self(void);
 int pthread_join(pthread_t thread, void **retval);
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

}//base



#endif //__THREAD_BASE_H

