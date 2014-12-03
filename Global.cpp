#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "Global.h"
#include "log_opr.h"
#include "time_calc.h"
#include "mem_calc.h"
#include "mem_check.h"


CCandy::CCandy(int line, char *file_name, char *func_name, int display_level)
{

	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData();
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = TimeCalcInf::e_createCandy;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	pCalcInf->m_funcName = func_name;
	pCalcInf->m_displayLevel = display_level;


	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);	
	return ;
}

CCandy::~CCandy()
{

	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData();
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = TimeCalcInf::e_destroyCandy;
	pCalcInf->m_threadId = pthread_self();
	
	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;
}

void CBugKiller::InsertTrace(int line, char *file_name, const char* fmt, ...)
{

	char content[4096];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);


	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData(strlen(content));
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = TimeCalcInf::e_insertTrace;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	strcpy(pCalcInf->m_pContent, content);

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;
}

void CBugKiller::InsertHex(int line, char *file_name, char *psBuf, int nBufLen)
{
	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData(nBufLen);
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = TimeCalcInf::e_insertHex;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	memcpy(pCalcInf->m_pContent, psBuf, nBufLen);

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;
}

void CBugKiller::InsertStrOnly(pthread_t threadId, const char* fmt, ...)
{
	char content[4096];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);


	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData(strlen(content));
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = TimeCalcInf::e_InsertStrOnly;
	pCalcInf->m_threadId = threadId;
	strcpy(pCalcInf->m_pContent, content);

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);

	return ;
}

void CBugKiller::DispAll()
{
	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData();
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = TimeCalcInf::e_dispAll;
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

	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData(strlen(content));
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = TimeCalcInf::e_insertTag;
	pCalcInf->m_threadId = pthread_self();
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	strcpy(pCalcInf->m_pContent, content);

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;	
}

std::string& CBugKiller::getBackTrace(std::string &backTrace)
{
#ifdef WRAP
	CGuardEnable guard(e_Mem);
	if (guard.needReturn())
	{
		return backTrace;
	}

	CalcMem::instance()->getBackTrace(backTrace);
#endif
	return backTrace;
}
void CBugKiller::printfMemInfMap()
{
	RECV_DATA *pRecvData = CTimeCalcInfManager::instance()->createRecvData();
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_opr = TimeCalcInf::e_printfMemInfMap;
	pCalcInf->m_threadId = pthread_self();

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
}

void CBugKiller::printfStackInfo(int line, char *file_name)
{
	std::string backTrace;
#ifdef WRAP	
	getBackTrace(backTrace);
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
	CMemCheck::instance();
	CalcMem::instance();	
	ThreadQueue::instance();
	CalcMemManager::instance();
	CLogOprManager::instance();
	CTimeCalcInfManager::instance();
	CTimeCalcManager::instance()->start();
}

void CBugKiller::stop()
{
	CTimeCalcManager::instance()->stop();
}


