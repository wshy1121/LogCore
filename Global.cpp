#include <stdarg.h>
#include <stdio.h>


#include "Global.h"
#include "time_calc.h"

CCandy::CCandy(int line, char *file_name, char *func_name, int display_level)
{
	CTimeCalc *pTimeCalc = new CTimeCalc(line, file_name, func_name, display_level);
	if (pTimeCalc == NULL)
	{
		return ;
	}	
}

CCandy::~CCandy()
{

	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->GetTraceInf();
	if (TraceInfo == NULL)
	{
		return ;
	}

	CTimeCalc *pTimeCalc = TraceInfo->calc_list.back();
	delete pTimeCalc;
}






void CBugKiller::InsertTrace(int line, char *file_name, const char* fmt, ...)
{

	char content[4096];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);
	
	CTimeCalcManager::instance()->InsertTrace(line, file_name, content);
	return ;
}

