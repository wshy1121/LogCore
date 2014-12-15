#include "stdafx.h"
#include "platform_base.h"



namespace base
{
#if WIN32
int usleep(int micro_second)
{
	Sleep(micro_second);
	return 0;
}
#else
int usleep(int micro_second)
{
	return ::usleep(micro_second);
}
#endif
}//base



