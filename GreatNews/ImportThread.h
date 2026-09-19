#pragma once

#include "NGMTsync.h"
#include "GNThread.h"
#include "FeedManagerLib.h"
#include "FeedTreeItem.h"

#define MM_IMPORT_FINISHED	(WM_APP + 300)
#define MM_IMPORT_STATUS	(WM_APP + 301)

class CImportThread :
	public CGNThread
{
public:
	CImportThread(void);
	virtual ~CImportThread(void);

	virtual DWORD Run();
	void Clear();

public:
	bool m_importWatchLabel;
	bool m_bUpdateAfterImport;
	bool m_bStop;
	HWND m_hwndToNotify;
	CNGEvent m_stopEvent;
	MSXML2::IXMLDOMDocumentPtr m_spDoc;

public:
	ULONG_PTR m_nDefaultGroup;
	HWND m_hwndTree;
	int m_cntGroupImported;
	int m_cntGroupSkipped;
	int m_cntGroupFailed;
	int m_cntFeedImported;
	int m_cntFeedSkipped;
	int m_cntFeedFailed;

private:
	void ImportFeedGroup(FeedGroupPtr group);
	void ImportNewsFeed(NewsFeedPtr feed);
	void PostStatusMessage(const CString& text);
	BOOL GetCheckState(HTREEITEM hItem) const;
	CFeedTreeItem* GetFeedTreeItem(HTREEITEM hItem);
};
