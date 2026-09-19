#pragma once

#include "FeedGroup.h"
#include "NewsFeed.h"
#include "FeedManager.h"
#include "NewsWatch.h"

class CFeedManagerLibHelper
{
public:
	enum TransactionSettings
	{
		NoTransaction = false,
		UseTransaction = true
	};
	
	static LPCTSTR m_newsItemFields;
	static LPCTSTR m_newsFeedFields;
	static LPCTSTR m_newsWatchFields;
	static LPCTSTR m_feedGroupFields;

	static void PopulateNewsFeed(CNewsFeed* nf, CppSQLite3Query& q);
	static void PopulateNewsItem(CNewsItem* ni, CppSQLite3Query& q);
	static void PopulateNewsWatch(CNewsWatch* nw, CppSQLite3Query& q);
	static void PopulateFeedGroup(CFeedGroup* fg, CppSQLite3Query& q);

	static size_t RetrieveNewsFromDB(NewsItemVector& newsItems, const CString& criteria, CNewsFilter* pNewsFilter, LPCTSTR orderBy);
	static size_t GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, const CString& criteria, CNewsFilter* pNewsFilter,  LPCTSTR orderBy);

	static NewsItemPtr GetExistingNewsItem(NewsItemPtr& item, bool bItemHasValidGuids, CppSQLite3DB& db);

	static void DBExec(LPCTSTR sql, TransactionSettings useTrx=NoTransaction);

	static void UpdateNewsFeed(CNewsFeed* nf, CppSQLite3DB& db);
	static int MarkNewsItemUnread(LONG_PTR itemId, CppSQLite3DB& db);
  static void SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts, LPCTSTR tableName, LPCTSTR idName);

public:
	static bool GetTitle(MSXML2::IXMLDOMElementPtr spElement, CString& title);
	static bool SafeGetAttr(MSXML2::IXMLDOMElementPtr spElement, LPCTSTR attr, CString& value);
	template <typename T>
	static bool SafeGetAttr(MSXML2::IXMLDOMElementPtr spElement, LPCTSTR attr, T& value)
	{
		_variant_t v = spElement->getAttribute(attr);
		if(v.vt == VT_NULL || v.vt == VT_EMPTY)
		{
			return false;
		}
		else
		{
			// try _variant_t extractor
			value = v;
			return true;
		}
	}
};
