#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>
#include <map>

#include "dbupgrader.h"

class CUpgrader1_0_0_104 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_104(void) : 
		CDbUpgrader(_T("1.0.0.103"), _T("1.0.0.104"))
	{
	}

	virtual ~CUpgrader1_0_0_104(void)
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
			sql += _T("update news_item set mark = (select count(*) from item_tag where item_tag.news_id=news_item.news_id) where mark<0;");
			sql += _T("update configurations set db_ver=104;");
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
