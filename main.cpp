#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Global.h"


//≤‚ ‘CTimeCalc π”√

void fun1()
{
	time_trace_level(3);
	time_stack("time_stack");
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
	while (1)
	{
		time_trace();
		usleep(1000*10000);
		fun0(100);
		
	}
	return NULL;
}

int main()
{
	pthread_t thread_id[10];
	
	for (int i=0; i<10; ++i)
	{
		pthread_create(&thread_id[i], NULL,test1,NULL);
	}

	for (int i=0; i<10; ++i)
	{
		pthread_join(thread_id[i], NULL);
	}
	return 0;
}


