// traceTest.cpp : Defines the entry point for the console application.
//
#ifdef WIN32
#define TRACEWORKER_EXPORTS
#include <windows.h>
#include "stdafx.h"
#endif
#include <signal.h>

#include "trace_worker.h"
#include "trace_worker.h"
#include "trace_base.h"

void fun0(int count)
{	trace_worker();
	trace_printf("count  %d", count);
	if (count == 0)
	{
		return ;
	}
	--count;
	fun0(count);
}

void *test1(void *pArg)
{
	while(1)
	{
		trace_worker();
		fun0(10);
		//CBase::usleep(1000*1000);
	}
}

void whenSigInt(int)
{
    trace_stop();
	printf("trace_stoptrace_stoptrace_stoptrace_stop\n");
	exit(0);
}

#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char *argv[])
#endif
{
    signal(SIGINT, whenSigInt);
    
	static char tmpChar[1348220];
	trace_start("./1111.cpp");
	memset(tmpChar, '1', sizeof(tmpChar));
	{
		trace_worker();
		trace_str(tmpChar, sizeof(tmpChar));
	}


	
    //trace_worker();
	//fun0(10);
#if 0
	const int threadNum = 2;
	CBase::pthread_t thread_id[threadNum];

	for (int i=0; i<threadNum; ++i)
	{
		CBase::pthread_create(&thread_id[i], NULL, test1, NULL);
	}
#endif	
	printf("to wait\n");
	while(1)
	{
#ifdef WIN32	    
        Sleep(100);
#else
		CBase::usleep(100*1000);
#endif
	}
	return 0;
}

