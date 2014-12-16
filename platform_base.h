#ifndef __PLATFORM_BASE_H
#define __PLATFORM_BASE_H

typedef struct TimeB {
	time_t         time;
	unsigned short millitm;
	short          timezone;
	short          dstflag;
}TimeB;
namespace base
{
int usleep(int micro_second);
int ftime(TimeB *tp);
}//base



#endif //__THREAD_BASE_H

