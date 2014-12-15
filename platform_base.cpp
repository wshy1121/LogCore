#include "stdafx.h"
#include "platform_base.h"



namespace base
{
int usleep(int micro_second)
{
	return ::usleep(micro_second);
}

}//base



