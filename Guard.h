//  "$Id: Guard.h 22437 2008-12-15 06:01:30Z yuan_shiyong $"
//   (c) Copyright 1992-2005, ZheJiang Dahua Information Technology Stock CO.LTD.
//                            All Rights Reserved
//
//	文 件 名： Guard.h
//	描    述:  守护类
//	修改记录： 2006-7-14 王恒文  <wang_hengwen@dhmail.com> 在原有的文件基础上进行了性能优化
//

#ifndef __GUARD_H__
#define __GUARD_H__

#include <MultiTask/Mutex.h>

///\brief 守护者类，对锁进行保护优化，自动释放锁
class CGuard
{
public:
	///\brief 构造函数
	inline CGuard(CMutex& Mutex)
		:m_Mutex(Mutex)
	{
		m_Mutex.Enter();
	};

	///\brief 析构函数
	inline ~CGuard()
	{
		m_Mutex.Leave();
	};
protected:
private:
	CMutex &m_Mutex;
};

#endif

