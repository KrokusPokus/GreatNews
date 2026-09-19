#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_99 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_99(void) : 
		CDbUpgrader(_T("1.0.0.98"), _T("1.0.0.99"))
	{
	}

	virtual ~CUpgrader1_0_0_99(void)
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
			sql += _T("alter table feed add column contentCode integer;");
			sql += _T("alter table news_item add column commentTrack integer;");
			sql += _T("alter table news_item add column contentCode integer;");
			sql += _T("update configurations set db_ver=99;");
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
