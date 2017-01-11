#ifndef _PLATFORM_BASE_H
#define _PLATFORM_BASE_H
#include <stdio.h>
#include <time.h>
typedef struct TimeB 
{
	time_t time;
	unsigned short int millitm;
	short int timezone;
	short int dstflag;
}TimeB;

#endif

