#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_107 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_107(void) : 
		CDbUpgrader(_T("1.0.0.106"), _T("1.0.0.107"))
	{
	}

	virtual ~CUpgrader1_0_0_107(void)
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
			db.execDML(_T("drop index idxFeedItem"));
			db.execDML(_T("CREATE INDEX idxFeedItem ON news_item (feed_id, retrieved, readtime)"));

			db.execDML(_T("update configurations set db_ver=107;"));

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
