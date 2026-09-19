#include "StdAfx.h"
#include "NewsWatch.h"
#include "FeedManager.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "FeedManagerLibHelper.h"
#include "TextSearch.h"
#include "NewsWatchCache.h"
#include "NewsItemCache.h"
#include "GMTimeLib.h"
#include "StringTokenizer.h"

CNewsWatch::CNewsWatch(void) : 
	m_id(0),m_bWatchSelectiveChannels(true),m_watchFlag(WATCH_TITLE),
	m_allWords(0),m_matchCase(0),m_wholeWord(1),
	m_txtColor(RGB(0,0,0)), m_bkColor(RGB(255,255,128)), m_order(0),
	m_pSearchFunc(NULL), m_nAction(Hightlight), m_unreadCount(0),m_bDisabled(false)
{
}

CNewsWatch::~CNewsWatch(void)
{
}

void CNewsWatch::Init(ULONG_PTR id)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from news_watch where watch_id = %d"),
			CFeedManagerLibHelper::m_newsWatchFields, id);
		CppSQLite3Query q = db.execQuery(sql);
		if(!q.eof())
		{
			CFeedManagerLibHelper::PopulateNewsWatch(this, q);
			q.finalize();
		}
		else
		{
			q.finalize();
			throw CExceptionBase(ERR_FM_DBERROR,_T("Cannot find specified news watch."));
		}
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}

void CNewsWatch::Save()
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		bool bIsUpdate = (m_id > 0);
		db.execDML(_T("begin immediate transaction;"));
		CString sql;
		if(!bIsUpdate)
		{
			int n = db.execScalar(_T("select count(*) from news_watch;"));
			m_order = n;
			// insert
			CppSQLite3Statement stmt = db.compileStatement(
				_T("insert into news_watch (")
				_T("name, ")
				_T("criteria, ")
				_T("selective_feed, ")
				_T("watch_flag, ")
				_T("match_case, ")
				_T("whole_word, ")
				_T("all_words, ")
				_T("front_color, ")
				_T("bk_color, ")
				_T("display_order, ")
				_T("options1, ")
				_T("options3, ")
				_T("options2) ")
				_T("values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"));
			stmt.bind(1,(LPCTSTR)m_title);
			stmt.bind(2,(LPCTSTR)m_matchCriteria);
			stmt.bind(3, m_bWatchSelectiveChannels);
			stmt.bind(4,m_watchFlag);
			stmt.bind(5,m_matchCase);
			stmt.bind(6,m_wholeWord);
			stmt.bind(7,m_allWords);
			stmt.bind(8,(int)m_txtColor);
			stmt.bind(9,(int)m_bkColor);
			stmt.bind(10,m_order);
			stmt.bind(11,(int)m_nAction);
			stmt.bind(12,(LPCTSTR)m_watchStyle);
			stmt.bind(13,(int)m_bDisabled);
			stmt.execDML();
			m_id = (UINT_PTR)db.lastRowId();
		}
		else
		{
			// update
			CppSQLite3Statement stmt = db.compileStatement(
				_T("update news_watch set name = ?, ")
				_T("criteria = ?, ")
				_T("selective_feed = ?, ")
				_T("watch_flag = ?, ")
				_T("match_case = ?, ")
				_T("whole_word = ?, ")
				_T("all_words = ?, ")
				_T("front_color = ?, ")
				_T("bk_color = ?, ")
				_T("display_order = ?, ")
				_T("options1 = ?, ")
				_T("options3 = ?, ")
				_T("options2 = ? ")
				_T("where watch_id = ?;"));
			stmt.bind(1,(LPCTSTR)m_title);
			stmt.bind(2,(LPCTSTR)m_matchCriteria);
			stmt.bind(3, m_bWatchSelectiveChannels);
			stmt.bind(4,m_watchFlag);
			stmt.bind(5,m_matchCase);
			stmt.bind(6,m_wholeWord);
			stmt.bind(7,m_allWords);
			stmt.bind(8,(int)m_txtColor);
			stmt.bind(9,(int)m_bkColor);
			stmt.bind(10,m_order);
			stmt.bind(11,(int)m_nAction);
			stmt.bind(12,(LPCTSTR)m_watchStyle);
			stmt.bind(13,(int)m_bDisabled);
			stmt.bind(14,(unsigned int)m_id);
			stmt.execDML();
		}

		// save selected channels
		sql.Format(_T("delete from news_watch_channel where watch_id = %d"),m_id);
		db.execDML(sql);
		CppSQLite3Statement stmt = db.compileStatement(
			_T("insert into news_watch_channel (watch_id, feed_id) values (?, ?)"));
		for(std::vector<ULONG_PTR>::iterator it=m_WatchChannelIDs.begin(); it!=m_WatchChannelIDs.end();++it)
		{
			stmt.bind(1,(unsigned int)m_id);
			stmt.bind(2,(long)*it);
			stmt.execDML();
			stmt.reset();
		}
		db.execDML(_T("commit transaction;"));
		db.close();

		if(bIsUpdate)
		{
			CNewsWatchCache::Empty();
			CNewsItemCache::ClearAllWatches();
		}

	}
	catch(CppSQLite3Exception& e)
	{
		CString str = e.errorMessage();
		if(str.Find(_T("column name is not unique"))>0)
			throw CExceptionBase(ERR_FM_DBERROR, _T("A news watch with the same name already exists."));
		else
			throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

}

void CNewsWatch::Rename(LPCTSTR newName)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CppSQLite3Statement stmt = db.compileStatement(_T("update news_watch set name = ? where watch_id = ? "));
		stmt.bind(1,newName);
		stmt.bind(2,(unsigned int)m_id);
		stmt.execDML();
		stmt.finalize();
		db.close();

		m_title = newName;
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}

void CNewsWatch::Delete()
{
	CString sql;
	sql.Format(_T("delete from news_watch where watch_id = %d"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

int CNewsWatch::RetrieveNewsToDB()
{
	ATLASSERT(false);
	return 0;
}

size_t CNewsWatch::GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, CNewsFilter* pNewsFilter) const
{
	CString criteria;
	criteria.Format(_T(" news_id in (select news_id from watch_item where watch_id = %d) "), m_id);
	LPCTSTR orderBy = NULL;
	if(pNewsFilter->GenOrderByClause().GetLength() == 0)
	{
		orderBy = _T(" order by retrieved desc ");
	}

	return CFeedManagerLibHelper::GetNewsItemIDs(newsIDs, criteria, pNewsFilter, orderBy);
}

CString CNewsWatch::GeneratePageHTML(int nPage)
{
	CString html;
	CString pager = GeneratePager();

	html.Append(pager);
	html.Append(GeneratePageContentHTML(nPage));
	html.Append(pager);
	
	return html;
}

CString CNewsWatch::GetContentID() const
{
	CString temp;
	temp.Format(_T("W%d_%d"), m_id, m_nCurrentPage);
	return temp;
}

CString CNewsWatch::GetHomeURL() const
{
	return "";
}

CString CNewsWatch::GetStyle() const
{
	return m_watchStyle;
}


int CNewsWatch::CheckUnread()
{
	CString criteria;
	criteria.Format(_T(" news_id in (select news_id from watch_item where watch_id = %d) "), m_id);
	return CNewsSource::CheckUnread(criteria);
}

CString CNewsWatch::GetNewsSourceName() const
{
	return m_title;
}

void CNewsWatch::MarkRead()
{
	CString sql;
	sql.Format(
		_T("update news_item set readtime=1 where")
		_T(" news_id in (select news_id from watch_item where watch_id = %d)")
		_T(" and readtime is null;"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

void CNewsWatch::MarkUnread()
{
	CString sql;
	sql.Format(
		_T("update news_item set readtime = null where")
		_T(" news_id in (select news_id from watch_item where watch_id = %d)")
		_T(" and readtime is not null;"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

std::vector<ULONG_PTR>& CNewsWatch::LoadWatchChannels()
{
	if(m_id <=0 || m_WatchChannelIDs.size()>0)
		return m_WatchChannelIDs;

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);
	try
	{
		CString sql;
		sql.Format(_T("select feed_id from news_watch_channel where watch_id = %d"), m_id);
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			m_WatchChannelIDs.push_back(q.getIntField(0));
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return m_WatchChannelIDs;
}

size_t CNewsWatch::GetAllNewsWatches(NewsWatchVector& watches, SortOptions sortOptions)
{
	watches.clear();
	watches.reserve(20);
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from news_watch"),CFeedManagerLibHelper::m_newsWatchFields);
		if(sortOptions == SortByName)
			sql.Append(_T(" order by name"));
		else if(sortOptions == SortByDisplayOrder)
			sql.Append(_T(" order by display_order"));

		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CNewsWatch* nw =  new CNewsWatch();
			CFeedManagerLibHelper::PopulateNewsWatch(nw, q);
			watches.push_back(nw);
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return watches.size();
}

bool CNewsWatch::Match(NewsItemPtr newsItem)
{
	if((m_watchFlag & WATCH_TITLE) && Match(newsItem->m_title))
		return true;

	if((m_watchFlag & WATCH_CONTENT) && Match(newsItem->m_description))
		return true;

	if((m_watchFlag & WATCH_AUTHOR) && Match(newsItem->m_author))
		return true;

	if((m_watchFlag & WATCH_URL) && Match(newsItem->m_url))
		return true;

	return false;
}

void CNewsWatch::SetupSearchFunc()
{
	if(m_matchCase)
	{
		if(m_wholeWord)
			m_pSearchFunc = textsearch::searchw;
		else
			m_pSearchFunc = textsearch::search;
	}
	else
	{
		if(m_wholeWord)
			m_pSearchFunc = textsearch::searchiw;
		else
			m_pSearchFunc = textsearch::searchi;
	}
}

bool CNewsWatch::Match(const CString& str)
{
	std::vector<CString>::iterator it;
	bool bMatch=false;
	for(it=m_vectWords.begin(); it!=m_vectWords.end(); ++it)
	{
		bMatch = (*m_pSearchFunc)(str, *it);
		if(m_allWords == MatchAny && bMatch)
			return true;
		if(m_allWords != MatchAny && !bMatch)
			return false;
	}
	return bMatch;
}

void CNewsWatch::Split()
{
	m_vectWords.clear();
	if(m_allWords == MatchExact)
	{
		m_vectWords.push_back(m_matchCriteria);
		return;
	}

	StringTokenizer st(m_matchCriteria);
	CString token = st.NextToken();
	while (token.GetLength() > 0)
	{
		m_vectWords.push_back(token);
		token = st.NextToken();
	};
}

size_t CNewsWatch::GetWatchChannels(NewsFeedVector& newsFeeds)
{
	newsFeeds.clear();

	if(m_bWatchSelectiveChannels)
		CNewsFeed::GetNewsFeeds(m_WatchChannelIDs,newsFeeds);
	else
		CNewsFeed::GetAllNewsFeeds(newsFeeds, true);

	return newsFeeds.size();
}

void CNewsWatch::CheckExistingItems()
{
	Split();
	SetupSearchFunc();

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		NewsFeedVector newsFeeds;
		GetWatchChannels(newsFeeds);

		std::vector<ULONG_PTR> WatchItemIDs;

		CNewsItem* pNewsItem = new CNewsItem();
		NewsItemPtr newsItem = pNewsItem;

		for(NewsFeedVector::iterator it=newsFeeds.begin(); it != newsFeeds.end(); ++it)
		{
			NewsFeedPtr newsFeed = *it;

			// loop through this channel's news items
			CString sql;
			sql.Format(_T("select %s from news_item where feed_id = %d"), CFeedManagerLibHelper::m_newsItemFields, newsFeed->m_id);
			CppSQLite3Query q = db.execQuery(sql);
			while(!q.eof())
			{
				CFeedManagerLibHelper::PopulateNewsItem(pNewsItem, q);
				AtlTrace(_T("matching news id=[%d]\n"), pNewsItem->m_id);
				if(Match(newsItem))
					WatchItemIDs.push_back(pNewsItem->m_id);

				q.nextRow();
			}
			q.finalize();
		}


		// delete existing watch items
		CString sql;
		sql.Format(_T("delete from watch_item where watch_id = %d;"), m_id);
		db.execDML(_T("begin immediate transaction;"));
		db.execDML(sql);

		// insert the new match id
		sql.Format(_T("insert into watch_item (watch_id, news_id) values (%d, ?);"), m_id);
		CppSQLite3Statement stmtWatch = db.compileStatement(sql);
		for(std::vector<ULONG_PTR>::iterator itID=WatchItemIDs.begin(); itID!=WatchItemIDs.end(); ++itID)
		{
			stmtWatch.bind(1, (long)*itID);
			stmtWatch.execDML();
			stmtWatch.reset();
		}
		stmtWatch.finalize();
		db.execDML(_T("commit transaction;"));
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

}

size_t CNewsWatch::GetNewsWatchesByChannel(NewsWatchVector& newsWatches, ULONG_PTR feedID)
{
	newsWatches.clear();

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format( _T("select %s from news_watch where selective_feed = 0 UNION ALL ")
					_T("select %s from news_watch where watch_id in (select watch_id from news_watch_channel where feed_id = %d)"),
					CFeedManagerLibHelper::m_newsWatchFields,
					CFeedManagerLibHelper::m_newsWatchFields,
					feedID);
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CNewsWatch* nw = new CNewsWatch();
			CFeedManagerLibHelper::PopulateNewsWatch(nw, q);
			nw->Split();
			nw->SetupSearchFunc();
			newsWatches.push_back(nw);
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return newsWatches.size();
}

void CNewsWatch::SetOrder(std::vector<ULONG_PTR>& watchIDs)
{
	if(watchIDs.empty())
		return;

	CNewsWatchCache::Empty();
	CNewsItemCache::ClearAllWatches();

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("update news_watch set display_order = ? where watch_id = ?"));
		CppSQLite3Statement stmtWatch = db.compileStatement(sql);
		db.execDML(_T("begin immediate transaction;"));
		for(int n=0; n<(int)watchIDs.size();++n)
		{
			stmtWatch.bind(1, n);
			stmtWatch.bind(2, (unsigned int)watchIDs[n]);
			stmtWatch.execDML();
			stmtWatch.reset();
		}
		stmtWatch.finalize();
		db.execDML(_T("commit transaction;"));
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}

void CNewsWatch::AssignChannelSelection(NewsFeedVector& feeds)
{
	m_WatchChannelIDs.clear();
	m_WatchChannelIDs.reserve(feeds.size());

	for(NewsFeedVector::iterator it=feeds.begin(); it!=feeds.end(); ++it)
	{
		NewsFeedPtr feed = *it;
		m_WatchChannelIDs.push_back(feed->m_id);
	}

}

void CNewsWatch::SaveToXml(MSXML2::IXMLDOMElementPtr spElement)
{
	spElement->setAttribute(_T("title"),(LPCTSTR)m_title);
	spElement->setAttribute(_T("matchCriteria"),(LPCTSTR)m_matchCriteria);
#ifdef _STL70_ // Have work around some of the WDK's STL's deficiencies here
	spElement->setAttribute(_T("selective"), (long)m_bWatchSelectiveChannels);
	spElement->setAttribute(_T("watchFlag"), m_watchFlag);
	spElement->setAttribute(_T("allWords"), (long)m_allWords);
	spElement->setAttribute(_T("matchCase"), (long)m_matchCase);
	spElement->setAttribute(_T("wholeWord"), (long)m_wholeWord);
	spElement->setAttribute(_T("txtColor"), (long)m_txtColor);
	spElement->setAttribute(_T("bkColor"), (long)m_bkColor);
#else
	spElement->setAttribute(_T("selective"), m_bWatchSelectiveChannels);
	spElement->setAttribute(_T("watchFlag"), m_watchFlag);
	spElement->setAttribute(_T("allWords"), m_allWords);
	spElement->setAttribute(_T("matchCase"), m_matchCase);
	spElement->setAttribute(_T("wholeWord"), m_wholeWord);
	spElement->setAttribute(_T("txtColor"), m_txtColor);
	spElement->setAttribute(_T("bkColor"), m_bkColor);
#endif

	// save channel selection
	if(m_bWatchSelectiveChannels)
	{
		LoadWatchChannels();
		NewsFeedVector newsFeeds;
		CNewsFeed::GetNewsFeeds(m_WatchChannelIDs,newsFeeds);
		
		MSXML2::IXMLDOMDocumentPtr spDoc = spElement->ownerDocument;
		for(NewsFeedVector::iterator it=newsFeeds.begin(); it!=newsFeeds.end(); ++it)
		{
			NewsFeedPtr feed = *it;
			MSXML2::IXMLDOMElementPtr spChannel = spDoc->createElement(_T("channel"));
			feed->SaveToXml(spChannel);
			spElement->appendChild(spChannel);
		}

	}

}

void CNewsWatch::LoadFromXml(MSXML2::IXMLDOMElementPtr spElement)
{
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("title"),m_title);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("matchCriteria"),m_matchCriteria);
#ifdef _STL70_ // Have work around some of the WDK's STL's deficiencies here
	long v;
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("selective"),v);m_bWatchSelectiveChannels=v;
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("watchFlag"),m_watchFlag);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("allWords"),v);m_allWords=v;
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("matchCase"),v);m_matchCase=v;
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("wholeWord"),v);m_wholeWord=v;
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("txtColor"),v);m_txtColor=v;
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("bkColor"),v);m_bkColor=v;
#else
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("selective"),m_bWatchSelectiveChannels);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("watchFlag"),m_watchFlag);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("allWords"),m_allWords);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("matchCase"),m_matchCase);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("wholeWord"),m_wholeWord);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("txtColor"),m_txtColor);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("bkColor"),m_bkColor);
#endif

	m_WatchChannelIDs.clear();
	if(m_bWatchSelectiveChannels)
	{
		MSXML2::IXMLDOMNodeListPtr channelList = spElement->selectNodes(_T("channel"));
		m_WatchChannelIDs.reserve(channelList->length);
		MSXML2::IXMLDOMElementPtr spChannel;
		while(NULL != (spChannel=channelList->nextNode()))
		{
			_bstr_t url = spChannel->getAttribute(_T("xmlUrl"));
			ULONG_PTR id = CNewsFeed::GetIdFromXmlUrl((LPCTSTR)url);
			if(id>0)
				m_WatchChannelIDs.push_back(id);
		}
	}
}


void CNewsWatch::CountUnread(std::map<ULONG_PTR, int>& mapUnread)
{
	CString sql = _T("select watch_id, count(*) from watch_item w join news_item n on w.news_id = n.news_id where n.readtime is null group by watch_id;");
	return CNewsSource::CountUnread(mapUnread, sql);
}

void CNewsWatch::DeleteMatchedItems()
{
	CString sql;
	sql.Format(_T("delete from news_item where news_id in (select news_id from watch_item where watch_id = %d);"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

void CNewsWatch::SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts)
{
	CFeedManagerLibHelper::SaveUnreadCount(counts, _T("news_watch"), _T("watch_id"));
}
