#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "Global.h"

//����CTimeCalcʹ��
extern "C" void* __real_malloc(size_t);

void fun1()
{	time_trace();
	time_printf("NULL");
	time_stack();
	time_str((char *)"1234", 4);
	char *tmp = (char *)malloc(32);
	strcpy(tmp, "1234567789");
	time_printf("NULL");
	tmp = (char *)realloc(tmp, 33);
	strcpy(tmp, "1234567789");
	free(tmp);
	tmp = (char *)calloc(1, 16);
	strcpy(tmp, "1234567789");
	free(tmp);
	tmp = new char[32];
	strcpy(tmp, "1234567789");
	delete tmp;
	time_printf("NULL");	
	std::string stack;
	//time_stack();
	time_printf("get_stack  %s", stack.c_str());
	{
		time_printf("NULL");
		time_trace_level(4);
		{
			time_printf("NULL");
			time_trace_level(3);
			{
				time_printf("NULL");
				time_trace_level(2);
				{
					time_printf("NULL");
					time_trace_level(1);
					{
						time_printf("NULL");
						time_trace_level(0);
					}

				}

			}

		}
	}
}
void fun0(int count)
{	time_trace();
	if (count == 0)
	{
		return ;
	}
	time_printf("1234");
	--count;
	fun0(count);
	fun1();
}

void* test1(void *pArg)
{	
	time_trace();
	fun0(100);
	return NULL;
}
void* printfMallocMap(void *pArg)
{
#ifdef WRAP
	while (1)
	{
		sleep(2);
		time_mem();
	}
#endif
	return NULL;
}
int main()
{
	time_start();

	pthread_t printfThreadId;
	pthread_create(&printfThreadId, NULL,printfMallocMap,NULL);

	const int threadNum = 50;
	pthread_t thread_id[threadNum];	
	while (1)
	{
		for (int i=0; i<threadNum; ++i)
		{
			pthread_create(&thread_id[i], NULL,test1,NULL);
		}
		//-------------------------------------------------------------
		for (int i=0; i<threadNum; ++i)
		{
			pthread_join(thread_id[i], NULL);
		}
		usleep(10*1000*1000);
	}
	return 0;
}


