#pragma once

#include <vector>
#include "loki\SmartPtr.h"
#include "NewsItem.h"
#include "NewsFeed.h"
#include "NewsSource.h"
#include "BatchContentGenerator.h"

#define WATCH_TITLE		0x0001
#define WATCH_CONTENT	0x0002
#define WATCH_AUTHOR	0x0004
#define WATCH_URL		0x0008

class CNewsWatch;
typedef Loki::SmartPtr<CNewsWatch, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> NewsWatchPtr;
typedef std::vector<NewsWatchPtr> NewsWatchVector;
typedef	bool (*SearchFunc)(LPCTSTR buffer, LPCTSTR keyword);

class CNewsWatch : public CBatchContentGenerator
{
public:
	enum MatchType
	{
		MatchAny = 0,
		MatchAll = 1,
		MatchExact = 2
	};
	enum SortOptions
	{
		SortNone,
		SortByName,
		SortByDisplayOrder
	};
	enum Action
	{
		Hightlight = 0,
		DeleteItem = 1,
		MarkReadDirectly = 2
	};

	CNewsWatch(void);
	virtual ~CNewsWatch(void);
	void Init(ULONG_PTR id);

public: // from CNewsSource
	int RetrieveNewsToDB();
	size_t GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, CNewsFilter* pNewsFilter) const;
	CString GetNewsSourceName() const;
	int CheckUnread();
	void Rename(LPCTSTR newName);
	void Delete();
	void Save();
	void MarkRead();
	void MarkUnread();
	static void SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts);

	void SaveToXml(MSXML2::IXMLDOMElementPtr spElement);
	void LoadFromXml(MSXML2::IXMLDOMElementPtr spElement);

public: // from CContentGenerator
	CString GeneratePageHTML(int nPage);
	CString GetContentID() const;
	CString GetHomeURL() const;
	CString GetStyle() const;
	virtual CString GetTitle() const { return m_title;}

public:
	void CheckExistingItems();
	void DeleteMatchedItems();
	size_t GetWatchChannels(NewsFeedVector& newsFeeds);
	void Split();
	void SetupSearchFunc();
	bool Match(NewsItemPtr newsItem);
	bool Match(const CString& str);
	std::vector<ULONG_PTR>& LoadWatchChannels();
	std::vector<ULONG_PTR>& GetWatchChannels() {return m_WatchChannelIDs;}
	void AssignChannelSelection(NewsFeedVector& feeds);

	static size_t GetAllNewsWatches(NewsWatchVector& watches, SortOptions sortOptions = SortNone);
	static size_t GetNewsWatchesByChannel(NewsWatchVector& newsWatches, ULONG_PTR feedID);
	static void SetOrder(std::vector<ULONG_PTR>& watchIDs);
	static void CountUnread(std::map<ULONG_PTR, int>& mapUnread);

public:
	UINT_PTR m_id;
	CString m_title;
	CString m_matchCriteria;
	int m_bWatchSelectiveChannels;
	long m_watchFlag;
	int m_allWords;
	int m_matchCase;
	int m_wholeWord;
	COLORREF m_txtColor;
	COLORREF m_bkColor;
	int m_order;
	int m_nAction;
	int m_unreadCount;
	CString m_watchStyle;
	bool m_bDisabled;

private:
	SearchFunc m_pSearchFunc;
	std::vector<CString> m_vectWords;

	std::vector<ULONG_PTR> m_WatchChannelIDs;
};

