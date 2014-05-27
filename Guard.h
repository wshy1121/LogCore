//  "$Id: Guard.h 22437 2008-12-15 06:01:30Z yuan_shiyong $"
//   (c) Copyright 1992-2005, ZheJiang Dahua Information Technology Stock CO.LTD.
//                            All Rights Reserved
//
//	�� �� ���� Guard.h
//	��    ��:  �ػ���
//	�޸ļ�¼�� 2006-7-14 ������  <wang_hengwen@dhmail.com> ��ԭ�е��ļ������Ͻ����������Ż�
//

#ifndef __GUARD_H__
#define __GUARD_H__

#include <MultiTask/Mutex.h>

///\brief �ػ����࣬�������б����Ż����Զ��ͷ���
class CGuard
{
public:
	///\brief ���캯��
	inline CGuard(CMutex& Mutex)
		:m_Mutex(Mutex)
	{
		m_Mutex.Enter();
	};

	///\brief ��������
	inline ~CGuard()
	{
		m_Mutex.Leave();
	};
protected:
private:
	CMutex &m_Mutex;
};

#endif

