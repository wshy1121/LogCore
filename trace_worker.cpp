#include "defs.h"


CTraceWorker::CTraceWorker() : m_socketClient(0)
{
	init();
}

CTraceWorker::~CTraceWorker()
{
#ifdef WIN32
	WSACleanup();
#endif
}

bool CTraceWorker::init()
{
#ifdef WIN32
	WSADATA data;
	WORD v = MAKEWORD( 2,0 );
	int ret = WSAStartup( v, & data );
	if( 0 != ret )
	{
		return false;
	}
#endif	
	return true;
}

bool CTraceWorker::connect()
{
	m_socketClient = socket( AF_INET, SOCK_STREAM, 0 );
	if( INVALID_SOCKET == m_socketClient )
	{
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family			= AF_INET;
	addr.sin_addr.s_addr	= inet_addr( "127.0.0.1" );
	addr.sin_port			= htons( 8889 );
	
	int ret = ::connect( m_socketClient, ( struct sockaddr * ) & addr, sizeof( sockaddr_in ) );
	if( SOCKET_ERROR == ret )
	{
		return false;
	}

	return true;
}

void CTraceWorker::close()
{
	closesocket( m_socketClient );
}

bool CTraceWorker::send( const char * buf, unsigned int len )
{
	int ret = ::send( m_socketClient, buf, len, 0 );
	if( SOCKET_ERROR == ret )
	{
		return false;
	}
	//*
	int _len = ::recv( m_socketClient, m_bufRecv, sizeof( m_bufRecv ), 0 );
	if( 0 >= _len )
	{
		this->close();
	}
	//*/
	return true;
}

char * CTraceWorker::getBuf()
{
	m_bRead = false;
	return m_bufRecv;
}

void CTraceWorker::run()
{
	fd_set fdRead;
	FD_ZERO( & fdRead );
	FD_SET( m_socketClient, & fdRead );
	struct timeval cctv = {0, 50};
	int count = select( 100, & fdRead, NULL, NULL, & cctv );
	while( 0 < count )
	{
		if( FD_ISSET( m_socketClient, & fdRead ) )
		{
			memset( m_bufRecv, 0, sizeof( m_bufRecv ) );
			int _len = ::recv( m_socketClient, m_bufRecv, sizeof( m_bufRecv ), 0 );
			if( 0 >= _len )
			{
				this->close();
				return;
			}
			m_bRead = true;
		}
		--count;
	}
}