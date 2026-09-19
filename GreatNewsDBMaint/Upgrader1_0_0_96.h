#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>

#include "dbupgrader.h"

class CUpgrader1_0_0_96 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_96(void) : 
		CDbUpgrader(_T("1.0.0.95"), _T("1.0.0.96"))
	{
	}

	virtual ~CUpgrader1_0_0_96(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);
			db.execDML(_T("alter table feed add column notify_newitem integer;"));
			db.execDML(_T("update configurations set db_ver=96;"));
		}
		catch(CppSQLite3Exception& e)
		{
			m_errorMsg = e.errorMessage();
			return false;
		}

		return true;
	}

};
