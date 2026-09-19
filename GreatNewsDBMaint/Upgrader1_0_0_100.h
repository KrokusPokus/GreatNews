#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_100 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_100(void) : 
		CDbUpgrader(_T("1.0.0.99"), _T("1.0.0.100"))
	{
	}

	virtual ~CUpgrader1_0_0_100(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			CString sql;
			sql += _T("alter table feed add column parent_feed_id integer;");
			sql += _T("alter table feed add column news_id integer;");
			sql += _T("update configurations set db_ver=100;");
			db.execDML(sql);
			db.close();

		}
		catch(CppSQLite3Exception& e)
		{
			m_errorMsg = e.errorMessage();
			return false;
		}

		return true;
	}

};
