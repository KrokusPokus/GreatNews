#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_102 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_102(void) : 
		CDbUpgrader(_T("1.0.0.101"), _T("1.0.0.102"))
	{
	}

	virtual ~CUpgrader1_0_0_102(void)
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
			sql += _T("alter table news_item add column content_code integer;");
			sql += _T("update configurations set db_ver=102;");
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
