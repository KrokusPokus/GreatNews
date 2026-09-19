#pragma once

#include <set>
#include "Browser.h"

			
class CDetectFeedBrowser : public CWindowImpl<CDetectFeedBrowser, CAxWindow>,
					 public CWebBrowser2<CDetectFeedBrowser>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CAxWindow::GetWndClassName())
	CDetectFeedBrowser(HANDLE hEvent) : m_hFinished(hEvent) {};

	BEGIN_MSG_MAP(CDetectFeedBrowser)
		CHAIN_MSG_MAP(CWebBrowser2<CDetectFeedBrowser>)
	END_MSG_MAP()

	BOOL OnNewWindow2(IDispatch** ppDisp)
	{
		return TRUE;
	}
	BOOL OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
	{
		return TRUE;
	}

	void OnProgressChange(long nProgress, long nProgressMax)
	{
		if(nProgress == nProgressMax)
		{
			::SetEvent(m_hFinished);
		}
	}
	//void OnNavigateComplete2(IDispatch* pDisp, const CString& szURL)
	//{
	//	::SetEvent(m_hFinished);
	//}

public:
	HANDLE m_hFinished;
};

