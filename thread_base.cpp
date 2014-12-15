#include "thread_base.h"



namespace base
{

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg)
{
	return ::pthread_create(thread, attr, start_routine, arg);
}


pthread_t pthread_self(void)
{
	return ::pthread_self();
}

int pthread_join(pthread_t thread, void **retval)
{
	return ::pthread_join(thread, retval);
}


int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	return ::pthread_mutex_destroy(mutex);
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	return ::pthread_mutex_init(mutex, attr);
}



	   

}//base



