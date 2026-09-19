#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_103 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_103(void) : 
		CDbUpgrader(_T("1.0.0.102"), _T("1.0.0.103"))
	{
	}

	virtual ~CUpgrader1_0_0_103(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CppSQLite3DB db;
			db.open(dbFilePath);

			// does nothing. The update was moved to CUpgrader1_0_0_103
			db.execDML(_T("update configurations set db_ver=103;"));

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
