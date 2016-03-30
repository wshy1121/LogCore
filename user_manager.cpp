#include "stdafx.h"
#include "user_manager.h"
#include "link_tool.h"
#include "safe_server.h"
#include "Sqlite/SqliteManager.h"


using namespace base;

extern CPthreadMutex g_insMutexCalc;

IParsePacket::IParsePacket()
:m_headCount(0)
,m_tailCount(0)
,m_packetPos(0)
,m_curPacketSize(0)
,m_maxBufferSize(1024*1024)
{
    m_packetBuffer = new char[m_maxBufferSize];
}

IParsePacket::~IParsePacket()
{
    delete []m_packetBuffer;
}

char &IParsePacket::charData()
{
    return m_packetBuffer[m_packetPos];
}

bool IParsePacket::parsePacket(char &charData, char **pPacket)
{
	if (m_curPacketSize == 0 && m_packetPos > 8)
	{
		memcpy(&m_curPacketSize, m_packetBuffer+4, 4);
	}
	
	if (m_curPacketSize > 0)
	{
		if (m_curPacketSize > m_maxBufferSize || m_packetPos > m_curPacketSize)
		{
			initPacketInf();
		}
	}
	
	switch (charData)
	{
		case '\x7B':
            m_tailCount = 0;
			++m_headCount;
			++m_packetPos;
			break;
		case '\x7D':
            if (m_headCount < 4)
			{
				initPacketInf();
			}
			else
            {
                ++m_tailCount;
            }
            
			++m_packetPos;
			if (m_tailCount >= 4)
			{
					if (m_curPacketSize == m_packetPos)
				{
					char *packet = new char[m_packetPos];
					memcpy(packet, m_packetBuffer + 8, m_packetPos - 12);
					*pPacket = packet;
					initPacketInf();
					return true;
				}
			}
			break;
		default:
            m_tailCount = 0;
			if (m_headCount < 4)
			{
				initPacketInf();
			}				
			++m_packetPos;
			break;
	}
	return false;
}

void IParsePacket::initPacketInf()
{
    m_headCount = 0;
    m_tailCount = 0;
    m_packetPos = 0;
    m_curPacketSize = 0;
}

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

