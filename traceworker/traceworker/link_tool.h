#ifndef _LINK_TOOL_H
#define _LINK_TOOL_H
#include <stdio.h>
#include "trace_base.h"

class CPthreadMutex
{
public:
	CPthreadMutex()
	{
		m_mutex = new CBase::pthread_mutex_t;
		CBase::pthread_mutex_init(m_mutex, NULL);
	}
	~CPthreadMutex()
	{
		delete m_mutex;
	}

	bool Enter()
	{
		CBase::pthread_mutex_lock(m_mutex);
		return true;
	}

	bool Leave()
	{
		CBase::pthread_mutex_unlock(m_mutex);
		return true;
	}
	
private:
	CBase::pthread_mutex_t *m_mutex;
};


class CGuardMutex
{
public:
	CGuardMutex(CPthreadMutex &mutex)
	:m_mutex(mutex)
	{
		m_mutex.Enter();
	}
	~CGuardMutex()
	{
		m_mutex.Leave();
	}
private:
	CPthreadMutex &m_mutex;
};
#endif
