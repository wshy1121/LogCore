#ifndef _LOCALE_FILE_H
#define _LOCALE_FILE_H
#include <string>
#include "IFile.h"
#include "link_tool.h"

class CLocaleFile : public IFile
{
public:
	CLocaleFile(IFile::FileKey &fileKey);
	~CLocaleFile();
public:
	static bool parseKey(const std::string &path, IFile::FileKey &fileKey);
public:
	virtual bool open();
	virtual int write(const char *data, int dataLen);
	virtual int close();
	virtual size_t size();
	virtual bool clean();
	virtual bool isOnline(){return true;}
	virtual void reConnect(){}
private:
	CPthreadMutex m_fpMutex;
	FILE *m_fp;
};
#endif
