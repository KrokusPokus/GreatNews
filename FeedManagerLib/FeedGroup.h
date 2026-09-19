#pragma once

#include <vector>
#include "loki\SmartPtr.h"
#include "NewsFeed.h"

class CFeedGroup;
typedef Loki::SmartPtr<CFeedGroup, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> FeedGroupPtr;
typedef std::vector<FeedGroupPtr> FeedGroupVector;

class CFeedGroup : public CBatchContentGenerator
{
public:
	enum SortOption
	{
		SortNone,
		SortByName
	};

	CFeedGroup(void);
	CFeedGroup(int id, LPCTSTR name, LPCTSTR desc);
	CFeedGroup(const CFeedGroup& f);
	virtual ~CFeedGroup(void);

	void Init(ULONG_PTR id);

public: // from CNewsSource
	int RetrieveNewsToDB();
	size_t GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs,CNewsFilter* pNewsFilter) const;
	CString GetNewsSourceName() const;
	int CheckUnread();
	void Delete();
	void Save();
	void Rename(LPCTSTR newName);
	void MarkRead();
	void MarkUnread();
	void SaveToXml(MSXML2::IXMLDOMElementPtr spElement);
	void LoadFromXml(MSXML2::IXMLDOMElementPtr spElement);
	bool IsRootGroup();

	static void SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts);
	static void SaveTreeState(std::vector<std::pair<ULONG_PTR,long> >& states);

public: // from CContentGenerator
	CString GeneratePageHTML(int nPage);
	CString GeneratePageContentHTML(int nPage);
	CString GetContentID() const;
	CString GetHomeURL() const;
	virtual CString GetTitle() const { return m_name;}

	void UpdateUsage();

	void ExportRss(MSXML2::IXMLDOMElementPtr& spChannel);

public:
	static FeedGroupPtr GetRootFeedGroup(void);
	size_t GetChildGroups(FeedGroupVector& groups, SortOption sortOption = SortNone) const;
	size_t GetNewsFeeds(NewsFeedVector& newsfeeds, bool bIncludeDisabled, SortOption sortOption = SortNone) const;
	static ULONG_PTR GetIdFromName(LPCTSTR name);

	int GetNumOfNewsItems(bool bUnreadOnly=false) const;
	int GetNumOfNewsFeeds() const;
	int GetNumOfLabelled() const;

public:
	ULONG_PTR m_id;
	ULONG_PTR m_parentID;
	CString m_name;
	CString m_desc;
	CString m_website;
	int m_unreadCount;
	bool m_bDisabled;
	bool m_bExpanded; // is the group expanded on UI channel tree?
};

inline bool LessGroup(const FeedGroupPtr& group1, const FeedGroupPtr& group2)
{
	return (group1->m_name.Compare(group2->m_name)<0);
};