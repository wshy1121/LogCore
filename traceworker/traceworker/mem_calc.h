#ifndef _MEM_CALC_H
#define _MEM_CALC_H

#include <map>
#include <stdio.h>
#include "trace_base.h"

typedef struct TraceInfoId
{
	CBase::pthread_t threadId;
	int clientId;
	int socket;
	bool operator < (const struct TraceInfoId &key) const
	{
		if (key.clientId < clientId)
		{
			return true;
		}
		else if (key.clientId == clientId)
		{
			return key.threadId < threadId;
		}
		return false;
	}
}TraceInfoId;

#endif
