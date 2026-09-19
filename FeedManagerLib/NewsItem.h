#pragma once

#include <vector>
#include <atltime.h>
#include "loki\SmartPtr.h"
#include "ContentGenerator.h"

#import "msxml3.dll"

class CNewsItem;
typedef Loki::SmartPtr<CNewsItem, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> NewsItemPtr;
typedef std::vector<NewsItemPtr> NewsItemVector;

class CNewsItem : public CContentGenerator
{
public:
	enum Color
	{
		DefaultColor = RGB(111,111,111)
	};
	enum Features
	{
		LabelThis = 0x1,
		EmailThis = 0x2,
		Delicious = 0x4,
		Furl = 0x8,
		Arthur = 0x10,
		BlogThis = 0x20,
		HighlightNewsWatch = 0x40,
		TrackComments = 0x80
	};
	static void SetItemFeatures(long selectedFeatures, bool bHighlightNewsWatch);

	CNewsItem(void);
	virtual ~CNewsItem(void);
	CNewsItem* Clone() const;
	void Init(ULONG_PTR id);

public: // from CContentGenerator
	void InitGenerator(int nPageSize);
	size_t GetNumOfItems() {return 1;}
	CString GeneratePageHTML(int nPage);
	CString GeneratePageContentHTML(int nPage);
	CString GetContentID() const;
	CString GetHomeURL() const;
	CString GetStyle() const;
	CString GetTitle() const;
	CString GetUnescapedTitle() const;

	// for html construction
	CString GetLabelThisUrl() const;
	LPCTSTR GetLabelThisUrlClass() const;
	CString GetTrackCommentUrl() const;
	LPCTSTR GetTrackCommentText() const;
	CString GetDivID() const;
	
	size_t GetItemTags(std::vector<ULONG_PTR>& tagIDs);
	void AddTag(ULONG_PTR tagID);
	void RemoveTag(ULONG_PTR tagID);

public:
	CString GetLink() const;

	CString internalGenerateHTML(bool bSingleItem);
	void ClearWatch();
	bool WatchStatusChecked();
	bool UpdateWatchColor();
	size_t LoadWatchIDs(std::vector<ULONG_PTR>& watches);
	static void BatchLoadWatchIDs(NewsItemVector& newsItems);
	COLORREF GetTextColor() { return m_textColor;}
	COLORREF GetBkColor() {return m_bkColor;}

	bool IsUnread();
	void SetRead();
	void SetUnread();
	void Save();
	void Delete();
	bool RemoveDangerousTags();
	void SetTrackComment(bool bTrack);

	void SetReadFlag(bool bRead=true) { m_notRead = bRead ? 0 : 1; }
	void UnescapeTitle();
	void Compress();
	void CalcContentCRC();
	void CleanCompress();
	void DecompressDescription(const BYTE* pStream, int size);
	BYTE* GetCompressStream() {return m_pCompressStream;}
	int GetCompressLen() {return m_compressSize;}

	void ExportRss(MSXML2::IXMLDOMElementPtr& spItem);
	MSXML2::IXMLDOMDocumentPtr GenerateFeedXml();

	static size_t GetNewsItemsByID(NewsItemVector& items, const std::vector<ULONG_PTR>& IDs);
	static void MarkItemsRead(std::vector<ULONG_PTR>& m_unreadIDs);
	static void MarkAllAsRead();
	static int GetNumOfAllItems();

public:
	LONG_PTR m_id;
	LONG_PTR m_feedID;
	CString m_url;
	CString m_title;
	CString m_description;
	CString m_guid;
	CString m_author;
	CString m_commentsURL;
	CString m_commentFeedURL;
	CString m_podCastingURL;
	CTime m_date;
	int m_marked;
	bool m_bTrackComments;
	std::vector<ULONG_PTR> m_vectWatches;
	unsigned int m_contentCRC;
	int m_notRead;

protected:
	BYTE* m_pCompressStream;
	int m_compressSize;

	bool m_bWatchChecked;
	COLORREF m_textColor;
	COLORREF m_bkColor;
	bool CalculateColors();

	static long m_selectedFeatures;

};


