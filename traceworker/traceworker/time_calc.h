#ifndef _TIME_CALC_H
#define _TIME_CALC_H

#include <map>
#include <list>
#include <stdio.h>
#include <string>
#include "platform_base.h"
#include "mem_calc.h"
#include "link_tool.h"

class CTimeCalc;

typedef std::list<CTimeCalc *> CTimeCalcList;

typedef struct FuncTraceInfo_t
{
	TimeB EndTime;
	int deep;
	std::string upString;
	CTimeCalcList calcList;
	TraceInfoId traceInfoId;
	
}FuncTraceInfo_t;

class CTimeCalc
{
public:
	friend class CTimeCalcManager;
	static CTimeCalc *createCTimeCalc(int line, char *file_name, char *func_name, int display_level, TraceInfoId &traceInfoId);
	static void destroyCTimeCalc(CTimeCalc *pTimeCalc);
public:
	CTimeCalc(int line, char *file_name, char *func_name, int display_level, TraceInfoId &traceInfoId);
	~CTimeCalc();
private:
	void insertEnterInfo(FuncTraceInfo_t *TraceInfo);
	void insertExitInfo(FuncTraceInfo_t *TraceInfo);
	void DealFuncExit();
	void DealFuncEnter();
	void initTimeCalc(CTimeCalcList &calcList);
	void exitTimeCalc(CTimeCalcList &calcList);
	CTimeCalc *getLastTimeCalc(CTimeCalcList &calcList);
	void setDisplayFlag(CTimeCalc *timeCalc);
private:
	bool m_displayFlag;
	int m_DisplayLevel;
	TraceInfoId m_traceInfoId;
	int m_noDisplayLevel;

	int m_Line;
	std::string m_FileName;
	std::string m_FuncName;
	TimeB m_StartTime;
	
};

class CTimeCalcManager
{
public:
	static CTimeCalcManager *instance();
public:
	FuncTraceInfo_t *CreateTraceInf(TraceInfoId &traceInfoId);
	FuncTraceInfo_t *GetTraceInf(TraceInfoId &traceInfoId);
	void printStrLog(TraceInfoId &traceInfoId, const char *logStr);
	void removeTraceInf(TraceInfoId &traceInfoId);
	void DestroyTraceInf(FuncTraceInfo_t *TraceInfo);
	void InsertTrace(int line, char *file_name, TraceInfoId &traceInfoId, const char *content);
	void packetHex(char *psBuf, int nBufLen, char *str, int strLen);
	void DispAll(int clientId, const char *content);
	void cleanAll(int clientId);
private:
	bool needPrint(CTimeCalcList &calcList);
	void insertTraceInfo(FuncTraceInfo_t *TraceInfo, int line, char *file_name, TraceInfoId &traceInfoId, const char *pStr);
private:
	typedef std::map<TraceInfoId, FuncTraceInfo_t *> TraceInfoMap;
	TraceInfoMap m_traceInfoMap;
	CPthreadMutex m_traceInfoMapMutex;
	
	static CTimeCalcManager *_instance;
};
#endif

