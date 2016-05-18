#include "stdafx.h"
#include "user_manager.h"
#include "link_tool.h"
#include "safe_server.h"
#include "Sqlite/SqliteManager.h"
#include "trace_server.h"

using namespace base;

extern CPthreadMutex g_insMutexCalc;

IParsePacket::IParsePacket()
:m_packetPos(0)
,m_maxBufferSize(1024*1024)
{
    m_packetBuffer = new char[m_maxBufferSize];
}

IParsePacket::~IParsePacket()
{
    delete []m_packetBuffer;
}

void IParsePacket::writeData(char *data, int dataLen)
{
}

void IParsePacket::setClientInf(std::shared_ptr<IClientInf> &clientInf)
{   trace_worker();
    m_clientInf = clientInf;
}

void IParsePacket::resetClientInf()
{   trace_worker();
    m_clientInf.reset();
}

char *IParsePacket::charData()
{
    return m_packetBuffer + m_packetPos;
}

IClientInf::IClientInf(INetServer *netServer)
:m_isLogined(false)
,m_userName("")
,m_passWord("")
,m_logPath("")
,m_traceFileInf(NULL)
,m_isBackClient(false)
,m_netServer(netServer)
{	trace_worker();
}

IClientInf::~IClientInf()
{	trace_worker();
}

void IClientInf::formatInf(std::string &inf)
{   trace_worker();
    char tmpChars[128];

    inf = "";
    base::snprintf(tmpChars, sizeof(tmpChars), "%d", m_clientId);
    inf += tmpChars;
    inf += "  ";
    base::snprintf(tmpChars, sizeof(tmpChars), "%d", m_socket);
    inf += tmpChars;
    inf += "  ";
    inf += inet_ntoa(m_clientAddr.sin_addr);
    inf += "  ";
    base::snprintf(tmpChars, sizeof(tmpChars), "%d", ntohs(m_clientAddr.sin_port));
    inf += tmpChars;
    inf += "  ";
    base::snprintf(tmpChars, sizeof(tmpChars), "%d", m_clientId);
    inf += tmpChars;
    inf += "  ";    
    if (m_traceFileInf)
    {        
        inf += m_traceFileInf->m_fileAddTime->getPath();
    }
    inf += "  ";
}

CUserManager* CUserManager::_instance = NULL;

//WSHY
CUserManager::CUserManager():m_isVerified(true)
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


bool CUserManager::login(char *userName, char *passWord, IClientInf *clientInf)
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

bool CUserManager::logout(IClientInf *clientInf)
{	trace_worker();
	if (!clientInf)
	{	trace_printf("NULL");
		return false;
	}
	clientInf->m_isLogined = false;
	trace_printf("NULL");	
	return true;
}

bool CUserManager::isLogined(IClientInf *clientInf)
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


bool CUserManager::initClientInf(char *userName, char *passWord, IClientInf *clientInf)
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


bool CUserManager::addClient(int clientId, IClientInf *clientInf)
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

void CUserManager::getClientInfs(UserInfMap &userInfMap)
{
    CGuardMutex guardMutex(m_userInfMapMutex);
    userInfMap = m_userInfMap;
}

