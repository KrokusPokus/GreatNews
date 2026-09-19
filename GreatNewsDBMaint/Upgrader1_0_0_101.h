#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_101 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_101(void) : 
		CDbUpgrader(_T("1.0.0.100"), _T("1.0.0.101"))
	{
	}

	virtual ~CUpgrader1_0_0_101(void)
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
			sql += _T("CREATE TRIGGER OnDelFeed2 AFTER DELETE ON feed \
for each row when old.news_id > 0 \
begin \
     update news_item set commentTrack = null where news_item.news_id = old.news_id; \
end;");
			sql += _T("CREATE TRIGGER tgrDeleteNews2 AFTER DELETE ON news_item \
for each row when old.commentTrack > 0 \
begin \
    delete from news_item where news_item.feed_id in (select feed_id from feed where feed.news_id=old.news_id); \
    delete from feed where feed.news_id=old.news_id; \
end;");
			sql += _T("update configurations set db_ver=101;");
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
