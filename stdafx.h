// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#ifndef __STDAFX_H
#define __STDAFX_H

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif						

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����

#ifndef WIN32
#include <unistd.h>
#include <pthread.h>
#else
#include <windows.h>
#endif

#include "platform_base.h"
#include "thread_base.h"
#include "mem_base.h"

#endif
// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
