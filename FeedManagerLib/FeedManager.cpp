#include "StdAfx.h"
#include "FeedManager.h"
#include <atltime.h>
#include "ExceptionBase.h"
#include "FeedManagerErrorCode.h"
#include <strsafe.h>

TCHAR CFeedManager::m_dbFileName[MAX_PATH];

void CFeedManager::Init(LPCTSTR dbFileName)
{
	StringCchCopy(m_dbFileName, MAX_PATH, dbFileName);
}

void CFeedManager::OpenDatabase(CppSQLite3DB& db)
{
	try
	{
		db.close();
		db.open(m_dbFileName);
		db.execDML(_T("PRAGMA read_uncommitted = 1;"));
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_CANNOTOPENDB, e.errorMessage());
	}
}


