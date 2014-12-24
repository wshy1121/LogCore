#include "Global.h"
#include "net_client.h"
#include <signal.h>

CNetClient netClient("127.0.0.1");
static void main_exit(int sig)
{
	printf("main_exit\n");
	printf("main_exit\n");
	netClient.disConnect();
	
	exit(sig);
}
int main()
{
	signal(SIGINT, &main_exit);
	 
	bool bRet = netClient.connect();
	if (bRet = false)
	{
		printf("connect failed\n");
		return 0;
	}
	//while (1)
	{
		char szText[32];
		netClient.send("1235", 5);
		memset(szText, 0, sizeof(szText));
		netClient.receive(szText,5);
		printf("szText	%s\n", szText);
	}
	netClient.disConnect();
	return 0;
}



