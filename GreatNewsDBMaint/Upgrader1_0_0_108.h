#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_108 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_108(void) : 
		CDbUpgrader(_T("1.0.0.107"), _T("1.0.0.108"))
	{
	}

	virtual ~CUpgrader1_0_0_108(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			db.execDML(_T("BEGIN exclusive TRANSACTION;"));
			db.execDML(_T("update feed_group set name='All News Feeds' where name='All News Channels';"));
			db.execDML(_T("alter table feed add column priority integer;"));
			db.execDML(_T("update configurations set db_ver=108;"));
			db.execDML(_T("commit transaction;"));

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
