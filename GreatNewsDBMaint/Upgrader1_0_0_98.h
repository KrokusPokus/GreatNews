#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_98 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_98(void) : 
		CDbUpgrader(_T("1.0.0.97"), _T("1.0.0.98"))
	{
	}

	virtual ~CUpgrader1_0_0_98(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			CString sql = _T("CREATE INDEX idxFeedItem ON news_item (feed_id, readtime);");
			sql += _T("alter table news_item add column commentCnt integer;");
			sql += _T("alter table news_item add column commentRss varchar(254);");
			sql += _T("alter table news_item add column commentUrl varchar(254);");
			sql += _T("drop index idxFeedId;");
			sql += _T("update configurations set db_ver=98;");
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
