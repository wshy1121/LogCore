// traceworker.cpp : Defines the exported functions for the DLL application.
//
#include "trace_worker.h"
#include "time_calc.h"
#include "log_opr.h"

bool CBugKiller::m_isStart = false;

// This is an example of an exported variable
TRACEWORKER_API int ntraceworker=0;

// This is an example of an exported function.
TRACEWORKER_API int fntraceworker(void)
{
	return 42;
}

CCandy::CCandy(int line, char *file_name, char *func_name, int display_level)
{
    if (!CBugKiller::m_isStart)
    {
        return ;
    }
	TraceInfoId traceInfoId;
	traceInfoId.threadId = CBase::pthread_self();
	traceInfoId.clientId = 0;
	CTimeCalc::createCTimeCalc(line, file_name, func_name, display_level, traceInfoId);
}

CCandy::~CCandy()
{
    if (!CBugKiller::m_isStart)
    {
        return ;
    }

	TraceInfoId traceInfoId;
	traceInfoId.threadId = CBase::pthread_self();
	traceInfoId.clientId = 0;
	
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->GetTraceInf(traceInfoId);
	if (TraceInfo == NULL || TraceInfo->calcList.size() == 0)
	{
		printf("GetTraceInf failed  tid  cid  %ld  %d\n", traceInfoId.threadId, traceInfoId.clientId);
		return ;
	}

	CTimeCalc *pTimeCalc = TraceInfo->calcList.back();
	
	CTimeCalc::destroyCTimeCalc(pTimeCalc);
}


void CBugKiller::startServer(const char *fileName)
{
    if (!CBugKiller::m_isStart)
    {
        return ;
    }

	if (CLogOprManager::instance()->openFile(0, (char *)fileName, std::string("")) != NULL)
    {
        m_isStart = true;
    }   
}

void CBugKiller::stopServer()
{
    if (!CBugKiller::m_isStart)
    {
        return ;
    }

    CTimeCalcManager::instance()->DispAll(0, "");
    CTimeCalcManager::instance()->cleanAll(0);    	
    CLogOprManager::instance()->closeFile(0);
}


void CBugKiller::InsertTrace(int line, char *file_name, const char *fmt, ...)
{
    if (!CBugKiller::m_isStart)
    {
        return ;
    }

	char content[4096];
	va_list ap;
	va_start(ap, fmt);
	CBase::vsnprintf(content, sizeof(content), fmt, ap);
	va_end(ap);

	
	TraceInfoId traceInfoId;
	traceInfoId.threadId = CBase::pthread_self();
	traceInfoId.clientId = 0;

	CTimeCalcManager::instance()->InsertTrace(line, file_name, traceInfoId, content);
}

void CBugKiller::InsertHex(int line, char *file_name, char *psBuf, int nBufLen)
{
    if (!CBugKiller::m_isStart)
    {
        return ;
    }

	TraceInfoId traceInfoId;
	traceInfoId.threadId = CBase::pthread_self();
	traceInfoId.clientId = 0;

	char content[4096];
	CTimeCalcManager::instance()->packetHex(psBuf, nBufLen, content, sizeof(content));
	CTimeCalcManager::instance()->InsertTrace(line, file_name, traceInfoId, content);
}


