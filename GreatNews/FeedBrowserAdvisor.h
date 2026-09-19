#pragma once

class CFeedBrowserAdvisor
{
public:
	virtual void OnUrl(const CString& url)=0;
	virtual void OnStatusTextChange(const CString& szText)=0;
	virtual void OnTitleChange(const CString& szText)=0;
	virtual void OnFoundFeed(size_t nNumOfFeeds)=0;
	virtual BOOL OnNewWindow2(IDispatch** ppDisp)=0;
	virtual BOOL OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)=0;
	virtual BOOL OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders)=0;
	virtual void OnMouseDown(long button, bool ctrl, bool bUserClickedLink, const CString& url)=0;
	virtual void OnProgressChange(long nProgress, long nProgressMax)=0;
};