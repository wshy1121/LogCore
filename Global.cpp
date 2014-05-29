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


pthread_mutex_t  *CTimeCalc::thread_map_mutex = NULL;



static std::map<pthread_t, FuncTraceInfo_t *> m_thread_map; 

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

	fp = OpenLogFile ((char *)".", sLogName, 1, 10, (char *)"log", nLogMode);
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
	//strcpy(time_tmp, ctime(&timep));
	time_tmp[strlen(time_tmp)-1] = '\0';
	
	fprintf(fp, "//thread id:%d   %s\n\n", (int)pthread_self(), time_tmp);


	fprintf(fp, "\n");
	fflush(fp);

	/* close file */
	fclose (fp);


	return 0;
}
void NextStep(const char *function, const char *fileName, int line)
{
	char s[80];
	printf("%s  %s  %d\n", function, fileName, line);
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
	if (!thread_map_mutex)
	{
		thread_map_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(thread_map_mutex, NULL);
	}
}

CTimeCalc::CTimeCalc(int line, char *file_name, char *func_name, int display_type)
{
	InitMutex();

	m_Line = line;
	m_FileName = file_name;
	m_FuncName = func_name;
	m_DisplayType = display_type;
	ftime(&m_StartTime);

	DealFuncEnter();
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
		TraceInfo->undisplay_deep = 0;

		if (m_DisplayType != 0)
		{
			TraceInfo->up_string += "//CTimeCalcCTimeCalcCTimeCalcCTimeCalcCTimeCalcCTimeCalc\n";
		}
		std::pair<pthread_t, FuncTraceInfo_t *> thread_map_pair(thread_id, TraceInfo);

		m_thread_map.insert(thread_map_pair);
	}
	return TraceInfo;
}



void CTimeCalc::DealFuncEnter()
{
	pthread_mutex_lock(thread_map_mutex);
	FuncTraceInfo_t *TraceInfo = GreatTraceInf();
	pthread_mutex_unlock(thread_map_mutex);

	char tmp[64];

	char time_tmp[128];
	strcpy(time_tmp, "wshy ");
	
	snprintf(tmp, sizeof(tmp), ":  %d  thread id:  %d  %s", m_Line, (int)pthread_self(), time_tmp);

	//-------------------

	if (m_DisplayType == 0)
	{
		if (TraceInfo->deep == 0)
		{
			TraceInfo->deep++;
			TraceInfo->undisplay_deep++;
			return ;
		}
		
		for (int i=0; i<TraceInfo->deep; ++i)
		{
			TraceInfo->up_string += "    ";
		}
		TraceInfo->up_string += "//unDisplayTraces func:    ";
		TraceInfo->up_string += m_FuncName;
		TraceInfo->up_string += "()  ";

		TraceInfo->up_string += tmp;
		TraceInfo->up_string += m_FileName;
		TraceInfo->up_string += "\n";

		for (int i=0; i<TraceInfo->deep; ++i)
		{
			TraceInfo->up_string += "    ";
		}
		TraceInfo->up_string += "//{\n";

		
		TraceInfo->deep++;
		TraceInfo->undisplay_deep++;
		
		return ;

	}
	else if (TraceInfo->undisplay_deep == 0)
	{
		//-------------------
		for (int i=0; i<TraceInfo->deep; ++i)
		{
			TraceInfo->up_string += "    ";
		}
		TraceInfo->up_string += m_FuncName;
		TraceInfo->up_string += "()";

		TraceInfo->up_string += tmp;
		TraceInfo->up_string += m_FileName;

		
		TraceInfo->up_string += "\n";
		for (int i=0; i<TraceInfo->deep; ++i)
		{
			TraceInfo->up_string += "    ";
		}
		TraceInfo->up_string += "{";

		struct timeb cur_time;
		ftime(&cur_time);

		
		snprintf(time_tmp, sizeof(time_tmp), "       //    cost second: %ld  %d  %ld  %d  route", cur_time.time - TraceInfo->EndTime.time, cur_time.millitm - TraceInfo->EndTime.millitm, cur_time.time, cur_time.millitm);
		TraceInfo->up_string += time_tmp;
		TraceInfo->up_string += "\n";
		
		TraceInfo->deep++;
	}
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

void CTimeCalc::DealFuncExit()
{
	pthread_mutex_lock(thread_map_mutex);
	FuncTraceInfo_t *TraceInfo = GetTraceInf();
	pthread_mutex_unlock(thread_map_mutex);

	if(TraceInfo)//如果查找到
	{
		pthread_t thread_id;
		thread_id = pthread_self(); 
		ftime(&TraceInfo->EndTime);
		//-------------------
		if (TraceInfo->deep < 1)
		{
			pthread_mutex_lock(thread_map_mutex);
			Debug_print((char *)"Debug", 3, (char *)"%s","//ERRERRERRERRERRERRERRERR");
			pthread_mutex_unlock(thread_map_mutex);
			return;
		}

		//-------------------
		//为删除日志所做的处理
		if (m_DisplayType == 0)
		{
			if (TraceInfo->deep == 1)
			{
				TraceInfo->undisplay_deep--;
				TraceInfo->deep--;
				return ;
			}
			
			for (int i=1; i<TraceInfo->deep; ++i)
			{
				TraceInfo->up_string += "    ";
			}
			
			TraceInfo->up_string += "//}\n";

			
			
			if (TraceInfo->deep == 1)
			{
				pthread_mutex_lock(thread_map_mutex);
				Debug_print((char *)"Debug", 3, (char *)"%s", TraceInfo->up_string.c_str());
				m_thread_map.erase(thread_id);
				pthread_mutex_unlock(thread_map_mutex);
				delete TraceInfo;
			}

			
			TraceInfo->undisplay_deep--;
			TraceInfo->deep--;
			return ;
		}
		else if (TraceInfo->undisplay_deep == 0)
		{
			//-------------------
			for (int i=1; i<TraceInfo->deep; ++i)
			{
				TraceInfo->up_string += "    ";
			}
			
			TraceInfo->up_string += "}";
			char tmp[128];
			//time_t timep;
			//time(&timep);


			char time_tmp[128];
			strcpy(time_tmp, "wshy");

			struct timeb cur_time; 
			ftime(&cur_time);

			snprintf(tmp, sizeof(tmp), "        //func  cost second: %ld  %d  %ld  %d     %s  %s  ", cur_time.time - m_StartTime.time, cur_time.millitm - m_StartTime.millitm, cur_time.time, cur_time.millitm, m_FuncName.c_str(), time_tmp);

			TraceInfo->up_string += tmp;			

			TraceInfo->up_string += "\n";
			//-------------------

			TraceInfo->deep--;

			if (TraceInfo->deep == 0)
			{
				
				pthread_mutex_lock(thread_map_mutex);
				Debug_print((char *)"Debug", 3, (char *)"%s", TraceInfo->up_string.c_str());
				m_thread_map.erase(thread_id);
				pthread_mutex_unlock(thread_map_mutex);
				delete TraceInfo;
			}
		}

	}
	else
	{
		pthread_mutex_lock(thread_map_mutex);
		Debug_print((char *)"Debug", 3, (char *)"%s","//ERRERRERRERRERRERRERRERR");
		pthread_mutex_unlock(thread_map_mutex);
		return;
	}

}
CTimeCalc::~CTimeCalc()
{
	DealFuncExit();
}

void CTimeCalc::InsertTrace(int line, char *file_name, const char* fmt, ...)
{
	InitMutex();
	
	va_list ap;
	va_start(ap,fmt);

	char time_tmp[128];
	strcpy(time_tmp, "wshy");

	struct timeb cur_time;
	ftime(&cur_time);
	
	char str[4096];
	vsnprintf(str,sizeof(str), fmt, ap);

	snprintf(str+strlen(str), sizeof(str)-strlen(str), "    %d    %s  %d  %s    %ld  ms %d", line, file_name, (int)pthread_self(), time_tmp, cur_time.time, cur_time.millitm);

	pthread_mutex_lock(thread_map_mutex);
	FuncTraceInfo_t *TraceInfo = GetTraceInf();
	pthread_mutex_unlock(thread_map_mutex);

	if(TraceInfo)//如果查找到
	{
		//为删除日志所做的处理
		if (TraceInfo->undisplay_deep)
		{
			return ;
		}
		
		//-------------------
		for (int i=0; i<TraceInfo->deep; ++i)
		{
			TraceInfo->up_string += "    ";
		}
		TraceInfo->up_string += "/*tag:";
		TraceInfo->up_string += str;
		TraceInfo->up_string += "*/\n";
		
		
	}  
	else
	{
		pthread_mutex_lock(thread_map_mutex);
		Debug_print((char *)"Debug", 3, (char *)"trace:/*%s*/", str);
		pthread_mutex_unlock(thread_map_mutex);
	}

	va_end(ap);      

	return ;

}

void CTimeCalc::InsertHex(int line, char *file_name, char *psBuf, int nBufLen)
{
	if (!thread_map_mutex)
	{
		thread_map_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(thread_map_mutex, NULL);
	}

	//time_t timep;
	//time(&timep);

	char time_tmp[128];
	strcpy(time_tmp, "wshy");


	struct timeb cur_time;
	ftime(&cur_time);
	
	char str[4096];
	/* save log msg in file */
	snprintf(str, sizeof(str), "hex%s:[%s][%d]len=%d\n", __FUNCTION__, file_name, line,nBufLen);

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




	snprintf(str+strlen(str), sizeof(str)-strlen(str), "    %d    %s  %d  %s    %ld  ms %d", line, file_name, (int)pthread_self(), time_tmp, cur_time.time, cur_time.millitm);
	
	pthread_t thread_id;
	thread_id = pthread_self(); 

	pthread_mutex_lock(thread_map_mutex);
	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it = m_thread_map.find(thread_id);
	pthread_mutex_unlock(thread_map_mutex);
	
	FuncTraceInfo_t *TraceInfo = NULL;

	pthread_mutex_lock(thread_map_mutex);
	if(it != m_thread_map.end())//如果查找到
	{	pthread_mutex_unlock(thread_map_mutex);
	
		TraceInfo = it->second;
		//为删除日志所做的处理
		if (TraceInfo->undisplay_deep)
		{
			return ;
		}
		
		//-------------------
		for (int i=0; i<TraceInfo->deep; ++i)
		{
			TraceInfo->up_string += "    ";
		}
		TraceInfo->up_string += "/*tag:";
		TraceInfo->up_string += str;
		TraceInfo->up_string += "*/\n";
		
		
	}  
	else
	{
		Debug_print((char *)"Debug", 3, (char *)"trace:/*%s*/", str);
		pthread_mutex_unlock(thread_map_mutex);
	}
 
	return ;

}


void CTimeCalc::InsertTag(int line, char *file_name, const char* fmt, ...)
{
	if (!thread_map_mutex)
	{
		thread_map_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(thread_map_mutex, NULL);
	}
	
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
	snprintf(str+strlen(str), sizeof(str)-strlen(str), "    %d    %s  %d  %s    %ld  ms %d", line, file_name, (int)pthread_self(), time_tmp, cur_time.time, cur_time.millitm);

	pthread_mutex_lock(thread_map_mutex);
	Debug_print((char *)"Debug", 3, (char *)"trace:/*%s*/", str);
	pthread_mutex_unlock(thread_map_mutex);

	va_end(ap);
	
	return ;
}

void CTimeCalc::DispAll()
{
	if (!thread_map_mutex)
	{
		thread_map_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(thread_map_mutex, NULL);
	}

	std::map<pthread_t, FuncTraceInfo_t *>::const_iterator   it;
	FuncTraceInfo_t *TraceInfo = NULL;

	pthread_mutex_lock(thread_map_mutex);
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
	pthread_mutex_unlock(thread_map_mutex);
	return ;
}
void CTimeCalc::DispTraces(int signo)
{
	if (!thread_map_mutex)
	{
		thread_map_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(thread_map_mutex, NULL);
	}
	
	pthread_mutex_lock(thread_map_mutex);
	Debug_print((char *)"Debug", 3, (char *)"//%s    %d","Except    DispTraces", signo);
	
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
		
		Debug_print((char *)"Debug", 3, (char *)"%s  signo:%d    %s","//ERRERRERRERRERRERRERRERR", signo, signo_inf);
		exit(0);
	}
	pthread_mutex_unlock(thread_map_mutex);
	return ;
}


void CTimeCalc::BackTrace()
{
#if 0
	CTimeCalc timeCalc(__LINE__, (char *)__FILE__, (char *)__FUNCTION__);
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

	CTimeCalc::InsertTrace(__LINE__, (char *)__FILE__, "%s", traceInf.c_str());
#endif
	return ;
}

