#include "StdAfx.h"
#include <set>
#include "NewsFeed.h"
#include "FeedManager.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "NewsFeedParser.h"
#include "FeedManagerLibHelper.h"
#include "GMTimeLib.h"
#include "NewsWatch.h"
#include "NewsItemCache.h"
#include "GNUtil.h"
#include "GMTimeLib.h"
#include "SearchChannelDef.h"
#include "BloglinesService.h"
#include "AsyncDownloadManager.h"
#include "SafeArrayVariant.h"
#include "NewsFeedCache.h"

HWND CNewsFeed::m_notifyWnd = NULL;
bool CNewsFeed::m_bMarkChangesUnread = true;

////////////////////////////////////////////////////////

bool CanUseFeedTimestamp(CNewsFeed& newsFeed, NewsItemVector& newsItems)
{
	CTime now = CGMTimeHelper::GetCurrentSysTime();
	if(newsFeed.m_lastModified > now) // set to future date?
		return false;

	for(NewsItemVector::iterator it = newsItems.begin(); it != newsItems.end(); ++it)
	{
		NewsItemPtr& item = *it;
		if(item->m_date> now // item set to future date?
			|| item->m_date > newsFeed.m_lastModified) // item newer than feed mod time?
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//
// HACK: we don't want to involve NewsFeedParserPtr in header file, 
//		otherwise we will have a circular reference
//
int RetrieveNewsToDB(CNewsFeed* pFeed, NewsFeedParserPtr spParser)
{
	NewsItemVector newsItems;

	spParser->ExtractNews(newsItems);
	for(NewsItemVector::iterator it = newsItems.begin(); it != newsItems.end(); ++it)
	{
		NewsItemPtr& item = *it;
		item->CalcContentCRC(); // update the content crc, so we can use it for checking existance
	}

	// check if the website hasn updated since we last update
	//if(spParser->m_newsFeed.m_lastModified != 0 && pFeed->m_lastChecked != 0)
	//{
	//	if(CanUseFeedTimestamp(spParser->m_newsFeed, newsItems)
	//		&& spParser->m_newsFeed.m_lastModified <= pFeed->m_lastChecked)
	//	{
	//		return 0; 
	//	}
	//}

	// pFeed->m_lastChecked = spParser->m_newsFeed.m_lastModified;
	int n=pFeed->RetrieveNewsToDB(newsItems, spParser->m_newsFeed);
	newsItems.clear();
	return n;
}

//
//
//
//////////////////////////////////////////////////////////////////////

TCHAR* CNewsFeed::ChannelCleanupMaxAgeString[] = {
												_T("Default"),
												_T("All"),
												_T("1 day"),
												_T("2 days"),
												_T("1 week"),
												_T("2 weeks"),
												_T("1 month"),
												_T("3 months"),
												_T("6 months"),
												_T("Never"),
												NULL
											};

int CNewsFeed::ChannelCleanupMaxAgeDays[] = {
												-1,
												0,
												1,
												2,
												7,
												14,
												30,
												90,
												180,
												-2,
												-3
											};

TCHAR* CNewsFeed::ChannelAutoUpdateString[] = {	_T("Default"),
														_T("1 min"),
														_T("5 min"),
														_T("10 min"),
														_T("15 min"),
														_T("30 min"),
														_T("1 hour"),
														_T("2 hours"),
														_T("3 hours"),
														_T("4 hours"),
														_T("Twice a day"),
														_T("Daily"),
														NULL};

int CNewsFeed::ChannelAutoUpdateSec[] = {
											0,
											1*60,
											5*60,
											10*60,
											15*60,
											30*60,
											60*60,
											120*60,
											180*60,
											240*60,
											12*60*60,
											24*60*60,
											0};

int CNewsFeed::GetChannelAutoUpdateFreqSec(INT_PTR nIndex) 
{
	ATLASSERT(nIndex < sizeof(ChannelAutoUpdateSec) / sizeof(INT_PTR));

	return ChannelAutoUpdateSec[nIndex];
}


CNewsFeed::CNewsFeed(void):
	m_id(0),m_groupID(0), m_bDisabled(false), m_usage(0),
	m_ttl(1), m_bloglineSubId(0), m_bBypassCache(false),m_unreadCount(0),
	m_updateFreq(0),m_cleanupMaxAge(0),m_notifyNewItem(0),
	m_numOfNewItemsInDays(0),
	m_bLimitItems(false),m_maxNumOfItems(200),
	m_bKeepInactive(false),
	m_nContentCRC(0),
	m_parent_feed_id(0),
	m_parent_item_id(0),
	m_bUseLogin(false)
{
	m_initTime = CGMTimeHelper::GetCurrentSysTime();
}


bool CNewsFeed::IsBloglinesSyncChannel() const
{
	return m_bloglineSubId != 0;
}

bool CNewsFeed::IsSearchChannel() const
{
	return CSearchChannelDef::IsSearchChannelUrl(m_url);
}

bool CNewsFeed::IsCommentChannel() const
{
	return m_parent_item_id != 0;
}

CNewsFeed::~CNewsFeed(void)
{
	ClearUpdate();
}

void CNewsFeed::UpdateProperties(const CNewsFeed& feed)
{
	//
	// update feed properties that were retrieved from feed xml
	//

	if(feed.m_description.GetLength())
		m_description = feed.m_description;
	if(feed.m_format.GetLength())
		m_format = feed.m_format;
	if(feed.m_image.GetLength())
		m_image = feed.m_image;
	if(feed.m_website.GetLength())
		m_website = feed.m_website;
	if(feed.m_language.GetLength())
		m_language = feed.m_language;

	m_lastModified = feed.m_lastModified;
	m_ttl = feed.m_ttl;
}

void CNewsFeed::CopyProperties(const CNewsFeed& feed, CopyLevel level)
{
	// At this level we only copy the properites that can be retrieved from web
	m_title = feed.m_title;
	m_description = feed.m_description;
	m_format = feed.m_format;
	m_image = feed.m_image;
	m_website = feed.m_website;
	m_language = feed.m_language;
	m_lastModified = feed.m_lastModified;
	m_ttl = feed.m_ttl;
	m_bloglineSubId = feed.m_bloglineSubId;

	// at this level we also copy user specified settings
	if(level > Web)
	{
		m_bUseLogin = feed.m_bUseLogin;
		m_loginName = feed.m_loginName;
		m_loginPassword = feed.m_loginPassword;
		m_bDisabled = feed.m_bDisabled;
		m_updateFreq = feed.m_updateFreq;
		m_cleanupMaxAge = feed.m_cleanupMaxAge;
		m_channelStyle = feed.m_channelStyle;
		m_notifyNewItem = feed.m_notifyNewItem;
		m_nContentCRC = feed.m_nContentCRC;
		m_parent_feed_id = feed.m_parent_feed_id;
		m_parent_item_id = feed.m_parent_item_id;
	}

	// at this level we copy everything else
	if(level > User)
	{
		m_id = feed.m_id;
	}
}

void CNewsFeed::Init(ULONG_PTR id)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from feed where feed_id = %d"), CFeedManagerLibHelper::m_newsFeedFields, id);
		CppSQLite3Query q = db.execQuery(sql);
		if(!q.eof())
		{
			CFeedManagerLibHelper::PopulateNewsFeed(this, q);
			q.finalize();
		}
		else
		{
			q.finalize();
			throw CExceptionBase(ERR_FM_NEWSFEEDNOTFOUND,_T("Cannot find specified news channel."));
		}
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}

void CNewsFeed::Save()
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		db.execDML(_T("begin immediate transaction;"));
		if(m_id <= 0)
		{
			// insert
			CppSQLite3Statement stmt = db.compileStatement(
				_T("insert into feed (")
				_T("feed_group_id,")	// 1
				_T("url,")				// 2
				_T("name,")				// 3
				_T("description,")		// 4
				_T("website,")			// 5
				_T("image,")			// 6
				_T("language,")			// 7
				_T("disabled,")			// 8
				_T("options1,")			// 9
				_T("update_freq,")		// 10
				_T("store_dur,")		// 11
				_T("options3,")			// 12
				_T("notify_newitem,")	// 13
				_T("limit_item,")		// 14
				_T("limit_item_number,")// 15
				_T("keep_inactive,")	// 16
				_T("parent_feed_id,")	// 17
				_T("news_id,")			// 18
				_T("use_login,")		// 19
				_T("login_name,")		// 20
				_T("login_pwd ")		// 21
				_T(") values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
			stmt.bind(1,m_groupID);
			stmt.bind(2,(LPCTSTR)m_url);
			stmt.bind(3,(LPCTSTR)m_title);
			m_description.IsEmpty() ? stmt.bindNull(4) : stmt.bind(4, (LPCTSTR)m_description);
			m_website.IsEmpty() ? stmt.bindNull(5) : stmt.bind(5,(LPCTSTR)m_website);
			m_image.IsEmpty() ? stmt.bindNull(6) : stmt.bind(6,(LPCTSTR)m_image);
			m_language.IsEmpty() ? stmt.bindNull(7) : stmt.bind(7,(LPCTSTR)m_language);
			stmt.bind(8,m_bDisabled ? 1 : 0);
			m_bloglineSubId != 0 ? stmt.bind(9,m_bloglineSubId) : stmt.bindNull(9);
			stmt.bind(10,m_updateFreq);
			stmt.bind(11,m_cleanupMaxAge);
			stmt.bind(12,m_channelStyle);
			stmt.bind(13,m_notifyNewItem);
			stmt.bind(14,m_bLimitItems);
			stmt.bind(15,m_maxNumOfItems);
			stmt.bind(16,m_bKeepInactive);
			stmt.bind(17,m_parent_feed_id);
			stmt.bind(18,m_parent_item_id);
			stmt.bind(19,m_bUseLogin ? 1 : 0);
			stmt.bind(20,(LPCTSTR)m_loginName);
			stmt.bind(21,(LPCTSTR)m_loginPassword);
			stmt.execDML();
			m_id = (LONG_PTR)db.lastRowId();
		}
		else
		{
			// update
			CFeedManagerLibHelper::UpdateNewsFeed(this, db);
		}
		db.execDML(_T("commit transaction;"));
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		CString str = e.errorMessage();
		if(str.Find(_T("column name is not unique"))>0)
			throw CUniqueNameException(ERR_FM_ERRSAVEFEED, _T("A channel with the same title already exists."));
		else
			throw CExceptionBase(ERR_FM_ERRSAVEFEED, e.errorMessage());
	}

}

void CNewsFeed::Rename(LPCTSTR newName)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
	CppSQLite3Statement stmt = db.compileStatement(_T("update feed set name = ? where feed_id=? "));
		stmt.bind(1,newName);
		stmt.bind(2,m_id);
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


void CNewsFeed::Delete()
{
	CString sql;
	sql.Format(_T("delete from feed where feed_id=%d"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
	CNewsFeedCache::RemoveFeed(m_id);
}

CString CNewsFeed::GetProcessedUrl()
{
	// conver search url
	if(IsSearchChannel())
	{
		CString url = m_url;
		CSearchChannelDef::ExpandUrl(url);
		return url;
	}
	else if(IsBloglinesSyncChannel())
	{
		return CGNSingleton<CBloglinesService>::Instance()->GetSyncChannelUrl(m_bloglineSubId);
	}
	else
	{
		return m_url;
	}
}

bool CNewsFeed::SkipUpdate() const
{
	if(m_lastChecked == 0 || m_ttl == 0)
		return false;

	CTimeSpan nowSpan = CGMTimeHelper::GetCurrentSysTime() - m_lastChecked;
	if(nowSpan.GetTotalMinutes()<m_ttl)
	{
		AtlTrace(_T("Channel[%s] update skipped due to TTL \n"), (LPCTSTR)m_title);
		return true;
	}

	return false;
}

bool CNewsFeed::UpdateDue(int seconds) const
{
	CTime lastChecked = (m_lastChecked!=0 ? m_lastChecked : m_initTime);

	CTimeSpan nowSpan = CGMTimeHelper::GetCurrentSysTime() - lastChecked;
	return (nowSpan.GetTotalSeconds()>=seconds);
}

int CNewsFeed::RetrieveNewsToDB()
{
	//
	// retrieve news from internet
	//
	try
	{
		NewsFeedParserPtr spParser = CNewsFeedParser::CreateParserFromURL(m_url);
		return ::RetrieveNewsToDB(this, spParser);
	}
	catch(CExceptionBase& e)
	{
		AtlTrace(_T("Failed to update channel[%s] due to %s\n"), (LPCTSTR)m_title, e.GetErrorMsg());
		throw;
	}

	return 0;
}

int CNewsFeed::RetrieveNewsToDB(NewsItemVector& newsItems, const CNewsFeed& feedWithInternetProperties)
{
	int numOfNewItems = 0;
	bool bTimeOrdered = true;
	bool bItemHasValidGuids = true;

	//
	// Check if this feed has correct time stamp
	//
	time_t nLastTimestamp = 0;
	CTime now = CGMTimeHelper::GetCurrentSysTime();
	for(NewsItemVector::reverse_iterator rit = newsItems.rbegin(); rit!=newsItems.rend();++rit)
	{
		NewsItemPtr& item = *rit;
		if(item->m_date == 0  // no timestamp?
			|| item->m_date > now) // timestamp set to future? (http://palmaddict.typepad.com)
		{
			// this feed doesn't have timestamp, we have to check them all
			bTimeOrdered = false;
			break; 
		}

		// we are looping from bottom up, newer news should be on top
		if(item->m_date.GetTime() >= nLastTimestamp) 
		{
			nLastTimestamp = (time_t)item->m_date.GetTime();
		}
		else
		{
			// some news are not in correct order, we have to check them all
			bTimeOrdered = false;
			break; 
		}
	}

	//
	// Check if items have valid guids
	//
	std::set<CString> guids;
	for(NewsItemVector::iterator it = newsItems.begin(); it!=newsItems.end();++it)
	{
		NewsItemPtr& item = *it;
		if(guids.find(item->m_guid) != guids.end())
		{
			bItemHasValidGuids = false;
			break;
		}

		guids.insert(item->m_guid);
	}
	guids.clear(); // clean it up to save some memory


	//
	// save news to database
	//
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		NewsWatchVector newsWatches;
		bool bWatchInited = false;

		NewsItemVector insertItems;
		NewsItemVector updateItems;
		insertItems.reserve(20);
		updateItems.reserve(20);


		CTime now = CGMTimeHelper::GetCurrentSysTime();
		CTime lastestItemUpdate;
		bool bSkipRest = false; // this flag is set when we found a existing item in a time-ordered feed
								// when this flag is set, we will only check updated items. No new item
								// will be inserted to avoid inserting items that's cleaned up.
		for(NewsItemVector::iterator it = newsItems.begin(); it!=newsItems.end();++it)
		{
			NewsItemPtr& item = *it;
			item->m_feedID = m_id;
			item->UnescapeTitle();

			// check if this item exists or not
			NewsItemPtr existingItem = CFeedManagerLibHelper::GetExistingNewsItem(item, bItemHasValidGuids, db);

			if(existingItem != NULL) // we found existing item
			{
				existingItem->UnescapeTitle();
				if(bTimeOrdered) // if the feed is ordered from newer to older, there's no new items since this one
				{
					bSkipRest = true;

					if(IsCommentChannel()) // if this is a comment feed, we will skip checking content as soon as we found existing one
						break;
				}
				
				if(bItemHasValidGuids) // we will only update item if the feed has valid guids
				{
					if(existingItem->m_title != item->m_title
						|| (existingItem->m_contentCRC != 0 && existingItem->m_contentCRC != item->m_contentCRC))
					{
						//found existing item, carry over its attributes
						item->m_id = existingItem->m_id;
						item->m_notRead = existingItem->m_notRead;

						// check if we need to mark the item to unread
						if(m_bMarkChangesUnread)
						{
							if(item->m_date > existingItem->m_date			// if the updated item has newer timestamp
								|| existingItem->GetUnescapedTitle() != item->GetUnescapedTitle())	// or title is different
							{
								item->m_notRead = 1;			// set item to unread
							}
						}

						// patch up this news item before saving
						//item->Compress();
						if(item->m_date == 0) // feed doesn't specify the time?
							item->m_date = now;
						if(item->m_author.GetLength() == 0 && this->m_author.GetLength() !=0)
							item->m_author = this->m_author; // use channel's author for the item

						updateItems.push_back(item);

						if(item->IsUnread() && !existingItem->IsUnread())
							numOfNewItems++;

						if(item->m_date > lastestItemUpdate)
							lastestItemUpdate = item->m_date;

					}
				}
			}
			else // new item
			{
				if(bSkipRest) // if this flag is set, this item is not really new. It's probably got cleaned up, so we should skip it
					continue;

				if(!bWatchInited)
				{
					CNewsWatch::GetNewsWatchesByChannel(newsWatches, m_id);
					bWatchInited = true;
				}

				bool bSkipNews = false;
				// check if there's any newsWatch interested in this item
				for(NewsWatchVector::iterator itWatch = newsWatches.begin(); itWatch!=newsWatches.end(); ++itWatch)
				{
					NewsWatchPtr& newsWatch = *itWatch;
					if(newsWatch->m_bDisabled || !newsWatch->Match(item))
					{
						continue;
					}

					if(newsWatch->m_nAction == CNewsWatch::DeleteItem)
					{
						bSkipNews = true;
						break; // no need to check other watches if this item is going to be ignored
					}


					if(newsWatch->m_nAction == CNewsWatch::MarkReadDirectly)
					{
						item->SetReadFlag(true);
					}

					item->m_vectWatches.push_back(newsWatch->m_id);

				}

				if(!bSkipNews)
				{
					// patch up this news item before saving
					//item->Compress();
					if(item->m_date == 0) // feed doesn't specify the time?
						item->m_date = now;
					if(item->m_author.GetLength() == 0 && this->m_author.GetLength() !=0)
						item->m_author = this->m_author; // use channel's author for the item

					insertItems.push_back(item);

					if(item->IsUnread())
						numOfNewItems++;

					if(item->m_date > lastestItemUpdate)
						lastestItemUpdate = item->m_date;
				}
			}
		}

		if(insertItems.empty() && updateItems.empty())
		{
			// no new item was found
			return 0;
		}
		
		//
		// write everything into database
		//
		AtlTrace(_T("Begining writing [%s]\n..."), (LPCTSTR)m_title);

		// update channel properties from internet
		UpdateProperties(feedWithInternetProperties);
		if(!m_lastModified.GetTime()				// if the feed has no timestamp
			|| m_lastModified < lastestItemUpdate)	// or has wrong timestamp
		{
			m_lastModified = lastestItemUpdate;		// use item's latest timestamp
		}

		db.execDML(_T("begin immediate transaction;"));
		if(!insertItems.empty())
		{
			CppSQLite3Statement stmtItem = db.compileStatement(
				_T("insert into news_item (")
				_T("feed_id,")		// 1
				_T("url,")			// 2
				_T("title,")		// 3
				_T("description,")	// 4
				_T("retrieved,")	// 5
				_T("item_guid,")	// 6
				_T("readtime,")		// 7
				_T("author,")		// 8
				_T("pod_url,")		// 9
				_T("commentRss,")	// 10
				_T("content_code")	// 11
				_T(") values (?,?,?,?,?,?,?,?,?,?,?);"));
			CppSQLite3Statement stmtWatch = db.compileStatement(
				_T("insert into watch_item (watch_id, news_id) values (?,?);"));
			for(NewsItemVector::reverse_iterator it = insertItems.rbegin(); it!=insertItems.rend();++it)
			{
				NewsItemPtr& item = *it;

				// save news item
				stmtItem.bind(1,item->m_feedID);
				stmtItem.bind(2,(LPCTSTR)item->m_url);
				stmtItem.bind(3,(LPCTSTR)item->m_title);
				//item->m_description.GetLength()==0 ?
				//	stmtItem.bindNull(4) : stmtItem.bind(4,item->GetCompressStream(), item->GetCompressLen());
				if (item->m_description.GetLength() == 0)
				{
					stmtItem.bindNull(4);
				}
				else
				{
					item->Compress();
					stmtItem.bind(4, item->GetCompressStream(), item->GetCompressLen());
					item->CleanCompress();
				}
				stmtItem.bind(5,item->m_date.GetTime());
				item->m_guid.GetLength()==0 ?
					stmtItem.bindNull(6) : stmtItem.bind(6,(LPCTSTR)item->m_guid);
				item->IsUnread() ?
					stmtItem.bindNull(7) : stmtItem.bind(7,1L);
				item->m_author.GetLength()==0 ? stmtItem.bindNull(8) : stmtItem.bind(8,(LPCTSTR)item->m_author);
				item->m_podCastingURL.GetLength()==0 ? stmtItem.bindNull(9):stmtItem.bind(9, (LPCTSTR)item->m_podCastingURL);
				item->m_commentFeedURL.GetLength()==0 ? stmtItem.bindNull(10):stmtItem.bind(10, (LPCTSTR)item->m_commentFeedURL);
				stmtItem.bind(11, item->m_contentCRC);
				stmtItem.execDML();
				item->m_id = (LONG_PTR)db.lastRowId();
				stmtItem.reset();

				// save watch item
				if(item->m_vectWatches.size())
				{
					std::vector<ULONG_PTR>::reverse_iterator it;
					for(it=item->m_vectWatches.rbegin(); it!=item->m_vectWatches.rend(); ++it)
					{
						stmtWatch.bind(1, (long)*it);
						stmtWatch.bind(2, item->m_id);
						stmtWatch.execDML();
						stmtWatch.reset();
					}
				}
			}
			stmtWatch.finalize();
			stmtItem.finalize();
		}

		if(!updateItems.empty())
		{
			CppSQLite3Statement stmtItem = db.compileStatement(
				_T("update news_item set ")
				_T("title = ?,")			// 1
				_T("description = ?,")		// 2
				_T("content_code = ?,")		// 3
				_T("retrieved = ?,")		// 4
				_T("readtime = ?,")			// 5
				_T("url = ? ")				// 6
				_T("where news_id = ?;"));	// 7
			for(NewsItemVector::iterator it = updateItems.begin(); it!=updateItems.end();++it)
			{
				NewsItemPtr& item = *it;
				stmtItem.bind(1,(LPCTSTR)item->m_title);
				//item->m_description.GetLength()==0 ?
				//	stmtItem.bindNull(2) : stmtItem.bind(2,item->GetCompressStream(), item->GetCompressLen());
				if (item->m_description.GetLength() == 0)
				{
					stmtItem.bindNull(2);
				}
				else
				{
					item->Compress();
					stmtItem.bind(2, item->GetCompressStream(), item->GetCompressLen());
					item->CleanCompress();
				}
				stmtItem.bind(3,item->m_contentCRC);
				stmtItem.bind(4,item->m_date.GetTime());
				item->IsUnread() ?
					stmtItem.bindNull(5) : stmtItem.bind(5,1L);
				stmtItem.bind(6,(LPCTSTR)item->m_url);
				stmtItem.bind(7,item->m_id);
				stmtItem.execDML();
				stmtItem.reset();
			}
			stmtItem.finalize();
		}

		// update channel properites and check timestamp
		CFeedManagerLibHelper::UpdateNewsFeed(this, db);
		if(IsCommentChannel())
		{
			// if this is a comment channel, the number of unread item is decided by if we
			// need to change parent item to unread.
			int numberOfChanged = CFeedManagerLibHelper::MarkNewsItemUnread(m_parent_item_id, db);
			numOfNewItems += numberOfChanged*65536; // high bytes store changed item. low bytes store new items of this comment feed
		}

		db.execDML(_T("commit transaction;"));
		db.close();

		// notify new items arrived if so configured
		if(m_notifyNewItem && ::IsWindow(m_notifyWnd))
		{
			NewsItemVector* pItems = new NewsItemVector();
			pItems->assign(insertItems.begin(), insertItems.end());
			::PostMessage(m_notifyWnd, MM_NEWITEM_ARRIVED, 0, (LPARAM)(void*)pItems);
		}

		AtlTrace(_T("End writing [%s]\n..."), (LPCTSTR)m_title);

	}
	catch(CppSQLite3Exception& e)
	{
		AtlTrace(_T("Failed to update channel[%s] due to %s\n"), (LPCTSTR)m_title, e.errorMessage());
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return numOfNewItems;
}

size_t CNewsFeed::GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, CNewsFilter* pNewsFilter) const
{
	CString criteria;
	criteria.Format(_T(" feed_id = %d "), m_id);
	LPCTSTR orderBy = NULL;
	if(pNewsFilter->GenOrderByClause().GetLength() == 0)
	{
		orderBy = _T(" order by news_id desc ");
	}
	return CFeedManagerLibHelper::GetNewsItemIDs(newsIDs, criteria, pNewsFilter, orderBy);
}

size_t CNewsFeed::GetNewsItems(NewsItemVector& items)
{
	CString criteria;
	criteria.Format(_T(" feed_id = %d "), m_id);
	return CFeedManagerLibHelper::RetrieveNewsFromDB(items, criteria, NULL, _T(""));
}


CString CNewsFeed::GenerateSectionHTML()
{
	CString html;

	CString titleText = m_website.GetLength() == 0 ? m_title : _T("<a href=\"") + m_website + _T("\">") + m_title + _T("</a>");
	html.AppendFormat(_T("<div class=\"gn_channeltitle\"><div class=\"gn_channeltitletext\">%s</div>\n"),
		(LPCTSTR)titleText);

	if(m_description.GetLength())
	{
		html.AppendFormat(_T("<div class=\"gn_channeldesc\">%s</div>\n"), (LPCTSTR)m_description);
	}

	if(m_image.GetLength() && m_image.CompareNoCase(_T("http://"))!=0 && m_image.CompareNoCase(_T("http:///"))!=0)
	{
		html.AppendFormat(_T("<div class=\"gn_channelimage\"><a href=\"%s\"><img src=\"%s\" border=\"0\"></a></div>\n"),
			m_website, m_image);
	}
	html.Append(_T("</div>\n"));
	return html;
}


CString CNewsFeed::GeneratePageHTML(int nPage)
{
	CString content = GeneratePageContentHTML(nPage);
	CString section = GenerateSectionHTML();
	CString pager = GeneratePager();
	CString html;
	html.Append(pager);
	html.Append(section);
	html.Append(content);
	html.Append(pager);
	return html;
}


CString CNewsFeed::GetContentID() const
{
	CString temp;
	temp.Format(_T("N%d_%d"), m_id, m_nCurrentPage);
	return temp;
}

int CNewsFeed::CheckUnread()
{
	CString criteria;
	criteria.Format(_T(" feed_id = %d "), m_id);
	return CNewsSource::CheckUnread(criteria);
}

CString CNewsFeed::GetNewsSourceName() const
{
	return m_title;
}

void CNewsFeed::MarkRead()
{
	CString sql;
	sql.Format(_T("update news_item set readtime=1 where feed_id = %d and readtime is null;")
				, m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	CNewsItemCache::Empty();
}

void CNewsFeed::MarkUnread()
{
	CString sql;
	sql.Format(_T("update news_item set readtime=null where feed_id = %d and readtime is not null;"),
					m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	CNewsItemCache::Empty();
}

size_t CNewsFeed::GetNewsFeeds(const std::vector<ULONG_PTR>& IDs, NewsFeedVector& newsfeeds)
{
	newsfeeds.clear();

	CString criteria = _T(" feed_id in (");
	for(std::vector<ULONG_PTR>::const_iterator it=IDs.begin(); it!=IDs.end(); ++it)
	{
		criteria.AppendFormat(_T("%d,"), *it);
	}
	criteria.TrimRight(_T(","));
	criteria.Append(_T(") "));
	CString sql;
	sql.Format(_T("select %s from feed where %s;"),
		CFeedManagerLibHelper::m_newsFeedFields,
		(LPCTSTR)criteria);


	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CNewsFeed* nf =  new CNewsFeed();
			CFeedManagerLibHelper::PopulateNewsFeed(nf, q);
			newsfeeds.push_back(nf);
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


long CNewsFeed::GetNewsFeedCount()
{
	long num = 0;

	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql = _T("select count(*) from feed;");
		num = db.execScalar(sql);
		db.close();
	}
	catch(...)
	{
	}

	return num;
}

size_t CNewsFeed::GetAllNewsFeeds(NewsFeedVector& newsfeeds,  bool bIncludeDisabled, SortOption sortOption)
{
	newsfeeds.clear();

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from feed"), CFeedManagerLibHelper::m_newsFeedFields);
		if(sortOption == SortById)
		{
			sql.Append(_T(" order by feed_id "));
		}
		else if(sortOption == SortByTitle)
		{
			sql.Append(_T(" order by name COLLATE GNUNICODE "));
		}
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CNewsFeed* nf =  new CNewsFeed();
			CFeedManagerLibHelper::PopulateNewsFeed(nf, q);
			if(!nf->m_bDisabled || bIncludeDisabled)
			{
				newsfeeds.push_back(nf);
			}
			else
			{
				delete nf;
			}
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

CString CNewsFeed::GetHomeURL() const
{
	return m_website;
}

void CNewsFeed::Cleanup(int defaultDays, bool bDeleteUnread, bool bDeleteMarked, bool bLimtItems, int numOfItemsToKeep)
{
	int days = -1;

	if(m_cleanupMaxAge == 0) // no channel level cleanup setting?
	{
		days = defaultDays;
	}
	else
	{
		ATLASSERT(m_cleanupMaxAge < sizeof(ChannelCleanupMaxAgeDays)/sizeof(int));
		days = ChannelCleanupMaxAgeDays[m_cleanupMaxAge];
		if(days < 0)
			return; // do nothing
	}

	CString where;

	CString sql;
	sql.Format(_T("delete from news_item where feed_id = %d "), m_id);
	if(days>0)
	{
		CTime time = CGMTimeHelper::DaysAgo(CGMTimeHelper::GetCurrentSysTime(), days);
		sql.AppendFormat(_T(" and retrieved < %d "), (int)time.GetTime() );
	}

	if(!bDeleteUnread)
		sql +=  "and readtime is not null ";
	if(!bDeleteMarked)
		sql +=  "and (mark is null or mark = 0)";

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	if(bLimtItems)
	{
		// check if we have channel level max item setting
		if(m_bLimitItems && m_maxNumOfItems> 0)
			numOfItemsToKeep = m_maxNumOfItems;

		// clean up to max num of item
		sql.Format(_T("delete from news_item where feed_id=%d and news_id not in \
						(select news_id from news_item \
							where feed_id = %d order by news_id desc limit %d) \
						and (mark is null or mark = 0)")
							, m_id
							, m_id
							, numOfItemsToKeep);
		CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
	}
}

ULONG_PTR CNewsFeed::GetIdFromXmlUrl(LPCTSTR url)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);
	ULONG_PTR id = 0;

	try
	{
		CppSQLite3Statement stmt = db.compileStatement(_T("select feed_id from feed where url=? "));
		stmt.bind(1,url);
		CppSQLite3Query q = stmt.execQuery();
		if(!q.eof())
		{
			id = q.getIntField(0);
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return id;
}

void CNewsFeed::SaveToXml(MSXML2::IXMLDOMElementPtr spElement)
{
	spElement->setAttribute(_T("type"), _T("rss"));
	spElement->setAttribute(_T("text"), (LPCTSTR)m_title);
	spElement->setAttribute(_T("title"), (LPCTSTR)m_title);
	spElement->setAttribute(_T("description"), (LPCTSTR)m_description);
	CString url = m_url;
	url.Replace(_T("feed://"),_T("http://")); 
	spElement->setAttribute(_T("xmlUrl"), (LPCTSTR)url);
	spElement->setAttribute(_T("htmlUrl"), (LPCTSTR)m_website);
	// spElement->setAttribute(_T("imageUrl"), (LPCTSTR)m_image);
}

void CNewsFeed::LoadFromXml(MSXML2::IXMLDOMElementPtr spElement)
{
	_bstr_t xmlUrl = spElement->getAttribute("xmlUrl");
	m_url = (LPCTSTR)xmlUrl;

	CFeedManagerLibHelper::GetTitle(spElement, m_title);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("description"),m_description);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("htmlUrl"),m_website);
	CFeedManagerLibHelper::SafeGetAttr(spElement,_T("imageUrl"),m_image);
}


int CNewsFeed::GetNumOfNewsItems(bool bUnreadOnly) const
{
	long num = 0;

	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql;
		sql.Format(_T("select count(*) from news_item where feed_id=%d %s;")
			, m_id
			, bUnreadOnly ? _T(" and readtime is null") : _T("") );
		num = db.execScalar(sql);
		db.close();
	}
	catch(...)
	{
	}

	return num;
}

int CNewsFeed::GetNumOfLabelled() const
{
	long num = 0;

	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);

		CString sql;
		sql.Format(_T("select count(*) from item_tag where news_id in (select news_id from news_item where feed_id = %d);"), m_id);
		num = db.execScalar(sql);
		db.close();
	}
	catch(...)
	{
	}

	return num;	
}


void CNewsFeed::UpdateUsage()
{
	CString sql;
	sql.Format(_T("update feed set usage=ifnull(usage,0)+2 where feed_id=%d"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}


size_t CNewsFeed::GetStatistics(NewsFeedVector& newsfeeds, Statistics statType, int numOfChannels, bool bStatsExcludeDisabled)
{
	newsfeeds.clear();

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from feed f"), CFeedManagerLibHelper::m_newsFeedFields);
		if(bStatsExcludeDisabled)
		{
			sql.Append(_T(" where f.disabled != 1 and f.feed_group_id not in (select feed_group_id from feed_group where disabled > 0) "));
		}

		if(statType == Most10Visited)
		{
			sql.Append(_T(" order by f.usage desc"));
		}
		else if(statType == Least10Visited)
		{
			sql.Append(_T(" order by f.usage"));
		}
		else if(statType == MostActive)
		{
			sql.Append(_T(" order by f.last_updated desc"));
		}
		else if(statType == LeastActive)
		{
			sql.Append(_T(" order by f.last_updated"));
		}

		sql.AppendFormat(_T(" limit %d "), numOfChannels);
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CNewsFeed* nf =  new CNewsFeed();
			CFeedManagerLibHelper::PopulateNewsFeed(nf, q);
			newsfeeds.push_back(nf);
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

size_t CNewsFeed::GetActivityStatistics(NewsFeedVector& feeds, Statistics statType, int numOfDays, int numOfChannels, bool bStatsExcludeDisabled)
{
	feeds.clear();
	CTime daysAgo = CGMTimeHelper::DaysAgo(CGMTimeHelper::GetCurrentSysTime(), numOfDays);

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select f.feed_id, num from feed f left join (select feed_id, count(news_id) num from ")
		_T("(select feed_id, news_id from news_item where retrieved >= %d) group by feed_id) n ")
		_T("on f.feed_id = n.feed_id "), (int)daysAgo.GetTime());

		if(bStatsExcludeDisabled)
			sql.Append(_T(" where f.disabled != 1 and f.feed_group_id not in ")
		_T("(select feed_group_id from feed_group where options1 > 0) "));

		sql.Append(	_T(" order by num "));
		if(statType == CNewsFeed::MostActive)
		{
			sql.Append(_T(" desc "));
		}
		sql.AppendFormat(_T(" limit %d"), numOfChannels);

		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(q.getIntField(0));
			feed->m_numOfNewItemsInDays = q.getIntField(1);

			feeds.push_back(feed);
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return feeds.size();
}

void CNewsFeed::StartDownloading(HANDLE semReadyToStore)
{
	m_updateErrMsg.Empty();

	CString url = GetProcessedUrl();
	CString normalizedUrl = CGNUtil::NormalizeURL(url);

	m_updateStartTime = CGMTimeHelper::GetCurrentSysTime();

	try
	{
		ATLASSERT(m_downloadContext == NULL);
		m_downloadContext = new CDownloadContext();

		CDownloadContext::DownloadOptions options = 
			m_bBypassCache ? CDownloadContext::BypassCache : CDownloadContext::NoOption;
		if(!IsBloglinesSyncChannel())
			m_downloadContext->SetDownloadParam(semReadyToStore,
												(LPCTSTR)normalizedUrl,
												m_bUseLogin ? (LPCTSTR)m_loginName : NULL,
												m_bUseLogin ? (LPCTSTR)m_loginPassword : NULL,
												options);
		else
		{
			CString acnt,pwd;
			CGNSingleton<CBloglinesService>::Instance()->GetAccountInfo(acnt, pwd);
			m_downloadContext->SetDownloadParam(semReadyToStore,
												(LPCTSTR)normalizedUrl,
												(LPCTSTR)acnt,
												(LPCTSTR)pwd,
												options);
		}

		if(!IsCommentChannel())	// comment channel cannot use this because WordPress has problem to tell comment feed and original feed
		{
			m_downloadContext->m_httpLastModified = this->m_httpLastModified;
			m_downloadContext->m_httpETag = this->m_httpETag;
		}

		CGNSingleton<CAsyncDownloadManager>::Instance()->StartDownload(m_downloadContext);

	}
	catch(_com_error& err)
	{
		_bstr_t desc = err.Description();
		m_updateErrMsg = (LPCTSTR)desc;
		throw CExceptionBase(ERR_FM_GENERICERR, (LPCTSTR)desc);
	}
}

void CNewsFeed::CheckTimeout(CTimeSpan& span)
{
	CTimeSpan nowSpan = CGMTimeHelper::GetCurrentSysTime() - m_updateStartTime;
	if(span < nowSpan)
	{
		AbortUpdate();
	}
}

bool CNewsFeed::IsDownloadReady()
{
	return (m_downloadContext != NULL && m_downloadContext->m_bReady == true);
}

void CNewsFeed::AbortUpdate()
{
	AtlTrace(_T("Channel Update Aborting...\n"));
	if(m_downloadContext != NULL)
		m_downloadContext->Abort();
}

void CNewsFeed::ClearUpdate()
{
	if(m_downloadContext != NULL)
	{
		if(m_downloadContext->m_contextID)
		{
			CGNSingleton<CAsyncDownloadManager>::Instance()->FinishDownload(m_downloadContext->m_contextID);
		}
		m_downloadContext = NULL;
	}
}

int CNewsFeed::StoreUpdate()
{
	m_bBypassCache = false; // user has to setup this flag everytime

	// In case of an error, try again 20 minutes later (Gets overriden by specific errors, eg #304)
	m_lastChecked = CGMTimeHelper::GetCurrentSysTime().GetTime() - GetChannelAutoUpdateFreqSec(m_updateFreq) + 20 * 60;

	// remove the m_downloadContext first
	DownloadContextPtr context = m_downloadContext;
	ClearUpdate();

	if(context->m_bFailed)
	{
		if(context->m_errorMsg.GetLength())
		{
			m_updateErrMsg.Format(_T("%s\nError Code[%d]\n"),
				(LPCTSTR)(context->m_errorMsg),
				context->m_errorCode);
		}
		else
		{
			m_updateErrMsg.Format(_T("Channel update failed due to unknown error.\nError Code[%d]\n"),
				context->m_errorCode);
		}
		AtlTrace(_T("%s\n"), (LPCTSTR)m_updateErrMsg);
		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, m_updateErrMsg);
	}

	long statusCode = context->m_httpStatusCode;

	if(statusCode != 200)
	{
		if(IsBloglinesSyncChannel())
		{
			switch(statusCode)
			{
			case 304: // the request produced no entries 
				m_lastChecked = CGMTimeHelper::GetCurrentSysTime();
				return 0;
			case 401: //incorrect email address or password 
				m_updateErrMsg = _T("Incorrect email address or password");
				throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, m_updateErrMsg);
			case 403: //invalid or missing BloglinesSubId 
				m_updateErrMsg = _T("Invalid Bloglines subscription");
				throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, m_updateErrMsg);
			case 410: //subscription has been deleted
				m_lastChecked = CGMTimeHelper::GetCurrentSysTime();
				Disable(true);
				return 0;
			}
		}
		else // regular feed
		{
			if(statusCode == 304) // not modified
			{
				AtlTrace(_T("Channel[%s] returns 304 \n"), (LPCTSTR)m_title);
				m_lastChecked = CGMTimeHelper::GetCurrentSysTime();
				return 0;
			}
			else if(statusCode == 410) // Discontinued Feed
			{
				m_lastChecked = CGMTimeHelper::GetCurrentSysTime();
				Disable(true);
				return 0;
			}
		}

		m_updateErrMsg.Format(_T("Error reading from channel website. Return status is [%d]"), statusCode);
		AtlTrace(_T("%s\n"), (LPCTSTR)m_updateErrMsg);
		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, m_updateErrMsg);
	}

	// save the header information for 304-conditional-get handling
	m_httpLastModified = context->m_httpLastModified;
	m_httpETag = context->m_httpETag;
	m_lastChecked = CGMTimeHelper::GetCurrentSysTime();

	if(m_nContentCRC!=0 && m_nContentCRC == context->m_dataCRC)
	{
		// content of the feed has changed since last time we checked
		AtlTrace(_T("Channel[%s] store skipped due to the same CRC \n"), (LPCTSTR)m_title);
		return 0;
	}
	m_nContentCRC = context->m_dataCRC;

	try
	{
		NewsFeedParserPtr spParser = CNewsFeedParser::CreateParser(m_url, context->m_data);

		return ::RetrieveNewsToDB(this, spParser);
	}
	catch(CExceptionBase& e)
	{
		m_updateErrMsg.Format(_T("Failed to update channel[%s] due to %s"), (LPCTSTR)m_title, e.GetErrorMsg());
		AtlTrace(_T("%s\n"), (LPCTSTR)m_updateErrMsg);
		throw;
	}

	return 0;
}



void CNewsFeed::CountUnread(std::map<ULONG_PTR, int>& mapUnread)
{
	CString sql;
	sql.Format(_T("select feed_id, count(*) from news_item where readtime is null group by feed_id"));

	return CNewsSource::CountUnread(mapUnread, sql);
}

void CNewsFeed::ExportRss(MSXML2::IXMLDOMElementPtr& spChannel)
{
	MSXML2::IXMLDOMDocumentPtr spDoc = spChannel->ownerDocument;

	AddElement(spDoc,spChannel,_T("title"), m_title);
	AddElement(spDoc,spChannel,_T("link"), m_website);
	AddElement(spDoc,spChannel,_T("description"),m_description);
	if(m_language.GetLength())
		AddElement(spDoc,spChannel,_T("language"),m_language);
	if(m_lastModified != 0)
		AddElement(spDoc,spChannel,_T("pubDate"),CGMTimeHelper::FormatRFC822Date(m_lastModified));
	AddElement(spDoc,spChannel,_T("generator"),_T("GreatNews 1.0"));
	AddElement(spDoc,spChannel,_T("docs"),_T("http://blogs.law.harvard.edu/tech/rss"));

	//if(m_image)
	//{
	//	MSXML2::IXMLDOMElementPtr image = spDoc->createElement(_T("image"));
	//	spChannel->appendChild(image);
	//	AddElement(spDoc,image,_T("url"),m_image);
	//}

}

void CNewsFeed::Disable(bool bDisable)
{
	m_bDisabled = bDisable;

	CString sql;
	sql.Format(_T("update feed set disabled=%d where feed_id=%d"), 
			bDisable? 1 : 0,
			m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

void CNewsFeed::SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts)
{
	CFeedManagerLibHelper::SaveUnreadCount(counts, _T("feed"), _T("feed_id"));
}


void CNewsFeed::DetachFromBloglines()
{
	m_bloglineSubId = 0;

	CString sql;
	sql.Format(_T("update feed set options1 = null where feed_id = %d"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

void CNewsFeed::KeepEvenInactive()
{
	m_bKeepInactive = true;

	CString sql;
	sql.Format(_T("update feed set keep_inactive=1 where feed_id=%d"), m_id);

	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);
}

void CNewsFeed::DeleteOrDisable()
{
	try
	{
		CString sql;
		sql.Format(_T("select count(*) from news_item where feed_id=%d and mark >=1 limit 1;"), m_id);

		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);
		long num = db.execScalar(sql);
		db.close();

		if(num>0) // do not delete feed that has tagged items
			Disable();
		else
			Delete();
	}
	catch(...)
	{
	}
}

NewsFeedPtr CNewsFeed::AddCommentFeed(NewsItemPtr& pItem)
{
	//NewsFeedParserPtr spParser = CNewsFeedParser::CreateParserFromURL(pItem->m_commentFeedURL);
	//if(spParser == NULL)
	//{
	//	return NULL;
	//}
	
	// create the comment feed
	CNewsFeed* newsFeed = new CNewsFeed();
	newsFeed->m_url = pItem->m_commentFeedURL;
	newsFeed->m_title = _T("Comments on \"")+pItem->m_title+ _T("\"");
	newsFeed->m_parent_feed_id = pItem->m_feedID;
	newsFeed->m_parent_item_id = pItem->m_id;
	newsFeed->Save();

	// now save the comments
//	::RetrieveNewsToDB(newsFeed, spParser);

	NewsFeedPtr feed(newsFeed);
	CNewsFeedCache::AddFeed(feed);

	return feed;
}

void CNewsFeed::NormalizeHomeUrl()
{
	//if(m_website != "./")
	//	return;

#define MAXURL INTERNET_MAX_URL_LENGTH
	TCHAR szUrl[MAXURL];
	DWORD dwSize = MAXURL;
	if(InternetCombineUrl (m_url,m_website,szUrl,&dwSize,ICU_BROWSER_MODE))
	{
		m_website = szUrl;
	}
}
