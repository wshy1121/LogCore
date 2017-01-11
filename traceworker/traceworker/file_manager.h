#ifndef _FILE_MANAGER_H
#define _FILE_MANAGER_H
#include <stdio.h>
#include <map>
#include "IFile.h"
#include "trace_base.h"
#include "link_tool.h"

class CFileManager
{
public:
	typedef std::map<IFile::FileKey, IFileHander> FileMap;
	
	IFileHander getFileHander(const std::string &path, std::string &clientIpAddr);
	static CFileManager *instance();
private:
	std::string nowTime();
	std::string &addFileAddr(std::string &fileName, std::string &clientIpAddr);
	std::string &addFileTime(std::string &fileName);
	bool getFileKey(const std::string &path, IFile::FileKey &fileKey);
	IFileHander createFileHander(IFile::FileKey &fileKey);
private:
	static CFileManager *_instance;

	FileMap m_fileMap;
	CPthreadMutex m_fileMapMutex;
};
#endif
