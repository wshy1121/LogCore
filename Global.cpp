#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


#include "Global.h"
#include "time_calc.h"
#include "link_tool.h"

CCandy::CCandy(int line, char *file_name, char *func_name, int display_level)
{

	RECV_DATA *pRecvData = new RECV_DATA;
	CTimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = CTimeCalcInf::e_createCandy;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	pCalcInf->m_funcName = func_name;
	pCalcInf->m_displayLevel = display_level;


	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);	
	return ;
	
#if 0
	CTimeCalc *pTimeCalc = new CTimeCalc(line, file_name, func_name, display_level, pthread_self());
	if (pTimeCalc == NULL)
	{
		return ;
	}
#endif
}

CCandy::~CCandy()
{

	RECV_DATA *pRecvData = new RECV_DATA;
	CTimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = CTimeCalcInf::e_destroyCandy;
	pCalcInf->m_threadId = pthread_self();
	
	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;
#if 0	
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->GetTraceInf();
	if (TraceInfo == NULL)
	{
		return ;
	}

	CTimeCalc *pTimeCalc = TraceInfo->calc_list.back();
	delete pTimeCalc;
#endif
}






void CBugKiller::InsertTrace(int line, char *file_name, const char* fmt, ...)
{

	char content[4096];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);


	RECV_DATA *pRecvData = new RECV_DATA;
	CTimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = CTimeCalcInf::e_insertTrace;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	pCalcInf->m_content = content;

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;

#if 0
	CTimeCalcManager::instance()->InsertTrace(line, file_name, pthread_self(), content);
	return ;
#endif	
}

void CBugKiller::InsertStrOnly(pthread_t threadId, const char* fmt, ...)
{
	char content[4096];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);


	RECV_DATA *pRecvData = new RECV_DATA;
	CTimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = CTimeCalcInf::e_InsertStrOnly;
	pCalcInf->m_threadId = threadId;
	pCalcInf->m_content = content;

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);

	return ;
}

void CBugKiller::DispAll()
{
	RECV_DATA *pRecvData = new RECV_DATA;
	CTimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = CTimeCalcInf::e_dispAll;
	pCalcInf->m_threadId = pthread_self();

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	//CTimeCalcManager::instance()->DispAll();
}


void CBugKiller::InsertTag(int line, char *file_name, const char* fmt, ...)
{
	char content[1024];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);

	RECV_DATA *pRecvData = new RECV_DATA;
	CTimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = CTimeCalcInf::e_insertTag;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	pCalcInf->m_content = content;

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;

#if 0
	CTimeCalcManager::instance()->InsertTag(line, file_name, content);
#endif	
}

std::string& CBugKiller::getBackTrace(std::string &backTrace)
{
#ifdef WRAP
	CalcMemManager::instance()->getBackTrace(backTrace);
#endif
	return backTrace;
}
void CBugKiller::printfMemInfMap()
{
	assert(0);
	CCandy candy(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, 0);
	CTimeCalcManager::instance()->printfMemInfMap();
}

void CBugKiller::printfStackInfo(int line, char *file_name)
{
	std::string backTrace;
#ifdef WRAP	
	CalcMemManager::instance()->getBackTrace(backTrace);
#endif
	InsertTrace(line, file_name, backTrace.c_str());
}

void CBugKiller::getStackInfo(std::string &stackInf)
{
	assert(0);
	CTimeCalcManager::instance()->getStackInfo(stackInf);
}


void CBugKiller::start()
{
	CalcMemManager::instance();	
	ThreadQueue::instance();
	CTimeCalcInfManager::instance();
	CTimeCalcManager::instance()->start();
}

void CBugKiller::stop()
{
	CTimeCalcManager::instance()->stop();
}


