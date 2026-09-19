#include "StdAfx.h"
#include "BlogTool.h"
#include "FeedManager.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "FeedManagerLibHelper.h"

CBlogTool::CBlogTool(void) : m_id(-1)
{
}

CBlogTool::CBlogTool(LPCTSTR name, LPCTSTR type, LPCTSTR url)
	: m_id(-1), m_name(name), m_type(type), m_url(url)
{
}

CBlogTool::~CBlogTool(void)
{
}

void CBlogTool::Save()
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		if(m_id <= 0)
		{
			// insert
			CppSQLite3Statement stmt = db.compileStatement(
				_T("insert into blog_tool (tool_name, tool_type, tool_url) values (?,?,?);"));
			stmt.bind(1,(LPCTSTR)m_name);
			stmt.bind(2,(LPCTSTR)m_type);
			stmt.bind(3,(LPCTSTR)m_url);
			db.execDML(_T("begin immediate transaction;"));
			stmt.execDML();
			m_id = (UINT_PTR)db.lastRowId();
			db.execDML(_T("commit transaction;"));
		}
		else
		{
			// update
			CppSQLite3Statement stmt = db.compileStatement(
				_T("update blog_tool set tool_name=?, tool_type=?, tool_url=? where tool_id=?;"));
			stmt.bind(1,(LPCTSTR)m_name);
			stmt.bind(2,(LPCTSTR)m_type);
			stmt.bind(3,(LPCTSTR)m_url);
			stmt.bind(4,(unsigned int)m_id);
			stmt.execDML();
		}

		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		CString str = e.errorMessage();
		if(str.Find(_T("column tool_name is not unique"))>0)
			throw CExceptionBase(ERR_FM_DBERROR, _T("A blog tool with the same name already exists."));
		else
			throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}

void CBlogTool::Delete()
{
	CString sql;
	sql.Format(_T("delete from blog_tool where tool_id = %d"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}


size_t CBlogTool::GetAllBlogTools(BlogToolVector& allTools)
{
	allTools.clear();
	allTools.reserve(20);

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql = _T("select tool_id, tool_name, tool_type, tool_url from blog_tool;");
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CBlogTool* tool = new CBlogTool();
			tool->m_id = q.getIntField(0);
			tool->m_name = q.getStringField(1);
			tool->m_type = q.getStringField(2);
			tool->m_url = q.getStringField(3);
			allTools.push_back(tool);

			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return allTools.size();
}

bool operator == (BlogToolPtr& tool, LPCTSTR name)
{
	return (tool->m_name == name);
};

