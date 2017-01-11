#include <time.h>
#include "file_manager.h"
#include "locale_file.h"

static CPthreadMutex g_insMutexCalc;

CFileManager *CFileManager::_instance = NULL;

CFileManager *CFileManager::instance()
{
	if (NULL == _instance)
	{
		CGuardMutex guarMutex(g_insMutexCalc);
		if (_instance == NULL)
		{
			_instance = new CFileManager;
		}
	}
	
	return _instance;
}

IFileHander CFileManager::getFileHander(const std::string &path, std::string &clientIpAddr)
{
	IFileHander fileHander;
	std::string filePath = path;

	filePath = addFileAddr(filePath, clientIpAddr);

	IFile::FileKey fileKey;
	if (getFileKey(filePath, fileKey) == false)
	{
		return fileHander;
	}

	{
		CGuardMutex guarMutex(m_fileMapMutex);
		FileMap::iterator iter = m_fileMap.find(fileKey);
		if (iter != m_fileMap.end())
		{
			fileHander = iter->second;
		}
		else
		{
			fileHander = createFileHander(fileKey);
			if (fileHander != NULL)
			{
				m_fileMap.insert(std::make_pair(fileKey, fileHander));
			}
		}
	}

	std::string &fileNameAddTime = fileKey.path;
	fileNameAddTime = addFileTime(fileNameAddTime);
	fileHander->setPath(fileNameAddTime);

	return fileHander;
}

IFileHander CFileManager::createFileHander(IFile::FileKey &fileKey)
{
	IFileHander fileHander;

	switch(fileKey.type)
	{
		case IFile::e_ftpFile:
			break;
		case IFile::e_localeFile:
			fileHander = IFileHander(new CLocaleFile(fileKey));
			break;
		default:
			break;
	}

	return fileHander;
}

bool CFileManager::getFileKey(const std::string &path, IFile::FileKey &fileKey)
{
	fileKey.type = IFile::tranceFileType(path);

	bool bRet = false;
	IFile::FileType &fileType = fileKey.type;
	switch(fileType)
	{
		case IFile::e_ftpFile:
			break;
		case IFile::e_localeFile:
			bRet = CLocaleFile::parseKey(path, fileKey);
			break;
		default:
			break;
	}

	if (bRet == false)
	{
		fileKey.type = IFile::e_errFile;
		fileKey.serInf.clear();
		fileKey.path.clear();
	}
	
	return bRet;
}

std::string &CFileManager::addFileAddr(std::string &fileName, std::string &clientIpAddr)
{
	int nameIndex = fileName.find_last_of('/');
	if (nameIndex > 0)
	{
		fileName = fileName.insert(nameIndex, '/' + clientIpAddr);
	}
	return fileName;
}

std::string &CFileManager::addFileTime(std::string &fileName)
{
	int nameIndex = fileName.find_last_of('.');
	if (nameIndex > 0)
	{
		fileName = fileName.insert(nameIndex, '_' + nowTime());
	}
	return fileName;
}
std::string CFileManager::nowTime()
{
	char nowTime[64];
	time_t now;
	struct tm *w = NULL;

	time(&now);
	w = CBase::localtime(&now);

	CBase::snprintf(nowTime, sizeof(nowTime), "%04d%02d%02d-%02d%02d%02d", w->tm_year+1900, w->tm_mon+1, w->tm_mday, w->tm_hour, w->tm_min, w->tm_sec);
	return nowTime;
}


