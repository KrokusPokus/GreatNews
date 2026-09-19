#pragma once
#include <atltime.h>
#include "GMTimeLib.h"

#include "dbupgrader.h"

class CUpgrader1_0_0_92 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_92(void) : 
		CDbUpgrader(_T("1.0.0.91"), _T("1.0.0.92"))
	{
	}

	virtual ~CUpgrader1_0_0_92(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			db.execDML(_T("alter table news_item add column pod_url varchar;"));
			db.execDML(_T("update configurations set db_ver=92;"));

		}
		catch(CppSQLite3Exception& e)
		{
			m_errorMsg = e.errorMessage();
			return false;
		}

		return true;
	}

};
