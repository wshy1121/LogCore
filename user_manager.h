#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

#include "mem_calc.h"
#include "log_opr.h"


class CUserManager;
class CVerifyHandle;
class CVerifyClient;
class CTraceHandle;
class CLogOprManager;


class IParsePacket
{
public:
	IParsePacket();
	virtual ~IParsePacket();
public:
	virtual bool parsePacket(char &charData, std::string &packet) = 0;    
    virtual void writeData(char *data, int dataLen);
	char &charData();
protected:
	unsigned int m_packetPos;
	char *m_packetBuffer;    
	const unsigned int m_maxBufferSize;
};


class IClientInf
{
public:
	friend class CUserManager;
	friend class CVerifyHandle;
	friend class CVerifyClient;
	friend class CTraceHandle;
	friend class INetServer;
	friend class CLogOprManager;
    friend class CCliManager;
	IClientInf();
	virtual ~IClientInf();
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
    boost::shared_ptr<IParsePacket> m_parsePacket;
    std::string m_clientIpAddr;
    int m_clientPort;
};
class CUserManager
{
public:
	static CUserManager* instance();	
	bool login(char *userName, char *passWord, IClientInf *clientInf);
	bool logout(IClientInf *clientInf);
	bool isLogined(IClientInf *clientInf);
	bool verifyAccess(char *access, int accessLen, char *accessRep);
	bool isVerified();
	bool addClient(int clientId, IClientInf *clientInf);
	bool removeClient(int clientId);
private:	
	CUserManager();
	bool initClientInf(char *userName, char *passWord, IClientInf *clientInf);
private:
	static  CUserManager* _instance;
	bool m_isVerified;	
	base::CPthreadMutex m_userInfMapMutex;
	typedef std::map<int, IClientInf *> UserInfMap;
	UserInfMap m_userInfMap;
};
#endif

