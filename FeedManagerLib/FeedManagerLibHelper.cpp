#include "StdAfx.h"
#include "FeedManagerLibHelper.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "GMTimeLib.h"

LPCTSTR CFeedManagerLibHelper::m_newsFeedFields = 
	_T("feed_id,")			// 0
	_T("feed_group_id,")	// 1
	_T("url,")				// 2
	_T("name,")				// 3
	_T("description,")		// 4
	_T("image,")			// 5
	_T("website,")			// 6
	_T("last_updated,")		// 7
	_T("language,")			// 8
	_T("disabled,")			// 9
	_T("usage,")			// 10
	_T("options1,")			// 11
	_T("unread_count,")		// 12
	_T("hd_last_mod,")		// 13
	_T("hd_etag,")			// 14
	_T("update_freq,")		// 15
	_T("store_dur,")		// 16
	_T("options3,")			// 17
	_T("notify_newitem,")	// 18
	_T("limit_item,")		// 19
	_T("limit_item_number,")// 20
	_T("keep_inactive,")	// 21
	_T("contentCode,")		// 22
	_T("parent_feed_id,")	// 23
	_T("news_id,")			// 24
	_T("use_login,")		// 25
	_T("login_name,")		// 26
	_T("login_pwd ");		// 27

void CFeedManagerLibHelper::PopulateNewsFeed(CNewsFeed* nf, CppSQLite3Query& q)
{
	nf->m_id = q.getIntField(0);
	nf->m_groupID = q.getIntField(1);
	nf->m_url = q.getStringField(2);
	nf->m_title = q.getStringField(3);
	nf->m_description = q.getStringField(4);
	nf->m_image = q.getStringField(5, _T(""));
	nf->m_website = q.getStringField(6, _T(""));
	nf->m_lastModified = q.getIntField(7, 0);
	/*
	if (nf->m_lastModified > 0)
		nf->m_lastChecked = nf->m_lastModified;
	*/
	nf->m_language = q.getStringField(8, _T(""));
	nf->m_bDisabled = (q.getIntField(9, 0)!=0);
	nf->m_usage = q.getIntField(10, 0);
	nf->m_bloglineSubId = q.getIntField(11,0);
	nf->m_unreadCount = q.getIntField(12,0);
	nf->m_httpLastModified = q.getStringField(13,_T(""));
	nf->m_httpETag = q.getStringField(14, _T(""));
	nf->m_updateFreq = q.getIntField(15,0);
	nf->m_cleanupMaxAge = q.getIntField(16,0);
	nf->m_channelStyle = q.getStringField(17, _T(""));
	nf->m_notifyNewItem = q.getIntField(18,0);
	nf->m_bLimitItems = (q.getIntField(19)!=0);
	nf->m_maxNumOfItems = q.getIntField(20, 200);
	nf->m_bKeepInactive = (q.getIntField(21)!=0);
	nf->m_nContentCRC = q.getUnsignedIntField(22);
	nf->m_parent_feed_id = q.getIntField(23);
	nf->m_parent_item_id = q.getIntField(24);
	nf->m_bUseLogin = (q.getIntField(25)!=0);
	nf->m_loginName = q.getStringField(26);
	nf->m_loginPassword = q.getStringField(27);
}

LPCTSTR CFeedManagerLibHelper::m_newsItemFields = 
	_T("news_id,")			// 0
	_T("feed_id,")			// 1
	_T("url,")				// 2
	_T("title,")			// 3
	_T("description,")		// 4
	_T("retrieved,")		// 5
	_T("readtime,")			// 6
	_T("item_guid,")		// 7
	_T("mark,")				// 8
	_T("author,")			// 9
	_T("pod_url,")			// 10
	_T("commentRss,")		// 11
	_T("commentTrack ");	// 12

void CFeedManagerLibHelper::PopulateNewsItem(CNewsItem* ni, CppSQLite3Query& q)
{
	ni->m_id = q.getIntField(0);
	ni->m_feedID = q.getIntField(1);
	ni->m_url = q.getStringField(2);
	ni->m_title = q.getStringField(3);
	if(!q.fieldIsNull(4))
	{
		int nLen=0;
		const BYTE* pStream = q.getBlobField(4, nLen);
		ni->DecompressDescription(pStream, nLen);
	}
	ni->m_date = q.getIntField(5, 0);
	ni->SetReadFlag(!q.fieldIsNull(6));
	ni->m_guid = q.getStringField(7, _T(""));
	ni->m_marked = q.getIntField(8, 0);
	ni->m_author = q.getStringField(9, _T(""));
	ni->m_podCastingURL = q.getStringField(10, _T(""));
	ni->m_commentFeedURL = q.getStringField(11, _T(""));
	ni->m_bTrackComments = (q.getIntField(12)!=0);
}


LPCTSTR CFeedManagerLibHelper::m_newsWatchFields = 
	_T("watch_id,")			// 0
	_T("name,")				// 1
	_T("criteria,")			// 2
	_T("selective_feed,")	// 3
	_T("watch_flag,")		// 4
	_T("match_case,")		// 5
	_T("whole_word,")		// 6
	_T("all_words,")		// 7
	_T("front_color,")		// 8
	_T("bk_color,")			// 9
	_T("display_order,")	// 10
	_T("options1,")			// 11
	_T("unread_count,")		// 12
	_T("options3,")			// 13
	_T("options2");		// 14

void CFeedManagerLibHelper::PopulateNewsWatch(CNewsWatch* nw, CppSQLite3Query& q)
{
	nw->m_id = q.getIntField(0);
	nw->m_title = q.getStringField(1);
	nw->m_matchCriteria = q.getStringField(2);
	nw->m_bWatchSelectiveChannels = (q.getIntField(3) > 0);
	nw->m_watchFlag = q.getIntField(4);
	nw->m_matchCase = q.getIntField(5);
	nw->m_wholeWord = q.getIntField(6);
	nw->m_allWords = q.getIntField(7);
	if(!q.fieldIsNull(8))
		nw->m_txtColor = q.getIntField(8);
	if(!q.fieldIsNull(9))
		nw->m_bkColor = q.getIntField(9);
	nw->m_order = q.getIntField(10);
	nw->m_nAction = (CNewsWatch::Action)q.getIntField(11, 0);
	nw->m_unreadCount = q.getIntField(12,0);
	nw->m_watchStyle = q.getStringField(13);
	nw->m_bDisabled = (q.getIntField(14) != 0);
}

LPCTSTR CFeedManagerLibHelper::m_feedGroupFields = 
	_T("feed_group_id,")	// 0
	_T("name,")				// 1
	_T("description,")		// 2
	_T("options3,")			// 3
	_T("unread_count,")		// 4
	_T("options1,")			// 5
	_T("parent_group,")		// 6
	_T("options2");		// 7

void CFeedManagerLibHelper::PopulateFeedGroup(CFeedGroup* fg, CppSQLite3Query& q)
{
	fg->m_id = q.getIntField(0);
	fg->m_name = q.getStringField(1);
	fg->m_desc = q.getStringField(2);
	fg->m_website = q.getStringField(3, _T(""));
	fg->m_unreadCount = q.getIntField(4, 0);
	fg->m_bDisabled = (q.getIntField(5, 0)!=0);
	fg->m_parentID = q.getIntField(6, 0);
	fg->m_bExpanded = (q.getIntField(7, 0)!=0);

}

NewsItemPtr CFeedManagerLibHelper::GetExistingNewsItem(NewsItemPtr& item, bool bItemHasValidGuids, CppSQLite3DB& db)
{
	NewsItemPtr existingItem = NULL;
	try
	{
		CppSQLite3Statement stmt;
		if(bItemHasValidGuids)
		{
			if(item->m_guid.Find(_T("http://hosted.ap.org/")) == 0)
			{
				// this is a AP's feed that generate new guids for existing items everytime @!$!#@$#$
				CString guid = item->m_guid;
				int pos = guid.Find(_T('?'));
				if(pos>0)
					guid = guid.Left(pos)+ _T("%");

				stmt = db.compileStatement(
					_T("select ")
					_T("news_id,")				// 1
					_T("title,")				// 2
					_T("content_code,")			// 3
					_T("retrieved,")			// 4
					_T("readtime ")				// 5
					_T("from news_item where ")
					_T("feed_id = ? and ")		// 1
					_T("content_code = ? and ")	// 2
					_T("title = ? limit 1"));	// 3
				stmt.bind(1, item->m_feedID);
				stmt.bind(2, item->m_contentCRC);
				stmt.bind(3, (LPCTSTR)item->m_title);
			}
			else
			{
				stmt = db.compileStatement(
					_T("select ")
					_T("news_id,")				// 0
					_T("title,")				// 1
					_T("content_code,")			// 2
					_T("retrieved,")			// 3
					_T("readtime ")				// 4
					_T("from news_item where ")
					_T("item_guid = ? and ")	// 1
					_T("feed_id = ? limit 1"));	// 2
				stmt.bind(1, (LPCTSTR)item->m_guid);
				stmt.bind(2, item->m_feedID);
			}
		}
		else
		{
			// if the guid is invalid, we add title to match criteria
			stmt = db.compileStatement(
				_T("select ")
				_T("news_id,")				// 1
				_T("title,")				// 2
				_T("content_code,")			// 3
				_T("retrieved,")			// 4
				_T("readtime ")				// 5
				_T("from news_item where ")
				_T("item_guid = ? and ")	// 1
				_T("feed_id = ? and ")		// 2
				_T("title = ? limit 1"));	// 3
			stmt.bind(1, (LPCTSTR)item->m_guid);
			stmt.bind(2, item->m_feedID);
			stmt.bind(3,(LPCTSTR)item->m_title);
		}

		CppSQLite3Query q = stmt.execQuery();
		if(!q.eof())
		{
			CNewsItem* ni = new CNewsItem();
			ni->m_id = q.getIntField(0);
			ni->m_title = q.getStringField(1);
			ni->m_contentCRC = q.getUnsignedIntField(2, 0);
			ni->m_date = q.getIntField(3, 0);
			ni->SetReadFlag(!q.fieldIsNull(4));
			existingItem = ni;
		}
		q.finalize();
		stmt.finalize();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return existingItem;
}


size_t CFeedManagerLibHelper::GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, const CString& criteria, CNewsFilter* pNewsFilter, LPCTSTR orderBy)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	newsIDs.clear();

	try
	{
		CString sql;
		sql.Format(_T("select news_id from news_item where %s"), (LPCTSTR)criteria);
		if(pNewsFilter)
		{
			CString filterWhere = pNewsFilter->GenerateWhereClause();
			if(filterWhere.GetLength())
			{
				sql.AppendFormat(_T(" AND %s"), filterWhere);
			}
		}

		if(orderBy != NULL)
			sql.Append(orderBy);
		else if(pNewsFilter)
			sql.AppendFormat(_T(" order by %s "), (LPCTSTR)pNewsFilter->GenOrderByClause());

		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			newsIDs.push_back((ULONG_PTR)q.getIntField(0));
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return newsIDs.size();
}

size_t CFeedManagerLibHelper::RetrieveNewsFromDB(NewsItemVector& newsItems, const CString& criteria, CNewsFilter* pNewsFilter, LPCTSTR orderBy)
{
	newsItems.clear();

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select %s from news_item where %s"), CFeedManagerLibHelper::m_newsItemFields, (LPCTSTR)criteria);
		if(pNewsFilter)
		{
			CString filterWhere = pNewsFilter->GenerateWhereClause();
			if(filterWhere.GetLength())
			{
				sql.AppendFormat(_T(" AND %s"), filterWhere);
			}
		}
		if(orderBy != NULL)
			sql.Append(orderBy);
		else if(pNewsFilter)
			sql.AppendFormat(_T(" order by %s "), (LPCTSTR)pNewsFilter->GenOrderByClause());

		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			CNewsItem* ni =  new CNewsItem();
			CFeedManagerLibHelper::PopulateNewsItem(ni, q);
			newsItems.push_back(ni);
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return newsItems.size();
}

void CFeedManagerLibHelper::DBExec(LPCTSTR sql, TransactionSettings useTrx)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		if(useTrx==UseTransaction)
			db.execDML(_T("begin immediate transaction;"));
		db.execDML(sql);
		if(useTrx==UseTransaction)
			db.execDML(_T("commit transaction;"));
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}
}

bool CFeedManagerLibHelper::GetTitle(MSXML2::IXMLDOMElementPtr spElement, CString& title)
{
	_variant_t v = spElement->getAttribute(_T("text"));
	if(v.vt == VT_BSTR)
	{
		title = v.bstrVal;
		return true;
	}

	v = spElement->getAttribute(_T("title"));
	if(v.vt == VT_BSTR)
	{
		title = v.bstrVal;
		return true;
	}

	return false;
}

bool CFeedManagerLibHelper::SafeGetAttr(MSXML2::IXMLDOMElementPtr spElement, LPCTSTR attr, CString& value)
{
	_variant_t v = spElement->getAttribute(attr);
	if(v.vt == VT_BSTR)
	{
		value = v.bstrVal;
		return true;
	}
	else
	{
		return false;
	}
}

int CFeedManagerLibHelper::MarkNewsItemUnread(LONG_PTR itemId, CppSQLite3DB& db)
{
	// update
	CppSQLite3Statement stmt = db.compileStatement(
		_T("update news_item set readtime = null where news_id = ? and readtime is not null"));
	stmt.bind(1, itemId);
	return stmt.execDML();
}

void CFeedManagerLibHelper::UpdateNewsFeed(CNewsFeed* nf, CppSQLite3DB& db)
{
	// update
	CppSQLite3Statement stmt = db.compileStatement(
		_T("update feed set ")
		_T("feed_group_id = ?,")	// 1
		_T("url = ?,")				// 2
		_T("name = ?,")				// 3
		_T("description = ?,")		// 4
		_T("website = ?,")			// 5
		_T("image = ?,")			// 6
		_T("language = ?,")			// 7
		_T("disabled = ?,")			// 8
		_T("last_updated = ?,")		// 9
		_T("options1 = ?,")			// 10
		_T("hd_last_mod = ?,")		// 11
		_T("hd_etag = ?,")			// 12
		_T("update_freq = ?,")		// 13
		_T("store_dur = ?,")		// 14
		_T("options3 = ?,")			// 15
		_T("notify_newitem = ?,")	// 16
		_T("limit_item = ?,")		// 17
		_T("limit_item_number = ?,")// 18
		_T("keep_inactive = ?,")	// 19
		_T("contentCode = ?,")		// 20
		_T("use_login = ?,")		// 21
		_T("login_name = ?,")		// 22
		_T("login_pwd = ? ")		// 23
		_T("where feed_id = ?"));
	stmt.bind(1,nf->m_groupID);
	stmt.bind(2,(LPCTSTR)nf->m_url);
	stmt.bind(3,(LPCTSTR)nf->m_title);
	stmt.bind(4,(LPCTSTR)nf->m_description);
	stmt.bind(5,(LPCTSTR)nf->m_website);
	stmt.bind(6,(LPCTSTR)nf->m_image);
	stmt.bind(7,(LPCTSTR)nf->m_language);
	stmt.bind(8,nf->m_bDisabled ? 1 : 0);
	stmt.bind(9,(int)nf->m_lastModified.GetTime());
	if(nf->m_bloglineSubId)
	{
		stmt.bind(10,(int)nf->m_bloglineSubId);
	}
	else
	{
		stmt.bindNull(10);
	}
	stmt.bind(11,(LPCTSTR)nf->m_httpLastModified);
	stmt.bind(12,(LPCTSTR)nf->m_httpETag);
	stmt.bind(13,nf->m_updateFreq);
	stmt.bind(14,nf->m_cleanupMaxAge);
	stmt.bind(15,nf->m_channelStyle);
	stmt.bind(16,nf->m_notifyNewItem);
	stmt.bind(17,nf->m_bLimitItems ? 1 : 0);
	stmt.bind(18,nf->m_maxNumOfItems);
	stmt.bind(19,nf->m_bKeepInactive? 1 : 0);
	stmt.bind(20,nf->m_nContentCRC);
	stmt.bind(21,nf->m_bUseLogin ? 1 : 0);
	stmt.bind(22,(LPCTSTR)nf->m_loginName);
	stmt.bind(23,(LPCTSTR)nf->m_loginPassword);
	stmt.bind(24,nf->m_id);

	stmt.execDML();
}

void CFeedManagerLibHelper::SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts, LPCTSTR tableName, LPCTSTR idName)
{
	if(counts.empty())
		return;

	CString sql;
	sql.Format(_T("update %s set unread_count = ? where %s = ?;"),tableName, idName);

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	db.execDML(_T("begin immediate transaction;"));
	CppSQLite3Statement stmt = db.compileStatement(sql);
	for(std::vector<std::pair<ULONG_PTR,INT_PTR> >::iterator it = counts.begin(); it != counts.end(); ++it)
	{
		std::pair<ULONG_PTR,INT_PTR>& values = *it;
		stmt.bind(1, values.second);
		stmt.bind(2, (unsigned int)values.first);
		stmt.execDML();
		stmt.reset();
	}
	stmt.finalize();
	db.execDML(_T("commit transaction;"));
	db.close();

}
