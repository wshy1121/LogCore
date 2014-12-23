#include "net_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


CNetClient::CNetClient(std::string sip) : m_sip(sip)
{
	return ;
}


bool CNetClient::connect()
{
	m_socketClient = socket(AF_INET, SOCK_STREAM, 0);
	if(INVALID_SOCKET == m_socketClient)
	{
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family 		= AF_INET;
	addr.sin_addr.s_addr	= inet_addr(m_sip.c_str());
	addr.sin_port			= htons(8889);
	
	int ret = ::connect(m_socketClient, (struct sockaddr *) & addr, sizeof(sockaddr_in));
	if(SOCKET_ERROR == ret)
	{
		return false;
	}
	return true;
}

bool CNetClient::disConnect()
{
	close(m_socketClient);
}


