#pragma once
#include <atltime.h>
#include "GMTimeLib.h"

#include "dbupgrader.h"

class CUpgrader1_0_0_91 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_91(void) : 
		CDbUpgrader(_T("1.0.0.90"), _T("1.0.0.91"))
	{
	}

	virtual ~CUpgrader1_0_0_91(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			db.execDML(_T("alter table feed add column unread_count integer;"));
			db.execDML(_T("alter table feed_group add column unread_count integer;"));
			db.execDML(_T("alter table tag_info add column unread_count integer;"));
			db.execDML(_T("alter table news_watch add column unread_count integer;"));
			db.execDML(_T("update configurations set db_ver=91;"));

			db.execDML(_T("vacuum;"));

			db.execDML(_T("update feed set unread_count = (select count(*) from news_item where news_item.feed_id=feed.feed_id and readtime is null);"));
			db.execDML(_T("update feed_group set unread_count = (select sum(feed.unread_count) from feed where feed.feed_group_id=feed_group.feed_group_id);"));
			db.execDML(_T("update feed_group set unread_count = (select count(*) from news_item where readtime is null) where feed_group_id=1;"));
			db.execDML(_T("update news_watch set unread_count = (select count(*) from news_item where readtime is null and news_id in (select news_id from watch_item where watch_item.watch_id=news_watch.watch_id));"));
			db.execDML(_T("update tag_info set unread_count = (select count(*) from news_item where readtime is null and news_id in (select news_id from item_tag where item_tag.tag_id=tag_info.tag_id));"));
		}
		catch(CppSQLite3Exception& e)
		{
			m_errorMsg = e.errorMessage();
			return false;
		}

		return true;
	}

};
