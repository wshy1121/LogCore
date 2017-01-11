#include <assert.h>
#include <string.h>
#include "time_calc.h"
#include "log_opr.h"

static CPthreadMutex g_insMutexCalc;

CTimeCalc *CTimeCalc::createCTimeCalc(int line, char *file_name, char *func_name, int display_level, TraceInfoId &traceInfoId)
{
	CTimeCalc *pTimeCalc = new CTimeCalc(line, file_name, func_name, display_level, traceInfoId);
	return pTimeCalc;
}

void CTimeCalc::destroyCTimeCalc(CTimeCalc *pTimeCalc)
{
	delete pTimeCalc;
}

CTimeCalc::CTimeCalc(int line, char *file_name, char *func_name, int display_level, TraceInfoId &traceInfoId)
:m_displayFlag(true)
{
	m_DisplayLevel = display_level;
	m_noDisplayLevel = display_level;
	m_Line = line;

	m_FileName = file_name;
	m_FuncName = func_name;

	CBase::ftime(&m_StartTime);
	m_traceInfoId = traceInfoId;
	DealFuncEnter();
}

CTimeCalc::~CTimeCalc()
{
	DealFuncExit();
}


void CTimeCalc::insertEnterInfo(FuncTraceInfo_t *TraceInfo)
{
	char tmp[512];
	CBase::snprintf(tmp, sizeof(tmp), ":  %4d  tid:%d  cid:%d  level  %4d ", m_Line, (int)m_traceInfoId.threadId, m_traceInfoId.clientId, m_DisplayLevel);
	if (this->m_DisplayLevel == 0)
	{
		TraceInfo->upString.append("#if 0\r\n");
	}
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->upString.append("\t");
	}
	TraceInfo->upString.append(m_FuncName);
	TraceInfo->upString.append("()");

	TraceInfo->upString.append(tmp);
	TraceInfo->upString.append(m_FileName);

	TraceInfo->upString.append("\n");
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->upString.append("\t");
	}
	TraceInfo->upString.append("{");

	TimeB cur_time;
	CBase::ftime(&cur_time);


	CBase::snprintf(tmp, sizeof(tmp), "\t//\tconst second: %4ld  %4d  %16ld  %4d  route  \n", (int)(cur_time.time - TraceInfo->EndTime.time), (int)(cur_time.millitm - TraceInfo->EndTime.millitm), (int)cur_time.time, (int)cur_time.millitm);
	TraceInfo->upString.append(tmp);
}

void CTimeCalc::insertExitInfo(FuncTraceInfo_t *TraceInfo)
{
	for (int i=1; i<TraceInfo->deep; ++i)
	{
		TraceInfo->upString.append("\t");
	}

	TraceInfo->upString.append("}");
	char tmp[512];

	TimeB cur_time;
	CBase::ftime(&cur_time);

	CBase::snprintf(tmp, sizeof(tmp), "\t//func  const second: %ld  %d  %ld  %d  %s  %s", (int)(cur_time.time-m_StartTime.time), (int)(cur_time.millitm-m_StartTime.millitm), (int)cur_time.time, (int)cur_time.millitm, m_FuncName.c_str(), "https://github.com/traceworker/traceworker.git");

	TraceInfo->upString.append(tmp);
	TraceInfo->upString.append("\n");

	if (this->m_DisplayLevel == 0)
	{
		TraceInfo->upString.append("#endif\r\n");
	}
}

void CTimeCalc::DealFuncEnter()
{
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->CreateTraceInf(m_traceInfoId);
	initTimeCalc(TraceInfo->calcList);
	if (!this->m_displayFlag)
	{
		return ;
	}
	insertEnterInfo(TraceInfo);
	TraceInfo->deep++;
}

void CTimeCalc::initTimeCalc(CTimeCalcList &calcList)
{
	CTimeCalc *timeCalc = getLastTimeCalc(calcList);
	setDisplayFlag(timeCalc);
	calcList.push_back(this);
}

CTimeCalc *CTimeCalc::getLastTimeCalc(CTimeCalcList &calcList)
{
	CTimeCalc *timeCalc = this;
	if (calcList.size())
	{
		timeCalc = calcList.back();
	}
	return timeCalc;
}

void CTimeCalc::setDisplayFlag(CTimeCalc *timeCalc)
{
	int noDisplayLevel = timeCalc->m_noDisplayLevel;
	if (m_DisplayLevel > noDisplayLevel)
	{
		m_displayFlag = false;
		m_noDisplayLevel = noDisplayLevel;
	}
}

void CTimeCalc::DealFuncExit()
{
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->GetTraceInf(m_traceInfoId);
	if (TraceInfo)
	{
		exitTimeCalc(TraceInfo->calcList);
		if (!this->m_displayFlag)
		{
			return ;
		}
		CBase::ftime(&TraceInfo->EndTime);
		//-----------------------------------------
		if (TraceInfo->deep < 1)
		{
			CTimeCalcManager::instance()->printStrLog(m_traceInfoId, "//ERRERRERRERRERRERRERRERRERRERRERR");
			return ;
		}

		insertExitInfo(TraceInfo);

		TraceInfo->deep--;

		if (TraceInfo->deep == 0)
		{
			CTimeCalcManager::instance()->printStrLog(m_traceInfoId, TraceInfo->upString.c_str());
			CTimeCalcManager::instance()->removeTraceInf(TraceInfo->traceInfoId);
			CTimeCalcManager::instance()->DestroyTraceInf(TraceInfo);
		}
	}
	else
	{
		CTimeCalcManager::instance()->printStrLog(m_traceInfoId, "//ERRERRERRERRERRERRERRERRERRERRERR");
	}
}

void CTimeCalc::exitTimeCalc(CTimeCalcList &calcList)
{
	calcList.pop_back();
}

CTimeCalcManager *CTimeCalcManager::_instance = NULL;

CTimeCalcManager *CTimeCalcManager::instance()
{
	if (_instance == NULL)
	{
		CGuardMutex guarMutex(g_insMutexCalc);
		if (_instance == NULL)
		{
			_instance = new CTimeCalcManager;
		}
	}
	return _instance;
}

FuncTraceInfo_t *CTimeCalcManager::CreateTraceInf(TraceInfoId &traceInfoId)
{
	CGuardMutex guarMutex(m_traceInfoMapMutex);
	TraceInfoMap::iterator iter = m_traceInfoMap.find(traceInfoId);
	FuncTraceInfo_t *TraceInfo = NULL;
	if (iter != m_traceInfoMap.end())
	{
		TraceInfo = iter->second;
	}
	else
	{
		TraceInfo = new FuncTraceInfo_t;
		assert(TraceInfo != NULL);
		
		CBase::ftime(&TraceInfo->EndTime);
		TraceInfo->deep = 0;
		TraceInfo->upString.append("//CTimeCalcCTimeCalcCTimeCalcCTimeCalcCTimeCalc\n");

		TraceInfo->traceInfoId = traceInfoId;
		m_traceInfoMap.insert(std::make_pair(traceInfoId, TraceInfo));
	}
	return TraceInfo;
}

FuncTraceInfo_t *CTimeCalcManager::GetTraceInf(TraceInfoId &traceInfoId)
{
	CGuardMutex guarMutex(m_traceInfoMapMutex);
	TraceInfoMap::iterator iter = m_traceInfoMap.find(traceInfoId);

	if (iter != m_traceInfoMap.end())
	{
		return iter->second;
	}
	return NULL;
}

void CTimeCalcManager::removeTraceInf(TraceInfoId &traceInfoId)
{
	CGuardMutex guarMutex(m_traceInfoMapMutex);
	m_traceInfoMap.erase(traceInfoId);
}

void CTimeCalcManager::DestroyTraceInf(FuncTraceInfo_t *TraceInfo)
{
	delete TraceInfo;
}

void CTimeCalcManager::printStrLog(TraceInfoId &traceInfoId, const char *logStr)
{
	CLogOprManager::instance()->writeFile(traceInfoId, (char *)logStr);
}

void CTimeCalcManager::InsertTrace(int line, char *file_name, TraceInfoId &traceInfoId, const char *content)
{
	FuncTraceInfo_t *TraceInfo = GetTraceInf(traceInfoId);
	if (TraceInfo && !needPrint(TraceInfo->calcList))
	{
		return ;
	}

	if (TraceInfo)
	{
		insertTraceInfo(TraceInfo, line, file_name, traceInfoId, content);
	}
	else
	{		
		char logStr[4096];
		printStrLog(traceInfoId, "trace:/*");
		printStrLog(traceInfoId, content);
		CBase::snprintf(logStr, sizeof(logStr), "  %d  %s  tid:%d  cid:%d */ \r\n", line, file_name, (int)traceInfoId.threadId, traceInfoId.clientId);
		printStrLog(traceInfoId, logStr);
	}
}

bool CTimeCalcManager::needPrint(CTimeCalcList &calcList)
{
	CTimeCalc *timeCalc = NULL;
	if (calcList.size())
	{
		timeCalc = calcList.back();
	}
	if (timeCalc && timeCalc->m_displayFlag)
	{
		return true;
	}
	return false;
}


void CTimeCalcManager::insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, TraceInfoId &traceInfoId, const char *pStr)
{
	TimeB cur_time;
	CBase::ftime(&cur_time);
	const char *selfInf = "create by 467831967@qq.com Wechat:traceworker";
	char tmp[512];
	memset(tmp, 0, sizeof(tmp));
	CBase::snprintf(tmp, sizeof(tmp), "  %4d  %s  tid:%d  cid:%d  %16ld  ms  %4d", line, file_name, (int)traceInfoId.threadId, traceInfoId.clientId, cur_time.time, cur_time.millitm);
	//CBase::snprintf(tmp, sizeof(tmp), "  %4d  %s  tid:%d  cid:%d  %16ld  ms  %4d    %s", line, file_name, (int)traceInfoId.threadId, traceInfoId.clientId, cur_time.time, cur_time.millitm, selfInf);

	//-----------------------
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->upString.append("\t");
	}
	TraceInfo->upString.append("/*tag:");

	TraceInfo->upString.append(pStr);
	TraceInfo->upString.append(tmp);
	TraceInfo->upString.append("*/\n");
}

void CTimeCalcManager::packetHex(char *psBuf, int nBufLen, char *str, int strLen)
{
	CBase::snprintf(str, strLen, "hex%s:len=%4d\n", __FUNCTION__, nBufLen);
	int curStrLen = strlen(str);

	int j = 0;
	char sLine[100];
	char sTemp[16];
	for (int i=0; i<nBufLen; ++i)
	{
		if (j == 0)
		{
			memset(sLine, ' ', sizeof(sLine));
			CBase::snprintf(sTemp, sizeof(sTemp), "%04d:", i);
			memcpy(sLine, sTemp, 5);
			CBase::snprintf(sTemp, sizeof(sTemp), ":%04d", i+15);
			memcpy(sLine+72, sTemp, 5);
		}

		CBase::snprintf(sTemp, sizeof(sTemp), "%02X ", (unsigned char)psBuf[i]);
		memcpy(&sLine[j*3+5+(j>7)], sTemp, 3);

		if (isprint((unsigned char)psBuf[i]))
		{
			sLine[j+55+(j>7)] = psBuf[i];
		}
		else
		{
			sLine[j+55+(j>7)] = '.';
		}
		++j;

		if (j == 16)
		{
			sLine[77] = 0;
			CBase::snprintf(str+strlen(str), strLen-strlen(str), "%s\n", sLine);
			j = 0;
			curStrLen += strlen(sLine) + 1;
			if (curStrLen > (strLen-1))
			{
				curStrLen = (strLen-1);
			}
		}
	}

	if (j)
	{
		sLine[77] = 0;
		CBase::snprintf(str+strlen(str), strLen-strlen(str), "%s\n", sLine);
		curStrLen += strlen(sLine) + 1;
	}
}

void CTimeCalcManager::DispAll(int clientId, const char *content)
{
    FuncTraceInfo_t *TraceInfo = NULL;
    
    CGuardMutex guarMutex(m_traceInfoMapMutex);
    
    TraceInfoMap::iterator iter;
    for (iter = m_traceInfoMap.begin(); iter != m_traceInfoMap.end(); ++iter)
    {
        TraceInfo = iter->second;
        if (TraceInfo->traceInfoId.clientId == clientId)
        {
            printStrLog(TraceInfo->traceInfoId, "#if 0");
            printStrLog(TraceInfo->traceInfoId, TraceInfo->upString.c_str());
            printStrLog(TraceInfo->traceInfoId, "#endif \n");
        }
    }

}

void CTimeCalcManager::cleanAll(int clientId)
{
    CGuardMutex guarMutex(m_traceInfoMapMutex);
    TraceInfoMap::iterator iter;
    FuncTraceInfo_t *TraceInfo = NULL;

    for (iter = m_traceInfoMap.begin(); iter != m_traceInfoMap.end(); )
    {
        TraceInfo = iter->second;
        m_traceInfoMap.erase(iter++);
        DestroyTraceInf(TraceInfo);
    }
}



