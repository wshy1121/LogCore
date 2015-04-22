#include "stdafx.h"
#include "user_manager.h"
#include "link_tool.h"
#include "safe_server.h"
#include "Sqlite\SqliteManager.h"


using namespace base;

extern CPthreadMutex g_insMutexCalc;

CUserInf::CUserInf():m_userName(""), m_passWord(""), m_logPath(""), m_isLogined(false)
{	trace_worker();
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


bool CUserManager::login(char *userName, char *passWord, CUserInf *userInf)
{	trace_worker();
	if (!userInf)
	{
		return false;
	}
	if (!initUserInf(userName, passWord, userInf))
	{	trace_printf("NULL");
		return false;
	}
	userInf->m_isLogined = true;
	return true;
}

bool CUserManager::logout(CUserInf *userInf)
{	trace_worker();
	if (!userInf)
	{	trace_printf("NULL");
		return false;
	}
	userInf->m_isLogined = true;
	trace_printf("NULL");	
	return true;
}

bool CUserManager::isLogined(CUserInf *userInf)
{	trace_worker();
	trace_printf("userInf->m_isLogined  %d", userInf->m_isLogined);
	return userInf->m_isLogined;
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


bool CUserManager::initUserInf(char *userName, char *passWord, CUserInf *userInf)
{	trace_worker();

	CppSQLite3Query query = CSqliteManager::instance()->execQuery("select logpath from userinf where name='%s' and password='%s'", userName, passWord);
	if(query.eof())
	{	trace_printf("NULL");
		return false;
	}
	userInf->m_userName = userName;
	userInf->m_passWord = passWord;
	userInf->m_logPath = query.fieldValue("logpath");
	trace_printf("userInf->m_logPath  %s", userInf->m_logPath.c_str());


	query.finalize();
	return true;
}


bool CUserManager::addUser(int clientId, CUserInf *userInf)
{	trace_worker();
	CGuardMutex guardMutex(m_userInfMapMutex);
	
	UserInfMap::iterator iter = m_userInfMap.find(clientId);
	if (iter != m_userInfMap.end())
	{	trace_printf("false");
		return false;
	}

	m_userInfMap.insert(std::make_pair(clientId, userInf));
	trace_printf("true");
	return true;
}

bool CUserManager::removeUser(int clientId)
{	trace_worker();
	CGuardMutex guardMutex(m_userInfMapMutex);
	
	UserInfMap::iterator iter = m_userInfMap.find(clientId);
	if (iter == m_userInfMap.end())
	{	trace_printf("false");
		return false;
	}

	m_userInfMap.erase(iter);
	trace_printf("true");	
	return true;
}

