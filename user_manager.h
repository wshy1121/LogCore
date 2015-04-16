#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include <map>
#include <string>
#include "mem_calc.h"


class CUserInf
{
public:
	CUserInf(char *userName, char *passWord);
private:
	std::string m_userName;
	std::string m_passWord;
};
class CUserManager
{
public:
	static CUserManager* instance();	
	bool login(TraceInfoId &traceInfoId, char *userName, char *passWord);
	bool isLogined(TraceInfoId &traceInfoId);
private:	
	CUserManager();
private:
	static  CUserManager* _instance;
	
	std::map<TraceInfoId, CUserInf *> m_userInfMap;
};
#endif

