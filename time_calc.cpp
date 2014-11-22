 #include "time_calc.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include "time_calc.h"
#include <map>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include "link_tool.h"
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


#define SINGLE_LINE				"--------------------------------------------------------------------------------\n"

CPthreadMutex g_insMutexCalc;
extern "C" void* __real_malloc(size_t);
extern "C" void __real_free(void* p);

void NextStep(const char *function, const char *fileName, int line)
{
	char s[80];
	printf("%s  %s  %4d\n", function, fileName, line);
	fgets(s, sizeof(s), stdin);
	return ;
}


CTimeCalc::CTimeCalc(int line, char *file_name, char *func_name, int display_level, pthread_t threadId) : 	m_displayFlag(true), 
																				m_DisplayLevel(display_level), 
																				m_noDisplayLevel(display_level), 
																				m_Line(line), 
																				m_FileName(file_name), 
																				m_FuncName(func_name)
{
	
	ftime(&m_StartTime);
	m_threadId = threadId;
	DealFuncEnter();

}


void CTimeCalc::insertEnterInfo(FuncTraceInfo_t *TraceInfo)
{
	char tmp[64];

	char time_tmp[128];
	snprintf(time_tmp, sizeof(time_tmp), "level  %4d ", m_DisplayLevel);
	
	snprintf(tmp, sizeof(tmp), ":  %4d  thread id:  %16d  %s", m_Line, (int)m_threadId, time_tmp);

	if (this->m_DisplayLevel == 0)
	{
		TraceInfo->up_string += "#if 0 \n";
	}
	
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->up_string += "\t";
	}
	TraceInfo->up_string += m_FuncName;
	TraceInfo->up_string += "()";

	TraceInfo->up_string += tmp;
	TraceInfo->up_string += m_FileName;

	
	TraceInfo->up_string += "\n";
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->up_string += "\t";
	}
	TraceInfo->up_string += "{";

	struct timeb cur_time;
	ftime(&cur_time);

	
	snprintf(time_tmp, sizeof(time_tmp), "       //    cost second: %4ld  %4d  %16ld  %4d  route", cur_time.time - TraceInfo->EndTime.time, cur_time.millitm - TraceInfo->EndTime.millitm, cur_time.time, cur_time.millitm);
	TraceInfo->up_string += time_tmp;
	TraceInfo->up_string += "\n";

	return ;
}

void CTimeCalc::insertExitInfo(FuncTraceInfo_t *TraceInfo)
{
	for (int i=1; i<TraceInfo->deep; ++i)
	{
		TraceInfo->up_string += "\t";
	}
	
	TraceInfo->up_string += "}";
	char tmp[128];


	char time_tmp[128];
	strcpy(time_tmp, "wshy");

	struct timeb cur_time; 
	ftime(&cur_time);

	snprintf(tmp, sizeof(tmp), "        //func  cost second: %4ld  %4d  %16ld  %4d     %s  %s  ", cur_time.time - m_StartTime.time, cur_time.millitm - m_StartTime.millitm, cur_time.time, cur_time.millitm, m_FuncName.c_str(), time_tmp);

	TraceInfo->up_string += tmp;			

	TraceInfo->up_string += "\n";
	if (this->m_DisplayLevel == 0)
	{
		TraceInfo->up_string += "#endif\n";
	}

	return ;
}


void CTimeCalc::DealFuncEnter()
{
	
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->CreatTraceInf(m_threadId);
	
	initTimeCalc(TraceInfo->calc_list);
	if (!this->m_displayFlag)
	{
		return ;
	}
	insertEnterInfo(TraceInfo);
	TraceInfo->deep++;

}


void CTimeCalc::initTimeCalc(CTimeCalcList &calc_list)
{
	CTimeCalc *timeCalc = getLastTimeCalc(calc_list);
	setDisplayFlag(timeCalc);
	calc_list.push_back(this);

}
void CTimeCalc::exitTimeCalc(CTimeCalcList &calc_list)
{
	calc_list.pop_back();
}
CTimeCalc *CTimeCalc::getLastTimeCalc(CTimeCalcList &calc_list)
{
	CTimeCalc *timeCalc = this;
	if (calc_list.size())
	{
		timeCalc = (calc_list.back());
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
	FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->GetTraceInf(m_threadId);

	if(TraceInfo)//如果查找到
	{
		exitTimeCalc(TraceInfo->calc_list);
		if (!this->m_displayFlag)
		{
			return ;
		}	
		ftime(&TraceInfo->EndTime);
		//-------------------
		if (TraceInfo->deep < 1)
		{
			CTimeCalcManager::instance()->printLog((char *)"%s","//ERRERRERRERRERRERRERRERR");
			return;
		}
	
		insertExitInfo(TraceInfo);

		TraceInfo->deep--;

		if (TraceInfo->deep == 0)
		{
			CTimeCalcManager::instance()->printLog((char *)"%s", TraceInfo->up_string.c_str());
			CTimeCalcManager::instance()->DestroyTraceInf(TraceInfo, m_threadId);
		}

	}
	else
	{
		CTimeCalcManager::instance()->printLog((char *)"%s","//ERRERRERRERRERRERRERRERR");
		return;
	}

}
CTimeCalc::~CTimeCalc()
{

	DealFuncExit();
}

CTimeCalcManager *CTimeCalcManager::_instance = NULL;
CTimeCalcManager::CTimeCalcManager():m_fp(NULL), 
										m_logName("./Debug.cpp")
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



FuncTraceInfo_t * CTimeCalcManager::CreatTraceInf(pthread_t threadId)
{	
	CGuardMutex guardMutex(m_thread_map_mutex);
	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it = m_thread_map.find(threadId);
	FuncTraceInfo_t *TraceInfo = NULL;

	if(it != m_thread_map.end())//如果查找到
	{
		TraceInfo = it->second;
	}
	else  
	{
		TraceInfo = new FuncTraceInfo_t;
		assert(TraceInfo != NULL);

		ftime(&TraceInfo->EndTime);
		
		TraceInfo->deep = 0;


		TraceInfo->up_string += "//CTimeCalcCTimeCalcCTimeCalcCTimeCalcCTimeCalcCTimeCalc\n";

		std::pair<pthread_t, FuncTraceInfo_t *> thread_map_pair(threadId, TraceInfo);

		m_thread_map.insert(thread_map_pair);
	}
	return TraceInfo;
}



void CTimeCalcManager::DestroyTraceInf(FuncTraceInfo_t *TraceInfo, pthread_t threadId)
{

	CGuardMutex guardMutex(m_thread_map_mutex);
	m_thread_map.erase(threadId);
	delete TraceInfo;
}

FuncTraceInfo_t * CTimeCalcManager::GetTraceInf(pthread_t threadI)
{
	CGuardMutex guardMutex(m_thread_map_mutex);
	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it = m_thread_map.find(threadI);
	if(it != m_thread_map.end())//如果查找到
	{
		return it->second;
	}
	
	return NULL;
}


void CTimeCalcManager::printStack(int line, char *file_name, const char* fmt, ...)
{
	threadQueueEnable(e_Mem);

	FuncTraceInfo_t *TraceInfo = GetTraceInf(pthread_self());
	if (!TraceInfo)
	{
		return ;
	}
	
	char str[4096];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(str,sizeof(str), fmt, ap);
	va_end(ap); 
	insertStackInfo(TraceInfo, line, file_name, str);

	return ;
}


void CTimeCalcManager::insertStackInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr)
{
	struct timeb cur_time;
	ftime(&cur_time);

	char tmp[128];

	//-------------------
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->up_string += "\t";
	}
	TraceInfo->up_string += "/*tag:";

	TraceInfo->up_string += pStr;
	TraceInfo->up_string += " ";

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

	snprintf(tmp, sizeof(tmp), "count %8d ", count);
	TraceInfo->up_string += tmp;
	TraceInfo->up_string += stackInf;

	snprintf(tmp, sizeof(tmp), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)pthread_self(), "wshy", cur_time.time, cur_time.millitm);
	TraceInfo->up_string += tmp;
	TraceInfo->up_string += "*/\n";

	return ;
}

void CTimeCalcManager::printfMemInfMap(pthread_t threadId)
{
#ifdef WRAP
	CalcMemManager::instance()->printfMemInfMap(threadId);
#endif
}
void CTimeCalcManager::getStackInfo(std::string &stackInf)
{
	threadQueueEnable(e_Mem);
		
	FuncTraceInfo_t *TraceInfo = GetTraceInf(pthread_self());
	if (!TraceInfo || !TraceInfo->calc_list.size())
	{
		return ;
	}
	getStackInfo(TraceInfo, stackInf);
	
	return ;
}
void CTimeCalcManager::getStackInfo(FuncTraceInfo_t *TraceInfo, std::string &stackInf)
{
	char tmp[64];
	CTimeCalc *timeCalc = NULL;
	CTimeCalcList::iterator it;
	for ( it=TraceInfo->calc_list.begin() ; it != TraceInfo->calc_list.end(); it++ )
	{
		timeCalc = *it;
		snprintf(tmp, sizeof(tmp), "%s%d_", timeCalc->m_FuncName.c_str(), timeCalc->m_Line);
		stackInf += tmp;
	}
	return ;
}



void CTimeCalcManager::InsertTrace(int line, char *file_name, pthread_t threadId, const char* content)
{
	FuncTraceInfo_t *TraceInfo = GetTraceInf(threadId);
	if (TraceInfo && !needPrint(TraceInfo->calc_list))
	{
		return ;
	}
	
	if(TraceInfo)//如果查找到
	{
		insertTraceInfo(TraceInfo, line, file_name, threadId, content);
	}  
	else
	{
		CTimeCalcManager::instance()->printLog((char *)"trace:/*%s  %d  %s*/", content, line, file_name);
	}


	return ;

}

void CTimeCalcManager::InsertStrOnly(pthread_t threadId, const char* fmt, ...)
{
	FuncTraceInfo_t *TraceInfo = GetTraceInf(threadId);
	if (TraceInfo && !needPrint(TraceInfo->calc_list))
	{
		return ;
	}
	
	char str[4096];
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(str,sizeof(str), fmt, ap);
	va_end(ap);      

	if(TraceInfo)//如果查找到
	{
		InsertStrOnlyInfo(TraceInfo, str);
	}  
	else
	{
		//CTimeCalcManager::instance()->printLog((char *)"trace:/*%s*/", str);
	}


	return ;

}

void CTimeCalcManager::InsertStrOnlyInfo(FuncTraceInfo_t *TraceInfo, char *pStr)
{
	struct timeb cur_time;
	ftime(&cur_time);

	//-------------------
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->up_string += "\t";
	}
	TraceInfo->up_string += "/*tag:";

	TraceInfo->up_string += pStr;
	TraceInfo->up_string += "*/\n";

	return ;
}


void CTimeCalcManager::insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, pthread_t threadId, const char *pStr)
{
	struct timeb cur_time;
	ftime(&cur_time);

	char tmp[128];
	snprintf(tmp, sizeof(tmp), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)threadId, "wshy", cur_time.time, cur_time.millitm);

	//-------------------
	for (int i=0; i<TraceInfo->deep; ++i)
	{
		TraceInfo->up_string += "\t";
	}
	TraceInfo->up_string += "/*tag:";

	TraceInfo->up_string += pStr;
	TraceInfo->up_string += tmp;
	TraceInfo->up_string += "*/\n";

	return ;
}

void CTimeCalcManager::InsertHex(int line, char *file_name, char *psBuf, int nBufLen)
{
	threadQueueEnable(e_Mem);

	char time_tmp[128];
	strcpy(time_tmp, "wshy");


	struct timeb cur_time;
	ftime(&cur_time);
	
	char str[4096];
	/* save log msg in file */
	snprintf(str, sizeof(str), "hex%s:[%s][%4d]len=%4d\n", __FUNCTION__, file_name, line,nBufLen);

	/* save log msg in file */
	int j = 0;
	char sLine[100], sTemp[12];
	for	(int i=0; i<nBufLen; i++)
	{
		
		/* initialize a new line */
		if (j==0)
		{
			memset (sLine,	' ', sizeof(sLine));
			sprintf (sTemp,	"%04d:", i );
			memcpy (sLine, sTemp, 5);
			sprintf (sTemp, ":%04d", i+15 );
			memcpy (sLine+72, sTemp, 5);
		}

		/* output psBuf value in hex */
		snprintf(sTemp, sizeof(sTemp), "%02X ", (unsigned	char)psBuf[i]);
		memcpy( &sLine[j*3+5+(j>7)], sTemp, 3);

		/* output psBuf in ascii */
		if ( isprint (psBuf[i]))
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
			snprintf(str+strlen(str), sizeof(str)-strlen(str), "%s\n", sLine);
			j=0;
		}
	}

	/* last line */
	if (j)
	{
		sLine[77]=0;
		snprintf(str+strlen(str), sizeof(str)-strlen(str), "%s\n",	sLine);
	}
	snprintf(str+strlen(str), sizeof(str)-strlen(str), "%80.80s\n", SINGLE_LINE);
	//--------------------------------------




	snprintf(str+strlen(str), sizeof(str)-strlen(str), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)pthread_self(), time_tmp, cur_time.time, cur_time.millitm);

	FuncTraceInfo_t *TraceInfo = GetTraceInf(pthread_self());
	
	if(TraceInfo)//如果查找到
	{	
		
		//-------------------
		for (int i=0; i<TraceInfo->deep; ++i)
		{
			TraceInfo->up_string += "\t";
		}
		TraceInfo->up_string += "/*tag:";
		TraceInfo->up_string += str;
		TraceInfo->up_string += "*/\n";
		
		
	}  
	else
	{
		printLog((char *)"trace:/*%s*/", str);
	}
	return ;

}


void CTimeCalcManager::InsertTag(int line, char *file_name, const char* content)
{
	
	struct timeb cur_time;
	ftime(&cur_time);

	char str[256];
	snprintf(str, sizeof(str), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)pthread_self(), "huang_yuan", cur_time.time, cur_time.millitm);

	printLog((char *)"trace:/*%s %s*/", content, str);
	return ;
}

void CTimeCalcManager::DispAll()
{

	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it;
	FuncTraceInfo_t *TraceInfo = NULL;

	CGuardMutex guardMutex(m_thread_map_mutex);
	for(it = m_thread_map.begin(); it != m_thread_map.end(); it++)
	{
		TraceInfo = it->second;
		if (TraceInfo)
		{
			//printf("%s\n", TraceInfo->up_string.c_str());
		}
	}

	std::string backTrace;
#ifdef WRAP	
	CalcMemManager::instance()->getBackTrace(backTrace);
#endif
	//printf("backTrace  %s\n", backTrace.c_str());

	printLog((char *)"%s", "#if 0");
       for(it = m_thread_map.begin(); it != m_thread_map.end(); it++)
	{
		TraceInfo = it->second;
		if (TraceInfo)
		{
			printLog((char *)"%s", TraceInfo->up_string.c_str());
		}
	}
	printLog((char *)"%s", "#endif");

	return ;
}
void CTimeCalcManager::DispTraces(int signo)
{
	threadQueueEnable(e_Mem);

	CGuardMutex guardMutex(m_thread_map_mutex);
	printLog((char *)"//%s    %2d","Except    DispTraces", signo);
	
	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it;

	FuncTraceInfo_t *TraceInfo = NULL;
       for(it = m_thread_map.begin(); it != m_thread_map.end(); it++)
	{
		TraceInfo = it->second;
		
		printLog((char *)"%s", TraceInfo->up_string.c_str());
	}
	if ((signo == SIGSEGV) || (signo == SIGINT) )
	{
		char signo_inf[64];
		switch(signo) 
		{
			case SIGSEGV:
				strcpy(signo_inf, "SIGSEGV");
				break;
			case SIGINT:
				strcpy(signo_inf, "SIGINT");
				break;
			default:
				break;
		}
		
		printLog((char *)"%s  signo:%2d    %s","//ERRERRERRERRERRERRERRERR", signo, signo_inf);
		exit(0);
	}

	return ;
}

bool CTimeCalcManager::needPrint(CTimeCalcList &calc_list)
{
	CTimeCalc *timeCalc = NULL;
	if (calc_list.size())
	{	
		timeCalc = (calc_list.back());
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
	fp = fopen (sLogName, "a+");
	return fp;
}
void CTimeCalcManager::printLog(char *sFmt, ...)
{

	va_list	ap;

	FILE *fp = NULL;

	/* open log file */ 

	CGuardMutex guardMutex(m_logFileMutex);
	fp = openLog (m_logName);
	if (fp == NULL)
		return ;

	/* save log msg in file */


	va_start(ap, sFmt);
	vfprintf(fp, sFmt, ap);
	va_end(ap);

	//time_t timep;
	//time(&timep);
	
	char time_tmp[128];
	strcpy(time_tmp, "creat by huang_yuan@dahuatech.com1");
	time_tmp[strlen(time_tmp)-1] = '\0';
	
	fprintf(fp, "//thread id:%16d   %s\n\n", (int)pthread_self(), time_tmp);


	fprintf(fp, "\n");
	fflush(fp);

	/* close file */
	fclose (fp);


	return ;
}



CTimeCalcInfManager *CTimeCalcInfManager::_instance = NULL;

CTimeCalcInfManager::CTimeCalcInfManager() : m_maxListSize(4), m_isLocked(false)
{
	pthread_create(&m_threadId, NULL,threadFunc,NULL);
}

CTimeCalcInfManager *CTimeCalcInfManager::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guardMutex(g_insMutexCalc);
		if (NULL == _instance)
		{
			_instance = new CTimeCalcInfManager;
		}
	}
	return _instance;
}

void *CTimeCalcInfManager::calcMalloc(int size)
{
	if (size <= 0)
	{
		return NULL;
	}
	
	void *pMem = NULL;
#ifdef WRAP
	pMem = __real_malloc(size);
#else
	pMem = malloc(size);
#endif
	((char *)pMem)[0] = '\n';
	return pMem;
}

void CTimeCalcInfManager::calcFree(void *pMem)
{
	if (pMem == NULL)
	{
		return ;
	}
#ifdef WRAP
	 __real_free(pMem);
#else
	 free(pMem);
#endif
	return ;
}

RECV_DATA *CTimeCalcInfManager::createRecvData(int contentLen)
{
	RECV_DATA *pRecvData = (RECV_DATA *)calcMalloc(sizeof(RECV_DATA));
	TimeCalcInf *pCalcInf = &pRecvData->calcInf;
	pCalcInf->m_pContent = (char *)calcMalloc(contentLen+1);
	
	pCalcInf->m_opr = TimeCalcInf::e_none;
	pCalcInf->m_threadId = -1;
	pCalcInf->m_line = -1;
	pCalcInf->m_fileName = NULL;
	pCalcInf->m_funcName = NULL;
	pCalcInf->m_displayLevel = -1;
	return pRecvData;
	
}
void CTimeCalcInfManager::destroyRecvData(RECV_DATA *pRecvData)
{
	calcFree(pRecvData->calcInf.m_pContent);
	calcFree(pRecvData);
}
void CTimeCalcInfManager::threadProc()
{	
	while(1)
	{

		if(m_recvList.empty())
		{
			usleep(10 * 1000);
			continue;
		}
		m_recvListMutex.Enter();
		struct node *pNode =  m_recvList.begin();
		RECV_DATA *pRecvData = recvDataContain(pNode);
		m_recvList.pop_front();	
		m_recvListMutex.Leave();
		
		dealRecvData(&pRecvData->calcInf);
		destroyRecvData(pRecvData);
	}
}


void* CTimeCalcInfManager::threadFunc(void *pArg)
{
	CTimeCalcInfManager::instance()->threadProc();
	return NULL;
}

void CTimeCalcInfManager::dealRecvData(TimeCalcInf *pCalcInf)
{	
	threadQueueEnable(e_Mem);	

	TimeCalcInf::TimeCalcOpr &opr = pCalcInf->m_opr;
	int threadId = pCalcInf->m_threadId;
	int line = pCalcInf->m_line;
	char *file_name = pCalcInf->m_fileName;
	char *func_name = pCalcInf->m_funcName;
	int display_level = pCalcInf->m_displayLevel;
	const char *content = pCalcInf->m_pContent;

	switch (opr)
	{
		case TimeCalcInf::e_createCandy:
			{
				CTimeCalc *pTimeCalc = new CTimeCalc(line, file_name, func_name, display_level, threadId);
				if (pTimeCalc == NULL)
				{
					break;
				}
				break;
			}
		case TimeCalcInf::e_destroyCandy:
			{
				FuncTraceInfo_t *TraceInfo = CTimeCalcManager::instance()->GetTraceInf(threadId);
				if (TraceInfo == NULL || TraceInfo->calc_list.size() == 0)
				{
					break ;
				}

				CTimeCalc *pTimeCalc = TraceInfo->calc_list.back();
				delete pTimeCalc;
				break;
			}
		case TimeCalcInf::e_insertTrace:
			{
				CTimeCalcManager::instance()->InsertTrace(line, file_name, threadId, content);
				break;

			}
		case TimeCalcInf::e_dispAll:
			{
				CTimeCalcManager::instance()->DispAll();
				break;

			}
		case TimeCalcInf::e_insertTag:
			{
				CTimeCalcManager::instance()->InsertTag(line, file_name, content);
				break;

			}
		case TimeCalcInf::e_InsertStrOnly:
			{
				CTimeCalcManager::instance()->InsertStrOnly(threadId, content);
				break;
			}
		case TimeCalcInf::e_printfMemInfMap:
			{
				CTimeCalcManager::instance()->printfMemInfMap(threadId);
				break;
			}
		default:
			break;
	}
	return ;
}

void CTimeCalcInfManager::pushRecvData(RECV_DATA *pRecvData)
{
	if (pRecvData == NULL)
	{
		return ;
	}
	
	m_recvListMutex.Enter();
	m_recvList.push_back(&pRecvData->node);
	m_recvListMutex.Leave();
	return ;
}


