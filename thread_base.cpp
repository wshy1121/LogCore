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

}//base



