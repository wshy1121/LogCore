

#include "Base/DEF.h"
#include  "stdio.h"
#include "MultiTask/Mutex.h"
#include "Base/Time.h"
#include "assert.h"
#include "MultiTask/Thread.h"

#define MY_MAGIC_NUM 		0xabababab		//写在申请内存头上的特殊数据标记
#define MALLOC_MEM_FLAG		0xAF			//申请过的内存块标记
#define FREE_MEM_FLAG		0xFA			//释放的内存块标记

#define PC_BUF_SIZE 16						//记录的堆栈长度


uint log_index;
uint g_mem_total;

int g_malloc_c;
int start_mem_trace = 0;
int start_back_trace = 1;
int g_free_unmatched;
int g_malloc_times = 0;
int g_free_times = 0;
int g_realloc_free = 0;
int g_realloc_times = 0;
uint64 g_malloc1_size = 0;
uint64 g_free1_size = 0;

CMutex mem_log_mutex;


struct mem_log
{
    void *ptr;
    int size;
};


struct mem_log_ex
{
    void *ptr;
    int size;
	int next_free;
};

// \brief 内存
enum MemSize
{
	less_10,
	between_10_20,
	between_20_40,
	between_40_100,
	between_100_1k,
	between_1k_16k,
	upper_16k,
	sizeNum,
};


uint memCheck[sizeNum] = {0};

//在每一块申请的内存前面加的头信息，有特殊数据标记，调用线程堆栈信息等
//头结构大小4 + 4 + 4*16 + 8 + 4
typedef struct my_mem_head
{
    //int pad[2];
    int size;
    int index;
    int pc[PC_BUF_SIZE];			//调用线程堆栈信息
    
    //int magic1;
    //int magic2;
    uint64 time;
    int magic;						//标记数据
}my_mem_head;


typedef struct my_mem_tail
{
    //int pad[1];
    //int size;
}my_mem_tail;

typedef struct my_thread_mem
{
	int threadID;
	uint64 mallocSize;
	uint64 freeSize;
}my_thread_mem;




#ifdef DEBUG_MEMCHECK

typedef struct SizeBetween
{
	uint minSize;
	uint maxSize;
}_sizeBetween;

static struct SizeBetween memBetween[sizeNum] =
{
		{0,10},
		{10,20},
		{20,40},
		{40,100},
		{100,1024},
		{1024, 16*1024},
		{16*1024, -1}
};

#define MEM_LOG_SIZE  (1024*1024*10)	//记录的内存块个数

struct mem_log_ex my_log[MEM_LOG_SIZE] = {0,};	// 申请内存的堆栈信息
struct mem_log my_copy_log[MEM_LOG_SIZE] = {0,};	

int first_log_add = 0;
int free_list_head = 0;

//查找内存地址是否已经在申请的记录当中
int find_ptr(void *ptr)
{
	int ret = 0;
	for (int i = 0; i < MEM_LOG_SIZE; i++)
	{
		if (0 == my_log[i].ptr)
		{
			continue;
		}
			
		if(ptr == (char *)my_log[i].ptr + sizeof(my_mem_head))
		{
			tracef("find ptr %p\n", ptr);
			ret = 1;
			break;
		}
	}

	return ret;
}

void combin_memLog()		// 合并堆栈信息相同的项
{
	uint64 now = CTime::getCurrentMilliSecond();
	memset(my_copy_log, 0, sizeof(my_copy_log));
	for (int i = 0; i < MEM_LOG_SIZE; i++)
	{
		if (my_log[i].ptr != NULL && my_log[i].size > 0)
		{
			my_mem_head *phead = (my_mem_head *)my_log[i].ptr;
			if (now - phead->time < 1000 * 60)	// 大于一分钟才统计
			{
				continue;
			}
		
			for (int j = 0; j < MEM_LOG_SIZE; j++)
			{
				if (my_copy_log[j].ptr == NULL)
				{
					my_copy_log[j].ptr = my_log[i].ptr;
					my_copy_log[j].size = my_log[i].size;
					break;
				}

				// 合并
				my_mem_head *p_copyhead = (my_mem_head *)my_copy_log[j].ptr;
				int k = 0;
				for (k = 0; k < PC_BUF_SIZE; k++)
				{
					if (phead->pc[k] != p_copyhead->pc[k])
					{
						break;
					}
				}

				if (k >= PC_BUF_SIZE)// 相同，则合并
				{
					my_copy_log[j].size += my_log[i].size;
					break;
				}
				
			}
		}
	}
}


#define THREAD_NUM 200
struct my_thread_mem mem_thread[THREAD_NUM] = {{0,}, };


void add_mem_for_thread(int size)
{
	int threadID = CThread::getID();

	int i = 0;
	int firstAvaliableIndex = -1;
	for (i = 0; i < THREAD_NUM; i++)
	{
		if (mem_thread[i].threadID == threadID)
		{
			mem_thread[i].mallocSize += size;
			break;
		}

		if (mem_thread[i].threadID == 0 && firstAvaliableIndex == -1)
		{
			firstAvaliableIndex = i;	
		}
	}

	if (i == THREAD_NUM && firstAvaliableIndex != -1)
	{
		mem_thread[firstAvaliableIndex].threadID = threadID;
		mem_thread[firstAvaliableIndex].mallocSize = size;
	}
}

void delete_mem_for_thread(int size)
{
	int threadID = CThread::getID();
	
	int i = 0;
	for (i = 0; i < THREAD_NUM; i++)
	{
		if (mem_thread[i].threadID == threadID)
		{
			mem_thread[i].freeSize += size;
			break;
		}
	}
}

void my_log_stat()
{
	#if 1
    int total = 0, hold = 0;
    uint64 now = CTime::getCurrentMilliSecond();

    if (start_mem_trace == 0)
    {
        trace("mllloc:%d free:%d realloc:%d\n", 
			g_malloc_times,
            g_free_times, 
            g_realloc_times);
        return ;
    }
    
    for (int i = 0; i < MEM_LOG_SIZE; i++)
    {
        if (my_log[i].ptr && my_log[i].size)
        {
            total++;

            my_mem_head *phead = (my_mem_head *)my_log[i].ptr;
            if (now - phead->time > 1000 * 60)
            {
                hold++;
            }
        }
    }

    trace("%d meme block in use, long time:%d\n", total, hold);

#ifdef FILE_LOG_MEM    
    fflush(fp);
#endif

    #endif
}



extern "C" {

#define UNW_LOCAL_ONLY
#include "Unwind/libunwind-arm.h"

void my_bt(int *list, int size)
{
    int i = 0;
    if (start_back_trace)
    {
        unw_cursor_t    cursor;
        unw_context_t   uc;
        unw_proc_info_t pip;
        unw_word_t      ip, sp;
        unw_word_t      off;

        unw_getcontext(&uc);
        unw_init_local(&cursor, &uc);

        while (unw_step(&cursor) > 0 && i < size) 
		{
        /*
            if (unw_is_signal_frame(&cursor) > 0) 
            { 
                for (i = 0; i < 16; i++) 
                {
                    if (i%4 == 0) 
                    {
                        OUT("\n-----------------------------------------------------------------\n");
                    }

                    if (!unw_get_reg(&cursor, UNW_ARM_R0 + i, &reg)) 
                    {
                        OUT("  R[%02d]=%08lx", i, (long)reg);
                    }
                    else
                    {
                        OUT("  R[%02d]=[--------]", i);
                    }
                }
                OUT("\n-----------------------------------------------------------------\n");
            }
        */
            unw_get_reg(&cursor, UNW_REG_IP, &ip);
		
            //unw_get_reg(&cursor, UNW_REG_SP, &sp);
            //printf("PC:[0x%08lx]\n", (long)ip);

            list[i++] = (int)ip;

        } /* while(1) */
        
    }
}




    
int my_add_log(void *ptr, size_t size)
{
	if (first_log_add == 0)//只初始化一次，初始化后会把标志置1
	{
		//为避免每次都从头找一遍，把空闲的内存首地址位置用first_log_add保存
		//初始化为0，每个都指向下一个内存块
		for (int i = 0; i < MEM_LOG_SIZE; i++)
		{
			my_log[i].next_free = i + 1;
		}

		//最后一块写成-1
		my_log[MEM_LOG_SIZE - 1].next_free = -1;
		
		first_log_add = 1;
	}


    if (start_mem_trace == 0)
    {
        return -1;
    }
        
    int ret = -1;

	if (free_list_head != -1)
	{
        if (my_log[free_list_head].ptr != 0)
        {
        	printf("error free list error. head;%d\n", free_list_head);
			return -1;
        }		

        my_log[free_list_head].ptr = ptr;
        my_log[free_list_head].size = size;	

		ret = free_list_head;
		free_list_head = my_log[free_list_head].next_free;
		
	}
	else
	{
    	tracef("error free list head is -1\n");
	}

  	#if 0
    for (i = 0; i < MEM_LOG_SIZE; i++)
    {
        log_index = (log_index + i) % MEM_LOG_SIZE;
        if (my_log[log_index].ptr == 0)
        {
            my_log[log_index].ptr = ptr;
            my_log[log_index].size = size;

            ret = log_index;
            break;
        }
    }

	
    if (i == MEM_LOG_SIZE)
    {
        trace("add log failed, not free space, ptr:%p\n", ptr);
        ret = -1;
    }
	#endif
	
    return ret;
}


void my_remove_log(int index, void *ptr, int size)
{
    if (start_mem_trace == 0 || index < 0 || index >= MEM_LOG_SIZE)
    {
        return;
    }

    if (my_log[index].ptr == ptr && my_log[index].size == size)
    {
        my_log[index].ptr = 0;
        my_log[index].size = 0;

		if (free_list_head == -1)
		{
	    	tracef("my_remove_log error free list head is -1\n");
			return;
		}

		my_log[index].next_free = free_list_head;
		free_list_head = index;

	}
    else
    {
        trace("remove log failed index:%d, ptr:%p,%d match record:%p,%d\n", 
			index,
            ptr,
            size,
            my_log[index].ptr,
            my_log[index].size
            );
    }
}

int get_mem_head_info(void *ptr, my_mem_head *ptrHead)
{
    my_mem_head *start = (my_mem_head*)ptr;
    start -= 1;

    if (start->magic == MY_MAGIC_NUM)
    {
        ptrHead->magic = MY_MAGIC_NUM;
        ptrHead->size = start->size;
        return 1;
    }
    else
    {
        //printf("get_mem_head_info head check failed.\n");
        return 0;
    }
}

void *del_mem_head(void *ptr)
{
    my_mem_head *start = (my_mem_head*)ptr;
    start -= 1;
    if (start->magic == MY_MAGIC_NUM)
    {
        g_mem_total -= start->size;
        
        my_remove_log(start->index, start, start->size);

        memset(start, 0, sizeof(my_mem_head));
        return start;
    }
    else
    {
        printf("del_mem_head head check failed.\n");
        assert(0);
    }

    g_free_unmatched++;
    return ptr;
}



void* add_mem_head(void *ptr, size_t size, int *pc_buf)
{
    my_mem_head *start = (my_mem_head*)ptr;

    for (int i = 0; i < PC_BUF_SIZE; i++)
    {
        start->pc[i] = pc_buf[i];
    }

    start->magic = MY_MAGIC_NUM;
    start->size = size;
    start->index = my_add_log(ptr, size);
    start->time = CTime::getCurrentMilliSecond();
    g_mem_total += size;
    return start + 1;
}



void *__real_malloc(int c);

void *__wrap_malloc(int c)
{
    if (start_mem_trace == 0)
    {
        //g_malloc_times++;
        
        return __real_malloc(c);
    }

    //g_malloc_c++;
    //return __real_malloc(c);

#if 1
    //mem_log_mutex.Enter();

    void *sys_addr, *user_addr;
    int pc_buf[PC_BUF_SIZE];

    memset(pc_buf, 0, sizeof(pc_buf));
    my_bt(pc_buf, PC_BUF_SIZE);

    sys_addr =__real_malloc(c + sizeof(my_mem_head));

    //check_raw_mem(sys_addr, c + sizeof(my_mem_head)+sizeof(my_mem_tail));

    //memset(sys_addr, 0xff, c + sizeof(my_mem_head)+sizeof(my_mem_tail));

	mem_log_mutex.Enter();
	g_malloc_times++;
	g_malloc1_size += c;

	for (int index = 0; index < sizeNum; index++)
	{
		if (c > memBetween[index].minSize && c <= memBetween[index].maxSize)
		{
			memCheck[index]++;
			break;
		}
	}
	
	user_addr = add_mem_head(sys_addr, c, pc_buf);
	add_mem_for_thread(c);
	mem_log_mutex.Leave();

	//标记一下已经申请过的内存
	memset((char *)user_addr, MALLOC_MEM_FLAG, c);

    //mem_log_mutex.Leave();
    return user_addr;
#endif
}


void __real_free(void* c);

void __wrap_free(void* user_addr)
{
    if (start_mem_trace == 0)
    {
       // g_free_times++;
        __real_free(user_addr);
        return;
    }

    //g_malloc_c++;
    //__real_free(user_addr);
    //return;

#if 1
    //CGuard guard(mem_log_mutex);
    
    void *sys_addr = user_addr;
    if (user_addr != 0)
    {
        my_mem_head head_info;
        int check_ret = get_mem_head_info(user_addr, &head_info);
        if (check_ret)
        {
            
			mem_log_mutex.Enter();
			g_free_times++;
			g_free1_size += head_info.size;
			sys_addr = del_mem_head(user_addr);
			delete_mem_for_thread(head_info.size);
			mem_log_mutex.Leave();

			//标记一下已经释放的内存
			memset((char *)sys_addr, FREE_MEM_FLAG, head_info.size + sizeof(my_mem_head));
        }
    }
    
    __real_free(sys_addr);
#endif
}


void *__real_realloc(void* c, int size);

void *__wrap_realloc(void* user_addr, int size)
{
    if (start_mem_trace == 0)
    {
        g_realloc_times++;
        return __real_realloc(user_addr, size);
    }


    //g_malloc_c++;
    //    return __real_realloc(user_addr, size);

#if 1
    //mem_log_mutex.Enter();
    
    void *sys_addr, *new_addr;

    int oldSize = 0;

    if (user_addr != 0)
    {
        g_realloc_free++;
        my_mem_head head_info;
        int check_ret = get_mem_head_info(user_addr, &head_info);
        if (check_ret == 0)
        {
            return __real_realloc(user_addr, size);
        }
        else
        {
            oldSize = head_info.size;
        }
    }

    g_realloc_times++;
    //mem_log_mutex.Leave();
    
    new_addr = malloc(size);
    int copy_size = size;
    assert(oldSize >= 0);
    assert(size >= 0);
    
    if (size > oldSize)
        copy_size = oldSize;

    if (user_addr && copy_size)
        memcpy(new_addr, user_addr, copy_size);

    free(user_addr);

    #if 0
    sys_addr = __real_realloc(sys_addr, size + sizeof(my_mem_head));

    int pc_buf[PC_BUF_SIZE];
    memset(pc_buf, 0, sizeof(pc_buf));
    my_bt(pc_buf, PC_BUF_SIZE);
    
    new_addr = add_mem_head(sys_addr, size, pc_buf);
    #endif
    
    return new_addr;
#endif
}

void *__real_calloc(size_t nelem, size_t elsize);


void *__wrap_calloc(size_t nelem, size_t elsize)
{
    if (start_mem_trace == 0)
    {
        return __real_calloc(nelem, elsize);
    }

    g_malloc_c++;
    return __real_calloc(nelem, elsize);

	#if 0
	size_t	size = nelem * elsize;
    void * allocation;

    allocation = malloc(size);
    memset(allocation, 0, size);

	return allocation;
    #endif
}
/*
void *__real_memalign(size_t alignment, size_t userSize);

void *__wrap_memalign(size_t alignment, size_t userSize)
{
    g_malloc_c++;
    //g_memalign_times++;
	void *ptr = __real_memalign(alignment, userSize);
    return ptr;
}
*/
} // extern "C"
#endif

//打印线程堆栈信息
void dump_mem_for_thread()
{
#ifdef DEBUG_MEMCHECK
	for (int i = 0; i < THREAD_NUM; i++)
	{
		if (mem_thread[i].threadID != 0)
		{
			infof(">>>>>>>>threadID: %d, malloc mem: %llu free mem: %llu m >>>>>>>\n", 
				mem_thread[i].threadID, mem_thread[i].mallocSize/1024/1024, 
				mem_thread[i].freeSize /1024 /1024);
		}
	}


	for (int i = 0; i < sizeNum; i++)
	{
		infof(">>>>>>mem between %u and %u>>>>num: %u \n",
			memBetween[i].minSize, memBetween[i].maxSize, memCheck[i]);
	}
#endif
}

void my_log_mem_trace()
{

#ifdef DEBUG_MEMCHECK
    int total = 0, hold = 0;
    uint64 now = CTime::getCurrentMilliSecond();

    FILE *fp = fopen("/home/mlog.txt", "w");
    if (!fp)
    {
        trace("Can not open file\n");
        return;
    }

	combin_memLog();	// 合并堆栈
/*
    char buf[128];
    for (int i = 0; i < MEM_LOG_SIZE; i++)
    {
        if (my_log[i].ptr && my_log[i].size)
        {
            my_mem_head *phead = (my_mem_head *)my_log[i].ptr;
            if (now - phead->time > 1000 * 60)
            {
                sprintf(buf, "pointer:%p backtrace: size:%d \n", phead, phead->size);
                fputs(buf, fp);
                for (int j = 0; j < PC_BUF_SIZE; j++)
                {
                    if (phead->pc[j])
                    {
                        sprintf(buf, "%p\n", (void *)phead->pc[j]);
                        fputs(buf, fp);
                    }
                }
                hold++;
            }

            
        }
    }
*/

	char buf[128] = {0};
    for (int i = 0; i < MEM_LOG_SIZE; i++)
    {
        if (my_copy_log[i].ptr && my_copy_log[i].size)
        {
            my_mem_head *phead = (my_mem_head *)my_copy_log[i].ptr;
            //if (now - phead->time > 1000 * 60)
           	//{
            	sprintf(buf, "pointer:%p backtrace: size:%d \n", phead, phead->size);
            	fputs(buf, fp);
            	for (int j = 0; j < PC_BUF_SIZE; j++)
            	{
                		if (phead->pc[j])
                		{
                    		sprintf(buf, "%p\n", (void *)phead->pc[j]);
                    		fputs(buf, fp);
                		}
            	}
            	hold++;
          	//}
        }
		else
		{
			break;
		}
    }

    fputs("\n", fp);
    fclose(fp);
#endif
}
