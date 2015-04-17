#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include <map>
#include <string>
#include "mem_calc.h"


class CUserManager;
class CUserInf
{
public:
	CUserInf(char *userName, char *passWord);
	~CUserInf();
public:
	void setAccess(bool isAccess);
private:
	std::string m_userName;
	std::string m_passWord;
};
class CUserManager
{
public:
	static CUserManager* instance();	
	bool login(TraceInfoId &traceInfoId, char *userName, char *passWord);
	bool logout(TraceInfoId &traceInfoId);
	bool isLogined(TraceInfoId &traceInfoId);
	bool dealVerify(char *access, int accessLen);
	bool isVerified();
private:	
	CUserManager();
private:
	static  CUserManager* _instance;
	typedef std::map<TraceInfoId, CUserInf *> UserInfMap;
	UserInfMap m_userInfMap;	
	bool m_isVerified;
};
#endif

