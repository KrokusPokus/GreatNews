#pragma once

#include <vector>
#include "loki\SmartPtr.h"
#include "NewsItem.h"
#include "NewsSource.h"
#include "BatchContentGenerator.h"
#include "DownloadContext.h"

#import "MSXML3.dll"

class CNewsFeed;
typedef Loki::SmartPtr<CNewsFeed, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> NewsFeedPtr;
typedef std::vector<NewsFeedPtr> NewsFeedVector;

#define MM_NEWITEM_ARRIVED		(WM_USER+500)


class CNewsFeed : public CBatchContentGenerator
{
public:
	enum CopyLevel
	{
		Web,
		User,
		All
	};
	enum SortOption
	{
		SortNone,
		SortById,
		SortByTitle
	};
	enum Statistics
	{
		Most10Visited,
		Least10Visited,
		MostActive,
		LeastActive
	};

	static TCHAR* ChannelCleanupMaxAgeString[];
	static TCHAR* ChannelAutoUpdateString[];
	static int GetChannelAutoUpdateFreqSec(INT_PTR nIndex);

	static bool m_bMarkChangesUnread;

	CNewsFeed(void);
	virtual ~CNewsFeed(void);
	void Init(ULONG_PTR id);
	void CopyProperties(const CNewsFeed& feed, CopyLevel level=Web);
	void UpdateProperties(const CNewsFeed& feed);
	CString GetProcessedUrl();
	bool IsSearchChannel() const;
	bool IsCommentChannel() const;
	bool IsBloglinesSyncChannel() const;
	void DetachFromBloglines();
	bool SkipUpdate() const;
	bool UpdateDue(int seconds) const;
	void NormalizeHomeUrl();

public: // Asynchnorous update support
	void StartDownloading(HANDLE semReadyToStore);
	bool IsDownloadReady();
	int StoreUpdate();
	void ClearUpdate();
	void AbortUpdate();
	void CheckTimeout(CTimeSpan& span);

public: // Asynchnorous update support
	CString m_updateErrMsg;
	CTime m_updateStartTime;
	bool m_bBypassCache;

public:
	DownloadContextPtr m_downloadContext;

public: // from CNewsSource
	int RetrieveNewsToDB(NewsItemVector& newsItems, const CNewsFeed& feedWithInternetProperties);
	int RetrieveNewsToDB();
	size_t GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, CNewsFilter* pNewsFilter) const;
	CString GetNewsSourceName() const;
	int CheckUnread();
	void Rename(LPCTSTR newName);
	void Delete();
	void Save();
	void MarkRead();
	void MarkUnread();
	void SaveToXml(MSXML2::IXMLDOMElementPtr spElement);
	void LoadFromXml(MSXML2::IXMLDOMElementPtr spElement);
	size_t GetNewsItems(NewsItemVector& items);
	void Disable(bool bDisable=true);
	static void SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts);

	int GetNumOfNewsItems(bool bUnreadOnly=false) const;
	int GetNumOfLabelled() const;

public: // from CContentGenerator
	CString GeneratePageHTML(int nPage);
	CString GetContentID() const;
	CString GetHomeURL() const;
	virtual CString GetStyle() const { return m_channelStyle; }
	virtual CString GetTitle() const { return m_title;}

	CString GenerateSectionHTML();
	void UpdateUsage();
	void ExportRss(MSXML2::IXMLDOMElementPtr& spChannel);
	void KeepEvenInactive();
	void DeleteOrDisable();

public:
	static NewsFeedPtr AddCommentFeed(NewsItemPtr& pItem);
	static long GetNewsFeedCount();
	static size_t GetAllNewsFeeds(NewsFeedVector& feeds, bool bIncludeDisabled, SortOption sortOption=SortNone);
	static size_t GetNewsFeeds(const std::vector<ULONG_PTR>& feedIDs, NewsFeedVector& feeds);
	static size_t GetStatistics(NewsFeedVector& feeds, Statistics statType, int numOfChannels, bool bStatsExcludeDisabled);
	static size_t GetActivityStatistics(NewsFeedVector& feeds, Statistics statType, int numOfDays, int numOfChannels, bool bStatsExcludeDisabled);
	static ULONG_PTR GetIdFromXmlUrl(LPCTSTR url);
	static void CountUnread(std::map<ULONG_PTR, int>& mapUnread);


	// CString GetNewsText();
	MSXML2::IXMLDOMDocument2Ptr GetNewsDom();
	void Cleanup(int days, bool bDeleteUnread, bool bDeleteMarked, bool bLimitItems, int numOfItemsToKeep);

public: 
	// feed properties
	LONG_PTR m_id;
	LONG_PTR m_groupID;
	CString m_url;
	CString m_title;
	CString m_description;
	CString m_website;
	CString m_format;
	CString m_image;
	CString m_language;
	CString m_author;
	bool m_bDisabled;
	UINT m_nContentCRC;
	CTime m_lastChecked;  // last time we check the feed
	CTime m_lastModified; // last time the feed was modified. Got from the feed itself
	long m_usage; // hitting score
	long m_ttl;// how long the update is valid
	long m_bloglineSubId;
	long m_unreadCount;
	CString m_httpLastModified;
	CString m_httpETag;
	INT_PTR m_updateFreq;
	INT_PTR m_cleanupMaxAge;
	CString m_channelStyle;
	int m_notifyNewItem;
	LONG_PTR m_parent_feed_id;
	LONG_PTR m_parent_item_id;
	bool m_bUseLogin;
	CString m_loginName;
	CString m_loginPassword;

	bool m_bLimitItems;
	int m_maxNumOfItems;
	bool m_bKeepInactive;

	static HWND m_notifyWnd;
	
	// for statistics
	int m_numOfNewItemsInDays;

private:
	static int ChannelCleanupMaxAgeDays[];
	static int ChannelAutoUpdateSec[];
	CTime m_initTime; // the time when this object is created
};

inline bool LessFeed(const NewsFeedPtr& feed1, const NewsFeedPtr& feed2)
{
	return (feed1->m_title.CompareNoCase(feed2->m_title)<0);
};
