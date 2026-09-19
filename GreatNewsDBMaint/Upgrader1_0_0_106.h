#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_106 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_106(void) : 
		CDbUpgrader(_T("1.0.0.105"), _T("1.0.0.106"))
	{
	}

	virtual ~CUpgrader1_0_0_106(void)
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
			db.execDML(_T("delete from feed where feed_group_id = 0 and (parent_feed_id is null or parent_feed_id=0);"));

			CString sql;
			sql += _T("\
CREATE TABLE feedtemp(\
feed_id integer PRIMARY KEY AUTOINCREMENT,\
feed_group_id integer NOT NULL,\
url varchar(250) NOT NULL,\
name varchar(50) NOT NULL UNIQUE,\
description varchar(250),\
image varchar(250),\
website varchar(250),\
last_updated datetime,\
language varchar(20),\
usage integer,\
disabled integer,\
options1 integer,\
options2 integer,\
options3 varchar(254),\
unread_count integer,\
hd_last_mod varchar,\
hd_etag varchar,\
update_freq integer,\
store_dur integer,\
notify_newitem integer,\
limit_item integer,\
limit_item_number integer,\
keep_inactive integer,\
contentCode integer,\
parent_feed_id integer,\
news_id integer,\
use_login integer,\
login_name varchar,\
login_pwd varchar);");
			db.execDML(sql);

			sql = _T("\
insert into feedtemp \
(feed_id,\
feed_group_id,\
url,\
name,\
description,\
image,\
website,\
last_updated,\
language,\
usage,\
disabled,\
options1,\
options2,\
options3,\
unread_count,\
hd_last_mod,\
hd_etag,\
update_freq,\
store_dur,\
notify_newitem,\
limit_item,\
limit_item_number,\
keep_inactive,\
contentCode,\
parent_feed_id,\
news_id) select * from feed;");
			db.execDML(sql);

			sql = _T("drop table feed;");
			db.execDML(sql);
			sql = _T("alter table feedtemp rename to feed;");
			db.execDML(sql);

			sql = _T("\
CREATE TRIGGER OnDelFeed AFTER DELETE ON feed \
begin \
delete from news_item where news_item.feed_id=OLD.feed_id; \
delete from news_watch_channel where feed_id=OLD.feed_id; \
end; \
CREATE TRIGGER OnDelFeed2 AFTER DELETE ON feed for each row when old.news_id > 0 \
begin \
update news_item set commentTrack = null where news_item.news_id = old.news_id; \
end;");
			db.execDML(sql);

			db.execDML(_T("update configurations set db_ver=106;"));

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
