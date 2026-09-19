#pragma once

#include "BatchContentGenerator.h"

#import <msxml3.dll>

class CTag;
typedef Loki::SmartPtr<CTag, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> TagPtr;
typedef std::vector<TagPtr> TagVector;

class CTag : public CBatchContentGenerator
{
public:
	CTag(void);
	virtual ~CTag(void);

public: // from CNewsSource
	int RetrieveNewsToDB();
	size_t GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, CNewsFilter* pNewsFilter) const;
	CString GetNewsSourceName() const;
	int CheckUnread();
	void Delete();
	void Save();
	void Rename(LPCTSTR newName);
	void MarkRead();
	void MarkUnread();
	void UnlabelAll();

	void SaveToXml(MSXML2::IXMLDOMElementPtr spElement);
	void LoadFromXml(MSXML2::IXMLDOMElementPtr spElement);

public: // from CContentGenerator
	CString GeneratePageHTML(int nPage);
	CString GetContentID() const;
	CString GetHomeURL() const;
	CString GetStyle() const;
	virtual CString GetTitle() const { return m_name;}

public:
	static size_t GetAllTags(TagVector& tags);
	static void CountUnread(std::map<ULONG_PTR, int>& mapUnread);
	static void CountTotal(std::map<ULONG_PTR, int>& mapUnread);
	static void SaveUnreadCount(std::vector<std::pair<ULONG_PTR,INT_PTR> >& counts);

	int GetNumOfItemsWithLabel(); // return num of items with this tag
public:
	UINT_PTR m_id;
	CString m_name;
	int m_unreadCount;
	CString m_tagStyle;
};
