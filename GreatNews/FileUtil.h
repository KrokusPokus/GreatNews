#pragma once

#include "atlwfile.h"

class CFileUtil
{
public:
	static BOOL FileExist(LPCTSTR fileName)
	{
		return CFile::FileExists(fileName);
	}

	static BOOL DeleteFile(LPCTSTR fileName)
	{
		return CFile::Delete(fileName);
	}

	static BOOL CopyFile(LPCTSTR lpExistingFileName,  LPCTSTR lpNewFileName, BOOL bFailIfExists)
	{
		return ::CopyFile(lpExistingFileName, lpNewFileName, bFailIfExists);
	}

	static BOOL Rename(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName)
	{
		return CFile::Rename(lpExistingFileName, lpNewFileName);
	}
};

