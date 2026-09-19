#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_105 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_105(void) : 
		CDbUpgrader(_T("1.0.0.104"), _T("1.0.0.105"))
	{
	}

	virtual ~CUpgrader1_0_0_105(void)
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
			sql += _T("update tag_info set unread_count = (select count(*) from item_tag where item_tag.tag_id=tag_info.tag_id);");
			sql += _T("update configurations set db_ver=105;");
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
