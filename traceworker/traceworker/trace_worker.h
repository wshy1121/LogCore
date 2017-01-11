// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TRACEWORKER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TRACEWORKER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifndef _TRACE_WORKER_H
#define _TRACE_WORKER_H

#ifdef WIN32
#ifdef TRACEWORKER_EXPORTS
#define TRACEWORKER_API __declspec(dllexport)
#else
#define TRACEWORKER_API __declspec(dllimport)
#endif
#else
#define TRACEWORKER_API
#endif
#include <string>


// This class is exported from the traceworker.dll



class TRACEWORKER_API CCandy
{
public:
	CCandy(int line=__LINE__, char *file_name=(char *)__FILE__, char *func_name=(char *)"__FUNCTION__", int display_level=100);
	~CCandy();
};

class TRACEWORKER_API CBugKiller
{
public:
	static void startServer(const char *fileName);
	static void stopServer();
	static void InsertTrace(int line, char *file_name, const char *fmt, ...);
	static void InsertHex(int line, char *file_name, char *psBuf, int nBufLen);
};


#if !defined(NO_CTIME_CALC)
#define trace_start(fileName) CBugKiller::startServer(fileName)
#define trace_stop() CBugKiller::stopServer()
#define trace_level(level) CCandy candy(__LINE__, (char *)__FILE__, (char *)__FUNCTION__, level)
#define trace_worker() trace_level(100)
#define trace_unworker() trace_level(0)
#define trace_printf(format, ...)  CBugKiller::InsertTrace(__LINE__, (char *)__FILE__, format, ## __VA_ARGS__)
#define trace_str(str, strLen) CBugKiller::InsertHex(__LINE__, (char *)__FILE__, (char *)str, (int)strLen)
#else
#endif


extern TRACEWORKER_API int ntraceworker;

TRACEWORKER_API int fntraceworker(void);

#endif
