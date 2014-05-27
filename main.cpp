#include <stdlib.h>
#include <stdio.h>
#include <string.h>



void calcMem(FILE *pFp)
{
	char buf[256];
	char name[32];
	char kb[8];
	int startMem[16];
	rewind(pFp);
	for (int i=0;; ++i)
	{
		memset(buf, 0, sizeof(buf));
		char *ptr1 = fgets(buf, 256, pFp);
		if (!ptr1)//说明数据已经读取完毕
		{
			break;
		}
		
		char *ptr2 = strpbrk(ptr1, " ");	
		int num = 0;
		num = atoi(ptr2);
		printf("num  %d\n", num);
	}
	
	return ;
}

int main()
{
	FILE *memFp = fopen("/proc/meminfo", "rb");
	if (!memFp)
	{
		return -1;
	}

	calcMem(memFp);

	fclose(memFp);
	return 0;
}


