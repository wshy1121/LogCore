#include "stdafx.h"
#include "user_manager.h"
#include "link_tool.h"
#include "safe_server.h"


using namespace base;

extern CPthreadMutex g_insMutexCalc;

CUserInf::CUserInf(char *userName, char *passWord)
{	trace_worker();
	m_userName = userName;
	m_passWord = passWord;
}

CUserInf::~CUserInf()
{	trace_worker();
}


CUserManager* CUserManager::_instance = NULL;

CUserManager::CUserManager():m_isVerified(false)
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

bool CUserManager::logout(TraceInfoId &traceInfoId)
{	trace_worker();
	trace_printf("m_userInfMap.size()  %d", m_userInfMap.size());
	UserInfMap::iterator pos = m_userInfMap.find(traceInfoId);
	trace_printf("NULL");	
	if (pos == m_userInfMap.end())
	{	trace_printf("NULL");
		return true;
	}
	trace_printf("NULL");

	CUserInf *userInf = pos->second;
	delete userInf;
	trace_printf("NULL");	
	m_userInfMap.erase(pos);
	trace_printf("NULL");	
	return true;
}

bool CUserManager::isLogined(TraceInfoId &traceInfoId)
{	trace_worker();
	bool isLogined = m_userInfMap.find(traceInfoId) != m_userInfMap.end();
	trace_printf("isLogined  %d", isLogined);
	return isLogined;
}


bool CUserManager::verifyAccess(char *access, int accessLen, char *accessRep)
{	trace_worker();
	bool bRet = CSafeServer::instance()->verifyAccess(access, accessLen, accessRep);
	if (!bRet)
	{	trace_printf("NULL");
		return false;
	}
	
	m_isVerified = true;
	trace_printf("m_isVerified  %d", m_isVerified);
	return true;
}


bool CUserManager::isVerified()
{	trace_worker();
	trace_printf("m_isVerified  %d", m_isVerified);
	return m_isVerified;
}


