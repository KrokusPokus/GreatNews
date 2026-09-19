#pragma once
#include <atltime.h>
#include "GMTimeLib.h"
#include <atltime.h>

#include "dbupgrader.h"

class CUpgrader1_0_0_95 :
	public CDbUpgrader
{
public:

	CUpgrader1_0_0_95(void) : 
		CDbUpgrader(_T("1.0.0.94"), _T("1.0.0.95"))
	{
	}

	virtual ~CUpgrader1_0_0_95(void)
	{
	}

public:
	bool Upgrade(LPCTSTR dbFilePath)
	{ 
		try
		{
			CString sql =_T("\
CREATE TABLE blog_tool_type ( \
	tool_type_id INTEGER PRIMARY KEY,  \
	tool_type_name VARCHAR(100) NOT NULL UNIQUE,  \
	default_url VARCHAR(255) NOT NULL);\
CREATE TABLE blog_tool (\
	tool_id INTEGER PRIMARY KEY, \
	tool_name VARCHAR(100) NOT NULL UNIQUE, \
	tool_type VARCHAR(50) NOT NULL, \
	tool_url VARCHAR(255) NOT NULL);\
");
			CString sql2=_T("\
INSERT INTO blog_tool_type (tool_type_name, default_url) values ('<Generic>','http://<yoursite.com>/post?title=%TITLE%&url=%URL%&text=%TEXT%');\
INSERT INTO blog_tool_type (tool_type_name, default_url) values ('WordPress','http://<yoursite.com>/blog/wp-admin/bookmarklet.php?popupurl=%LINK%&popuptitle=%TITLE%&text=%TEXT%');\
INSERT INTO blog_tool_type (tool_type_name, default_url) values ('TypePad','https://www.typepad.com/t/app/weblog/post?is_qp=1&qp_show=tb,ca,ac,ap,cb,ex,tm,kw&__mode=edit_entry&qp_title=%TITLE%&qp_href=%LINK%&qp_text=%TEXT%');\
INSERT INTO blog_tool_type (tool_type_name, default_url) values ('MovableType','http://<yoursite.com>/cgi-bin/mt.cgi?is_bm=1&bm_show=&__mode=view&_type=entry&title=%TITLE%&link_title=%TITLE%&link_href=%LINK%&text=%TEXT%');");
			sql2+=_T("\
INSERT INTO blog_tool_type (tool_type_name, default_url) values ('Blogger','http://www.blogger.com/blog-this.g?&n=%TITLE%&t=&u=%LINK%');\
INSERT INTO blog_tool_type (tool_type_name, default_url) values ('Drupal','http://<yoursite.com>/drupal/node/add/blog?edit[title]=%TITLE%&edit[body]=%TEXT%');\
");

			CppSQLite3DB db;
			db.open(dbFilePath);
			db.execDML(sql);
			db.execDML(sql2);
			db.execDML(_T("update configurations set db_ver=95;"));
		}
		catch(CppSQLite3Exception& e)
		{
			m_errorMsg = e.errorMessage();
			return false;
		}

		return true;
	}

};
