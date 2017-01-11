#include "IFile.h"

IFile::IFile()
{
	
}
IFile::~IFile()
{

}

IFile::FileType IFile::tranceFileType(const std::string &path)
{
	if (path.find("@ftp://") != std::string::npos)
	{
		return IFile::e_ftpFile;
	}
	return IFile::e_localeFile;
}
