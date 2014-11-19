#include <stdarg.h>
#include <stdio.h>


#include "Global.h"
#include "time_calc.h"
#include "link_tool.h"

CCandy::CCandy(int line, char *file_name, char *func_name, int display_level)
{
#if 0
	CTimeCalcInf *pCalcInf = new CTimeCalcInf;
	pCalcInf->m_opr = CTimeCalcInf::e_createCandy;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	pCalcInf->m_funcName = func_name;
	pCalcInf->m_displayLevel = display_level;


	CTimeCalcInfManager::instance()->dealRecvData(pCalcInf);	
#endif
	CTimeCalc *pTimeCalc = new CTimeCalc(line, file_name, func_name, display_level, pthread_self());
	if (pTimeCalc == NULL)
	{
		return ;
	}
}

CCandy::~CCandy()
{
#if 0
	CTimeCalcInf *pCalcInf = new CTimeCalcInf;
	pCalcInf->m_opr = CTimeCalcInf::e_destroyCandy;
	pCalcInf->m_threadId = pthread_self();
	
	CTimeCalcInfManager::instance()->dealRecvData(pCalcInf);
#endif
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->GetTraceInf(pthread_self());
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

#if 0
	CTimeCalcInf *pCalcInf = new CTimeCalcInf;
	pCalcInf->m_opr = CTimeCalcInf::e_insertTrace;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	pCalcInf->m_content = content;

	CTimeCalcInfManager::instance()->dealRecvData(pCalcInf);
#endif
	CTimeCalcManager::instance()->InsertTrace(line, file_name, pthread_self(), content);
	return ;
}

void CBugKiller::DispAll()
{
	 CTimeCalcManager::instance()->DispAll();
}


void CBugKiller::InsertTag(int line, char *file_name, const char* fmt, ...)
{
	char content[1024];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);
	
	CTimeCalcManager::instance()->InsertTag(line, file_name, content);
}

std::string& CBugKiller::getBackTrace(std::string &backTrace)
{
#ifdef WRAP
	CalcMem::instance()->getBackTrace(backTrace);
#endif
	return backTrace;
}
void CBugKiller::printfMemInfMap()
{
	CCandy candy(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, 0);
	CTimeCalcManager::instance()->printfMemInfMap();
}

void CBugKiller::printfStackInfo(int line, char *file_name)
{
	std::string backTrace;
#ifdef WRAP	
	CalcMem::instance()->getBackTrace(backTrace);
#endif
	CTimeCalcManager::instance()->InsertTrace(line, file_name, pthread_self(), backTrace.c_str());
}

void CBugKiller::getStackInfo(std::string &stackInf)
{
	CTimeCalcManager::instance()->getStackInfo(stackInf);
}


void CBugKiller::start()
{
	CTimeCalcManager::instance()->start();
}

void CBugKiller::stop()
{
	CTimeCalcManager::instance()->stop();
}


