#pragma once

#include "atlgdix.h"
#include "CustomTabCtrl.h"
#include "FeedBrowserAdvisor.h"
#include "ContentGenerator.h"
#include "FeedBrowser.h"

class CGNTabItem : public CCustomTabItem, 
				   public CFeedBrowserAdvisor
{
public:
	CGNTabItem();
	virtual ~CGNTabItem();

	FeedBrowserPtr m_browser;
	HWND m_hwndNotify;
	CString m_currentURL;

public: // from CFeedBrowserAdvisor
	virtual void OnUrl(const CString& url);
	virtual void OnStatusTextChange(const CString& szText);
	virtual void OnTitleChange(const CString& szText);
	virtual void OnFoundFeed(size_t nNumOfFeeds);
	virtual BOOL OnNewWindow2(IDispatch** ppDisp);
	virtual BOOL OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
	virtual BOOL OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders);
	virtual void OnMouseDown(long button, bool ctrl, bool bUserClickedLink, const CString& url);
	virtual void OnProgressChange(long nProgress, long nProgressMax);

public:
	void SetContentGenerator(ContentGeneratorPtr gen);
	ContentGeneratorPtr GetContentGenerator();
	void SetForwardAdvisor(CFeedBrowserAdvisor* pAdvisor);
	void ShowBrowser(int cmd=SW_SHOW);

protected:
	void RefreshTab();

protected:
	CFeedBrowserAdvisor* m_pForwardBrowserAdvisor;
	ContentGeneratorPtr m_contentGenerator;
};
