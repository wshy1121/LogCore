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
class INetServer;

class IParsePacket
{
public:
	IParsePacket();
	virtual ~IParsePacket();
public:
	virtual bool parsePacket(char *charData, int charDataLen, std::string &packet) = 0;    
    virtual void writeData(char *data, int dataLen);
    void setClientInf(std::shared_ptr<IClientInf> &clientInf);
    void resetClientInf();
	char *charData();
protected:
	unsigned int m_packetPos;
	char *m_packetBuffer;    
	const unsigned int m_maxBufferSize;    
    std::shared_ptr<IClientInf> m_clientInf;
};


class IClientInf
{
public:
	friend class CUserManager;
	friend class CVerifyHandle;
	friend class CVerifyClient;
	friend class CTraceHandle;
	friend class INetServer;
    friend class ITcpServer;
    friend class IUdpServer;
	friend class CLogOprManager;
    friend class CCliManager;
    friend class CTraceManager;
    friend class CCliParsePacket;
	IClientInf(INetServer *netServer);
	virtual ~IClientInf();
public:
	void setAccess(bool isAccess);
	std::string &getLogPath(){return m_logPath;}
	std::string &getfileName(){return m_fileName;}
    void formatInf(std::string &inf);
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
    sockaddr_in m_clientAddr;
    bool m_isBackClient;
    INetServer *m_netServer;
};
class CUserManager
{
public:
	typedef std::map<int, IClientInf *> UserInfMap;
	static CUserManager* instance();	
	bool login(char *userName, char *passWord, IClientInf *clientInf);
	bool logout(IClientInf *clientInf);
	bool isLogined(IClientInf *clientInf);
	bool verifyAccess(char *access, int accessLen, char *accessRep);
	bool isVerified();
	bool addClient(int clientId, IClientInf *clientInf);
	bool removeClient(int clientId);
    void getClientInfs(UserInfMap &userInfMap);
private:	
	CUserManager();
	bool initClientInf(char *userName, char *passWord, IClientInf *clientInf);
private:
	static  CUserManager* _instance;
	bool m_isVerified;	
	base::CPthreadMutex m_userInfMapMutex;
	UserInfMap m_userInfMap;
};
#endif

