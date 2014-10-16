#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Global.h"
#include "link_tool.h"

//����CTimeCalcʹ��
extern "C" void* __real_malloc(size_t);

void fun1()
{	time_trace();
	time_printf("NULL");
	char *tmp = new char[32];

	strcpy(tmp, "1234567789");
	time_printf("NULL");
	tmp = (char *)realloc(tmp, 33);
	strcpy(tmp, "1234567789");
	delete tmp;
	time_printf("NULL");
	tmp = (char *)calloc(1, 1024*4);
	strcpy(tmp, "1234567789");
	delete tmp;
	time_printf("NULL");	
	std::string stack;
	get_stack(stack);
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
	while (1)
	{
		time_trace();
		fun0(100);
		//testThreadQueue();
		usleep(1000*1000);		
	}
	return NULL;
}
void* printfMallocMap(void *pArg)
{
#ifdef WRAP
	CTimeCalcManager::instance()->start();
	while (1)
	{
		CalcMem::instance()->printfMallocMap();
		usleep(100*1000);		
	}
#endif
	return NULL;
}
int main()
{

	pthread_t thread_id[32];

	int i = 0;
	for (i=0; i<10;)
	{
		pthread_create(&thread_id[i++], NULL,test1,NULL);
	}
	pthread_create(&thread_id[i++], NULL,printfMallocMap,NULL);
	//-------------------------------------------------------------
	for (i=0; i<10;)
	{
		pthread_join(thread_id[i++], NULL);
	}
	pthread_join(thread_id[i++], NULL);
	
	return 0;
}


