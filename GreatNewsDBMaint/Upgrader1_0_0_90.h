#pragma once
#include <atltime.h>
#include "GMTimeLib.h"

#include "dbupgrader.h"

class CUpgrader1_0_0_90 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_90(void) : 
		CDbUpgrader(_T("1.0.0.85"), _T("1.0.0.90"))
	{
	}

	virtual ~CUpgrader1_0_0_90(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);


			CTime now = CGMTimeHelper::GetCurrentSysTime();
			CString now_s = CGMTimeHelper::FormatSqlDate(now);
			CTime time = CGMTimeHelper::DaysAgo(now, 2);
			CString time_s = CGMTimeHelper::FormatSqlDate(time);

			CString sql;
			sql.Format(_T("update news_item set published=null, readtime=1, retrieved=%d")
					,(int)time.GetTime());
			db.execDML(_T("begin exclusive transaction;"));
			db.execDML(sql);
			db.execDML(_T("update feed set last_updated = null;"));
			db.execDML(_T("update configurations set db_ver=90;"));
			db.execDML(_T("commit transaction;"));

			db.execDML(_T("vacuum;"));
		}
		catch(CppSQLite3Exception& e)
		{
			m_errorMsg = e.errorMessage();
			return false;
		}

		return true;
	}

};
