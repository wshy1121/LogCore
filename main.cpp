#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "json.h"
#include "Global.h"
#include "string_base.h"

//≤‚ ‘CTimeCalc π”√

void fun1()
{	time_trace();
	time_printf("NULL");
	time_stack();
	time_str((char *)"1234", 4);
	char *tmp = (char *)base::malloc(32);
	base::strcpy(tmp, "1234567789");
	time_printf("NULL");
	tmp = (char *)realloc(tmp, 33);
	base::strcpy(tmp, "1234567789");
	free(tmp);
	tmp = (char *)calloc(1, 16);
	base::strcpy(tmp, "1234567789");
	tmp = (char *)realloc(tmp, 33);
	base::strcpy(tmp, "1234567789");	
	free(tmp);
	tmp = new char[32];
	base::strcpy(tmp, "1234567789");
	tmp = (char *)realloc(tmp, 33);
	base::strcpy(tmp, "1234567789");		
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

const char *jsonData = "{\"opr\" : \"createCandy\", \"threadId\" : 100, \"line\" : 100, \"fileName\" : \"name\", \"funcName\" : \"name\", \"displayLevel\" : 100, \"content\" : \"I am Candy\"}";
int main()
{
	json_object *new_obj = NULL;
	json_object *json_para = NULL;
	new_obj = json_tokener_parse(jsonData);
	if (!new_obj)
	{
		debug_printf();
	}
	json_para = json_object_object_get(new_obj, "opr");
	if (json_para)
	{
		printf("%s\n", json_object_to_json_string(json_para));
	}
	
	json_object_put(new_obj);
	return 0;
	
	time_start();

	base::pthread_t printfThreadId;
	base::pthread_create(&printfThreadId, NULL,printfMallocMap,NULL);

	const int threadNum = 50;
	base::pthread_t thread_id[threadNum];	
	while (1)
	{
		for (int i=0; i<threadNum; ++i)
		{
			base::pthread_create(&thread_id[i], NULL,test1,NULL);
		}
		//-------------------------------------------------------------
		for (int i=0; i<threadNum; ++i)
		{
			base::pthread_join(thread_id[i], NULL);
		}
		base::usleep(10*1000*1000);
	}
	return 0;
}


