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


class IParsePacket
{
public:
	IParsePacket();
	virtual ~IParsePacket();
public:
	bool parsePacket(char &charData, char **pPacket);
	char &charData();
private:	
	void initPacketInf();
private:
	unsigned int m_headCount;
	unsigned int m_tailCount;	
	unsigned int m_packetPos;
	char *m_packetBuffer;
	unsigned int m_curPacketSize;
	const unsigned int m_maxBufferSize;
};


class CClientInf
{
public:
	friend class CUserManager;
	friend class CVerifyHandle;
	friend class CVerifyClient;
	friend class CTraceHandle;
	friend class INetServer;
	friend class CDataWorkManager;
	friend class CLogOprManager;
	CClientInf();
	~CClientInf();
public:
	void setAccess(bool isAccess);
	std::string &getLogPath(){return m_logPath;}
	std::string &getfileName(){return m_fileName;}
private:
	int m_socket;
	int m_clientId;	
	bool m_isLogined;
	std::string m_userName;
	std::string m_passWord;
	std::string m_logPath;
	std::string m_fileName;
	TraceFileInf *m_traceFileInf;
	IParsePacket m_parsePacket;
    std::string m_clientIpAddr;
    int m_clientPort;
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

