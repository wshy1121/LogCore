#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include <map>
#include <string>
#include "mem_calc.h"


class CUserManager;
class CVerifyHandle;
class CVerifyClient;
class CTraceHandle;
class CUserInf
{
public:
	friend CUserManager;
	friend CVerifyHandle;
	friend CVerifyClient;
	friend CTraceHandle;
	CUserInf();
	~CUserInf();
public:
	void setAccess(bool isAccess);
	std::string &getLogPath(){return m_logPath;}
	std::string &getfileName(){return m_fileName;}
private:
	bool m_isLogined;
	std::string m_userName;
	std::string m_passWord;
	std::string m_logPath;
	std::string m_fileName;
};
class CUserManager
{
public:
	static CUserManager* instance();	
	bool login(char *userName, char *passWord, CUserInf *userInf);
	bool logout(CUserInf *userInf);
	bool isLogined(CUserInf *userInf);
	bool verifyAccess(char *access, int accessLen, char *accessRep);
	bool isVerified();
	bool addUser(int clientId, CUserInf *userInf);
	bool removeUser(int clientId);
private:	
	CUserManager();
	bool initUserInf(char *userName, char *passWord, CUserInf *userInf);
private:
	static  CUserManager* _instance;
	bool m_isVerified;	
	base::CPthreadMutex m_userInfMapMutex;
	typedef std::map<int, CUserInf *> UserInfMap;
	UserInfMap m_userInfMap;
};
#endif

