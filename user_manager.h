#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include <map>
#include <string>
#include "mem_calc.h"
#include "log_opr.h"


class CUserManager;
class CVerifyHandle;
class CVerifyClient;
class CTraceHandle;
class CNetServer;
class CDataWorkManager;
class CLogOprManager;

class CClientInf
{
public:
	friend CUserManager;
	friend CVerifyHandle;
	friend CVerifyClient;
	friend CTraceHandle;
	friend CNetServer;
	friend CDataWorkManager;
	friend CLogOprManager;
	CClientInf();
	~CClientInf();
public:
	void setAccess(bool isAccess);
	std::string &getLogPath(){return m_logPath;}
	std::string &getfileName(){return m_fileName;}
private:
	SOCKET m_socket;
	int m_clientId;	
	bool m_isLogined;
	std::string m_userName;
	std::string m_passWord;
	std::string m_logPath;
	std::string m_fileName;
	TraceFileInf *m_traceFileInf;
};
class CUserManager
{
public:
	static CUserManager* instance();	
	bool login(char *userName, char *passWord, CClientInf *clientInf);
	bool logout(CClientInf *clientInf);
	bool isLogined(CClientInf *clientInf);
	bool verifyAccess(char *access, int accessLen, char *accessRep);
	bool isVerified();
	bool addClient(int clientId, CClientInf *clientInf);
	bool removeClient(int clientId);
private:	
	CUserManager();
	bool initClientInf(char *userName, char *passWord, CClientInf *clientInf);
private:
	static  CUserManager* _instance;
	bool m_isVerified;	
	base::CPthreadMutex m_userInfMapMutex;
	typedef std::map<int, CClientInf *> UserInfMap;
	UserInfMap m_userInfMap;
};
#endif

