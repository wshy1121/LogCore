#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include <map>
#include <string>
#include "mem_calc.h"


class CUserManager;
class CUserInf
{
public:
	friend CUserManager;
	CUserInf();
	~CUserInf();
public:
	void setAccess(bool isAccess);
private:
	bool m_isLogined;
	std::string m_userName;
	std::string m_passWord;
	std::string m_logPath;
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
private:	
	CUserManager();
	bool initUserInf(char *userName, char *passWord, CUserInf *userInf);
private:
	static  CUserManager* _instance;
	bool m_isVerified;
};
#endif

