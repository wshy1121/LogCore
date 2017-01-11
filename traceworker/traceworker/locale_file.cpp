#include <sys/stat.h>
#include "locale_file.h"

CLocaleFile::CLocaleFile(IFile::FileKey &fileKey)
{
	m_path = fileKey.path;
}

CLocaleFile::~CLocaleFile()
{

}

bool CLocaleFile::open()
{
	const char *fileName = m_path.c_str();
	CGuardMutex guarMutex(m_fpMutex);
	
	m_fp = CBase::fopen(fileName, "a+");
	if (m_fp == NULL)
	{
		printf("CLocaleFile::open  %s failed\n", fileName);
		return false;
	}
	
	return true;
}

int CLocaleFile::write(const char *data, int dataLen)
{
	CGuardMutex guarMutex(m_fpMutex);
	return fwrite(data, dataLen, 1, m_fp);
}

int CLocaleFile::close()
{
	CGuardMutex guarMutex(m_fpMutex);

	int ret = fclose(m_fp);
	if (ret == 0)
	{
		m_fp = NULL;
		return true;
	}
	return false;
}

size_t CLocaleFile::size()
{
	return CBase::filesize(m_path.c_str());
}

bool CLocaleFile::clean()
{
	FILE *fp = NULL;
	fp = CBase::fopen(m_path.c_str(), "w");
	if (fp == NULL)
	{
		return true;
	}
	fclose(fp);
	return true;
}

bool CLocaleFile::parseKey(const std::string &path, IFile::FileKey &fileKey)
{
	fileKey.type = e_localeFile;
	fileKey.serInf = path;
	fileKey.path = path;
	return true;
}



