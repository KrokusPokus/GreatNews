#include "StdAfx.h"
#include "FeedGroup.h"
#include "FeedManager.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "FeedManagerLibHelper.h"
#include "NewsItemCache.h"
#include "GMTimeLib.h"
#include "NewsFeedCache.h"

CFeedGroup::CFeedGroup(void) : 
	m_id(0),m_parentID(1),m_unreadCount(0),m_bDisabled(false),m_bExpanded(false)
{
}

CFeedGroup::CFeedGroup(const CFeedGroup& f) : 
	m_id(f.m_id),
	m_name(f.m_name),
	m_desc(f.m_desc),
	m_parentID(f.m_parentID),
	m_website(f.m_website),
	m_bDisabled(f.m_bDisabled),
	m_bExpanded(f.m_bExpanded)
{
}

CFeedGroup::CFeedGroup(int id, LPCTSTR name, LPCTSTR desc) :
	m_id(id),
	m_name(name),
	m_desc(desc),
	m_parentID(1),
	m_unreadCount(0),
	m_bDisabled(false),
	m_bExpanded(false)
{
}

CFeedGroup::~CFeedGroup(void)
{
}

void CFeedGroup::Init(ULONG_PTR id)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from feed_group where feed_group_id = %d"),
			CFeedManagerLibHelper::m_feedGroupFields,
			id);
		CppSQLite3Query q = db.execQuery(sql);
		if(!q.eof())
		{
			CFeedManagerLibHelper::PopulateFeedGroup(this, q);
			q.finalize();
		}
		else
		{
			q.finalize();
			throw CExceptionBase(ERR_FM_FEEDGROUPNOTFOUND,_T("Cannot find newsfeed group with specified id."));
		}
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

}

void CFeedGroup::Save()
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
				_T("insert into feed_group (")
				_T("name, ")            // 1
				_T("description, ")     // 2
				_T("parent_group, ")    // 3
				_T("options3, ")        // 4
				_T("options1")          // 5
				_T(") values (?,?,?,?,?);"));
			stmt.bind(1,(LPCTSTR)m_name);
			stmt.bind(2,(LPCTSTR)m_desc);
			if(IsRootGroup()) // this is root!
			{
				stmt.bindNull(3);
			}
			else
			{
				stmt.bind(3,(int)m_parentID);
			}
			stmt.bind(4,(LPCTSTR)m_website);
			stmt.bind(5,(int)m_bDisabled);
			db.execDML(_T("begin immediate transaction;"));
			stmt.execDML();
			m_id = (ULONG_PTR)db.lastRowId();
			db.execDML(_T("commit transaction;"));
		}
		else
		{
			// update
			CppSQLite3Statement stmt = db.compileStatement(
				_T("update feed_group set ")
				_T(" name = ?,")             // 1
				_T(" description = ?,")      // 2
				_T(" parent_group = ?,")     // 3
				_T(" options3 = ?,")         // 4
				_T(" options1 = ?,")         // 5
				_T(" options2 = ? where")    // 6
				_T(" feed_group_id = ?;"));  // 7
			stmt.bind(1,(LPCTSTR)m_name);
			stmt.bind(2,(LPCTSTR)m_desc);
			if(IsRootGroup()) // this is root!
				stmt.bindNull(3);
			else
				stmt.bind(3,(int)m_parentID);
			stmt.bind(4,(LPCTSTR)m_website);
			stmt.bind(5,(int)m_bDisabled);
			stmt.bind(6,(int)m_bExpanded);
			stmt.bind(7,(unsigned int)m_id);
			stmt.execDML();
		}
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		CString str = e.errorMessage();
		if(str.Find(_T("column name is not unique"))>0)
			throw CExceptionBase(ERR_FM_DBERROR, _T("A channel group with the same name already exists."));
		else
			throw CExceptionBase(ERR_FM_GENERICERR, e.errorMessage());
	}

}

void CFeedGroup::Rename(LPCTSTR newName)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CppSQLite3Statement stmt = db.compileStatement(_T("update feed_group set name = ? where feed_group_id = ? "));
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

void CFeedGroup::Delete()
{
	CString sql;
	sql.Format(_T("delete from feed_group where feed_group_id = %d"), m_id);
	
	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}


FeedGroupPtr CFeedGroup::GetRootFeedGroup(void)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from feed_group where parent_group is null"),
			CFeedManagerLibHelper::m_feedGroupFields);
		CppSQLite3Query q = db.execQuery(sql);
		if(!q.eof())
		{
			CFeedGroup* pGroup = new CFeedGroup();
			CFeedManagerLibHelper::PopulateFeedGroup(pGroup, q);
			q.finalize();
			return FeedGroupPtr(pGroup);
		}
		else
		{
			q.finalize();
			throw CExceptionBase(ERR_FM_FEEDGROUPNOTFOUND,_T("Cannot find root channel group."));
		}
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	// should never been here
	return NULL;
}

size_t CFeedGroup::GetChildGroups(FeedGroupVector& childGroups, SortOption sortOption) const
{
	childGroups.clear();
	childGroups.reserve(10);

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from feed_group where parent_group = %d"),
			CFeedManagerLibHelper::m_feedGroupFields, m_id);
		if(sortOption == SortByName)
		{
			sql.Append(_T(" order by name COLLATE GNUNICODE "));
		}
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CFeedGroup* fg =  new CFeedGroup();
			CFeedManagerLibHelper::PopulateFeedGroup(fg, q);
			childGroups.push_back(fg);
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return childGroups.size();
}

int CFeedGroup::GetNumOfNewsFeeds() const
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	int n=0;
	try
	{
		CString sql;
		sql.Format(_T("select count(*) from feed where feed_group_id = %d"), m_id);
		n = db.execScalar(sql);
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return n;
}

int CFeedGroup::GetNumOfLabelled() const
{
	long num = 0;

	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql;
		sql.Format(_T("select count(*) from item_tag where news_id in ")
			_T("(select news_id from news_item where feed_id in ")
			_T("(select feed_id from feed where feed_group_id = %d));"), m_id);
		num = db.execScalar(sql);
		db.close();
	}
	catch(...)
	{
	}

	return num;	
}

size_t CFeedGroup::GetNewsFeeds(NewsFeedVector& newsfeeds, bool bIncludeDisabled, SortOption sortOption) const
{
	newsfeeds.clear();
	newsfeeds.reserve(10);

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from feed where feed_group_id = %d"), CFeedManagerLibHelper::m_newsFeedFields, m_id);
		if(sortOption == SortByName)
			sql.Append(_T(" order by name"));
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CNewsFeed* nf =  new CNewsFeed();
			CFeedManagerLibHelper::PopulateNewsFeed(nf, q);
			if(!nf->m_bDisabled || bIncludeDisabled)
				newsfeeds.push_back(nf);
			else
				delete nf;
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return newsfeeds.size();
}

size_t CFeedGroup::GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, CNewsFilter* pNewsFilter) const
{
	CString criteria;
	criteria.Format(_T(" feed_id in (select feed_id from feed where feed_group_id = %d) "), m_id);
	LPCTSTR orderBy = NULL;
	if(pNewsFilter->GenOrderByClause().GetLength() == 0)
		orderBy = _T(" order by feed_id, news_id desc ");
	return CFeedManagerLibHelper::GetNewsItemIDs(newsIDs, criteria, pNewsFilter, orderBy);
}

CString CFeedGroup::GeneratePageHTML(int nPage)
{
	CString content = GeneratePageContentHTML(nPage);
	CString pager = GeneratePager();
	CString html;
	html.Append(pager);
	html.Append(content);
	html.Append(pager);
	return html;
}

CString CFeedGroup::GeneratePageContentHTML(int nPage)
{
	NewsItemVector newsItems;
	GetCurrentPageItems(newsItems, nPage);

	CString html;

	NewsFeedPtr lastNewsFeed;
	bool bSingleItem = false;
	for(NewsItemVector::iterator it=newsItems.begin(); it != newsItems.end(); ++it)
	{
		NewsItemPtr& spItem = *it;
		if(lastNewsFeed == NULL || lastNewsFeed->m_id != spItem->m_feedID)
		{
			NewsItemVector::iterator itNext = it+1;
			bSingleItem = (itNext==newsItems.end()  // this is the end of vector
				|| ((NewsItemPtr&)*itNext)->m_feedID != spItem->m_feedID); // or next item is in different channel

			lastNewsFeed = CNewsFeedCache::GetNewsFeed(spItem->m_feedID);
			html.Append(lastNewsFeed->GenerateSectionHTML());
		}
		html.Append(spItem->internalGenerateHTML(bSingleItem));
	}

	return html;
}

CString CFeedGroup::GetContentID() const
{
	CString temp;
	temp.Format(_T("G%d_%d"), m_id, m_nCurrentPage);
	return temp;
}

int CFeedGroup::CheckUnread()
{
	CString criteria;
	criteria.Format(_T(" feed_id in (select feed_id from feed where feed_group_id = %d)"), m_id);
	return CNewsSource::CheckUnread(criteria);
}

CString CFeedGroup::GetNewsSourceName() const
{
	return m_name;
}

int CFeedGroup::RetrieveNewsToDB()
{
	int numOfNewItems = 0;
	NewsFeedVector newsFeeds;
	GetNewsFeeds(newsFeeds, false);
	for(NewsFeedVector::iterator it = newsFeeds.begin(); it!=newsFeeds.end(); ++it)
	{
		NewsFeedPtr newsFeed = *it;
		numOfNewItems += newsFeed->RetrieveNewsToDB();
	}
	return numOfNewItems;
}

void CFeedGroup::MarkRead()
{
	CString sql;
	sql.Format( _T("update news_item set readtime = 1 where feed_id in ")
			    _T("(select feed_id from feed where feed_group_id = %d) ")
			    _T("and readtime is null;"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	CNewsItemCache::Empty();
}

void CFeedGroup::MarkUnread()
{
	CString sql;
	sql.Format(_T("update news_item set readtime=null where feed_id in ")
			   _T("(select feed_id from feed where feed_group_id=%d) ")
			   _T("and readtime is not null;"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	CNewsItemCache::Empty();
}

CString CFeedGroup::GetHomeURL() const
{
	return m_website;
}

ULONG_PTR CFeedGroup::GetIdFromName(LPCTSTR name)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);
	ULONG_PTR id = 0;

	try
	{
		CppSQLite3Statement stmt = db.compileStatement(_T("select feed_group_id from feed_group where name = ? "));
		stmt.bind(1,name);
		CppSQLite3Query q = stmt.execQuery();
		if(!q.eof())
			id = q.getIntField(0);
		q.finalize();
		stmt.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return id;
}

void CFeedGroup::SaveToXml(MSXML2::IXMLDOMElementPtr spElement)
{
	spElement->setAttribute("text", (LPCTSTR)m_name);
	spElement->setAttribute("title", (LPCTSTR)m_name);
	spElement->setAttribute("description", (LPCTSTR)m_desc);
	spElement->setAttribute("htmlUrl", (LPCTSTR)m_website);
}

void CFeedGroup::LoadFromXml(MSXML2::IXMLDOMElementPtr spElement)
{
	CFeedManagerLibHelper::GetTitle(spElement, m_name);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("description"),m_desc);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("htmlUrl"),m_website);
}

int CFeedGroup::GetNumOfNewsItems(bool bUnreadOnly) const
{
	long num = 0;

	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql;
		sql.Format( _T("select count(*) from news_item where feed_id in ")
					_T("(select feed_id from feed where feed_group_id=%d) %s;"),
					m_id, bUnreadOnly ? _T(" and readtime is null") : _T("") );
		num = db.execScalar(sql);
		db.close();
	}
	catch(...)
	{
	}

	return num;
}


void CFeedGroup::UpdateUsage()
{
	CString sql;
	sql.Format(_T("update feed set usage = ifnull(usage, 0) + 1 where feed_group_id = %d;"), m_id);
	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

void CFeedGroup::ExportRss(MSXML2::IXMLDOMElementPtr& spChannel)
{
	CBatchContentGenerator::ExportRss(spChannel);
	if(m_website.GetLength())
	{
		MSXML2::IXMLDOMElementPtr link = spChannel->selectSingleNode(_T("link"));
		link->text = (LPCTSTR)m_website;
	}

	if(m_desc.GetLength())
	{
		MSXML2::IXMLDOMElementPtr desc = spChannel->selectSingleNode(_T("description"));
		desc->text = (LPCTSTR)m_desc;
	}
}

void CFeedGroup::SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts)
{
	CFeedManagerLibHelper::SaveUnreadCount(counts, _T("feed_group"), _T("feed_group_id"));
}

void CFeedGroup::SaveTreeState(std::vector<std::pair<ULONG_PTR,long> >& states)
{
	if(states.empty())
		return;

	CString sql = _T("update feed_group set options2 = ? where feed_group_id = ?;");

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	db.execDML(_T("begin immediate transaction;"));
	CppSQLite3Statement stmt = db.compileStatement(sql);
	for(std::vector<std::pair<ULONG_PTR,long> >::iterator it = states.begin(); it!=states.end(); ++it)
	{
		std::pair<ULONG_PTR,long>& values = *it;
		stmt.bind(1, values.second);
		stmt.bind(2, (unsigned int)values.first);
		stmt.execDML();
		stmt.reset();
	}
	stmt.finalize();
	db.execDML(_T("commit transaction;"));
	db.close();
}

bool CFeedGroup::IsRootGroup()
{
	return (m_parentID == 0);
}
