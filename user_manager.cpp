#include "stdafx.h"
#include "user_manager.h"
#include "link_tool.h"


using namespace base;

extern CPthreadMutex g_insMutexCalc;


CUserManager* CUserManager::_instance = NULL;

CUserManager::CUserManager()
{
	return ;
}

CUserManager* CUserManager::instance() 
{	
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CUserManager;
		}
	}
	return _instance;
}


bool CUserManager::login(TraceInfoId &traceInfoId, char *userName, char *passWord)
{	trace_worker();
	if (m_userInfMap.find(traceInfoId) != m_userInfMap.end())
	{
		trace_printf("has logined");
		return true;
	}

	CUserInf *userInf = new CUserInf(userName, passWord);
	m_userInfMap.insert(std::make_pair(traceInfoId, userInf));
	return true;
}
bool CUserManager::isLogined(TraceInfoId &traceInfoId)
{	trace_worker();
	bool isLogined = m_userInfMap.find(traceInfoId) != m_userInfMap.end();
	trace_printf("isLogined  %d", isLogined);
	return isLogined;
}

CUserInf::CUserInf(char *userName, char *passWord)
{
	m_userName = userName;
	m_passWord = passWord;
}

