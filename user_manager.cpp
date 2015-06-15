#include "stdafx.h"
#include "user_manager.h"
#include "link_tool.h"
#include "safe_server.h"
#include "Sqlite/SqliteManager.h"


using namespace base;

extern CPthreadMutex g_insMutexCalc;

CClientInf::CClientInf():	m_isLogined(false),
							m_userName(""), m_passWord(""), m_logPath(""), 
							m_traceFileInf(NULL)
{	trace_worker();
}

CClientInf::~CClientInf()
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


bool CUserManager::login(char *userName, char *passWord, CClientInf *clientInf)
{	trace_worker();
	if (!clientInf)
	{
		return false;
	}
	if (!initClientInf(userName, passWord, clientInf))
	{	trace_printf("NULL");
		return false;
	}
	clientInf->m_isLogined = true;
	return true;
}

bool CUserManager::logout(CClientInf *clientInf)
{	trace_worker();
	if (!clientInf)
	{	trace_printf("NULL");
		return false;
	}
	clientInf->m_isLogined = false;
	trace_printf("NULL");	
	return true;
}

bool CUserManager::isLogined(CClientInf *clientInf)
{	trace_worker();
	trace_printf("clientInf->m_isLogined  %d", clientInf->m_isLogined);
	return clientInf->m_isLogined;
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


bool CUserManager::initClientInf(char *userName, char *passWord, CClientInf *clientInf)
{	trace_worker();

	CppSQLite3Query query = CSqliteManager::instance()->execQuery("select logpath from userinf where name='%s' and password='%s'", userName, passWord);
	if(query.eof())
	{	trace_printf("NULL");
		return false;
	}
	clientInf->m_userName = userName;
	clientInf->m_passWord = passWord;
	clientInf->m_logPath = query.fieldValue("logpath");
	trace_printf("userInf->m_logPath  %s", clientInf->m_logPath.c_str());


	query.finalize();
	return true;
}


bool CUserManager::addClient(int clientId, CClientInf *clientInf)
{	trace_worker();
	CGuardMutex guardMutex(m_userInfMapMutex);
	
	UserInfMap::iterator iter = m_userInfMap.find(clientId);
	if (iter != m_userInfMap.end())
	{	trace_printf("false");
		return false;
	}

	m_userInfMap.insert(std::make_pair(clientId, clientInf));
	trace_printf("true");
	return true;
}

bool CUserManager::removeClient(int clientId)
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

