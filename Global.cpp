#include "stdafx.h"
#include "string_base.h"
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

	RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_oper = "createCandy";
	pCalcInf->m_traceInfoId.threadId = CBase::pthread_self();
	pCalcInf->m_traceInfoId.clientId = -1;
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	pCalcInf->m_funcName = func_name;
	pCalcInf->m_displayLevel = display_level;


	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);	
	return ;
}

CCandy::~CCandy()
{

	RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_oper = "destroyCandy";
	pCalcInf->m_traceInfoId.threadId = CBase::pthread_self();
	pCalcInf->m_traceInfoId.clientId = -1;
	
	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;
}

void CBugKiller::InsertTrace(int line, char *file_name, const char* fmt, ...)
{

	char content[4096];
	va_list ap;
	va_start(ap,fmt);
	base::vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);


	RECV_DATA *pRecvData = IDealDataHandle::createRecvData((int)strlen(content));
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_oper = "insertTrace";
	pCalcInf->m_traceInfoId.threadId = CBase::pthread_self();
	pCalcInf->m_traceInfoId.clientId = -1;
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	base::strcpy(pCalcInf->m_pContent, content);

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;
}

void CBugKiller::InsertHex(int line, char *file_name, char *psBuf, int nBufLen)
{
	RECV_DATA *pRecvData = IDealDataHandle::createRecvData(nBufLen);
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_oper = "insertHex";
	pCalcInf->m_traceInfoId.threadId = CBase::pthread_self();
	pCalcInf->m_traceInfoId.clientId = -1;
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	memcpy(pCalcInf->m_pContent, psBuf, nBufLen);

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	return ;
}

void CBugKiller::InsertStrOnly(TraceInfoId &traceInfoId, const char* fmt, ...)
{
	char content[4096];
	va_list ap;
	va_start(ap,fmt);
	base::vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);


	RECV_DATA *pRecvData = IDealDataHandle::createRecvData((int)strlen(content));
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_oper = "insertStrOnly";
	pCalcInf->m_traceInfoId = traceInfoId;
	base::strcpy(pCalcInf->m_pContent, content);

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);

	return ;
}

void CBugKiller::DispAll()
{
	std::string backtrace;
	CalcMem::instance()->getBackTrace(backtrace);

	RECV_DATA *pRecvData = IDealDataHandle::createRecvData((int)backtrace.size());
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_oper = "dispAll";
	pCalcInf->m_traceInfoId.threadId = CBase::pthread_self();
	pCalcInf->m_traceInfoId.clientId = -1;
	base::strcpy(pCalcInf->m_pContent, backtrace.c_str());

	CTimeCalcInfManager::instance()->pushRecvData(pRecvData);
	//CTimeCalcManager::instance()->DispAll();
}


void CBugKiller::InsertTag(int line, char *file_name, const char* fmt, ...)
{
	char content[1024];
	va_list ap;
	va_start(ap,fmt);
	base::vsnprintf(content,sizeof(content), fmt, ap);
	va_end(ap);

	RECV_DATA *pRecvData = IDealDataHandle::createRecvData((int)strlen(content));
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_oper = "insertTag";
	pCalcInf->m_traceInfoId.threadId = CBase::pthread_self();
	pCalcInf->m_traceInfoId.clientId = -1;
	pCalcInf->m_line = line;
	pCalcInf->m_fileName = file_name;
	base::strcpy(pCalcInf->m_pContent, content);

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
	RECV_DATA *pRecvData = IDealDataHandle::createRecvData();
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;

	pCalcInf->m_oper = "printfMemInfMap";
	pCalcInf->m_traceInfoId.threadId = CBase::pthread_self();
	pCalcInf->m_traceInfoId.clientId = -1;

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

void CBugKiller::startServer()
{
	CMemCheck::instance();
	CalcMem::instance();	
	ThreadQueue::instance();
	CalcMemManager::instance();
	CLogOprManager::instance();
	CTimeCalcInfManager::instance();
	CTimeCalcManager::instance();
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


