#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>

#include "dbupgrader.h"

class CUpgrader1_0_0_94 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_94(void) : 
		CDbUpgrader(_T("1.0.0.93"), _T("1.0.0.94"))
	{
	}

	virtual ~CUpgrader1_0_0_94(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{

			CppSQLite3DB db;
			db.open(dbFilePath);

			db.execDML(_T("alter table feed add column hd_last_mod varchar varchar;"));
			db.execDML(_T("alter table feed add column hd_etag varchar;"));
			db.execDML(_T("alter table feed add column update_freq int;"));
			db.execDML(_T("alter table feed add column store_dur int;"));
			db.execDML(_T("update configurations set db_ver=94;"));
		}
		catch(CppSQLite3Exception& e)
		{
			m_errorMsg = e.errorMessage();
			return false;
		}

		return true;
	}

};
