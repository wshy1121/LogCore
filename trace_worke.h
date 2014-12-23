#ifndef __TRACE_WORKER_H__
#define __TRACE_WORKER_H__

#ifdef WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

class CTraceWorker
{
public:
	CTraceWorker();
	~CTraceWorker();
public:
	bool init();
	bool connect();
	void close();
	bool send( const char * buf, unsigned int len );
private:
	bool init();
private:
	SOCKET m_socketClient;
	bool m_bRead;
};

#endif//__GAME_SOCKET_H__