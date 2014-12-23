#include "Global.h"

int main()
{
	CBugKiller::startServer();
	while (1)
	{
		sleep(1);
	}
	return 0;
}



