#include "StdAfx.h"
#include "Tag.h"
#include "FeedManager.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "FeedManagerLibHelper.h"
#include "GMTimeLib.h"

CTag::CTag(void) : m_id(0),m_unreadCount(0)
{
}

CTag::~CTag(void)
{
}

void CTag::Save()
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
				_T("insert into tag_info (tag_name, options3) values (?, ?);"));
			stmt.bind(1,(LPCTSTR)m_name);
			stmt.bind(2,(LPCTSTR)m_tagStyle);
			db.execDML(_T("begin immediate transaction;"));
			stmt.execDML();
			m_id = (UINT_PTR)db.lastRowId();
			db.execDML(_T("commit transaction;"));
		}
		else
		{
			// update
	        CppSQLite3Statement stmt = db.compileStatement(
				_T("update tag_info set tag_name = ?, options3 = ? where tag_id = ?;"));
			stmt.bind(1,(LPCTSTR)m_name);
			stmt.bind(2,(LPCTSTR)m_tagStyle);
			stmt.bind(3,(unsigned int)m_id);
			stmt.execDML();
		}

		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		CString str = e.errorMessage();
		if(str.Find(_T("column tag_name is not unique"))>0)
			throw CExceptionBase(ERR_FM_DBERROR, _T("A label with the same name already exists."));
		else
			throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

}

void CTag::Rename(LPCTSTR newName)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CppSQLite3Statement stmt = db.compileStatement(_T("update tag_info set tag_name = ? where tag_id = ? "));
		stmt.bind(1,newName);
		stmt.bind(2,(unsigned int)m_id);
		stmt.execDML();
		stmt.finalize();
		db.close();

		m_name = newName;
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}

void CTag::Delete()
{
	CString sql;
	sql.Format(_T("delete from tag_info where tag_id = %d"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

}

int CTag::RetrieveNewsToDB()
{
	ATLASSERT(false);
	return 0;
}

size_t CTag::GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, CNewsFilter* pNewsFilter) const
{
	CString criteria;
	criteria.Format(_T(" news_id in (select news_id from item_tag where tag_id = %d) "), m_id);
	LPCTSTR orderBy = NULL;
	if(pNewsFilter->GenOrderByClause().GetLength() == 0)
		orderBy = _T(" order by retrieved desc ");

	// we don't want to apply status filter here
	CNewsFilter tagFilter(*pNewsFilter);
	tagFilter.m_newsStatus = CNewsFilter::All;

	return CFeedManagerLibHelper::GetNewsItemIDs(newsIDs, criteria, &tagFilter, orderBy);
}


CString CTag::GeneratePageHTML(int nPage)
{
	CString content = GeneratePageContentHTML(nPage);
	CString pager = GeneratePager();
	CString html;
	html.Append(pager);
	html.Append(content);
	html.Append(pager);
	return html;
}

CString CTag::GetContentID() const
{
	CString temp;
	temp.Format(_T("T%d_%d"), m_id, m_nCurrentPage);
	return temp;
}

int CTag::CheckUnread()
{
	CString criteria;
	criteria.Format(_T(" news_id in (select news_id from item_tag where tag_id = %d) "), m_id);
	return CNewsSource::CheckUnread(criteria);
}

CString CTag::GetNewsSourceName() const
{
	return m_name;
}

CString CTag::GetHomeURL() const
{
	return "";
}

CString CTag::GetStyle() const
{
	return m_tagStyle;
}

void CTag::MarkRead()
{
	CString sql;
	sql.Format( _T("update news_item set readtime=1 where")
				_T(" news_id in (select news_id from item_tag where tag_id = %d)")
				_T(" and readtime is null;"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

}

void CTag::MarkUnread()
{
	CString sql;
	sql.Format( _T("update news_item set readtime = null where")
				_T(" news_id in (select news_id from item_tag where tag_id = %d)")
				_T(" and readtime is not null;"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

size_t CTag::GetAllTags(TagVector& tags)
{
	tags.clear();
	tags.reserve(20);
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql = _T("select tag_id, tag_name, unread_count, options3 from tag_info order by tag_name;");
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CTag* tag =  new CTag();
			tag->m_id = q.getIntField(0);
			tag->m_name = q.getStringField(1);
			tag->m_unreadCount = q.getIntField(2,0);
			tag->m_tagStyle = q.getStringField(3);

			tags.push_back(tag);
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return tags.size();
}


void CTag::SaveToXml(MSXML2::IXMLDOMElementPtr spElement)
{
	spElement->setAttribute(_T("text"), (LPCTSTR)m_name);
}

void CTag::LoadFromXml(MSXML2::IXMLDOMElementPtr spElement)
{
	CFeedManagerLibHelper::GetTitle(spElement, m_name);
}

void CTag::CountUnread(std::map<ULONG_PTR, int>& mapUnread)
{
	CString sql = 
		_T("select tag_id, count(*) from item_tag w ")
		_T("join news_item n on w.news_id = n.news_id ")
		_T("where n.readtime is null group by tag_id;");
	return CNewsSource::CountUnread(mapUnread, sql);
}

void CTag::CountTotal(std::map<ULONG_PTR, int>& mapUnread)
{
	CString sql = _T("select tag_id, count(*) from item_tag group by tag_id;");
	return CNewsSource::CountUnread(mapUnread, sql);
}

void CTag::SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts)
{
	CFeedManagerLibHelper::SaveUnreadCount(counts, _T("tag_info"), _T("tag_id"));
}

int CTag::GetNumOfItemsWithLabel()
{
	if(m_id<=0) // new item?
		return 0;

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select count(*) from item_tag where tag_id = %d;"),m_id);
		int n = db.execScalar(sql);
		db.close();
		return n;
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return 0;
}

void CTag::UnlabelAll()
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		// trigger will change the item label count. 
		sql.Format( _T("delete from item_tag where tag_id = %d;")
					_T("update tag_info set unread_count = 0 where tag_id = %d;"), m_id, m_id);
		int n = db.execDML(sql);
		db.close();
		m_unreadCount = 0;
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}
