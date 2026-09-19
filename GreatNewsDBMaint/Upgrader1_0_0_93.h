#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>

#include "dbupgrader.h"

class CUpgrader1_0_0_93 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_93(void) : 
		CDbUpgrader(_T("1.0.0.92"), _T("1.0.0.93"))
	{
	}

	virtual ~CUpgrader1_0_0_93(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CTime now = CTime::GetCurrentTime();
			CString sql;
			sql.Format(_T("update configurations set options1=%d, options2=%d")
				,(int)now.GetTime()
				,(int)now.GetTime());

			CppSQLite3DB db;
			db.open(dbFilePath);
			db.execDML(sql);

			db.execDML(_T("update configurations set db_ver=93;"));
		}
		catch(CppSQLite3Exception& e)
		{
			m_errorMsg = e.errorMessage();
			return false;
		}

		return true;
	}

};
