#include "stdafx.h"
#include "string_base.h"
#include "time_calc.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "time_calc.h"
#include <map>
#include <sys/stat.h>
#include "mem_calc.h"
#include "log_opr.h"
#include "net_server.h"

#ifdef WRAP
#include <execinfo.h>
#endif


/* 日志文件路径的长度的最大值 */
#define MAX_LOG_PATH_LEN		128
/* 日志文件名的长度的最大值 */
#define MAX_LOG_NAME_LEN		40
#define LOG_DEFAULT_NAME		"DefaultLogName"


 
#define LOG_MODE_ERROR			1
#define LOG_MODE_NORMAL			2
#define LOG_MODE_DEBUG			3


#define LOG_SWITCH_MODE_DATE	1
#define LOG_SWITCH_MODE_SIZE	2

#define LOG_SIZE_UNIT			1000000


/* 日志文件错误基本号	*/
#define LOG_ERR_CODE_BASE		40000


#define SINGLE_LINE				"--------------------------------------------------------------------------------\r\n"
using namespace base;
extern CPthreadMutex g_insMutexCalc;

void NextStep(const char *function, const char *fileName, int line)
{
	char s[80];
	printf("%s  %s  %4d\r\n", function, fileName, line);
	fgets(s, sizeof(s), stdin);
	return ;
}


void CTimeCalc::exit()
{
    DealFuncExit();

    base::free(m_FileName);
    base::free(m_FuncName);
    base::free(m_preFileName);
    base::free(m_preFuncName);
    m_FileName = NULL;
    m_FuncName = NULL;
    m_preFileName = NULL;
    m_preFuncName = NULL;
}

void CTimeCalc::init(int line[], char *file_name[], char *func_name[], int display_level, TraceInfoId &traceInfoId)
{
	m_displayFlag = true;
	m_DisplayLevel = display_level;
	m_noDisplayLevel = display_level;		
	m_Line = line[0];

	m_FileName = (char *)base::malloc(strlen(file_name[0]) + 1);
	base::strcpy(m_FileName, file_name[0]);
	m_FuncName = (char *)base::malloc(strlen(func_name[0]) + 1);
	base::strcpy(m_FuncName, func_name[0]);

	m_preLine = line[1];
	m_preFileName = (char *)base::malloc(strlen(file_name[1]) + 1);
	base::strcpy(m_preFileName, file_name[1]);
	m_preFuncName = (char *)base::malloc(strlen(func_name[1]) + 1);
	base::strcpy(m_preFuncName, func_name[1]);

	base::ftime(&m_StartTime);
	m_traceInfoId = traceInfoId;
	DealFuncEnter();
}

CTimeCalc * CTimeCalc::createCTimeCalc(int line[], char *file_name[], char *func_name[], int display_level, TraceInfoId &traceInfoId) 
{
	CTimeCalc *pTimeCalc = (CTimeCalc *)base::malloc(sizeof(CTimeCalc));
	if (pTimeCalc)
	{
		pTimeCalc->init(line, file_name, func_name, display_level, traceInfoId);
	}
	return pTimeCalc;

}

void CTimeCalc::destroyCTimeCalc(CTimeCalc *pTimeCalc)
{
	pTimeCalc->exit();
	base::free(pTimeCalc);

}
void CTimeCalc::insertEnterInfo(FuncTraceInfo_t *TraceInfo)
{
	char tmp[512];
 	
	base::snprintf(tmp, sizeof(tmp), ":  %4d  tid:%d  cid:%d  level  %4d ", m_Line, (int)m_traceInfoId.threadId, m_traceInfoId.clientId, m_DisplayLevel);

	if (this->m_DisplayLevel == 0)
	{
		TraceInfo->pUpString->append("#if 0 \r\n");
	}
	
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->pUpString->append("\t");
	}
	TraceInfo->pUpString->append(m_FuncName);
	TraceInfo->pUpString->append("()");

	TraceInfo->pUpString->append(tmp);
	TraceInfo->pUpString->append(m_FileName);


	TraceInfo->pUpString->append("\r\n");
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->pUpString->append("\t");
	}
	TraceInfo->pUpString->append("{");

	TimeB cur_time;
	base::ftime(&cur_time);

	
	base::snprintf(tmp, sizeof(tmp), "\t//\tcost second: %4ld  %4d  %16ld  %4d  route  ", (int)(cur_time.time - TraceInfo->EndTime.time), (int)(cur_time.millitm - TraceInfo->EndTime.millitm), (int)cur_time.time, (int)cur_time.millitm);
	TraceInfo->pUpString->append(tmp);

	base::snprintf(tmp, sizeof(tmp),"%s  %d  %s\r\n", m_preFuncName, m_preLine, m_preFileName);
	TraceInfo->pUpString->append(tmp);
	
	return ;
}

void CTimeCalc::insertExitInfo(FuncTraceInfo_t *TraceInfo)
{
	for (int i=1; i<TraceInfo->deep; ++i)
	{
		TraceInfo->pUpString->append("\t");
	}

	TraceInfo->pUpString->append("}");
	char tmp[512];

	TimeB cur_time; 
	base::ftime(&cur_time);

	base::snprintf(tmp, sizeof(tmp), "\t//func  cost second: %ld  %d  %ld  %d     %s  %s  ", (int)(cur_time.time - m_StartTime.time), (int)(cur_time.millitm - m_StartTime.millitm), (int)cur_time.time, (int)cur_time.millitm, m_FuncName, "https://github.com/traceworker/traceworker.git");

	TraceInfo->pUpString->append(tmp);
	TraceInfo->pUpString->append("\r\n");
		
	if (this->m_DisplayLevel == 0)
	{
		TraceInfo->pUpString->append("#endif\r\n");
	}

	return ;
}


void CTimeCalc::DealFuncEnter()
{
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->CreatTraceInf(m_traceInfoId);
	
	initTimeCalc(TraceInfo->pCalcList);
	if (!this->m_displayFlag)
	{
		return ;
	}
	insertEnterInfo(TraceInfo);
	TraceInfo->deep++;

}


void CTimeCalc::initTimeCalc(CList *pCalcList)
{
	CTimeCalc *timeCalc = getLastTimeCalc(pCalcList);
	setDisplayFlag(timeCalc);
	pCalcList->push_back(&this->m_node);

}
void CTimeCalc::exitTimeCalc(CList *pCalcList)
{
	pCalcList->pop_back();
}
CTimeCalc *CTimeCalc::getLastTimeCalc(CList *pCalcList)
{
	CTimeCalc *timeCalc = this;
	if (pCalcList->size())
	{
		timeCalc = CTimeCalcContain(pCalcList->back());
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

	if(TraceInfo)//如果查找到
	{
		exitTimeCalc(TraceInfo->pCalcList);
		if (!this->m_displayFlag)
		{
			return ;
		}	
		base::ftime(&TraceInfo->EndTime);
		//-------------------
		if (TraceInfo->deep < 1)
		{
			CTimeCalcManager::instance()->printStrLog(m_traceInfoId, "//ERRERRERRERRERRERRERRERR");
			return;
		}
	
		insertExitInfo(TraceInfo);

		TraceInfo->deep--;

		if (TraceInfo->deep == 0)
		{
			CTimeCalcManager::instance()->printStrLog(m_traceInfoId, TraceInfo->pUpString->c_str());
            CTimeCalcManager::instance()->removeTraceInf(TraceInfo->traceInfoId);
			CTimeCalcManager::instance()->DestroyTraceInf(TraceInfo);
		}

	}
	else
	{
		CTimeCalcManager::instance()->printStrLog(m_traceInfoId, "//ERRERRERRERRERRERRERRERR");
		return;
	}

}


CTimeCalcManager *CTimeCalcManager::_instance = NULL;
CTimeCalcManager::CTimeCalcManager()
:m_fp(NULL)
,m_logName("./Debug.cpp")
{
}
CTimeCalcManager::~CTimeCalcManager()
{
}

CTimeCalcManager *CTimeCalcManager::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CTimeCalcManager;
		}
	}
	return _instance;
}



FuncTraceInfo_t * CTimeCalcManager::CreatTraceInf(TraceInfoId &traceInfoId)
{	
	CGuardMutex guardMutex(m_traceInfoMapMutex);
	FuncTraceInfo_t traceInfo;
	traceInfo.traceInfoId = traceInfoId;
    TraceInfoMap::iterator iter = m_traceInfoMap.find(traceInfoId);

	FuncTraceInfo_t *TraceInfo = NULL;
	if (iter != m_traceInfoMap.end())//如果查找到
	{
		TraceInfo = iter->second;
	}
	else  
	{
		TraceInfo = (FuncTraceInfo_t *)base::malloc(sizeof(FuncTraceInfo_t));
		assert(TraceInfo != NULL);

		base::ftime(&TraceInfo->EndTime);
		
		TraceInfo->deep = 0;

		TraceInfo->pUpString = CString::createCString();
		TraceInfo->pUpString->append("//CTimeCalcCTimeCalcCTimeCalcCTimeCalcCTimeCalcCTimeCalc\r\n");

		TraceInfo->traceInfoId = traceInfoId;
		TraceInfo->pCalcList = CList::createCList();

		m_traceInfoMap.insert(std::make_pair(traceInfoId, TraceInfo));
	}
	return TraceInfo;
}



void CTimeCalcManager::removeTraceInf(TraceInfoId &traceInfoId)
{
	CGuardMutex guardMutex(m_traceInfoMapMutex);
    m_traceInfoMap.erase(traceInfoId);
}

void CTimeCalcManager::DestroyTraceInf(FuncTraceInfo_t *TraceInfo)
{
	CString::destroyCString(TraceInfo->pUpString);
	CList::destroyClist(TraceInfo->pCalcList);
	base::free(TraceInfo);
}

FuncTraceInfo_t * CTimeCalcManager::GetTraceInf(TraceInfoId &traceInfoId)
{
	CGuardMutex guardMutex(m_traceInfoMapMutex);
	FuncTraceInfo_t traceInfo;
	traceInfo.traceInfoId = traceInfoId;
	TraceInfoMap::iterator iter = m_traceInfoMap.find(traceInfoId);

	if (iter != m_traceInfoMap.end())//如果查找到
	{
		return iter->second;
	}	
	
	return NULL;
}


void CTimeCalcManager::printStack(int line, char *file_name, const char* fmt, ...)
{
	TraceInfoId traceInfoId;
	traceInfoId.threadId = CBase::pthread_self();
	traceInfoId.clientId = -1;
	
	FuncTraceInfo_t *TraceInfo = GetTraceInf(traceInfoId);
	if (!TraceInfo)
	{
		return ;
	}
	
	char str[4096];
	va_list ap;
	va_start(ap,fmt);
	base::vsnprintf(str,sizeof(str), fmt, ap);
	va_end(ap); 
	insertStackInfo(TraceInfo, line, file_name, str);

	return ;
}


void CTimeCalcManager::insertStackInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr)
{
	TimeB cur_time;
	base::ftime(&cur_time);

	char tmp[512];

	//-------------------
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->pUpString->append("\t");
	}
	TraceInfo->pUpString->append("/*tag:");

	TraceInfo->pUpString->append(pStr);
	TraceInfo->pUpString->append(" ");

	std::string stackInf;
	getStackInfo(TraceInfo, stackInf);
	
	int count = 0;
	if (m_stack_inf_map.find(stackInf) == m_stack_inf_map.end())
	{
		m_stack_inf_map[stackInf] = 0;
	}
	else
	{
		count = m_stack_inf_map[stackInf];
		++count;
		m_stack_inf_map[stackInf] = count;
	}

	base::snprintf(tmp, sizeof(tmp), "count %8d ", count);
	TraceInfo->pUpString->append(tmp);
	TraceInfo->pUpString->append(stackInf.c_str());


	base::snprintf(tmp, sizeof(tmp), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)CBase::pthread_self(), "wshy", cur_time.time, cur_time.millitm);
	TraceInfo->pUpString->append(tmp);
	TraceInfo->pUpString->append("*/\r\n");

	return ;
}

void CTimeCalcManager::printfMemInfMap(TraceInfoId &traceInfoId)
{
#ifdef WRAP
	CalcMem::instance()->printfMemInfMap(traceInfoId);
#endif
}
void CTimeCalcManager::getStackInfo(std::string &stackInf)
{
	TraceInfoId traceInfoId;
	traceInfoId.threadId = CBase::pthread_self();
	traceInfoId.clientId = -1;

	FuncTraceInfo_t *TraceInfo = GetTraceInf(traceInfoId);
	if (!TraceInfo || !TraceInfo->pCalcList->size())
	{
		return ;
	}
	getStackInfo(TraceInfo, stackInf);
	
	return ;
}
void CTimeCalcManager::getStackInfo(FuncTraceInfo_t *TraceInfo, std::string &stackInf)
{
	char tmp[512];
	CTimeCalc *timeCalc = NULL;
	struct node *pNode = NULL;
	CList *pCalcList = TraceInfo->pCalcList;

	while (pCalcList->size())
	{
		pNode =  pCalcList->begin();
		timeCalc = CTimeCalcContain(pNode);
		pCalcList->pop_front();
		
		base::snprintf(tmp, sizeof(tmp), "%s%d_", timeCalc->m_FuncName, timeCalc->m_Line);
		stackInf += tmp;
	}
	return ;
}



void CTimeCalcManager::InsertTrace(int line, char *file_name, TraceInfoId &traceInfoId, const char* content)
{
	FuncTraceInfo_t *TraceInfo = GetTraceInf(traceInfoId);
	if (TraceInfo && !needPrint(TraceInfo->pCalcList))
	{
		return ;
	}
	
	if(TraceInfo)//如果查找到
	{
		insertTraceInfo(TraceInfo, line, file_name, traceInfoId, content);
	}  
	else
	{
		char logStr[512];
		base::snprintf(logStr, sizeof(logStr), "trace:/*%s  %d  %s  tid:%d  cid:%d */\r\n", content, line, file_name, (int)traceInfoId.threadId, traceInfoId.clientId);
		CTimeCalcManager::instance()->printStrLog(traceInfoId, logStr);
	}


	return ;

}

void CTimeCalcManager::InsertStrOnly(TraceInfoId &traceInfoId, const char* fmt, ...)
{
	FuncTraceInfo_t *TraceInfo = GetTraceInf(traceInfoId);
	if (TraceInfo && !needPrint(TraceInfo->pCalcList))
	{
		return ;
	}
	
	char str[4096];
	va_list ap;
	va_start(ap,fmt);
	base::vsnprintf(str,sizeof(str), fmt, ap);
	va_end(ap);      

	if(TraceInfo)//如果查找到
	{
		InsertStrOnlyInfo(TraceInfo, str);
	}  
	else
	{
		CTimeCalcManager::instance()->printLog(traceInfoId, (char *)"trace:/*%s*/", str);
	}


	return ;

}

void CTimeCalcManager::InsertStrOnlyInfo(FuncTraceInfo_t *TraceInfo, char *pStr)
{

	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->pUpString->append("\t");
	}
	TraceInfo->pUpString->append("/*tag:");

	TraceInfo->pUpString->append(pStr);
	TraceInfo->pUpString->append("*/\r\n");


	return ;
}


void CTimeCalcManager::insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, TraceInfoId &traceInfoId, const char *pStr)
{
	TimeB cur_time;
	base::ftime(&cur_time);
	char *selfInf = (char *)"creat by 467831967@qq.com Wechat:traceworker";
	char tmp[512];
	base::snprintf(tmp, sizeof(tmp), "    %4d    %s  tid:%d  cid:%d  %16ld  ms %4d    %s", line, file_name, (int)traceInfoId.threadId, traceInfoId.clientId, cur_time.time, cur_time.millitm, selfInf);

	//-------------------
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->pUpString->append("\t");
	}
	TraceInfo->pUpString->append("/*tag:");

	TraceInfo->pUpString->append(pStr);
	TraceInfo->pUpString->append(tmp);
	TraceInfo->pUpString->append("*/\r\n");

	return ;
}

void CTimeCalcManager::InsertHex(TraceInfoId &traceInfoId, int line, char *file_name, char *psBuf, int nBufLen)
{
	TimeB cur_time;
	base::ftime(&cur_time);
	
	char str[4096];
	/* save log msg in file */
	base::snprintf(str, sizeof(str), "hex%s:[%s][%4d]len=%4d\r\n", __FUNCTION__, file_name, line,nBufLen);

	/* save log msg in file */
	int j = 0;
	char sLine[100], sTemp[12];
	for	(int i=0; i<nBufLen; i++)
	{
		
		/* initialize a new line */
		if (j==0)
		{
			memset (sLine,	' ', sizeof(sLine));
			base::snprintf (sTemp,	sizeof(sTemp), "%04d:", i );
			memcpy (sLine, sTemp, 5);
			base::snprintf (sTemp, sizeof(sTemp), ":%04d", i+15 );
			memcpy (sLine+72, sTemp, 5);
		}

		/* output psBuf value in hex */
		base::snprintf(sTemp, sizeof(sTemp), "%02X ", (unsigned	char)psBuf[i]);
		memcpy( &sLine[j*3+5+(j>7)], sTemp, 3);

		/* output psBuf in ascii */
		if ( isprint ((unsigned char)psBuf[i]))
		{
			sLine[j+55+(j>7)]=psBuf[i];
		}
		else
		{
			sLine[j+55+(j>7)]='.';
		}
		j++;

		/* output the line to file */
		if (j==16)
		{
			sLine[77]=0;
			base::snprintf(str+strlen(str), sizeof(str)-strlen(str), "%s\r\n", sLine);
			j=0;
		}
	}

	/* last line */
	if (j)
	{
		sLine[77]=0;
		base::snprintf(str+strlen(str), sizeof(str)-strlen(str), "%s\r\n",	sLine);
	}
	base::snprintf(str+strlen(str), sizeof(str)-strlen(str), "%80.80s\r\n", SINGLE_LINE);
	//--------------------------------------




	base::snprintf(str+strlen(str), sizeof(str)-strlen(str), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)CBase::pthread_self(), "wshy", cur_time.time, cur_time.millitm);

	FuncTraceInfo_t *TraceInfo = GetTraceInf(traceInfoId);
	
	if(TraceInfo)//如果查找到
	{	
		
		//-------------------
		for (int i=0; i<TraceInfo->deep; ++i)
		{
			TraceInfo->pUpString->append("\t");
		}
		TraceInfo->pUpString->append("/*tag:");
		TraceInfo->pUpString->append(str);
		TraceInfo->pUpString->append("*/\r\n");

		
	}  
	else
	{
		printLog(traceInfoId, (char *)"trace:/*%s*/", str);
	}
	return ;

}


void CTimeCalcManager::InsertTag(TraceInfoId &traceInfoId, int line, char *file_name, const char* content)
{
	
	TimeB cur_time;
	base::ftime(&cur_time);

	char str[512];
	base::snprintf(str, sizeof(str), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)CBase::pthread_self(), "huang_yuan", cur_time.time, cur_time.millitm);

	printLog(traceInfoId, (char *)"trace:/*%s %s*/", content, str);
	return ;
}

void CTimeCalcManager::DispAll(int clientId, const char* content)
{
	FuncTraceInfo_t *TraceInfo = NULL;

	CGuardMutex guardMutex(m_traceInfoMapMutex);
	
    TraceInfoMap::iterator iter;	
#ifndef WIN32	
    for (iter = m_traceInfoMap.begin(); iter != m_traceInfoMap.end(); ++iter)    
	{
		if (iter != m_traceInfoMap.end())//如果查找到
		{
			TraceInfo = iter->second;
			if (TraceInfo->traceInfoId.clientId == clientId)
			{
				//printf("%s\r\n", TraceInfo->pUpString->c_str());
			}
		}
	}
#endif
	for (iter = m_traceInfoMap.begin(); iter != m_traceInfoMap.end(); ++iter)    
	{
		if (iter != m_traceInfoMap.end())//如果查找到
		{
			TraceInfo = iter->second;
			if (TraceInfo->traceInfoId.clientId == clientId)
			{
				printStrLog(TraceInfo->traceInfoId, "#if 0");
				printStrLog(TraceInfo->traceInfoId, TraceInfo->pUpString->c_str());
				printStrLog(TraceInfo->traceInfoId, "#endif\r\n");
			}
		}
	}

	return ;
}
void CTimeCalcManager::getTraceInfStr(int clientId, std::string &traceInfStr)
{
    traceInfStr = "";
	FuncTraceInfo_t *TraceInfo = NULL;
	CGuardMutex guardMutex(m_traceInfoMapMutex);
    TraceInfoMap::iterator iter;	
    for (iter = m_traceInfoMap.begin(); iter != m_traceInfoMap.end(); ++iter)    
	{
		if (iter != m_traceInfoMap.end())//如果查找到
		{
			TraceInfo = iter->second;
			if (TraceInfo->traceInfoId.clientId == clientId)
			{
                traceInfStr = TraceInfo->pUpString->c_str();
                break;
			}
		}
	}
    return ;
}

void CTimeCalcManager::cleanAll(int clientId)
{
	CGuardMutex guardMutex(m_traceInfoMapMutex);
	TraceInfoMap::iterator iter;	
	FuncTraceInfo_t *TraceInfo = NULL;

    for(iter = m_traceInfoMap.begin(); iter!=m_traceInfoMap.end(); )
    {
        TraceInfo = iter->second;
        m_traceInfoMap.erase(iter++);

        DestroyTraceInf(TraceInfo);
    }
}


bool CTimeCalcManager::needPrint(CList *pCalcList)
{
	CTimeCalc *timeCalc = NULL;
	if (pCalcList->size())
	{	
		timeCalc = CTimeCalcContain(pCalcList->back());
	}
	if (timeCalc && timeCalc->m_displayFlag)
	{
		return true;
	}
	
	return false;
}

void CTimeCalcManager::start()
{
#ifdef WRAP
	ThreadQueue::start();
#endif
}

void CTimeCalcManager::stop()
{
#ifdef WRAP
	ThreadQueue::stop();
#endif
}

FILE *CTimeCalcManager::openLog(const char *sLogName)
{
	FILE *fp = NULL;
	fp = base::fopen (sLogName, "a+");
	return fp;
}
void CTimeCalcManager::printLog(TraceInfoId &traceInfoId, char *sFmt, ...)
{
	char logStr[512];
	va_list ap;
	va_start(ap,sFmt);
	base::vsnprintf(logStr, sizeof(logStr), sFmt, ap);
	va_end(ap);

	CLogOprManager::instance()->writeFile(traceInfoId, logStr);
	
	base::snprintf(logStr, sizeof(logStr), "//thread id:%16d  creat by huang_yuan@dahuatech.com\r\n\r\n", (int)CBase::pthread_self());

	CLogOprManager::instance()->writeFile(traceInfoId, logStr);
	return ;
}

void CTimeCalcManager::printStrLog(TraceInfoId &traceInfoId, const char *logStr)
{
	CLogOprManager::instance()->writeFile(traceInfoId, (char *)logStr);
}


