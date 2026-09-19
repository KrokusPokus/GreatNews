#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_97 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_97(void) : 
		CDbUpgrader(_T("1.0.0.96"), _T("1.0.0.97"))
	{
	}

	virtual ~CUpgrader1_0_0_97(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			// update last modified
			CppSQLite3Query q = db.execQuery(_T("select max(retrieved) m, feed_id fid from news_item group by feed_id"));
			std::map<long, long> myMap;
			while(!q.eof())
			{
				long date = q.getIntField(0);
				long feedID = q.getIntField(1);
				if(date>0)
				{
					myMap[feedID]=date;
				}
				q.nextRow();
			}
			q.finalize();
			db.close();

			db.open(dbFilePath);
			db.execDML(_T("BEGIN TRANSACTION;"));
			CppSQLite3Statement stmt = db.compileStatement(_T("update feed set last_updated=? where feed_id=?;"));

			for(std::map<long, long>::iterator it = myMap.begin(); it!=myMap.end(); it++)
			{
				long feedID = it->first;
				long date = it->second;

				stmt.bind(1,date);
				stmt.bind(2,feedID);
				int n = stmt.execDML();
				stmt.reset();
			}
			stmt.finalize();
			db.execDML(_T("COMMIT TRANSACTION;"));
			db.close();

			db.open(dbFilePath);
			db.execDML(_T("alter table feed add column limit_item integer;"));
			db.execDML(_T("alter table feed add column limit_item_number integer;"));
			db.execDML(_T("alter table feed add column keep_inactive integer;"));
			db.execDML(_T("update configurations set db_ver=97;"));
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
