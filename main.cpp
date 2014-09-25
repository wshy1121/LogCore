#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Global.h"
#include "link_tool.h"

//≤‚ ‘CTimeCalc π”√
extern "C" void* __real_malloc(size_t);

void fun1()
{
	char *tmp = new char[32];

	strcpy(tmp, "1234567789");
	delete tmp;
	
	time_trace_level(3);
	std::string stack;
	get_stack(stack);
	time_printf("%s", stack.c_str());
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

ThreadQueue threadQueue;

void testThreadQueue()
{

	pthread_t threadId = 1;
	ThreadNode *queue_node = NULL;

	for (int i=0; i<5; ++i)
	{
		queue_node = (ThreadNode *)__real_malloc(sizeof(ThreadNode));
		threadQueue.initThreadNode(queue_node, true, ++threadId);
		threadQueue.putQueue(queue_node);
	}

	//threadQueue.dispQueue();
	threadQueue.clearQueue();

	for (int i=0; i<5; ++i)
	{
		queue_node = (ThreadNode *)__real_malloc(sizeof(ThreadNode));
		threadQueue.initThreadNode(queue_node, true, ++threadId);
		threadQueue.putQueue(queue_node);
	}
	
	threadQueue.clearQueue();

	return ;
}
void* test1(void *pArg)
{	
	while (1)
	{
		//time_trace();
		testThreadQueue();
		//fun0(100);
		usleep(100*1000);		
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


