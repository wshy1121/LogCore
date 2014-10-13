 #include "Global.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include "Global.h"
#include <map>
#include <stdarg.h>
#include <sys/stat.h>
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

pthread_mutex_t  *CTimeCalc::m_thread_map_mutex = NULL;

std::map<pthread_t, FuncTraceInfo_t *> CTimeCalc::m_thread_map; 
std::map<std::string, int > CTimeCalc::m_stack_inf_map;



/*****************************************************************************/
/* FUNC:   int OpenLogFile (char *sLogFilePath, char *sLogName,              */
/*                          int nLogSwitchMode, int nLogSize,                */
/*                          char *sDate, FILE *fp)                           */
/* INPUT:  sLogFilePath: 日志路径                                            */
/*         sLogName: 日志文件名                                              */
/*         nLogSwitchMode: 日志切换模式                                      */
/*                   LOG_SWITCH_MODE_SIZE, LOG_SWITCH_MODE_DATE              */
/*         nLogSize: LOG_SWITCH_MODE_SIZE模式下文件大小                      */
/*         sDateTime: 当前时间, YYYYMMDDhhmmss                               */
/* OUTPUT: fp: 打开的日志文件的指针                                          */
/* RETURN: 0: 成功, 其它: 失败                                               */
/* DESC:   根据nLogSwitchMode, 打开日志文件                                  */
/*         LOG_SWITCH_MODE_SIZE: 当文件大小(M)达到nLogSize, 切换到新文件,    */
/*                               原文件改名为文件名中带有时间                */
/*                               xx.log.YYYYMMDDhhmmss                       */
/*         LOG_SWITCH_MODE_DATE: 日志文件名带有日期, xx.log.YYYYMMDD         */
/*****************************************************************************/
static FILE *OpenLogFile (char *sLogFilePath, char *sLogName, int nLogSwitchMode, int nLogSize, char *sDateTime ,int nLogMode)
{
	FILE *fp = NULL;
	char		sFullLogName[MAX_LOG_PATH_LEN + MAX_LOG_NAME_LEN];
	char		sFullBakLogName[MAX_LOG_PATH_LEN + MAX_LOG_NAME_LEN];
	int			nReturnCode;
	struct stat	statbuf;

	memset (sFullLogName, 0x00, sizeof (sFullLogName));

	/* set log file name */
	if (!sLogName || strlen(sLogName) == 0)
		sprintf (sFullLogName, "%s/%s", ".", LOG_DEFAULT_NAME);
	else
		sprintf (sFullLogName, "%s/%s", ".", sLogName);

	if(nLogMode == LOG_MODE_ERROR)
	{
		strncat (sFullLogName, ".Error", 6);
	}
	else
	{
		if (nLogSwitchMode == LOG_SWITCH_MODE_DATE)
		{
			/* append date in log file name */
			strncat (sFullLogName, ".", 1);
			strncat (sFullLogName, sDateTime, 8);
		}
		else
		{
			/* this is LOG_SWITCH_MODE_SIZE */
			/* check file size */
			memset (&statbuf, 0x00, sizeof(statbuf));
			nReturnCode = stat (sFullLogName, &statbuf);
			if ( nReturnCode == 0 && statbuf.st_size >= LOG_SIZE_UNIT * nLogSize )
			{
				memset (sFullBakLogName, 0x00, sizeof(sFullBakLogName));
				sprintf (sFullBakLogName, "%s.%s", sFullLogName, sDateTime);
				rename (sFullLogName, sFullBakLogName);
			}
		}
	}
	/* open log file */
	fp = fopen (sFullLogName, "a+");
	return fp;
}
 
int Debug_print(char *sLogName, int nLogMode, char *sFmt, ...)
{
	char tmp[32];
	va_list	ap;

	FILE *fp = NULL;

	va_start(ap, sFmt);
	vsnprintf(tmp, sizeof(tmp), sFmt, ap);
	va_end(ap);
	if (strlen(tmp) == 0)
	{
		return 0;
	}
	/* open log file */ 

	fp = OpenLogFile ((char *)".", sLogName, 1, 10, (char *)"cpp", nLogMode);
	if (fp == NULL)
		return (-1);

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


	return 0;
}
void NextStep(const char *function, const char *fileName, int line)
{
	char s[80];
	printf("%s  %s  %4d\n", function, fileName, line);
	fgets(s, sizeof(s), stdin);
	return ;
}
void CTimeCalc::calcStartMem()
{
	FILE *fp = fopen("/proc/buddyinfo", "rb");
	if (!fp)
	{
		return ;
	}
	
	char buf[256];
	rewind(fp);
	for (int i=0;i<64; ++i)
	{
		memset(buf, 0, sizeof(buf));
		char *ptr1 = fgets(buf, sizeof(buf), fp);
		if (!ptr1)//说明数据已经读取完毕
		{
			break;
		}
		
		char *ptr2 = strpbrk(ptr1, " ");	
		int num = 0;
		num = atoi(ptr2);
		m_startMem[i] = num;
	}
	fclose(fp);
	return ;
}

void CTimeCalc::calcEndMem()
{
	FILE *fp = fopen("/proc/buddyinfo", "rb");
	if (!fp)
	{
		return ;
	}
	char buf[256];
	
	rewind(fp);
	for (int i=0;i<64; ++i)
	{
		memset(buf, 0, sizeof(buf));
		char *ptr1 = fgets(buf, sizeof(buf), fp);
		if (!ptr1)//说明数据已经读取完毕
		{
			break;
		}
		
		char *ptr2 = strpbrk(ptr1, " ");	
		int num = 0;
		num = atoi(ptr2);
		m_endMem[i] = num;
	}
	fclose(fp);
	return ;
}

void CTimeCalc::InitMutex()
{
	//初始化部分
	if (!m_thread_map_mutex)
	{
		m_thread_map_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(m_thread_map_mutex, NULL);
	}
}

bool CTimeCalc::isInitFinished()
{
	if (m_thread_map_mutex)
	{
		return true;
	}
	return false;
}
CTimeCalc::CTimeCalc(int line, char *file_name, char *func_name, int display_level) : 	m_displayFlag(true), 
																				m_DisplayLevel(display_level), 
																				m_noDisplayLevel(display_level), 
																				m_Line(line), 
																				m_FileName(file_name), 
																				m_FuncName(func_name)
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return;
	}

	queue_node->enable[e_TimeCalc] = false;
	
	InitMutex();
	ftime(&m_StartTime);
	DealFuncEnter();

	queue_node->enable[e_TimeCalc] = true;

}

FuncTraceInfo_t * CTimeCalc::GreatTraceInf()
{
	pthread_t thread_id = pthread_self();
	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it = m_thread_map.find(thread_id);
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

		std::pair<pthread_t, FuncTraceInfo_t *> thread_map_pair(thread_id, TraceInfo);

		m_thread_map.insert(thread_map_pair);
	}
	return TraceInfo;
}

FuncTraceInfo_t * CTimeCalc::GetTraceInf()
{
	pthread_t thread_id;
	thread_id = pthread_self(); 

	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it = m_thread_map.find(thread_id);
	if(it != m_thread_map.end())//如果查找到
	{
		return it->second;
	}
	return NULL;

}

void CTimeCalc::insertEnterInfo(FuncTraceInfo_t *TraceInfo)
{
	char tmp[64];

	char time_tmp[128];
	snprintf(time_tmp, sizeof(time_tmp), "level  %4d ", m_DisplayLevel);
	
	snprintf(tmp, sizeof(tmp), ":  %4d  thread id:  %16d  %s", m_Line, (int)pthread_self(), time_tmp);

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

void CTimeCalc::insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr)
{
	struct timeb cur_time;
	ftime(&cur_time);

	char tmp[128];
	snprintf(tmp, sizeof(tmp), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)pthread_self(), "wshy", cur_time.time, cur_time.millitm);

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

void CTimeCalc::DealFuncEnter()
{
	pthread_mutex_lock(m_thread_map_mutex);
	FuncTraceInfo_t *TraceInfo = GreatTraceInf();
	pthread_mutex_unlock(m_thread_map_mutex);
	
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
	pthread_mutex_lock(m_thread_map_mutex);
	FuncTraceInfo_t *TraceInfo = GetTraceInf();
	pthread_mutex_unlock(m_thread_map_mutex);

	exitTimeCalc(TraceInfo->calc_list);
	if(TraceInfo)//如果查找到
	{
		if (!this->m_displayFlag)
		{
			return ;
		}	
		pthread_t thread_id;
		thread_id = pthread_self(); 
		ftime(&TraceInfo->EndTime);
		//-------------------
		if (TraceInfo->deep < 1)
		{
			pthread_mutex_lock(m_thread_map_mutex);
			Debug_print((char *)"Debug", 3, (char *)"%s","//ERRERRERRERRERRERRERRERR");
			pthread_mutex_unlock(m_thread_map_mutex);
			return;
		}
	
		insertExitInfo(TraceInfo);

		TraceInfo->deep--;

		if (TraceInfo->deep == 0)
		{
			
			pthread_mutex_lock(m_thread_map_mutex);
			Debug_print((char *)"Debug", 3, (char *)"%s", TraceInfo->up_string.c_str());
			m_thread_map.erase(thread_id);
			pthread_mutex_unlock(m_thread_map_mutex);
			delete TraceInfo;
		}

	}
	else
	{
		pthread_mutex_lock(m_thread_map_mutex);
		Debug_print((char *)"Debug", 3, (char *)"%s","//ERRERRERRERRERRERRERRERR");
		pthread_mutex_unlock(m_thread_map_mutex);
		return;
	}

}
CTimeCalc::~CTimeCalc()
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return ;
	}
	queue_node->enable[e_TimeCalc] = false;

	DealFuncExit();

	queue_node->enable[e_TimeCalc] = true;

}




bool CTimeCalc::needPrint(CTimeCalcList &calc_list)
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

void CTimeCalc::printStack(int line, char *file_name, const char* fmt, ...)
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return ;
	}
	queue_node->enable[e_TimeCalc] = false;

	InitMutex();
	pthread_mutex_lock(m_thread_map_mutex);
	FuncTraceInfo_t *TraceInfo = GetTraceInf();
	pthread_mutex_unlock(m_thread_map_mutex);	
	if (!TraceInfo)
	{
		queue_node->enable[e_TimeCalc] = true;
		return ;
	}

	va_list ap;
	va_start(ap,fmt);
	char str[4096];
	vsnprintf(str,sizeof(str), fmt, ap);

	insertStackInfo(TraceInfo, line, file_name, str);

	queue_node->enable[e_TimeCalc] = true;
	return ;
}

void CTimeCalc::insertStackInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, char *pStr)
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
bool CTimeCalc::getStackInfo(std::string &stackInf)
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return true;
	}
	queue_node->enable[e_TimeCalc] = false;
	
	InitMutex();
	pthread_mutex_lock(m_thread_map_mutex);
	FuncTraceInfo_t *TraceInfo = GetTraceInf();
	pthread_mutex_unlock(m_thread_map_mutex);	
	if (!TraceInfo || !TraceInfo->calc_list.size())
	{
		queue_node->enable[e_TimeCalc] = true;
		return false;
	}
	getStackInfo(TraceInfo, stackInf);
	
	queue_node->enable[e_TimeCalc] = true;
	return true;
}
void CTimeCalc::getStackInfo(FuncTraceInfo_t *TraceInfo, std::string &stackInf)
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
void CTimeCalc::InsertTrace(int line, char *file_name, const char* fmt, ...)
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return;
	}

	queue_node->enable[e_TimeCalc] = false;

	InitMutex();
	pthread_mutex_lock(m_thread_map_mutex);
	FuncTraceInfo_t *TraceInfo = GetTraceInf();
	pthread_mutex_unlock(m_thread_map_mutex);
	if (!needPrint(TraceInfo->calc_list))
	{
		queue_node->enable[e_TimeCalc] = true;
		return ;
	}

	va_list ap;
	va_start(ap,fmt);
	char str[4096];
	vsnprintf(str,sizeof(str), fmt, ap);

	if(TraceInfo)//如果查找到
	{
		insertTraceInfo(TraceInfo, line, file_name, str);
	}  
	else
	{
		pthread_mutex_lock(m_thread_map_mutex);
		Debug_print((char *)"Debug", 3, (char *)"trace:/*%s*/", str);
		pthread_mutex_unlock(m_thread_map_mutex);
	}

	va_end(ap);      

	queue_node->enable[e_TimeCalc] = true;
	return ;

}

void CTimeCalc::InsertHex(int line, char *file_name, char *psBuf, int nBufLen)
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return ;
	}
	queue_node->enable[e_TimeCalc] = false;

	InitMutex();

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

	pthread_mutex_lock(m_thread_map_mutex);	
	FuncTraceInfo_t *TraceInfo = GetTraceInf();
	pthread_mutex_unlock(m_thread_map_mutex);
	
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
		pthread_mutex_lock(m_thread_map_mutex);	
		Debug_print((char *)"Debug", 3, (char *)"trace:/*%s*/", str);
		pthread_mutex_unlock(m_thread_map_mutex);
	}
 	queue_node->enable[e_TimeCalc] = true;
	return ;

}


void CTimeCalc::InsertTag(int line, char *file_name, const char* fmt, ...)
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return;
	}
	queue_node->enable[e_TimeCalc] = false;

	InitMutex();
	
	va_list ap;
	va_start(ap,fmt);

	//time_t timep;
	//time(&timep);

	char time_tmp[128];
	strcpy(time_tmp, "wshy");

	struct timeb cur_time;
	ftime(&cur_time);
	
	char str[1024];
	vsnprintf(str,sizeof(str), fmt, ap);
	snprintf(str+strlen(str), sizeof(str)-strlen(str), "    %4d    %s  %16d  %s    %16ld  ms %4d", line, file_name, (int)pthread_self(), time_tmp, cur_time.time, cur_time.millitm);

	pthread_mutex_lock(m_thread_map_mutex);
	Debug_print((char *)"Debug", 3, (char *)"trace:/*%s*/", str);
	pthread_mutex_unlock(m_thread_map_mutex);

	va_end(ap);

	queue_node->enable[e_TimeCalc] = true;
	return ;
}

void CTimeCalc::DispAll()
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return ;
	}
	queue_node->enable[e_TimeCalc] = false;

	InitMutex();

	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it;
	FuncTraceInfo_t *TraceInfo = NULL;

	pthread_mutex_lock(m_thread_map_mutex);
	Debug_print((char *)"Debug", 3, (char *)"%s", "#if 0");
       for(it = m_thread_map.begin(); it != m_thread_map.end(); it++)
	{
		TraceInfo = it->second;
		if (TraceInfo)
		{
			Debug_print((char *)"Debug", 3, (char *)"%s", TraceInfo->up_string.c_str());
		}
	}
	Debug_print((char *)"Debug", 3, (char *)"%s", "#endif");
	pthread_mutex_unlock(m_thread_map_mutex);

	queue_node->enable[e_TimeCalc] = true;
	return ;
}
void CTimeCalc::DispTraces(int signo)
{
	ThreadNode *queue_node = ThreadQueue::instance()->getQueueNode(pthread_self());
	if (!queue_node->enable[e_TimeCalc])
	{
		return ;
	}
	queue_node->enable[e_TimeCalc] = false;

	InitMutex();
	
	pthread_mutex_lock(m_thread_map_mutex);
	Debug_print((char *)"Debug", 3, (char *)"//%s    %2d","Except    DispTraces", signo);
	
	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it;

	FuncTraceInfo_t *TraceInfo = NULL;
       for(it = m_thread_map.begin(); it != m_thread_map.end(); it++)
	{
		TraceInfo = it->second;
		
		Debug_print((char *)"Debug", 3, (char *)"%s", TraceInfo->up_string.c_str());
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
		
		Debug_print((char *)"Debug", 3, (char *)"%s  signo:%2d    %s","//ERRERRERRERRERRERRERRERR", signo, signo_inf);
		exit(0);
	}
	pthread_mutex_unlock(m_thread_map_mutex);

	queue_node->enable[e_TimeCalc] = true;
	return ;
}


void CTimeCalc::BackTrace()
{
#ifdef WRAP
	//CTimeCalc timeCalc(__LINE__, (char *)__FILE__, (char *)__FUNCTION__);
       void *stack_addr[10];
       int layer;
       int i;
	std::string traceInf = "addr2line -e ./Challenge_Debug -f -C  ";
	char tmp[256];
	
	/* 通过调用libc函数实现 */
	layer = backtrace(stack_addr, 10);
	for(i = 0; i < layer; i++)
	{
		snprintf(tmp, sizeof(tmp), "%p  ", stack_addr[i]);
		traceInf += tmp;
	}
	printf("%s\n", traceInf.c_str());
	//CTimeCalc::InsertTrace(__LINE__, (char *)__FILE__, "%s", traceInf.c_str());
#endif
	return ;
}

