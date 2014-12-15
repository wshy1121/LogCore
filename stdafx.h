#ifndef __STDAFX_H
#define __STDAFX_H

#ifndef WIN32
#include <unistd.h>
#include <pthread.h>
#include "platform_base.h"
#include "thread_base.h"
#include "mem_base.h"
#else
#include <windows.h>
#endif

#endif  //__STDAFX_H

