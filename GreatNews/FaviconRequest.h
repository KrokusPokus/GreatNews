#pragma once

#include <map>
#include "NGMTSync.h"
#include "Request.h"
#include "NewsFeed.h"

#define MM_FAVICONREADY		WM_USER + 300

class CFaviconRequest :
	public CRequest
{
public:
	CFaviconRequest();
	virtual ~CFaviconRequest(void);

	virtual HRESULT HandleRequest();

	void AddFeed(NewsFeedPtr& feed);
	void SetNotifyWnd(HWND wndToNotify);
	void SignalShutdown();

private:
	bool DownloadFavicon(ULONG_PTR feedID, const CString& strUrl);

	HWND m_wndToNotify;

	CComAutoCriticalSection m_csToBeUpdated;
	CNGSemaphore m_semHasRequest;
	std::map<LONG_PTR, CString> m_feedToCheck;
};
