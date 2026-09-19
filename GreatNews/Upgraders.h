#pragma once

#include <atltime.h>
#include "GMTimeLib.h"
#include <map>
#include "DbUpgrader.h"

class CUpgrader1_0_0_102 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_102(void) : 
		CDbUpgrader(_T("1.0.0.101"), _T("1.0.0.102"))
	{
	}

	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			CString sql;
			sql += _T("ALTER TABLE news_item ADD COLUMN content_code INTEGER;");
			sql += _T("UPDATE configurations SET db_ver = 102;");
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

class CUpgrader1_0_0_103 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_103(void) : 
		CDbUpgrader(_T("1.0.0.102"), _T("1.0.0.103"))
	{
	}

	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			// does nothing. The update was moved to CUpgrader1_0_0_103
			db.execDML(_T("UPDATE configurations SET db_ver = 103;"));

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

class CUpgrader1_0_0_104 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_104(void) : 
		CDbUpgrader(_T("1.0.0.103"), _T("1.0.0.104"))
	{
	}

	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			CString sql;
			sql += _T("UPDATE news_item SET mark = (SELECT COUNT(*) FROM item_tag WHERE item_tag.news_id = news_item.news_id) WHERE mark < 0;");
			sql += _T("UPDATE configurations SET db_ver = 104;");
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

class CUpgrader1_0_0_105 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_105(void) : 
		CDbUpgrader(_T("1.0.0.104"), _T("1.0.0.105"))
	{
	}

	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			CString sql;
			sql += _T("UPDATE tag_info SET unread_count = (SELECT COUNT(*) FROM item_tag WHERE item_tag.tag_id = tag_info.tag_id);");
			sql += _T("UPDATE configurations SET db_ver = 105;");
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

class CUpgrader1_0_0_106 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_106(void) : 
		CDbUpgrader(_T("1.0.0.105"), _T("1.0.0.106"))
	{
	}

	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			db.execDML(_T("BEGIN EXCLUSIVE TRANSACTION;"));
			db.execDML(_T("DELETE FROM feed WHERE feed_group_id = 0 AND (parent_feed_id IS NULL OR parent_feed_id = 0);"));

			CString sql;
			sql +=  _T("CREATE TABLE feedtemp(")
					_T("feed_id INTEGER PRIMARY KEY AUTOINCREMENT, ")
					_T("feed_group_id INTEGER NOT NULL, ")
					_T("url varchar(250) NOT NULL, ")
					_T("name varchar(50) NOT NULL UNIQUE, ")
					_T("description VARCHAR(250), ")
					_T("image VARCHAR(250), ")
					_T("website VARCHAR(250), ")
					_T("last_updated DATETIME, ")
					_T("language VARCHAR(20), ")
					_T("usage INTEGER, ")
					_T("disabled INTEGER, ")
					_T("options1 INTEGER, ")
					_T("options2 INTEGER, ")
					_T("options3 VARCHAR(254), ")
					_T("unread_count INTEGER, ")
					_T("hd_last_mod VARCHAR, ")
					_T("hd_etag VARCHAR, ")
					_T("update_freq INTEGER, ")
					_T("store_dur INTEGER, ")
					_T("notify_newitem INTEGER, ")
					_T("limit_item INTEGER, ")
					_T("limit_item_number INTEGER, ")
					_T("keep_inactive INTEGER, ")
					_T("contentCode INTEGER, ")
					_T("parent_feed_id INTEGER, ")
					_T("news_id INTEGER, ")
					_T("use_login INTEGER, ")
					_T("login_name VARCHAR, ")
					_T("login_pwd VARCHAR);");
			db.execDML(sql);

			sql =   _T("INSERT INTO feedtemp (")
					_T("feed_id, ")
					_T("feed_group_id, ")
					_T("url, ")
					_T("name, ")
					_T("description, ")
					_T("image, ")
					_T("website, ")
					_T("last_updated, ")
					_T("language, ")
					_T("usage, ")
					_T("disabled, ")
					_T("options1, ")
					_T("options2, ")
					_T("options3, ")
					_T("unread_count, ")
					_T("hd_last_mod, ")
					_T("hd_etag, ")
					_T("update_freq, ")
					_T("store_dur, ")
					_T("notify_newitem, ")
					_T("limit_item, ")
					_T("limit_item_number, ")
					_T("keep_inactive, ")
					_T("contentCode, ")
					_T("parent_feed_id, ")
					_T("news_id) SELECT * FROM feed;");
			db.execDML(sql);

			sql = _T("DROP TABLE feed;");
			db.execDML(sql);
			sql = _T("ALTER TABLE feedtemp RENAME TO feed;");
			db.execDML(sql);

			sql =   _T("CREATE TRIGGER OnDelFeed AFTER DELETE ON feed ")
					_T("BEGIN ")
					_T("DELETE FROM news_item WHERE news_item.feed_id = OLD.feed_id; ")
					_T("DELETE FROM news_watch_channel WHERE feed_id = OLD.feed_id; ")
					_T("END; ")
					_T("CREATE TRIGGER OnDelFeed2 AFTER DELETE ON feed FOR EACH ROW WHEN old.news_id > 0 ")
					_T("BEGIN ")
					_T("UPDATE news_item SET commentTrack = NULL WHERE news_item.news_id = old.news_id; ")
					_T("END;");
			db.execDML(sql);

			db.execDML(_T("UPDATE configurations SET db_ver = 106;"));

			db.execDML(_T("COMMIT TRANSACTION;"));

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

class CUpgrader1_0_0_107 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_107(void) : 
		CDbUpgrader(_T("1.0.0.106"), _T("1.0.0.107"))
	{
	}

	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			db.execDML(_T("BEGIN EXCLUSIVE TRANSACTION;"));
			db.execDML(_T("DROP INDEX idxFeedItem"));
			db.execDML(_T("CREATE INDEX idxFeedItem ON news_item (feed_id, retrieved, readtime)"));

			db.execDML(_T("UPDATE configurations SET db_ver = 107;"));

			db.execDML(_T("COMMIT TRANSACTION;"));

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

class CUpgrader1_0_0_108 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_108(void) : 
		CDbUpgrader(_T("1.0.0.107"), _T("1.0.0.108"))
	{
	}

	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			db.execDML(_T("BEGIN EXCLUSIVE TRANSACTION;"));
			db.execDML(_T("UPDATE feed_group SET name = 'All News Feeds' WHERE name = 'All News Channels';"));
			db.execDML(_T("ALTER TABLE feed ADD COLUMN priority INTEGER;"));
			db.execDML(_T("UPDATE configurations SET db_ver = 108;"));
			db.execDML(_T("COMMIT TRANSACTION;"));

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

class CUpgrader1_0_0_109 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_109(void) : 
		CDbUpgrader(_T("1.0.0.108"), _T("1.0.0.109"))
	{
	}

	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);
			db.execDML(_T("BEGIN EXCLUSIVE TRANSACTION;"));
			db.execDML(_T("CREATE TABLE configuration(key VARCHAR(50) NOT NULL UNIQUE, value VARCHAR(250));"));
			db.execDML(_T("INSERT INTO configuration SELECT bloglines_account AS key, options7 AS value FROM configurations LIMIT 1;"));
			db.execDML(_T("INSERT INTO configuration SELECT bloglines_password AS key, options8 AS value FROM configurations LIMIT 1;"));
			db.execDML(_T("INSERT INTO configuration SELECT bloglines_dontmarkread AS key, options6 AS value FROM configurations LIMIT 1;"));
			db.execDML(_T("INSERT INTO configuration SELECT last_start AS key, options1 AS value FROM configurations LIMIT 1;"));
			db.execDML(_T("INSERT INTO configuration SELECT last_shutdown AS key, options2 AS value FROM configurations LIMIT 1;"));
			db.execDML(_T("INSERT INTO configuration values('db_version',109);"));
			db.execDML(_T("DROP TABLE configurations;"));
			db.execDML(_T("COMMIT TRANSACTION;"));
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
