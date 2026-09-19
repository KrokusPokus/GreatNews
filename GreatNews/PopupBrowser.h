#pragma once

#include <set>
#include "Browser.h"

class CPopupBrowserAdvisor
{
public:
	virtual void OnPopBeforeNav(const CString& szURL)=0;
};
				
class CPopupBrowser : public CWindowImpl<CPopupBrowser, CAxWindow>,
					 public CWebBrowser2<CPopupBrowser>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CAxWindow::GetWndClassName())
	CPopupBrowser(CPopupBrowserAdvisor& advisor) : m_advisor(advisor) {};

	BEGIN_MSG_MAP(CPopupBrowser)
		CHAIN_MSG_MAP(CWebBrowser2<CPopupBrowser>)
	END_MSG_MAP()

	BOOL OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders)
	{
		m_advisor.OnPopBeforeNav(szURL);

		return TRUE; // block it
	}

private:
	CPopupBrowserAdvisor& m_advisor;
};

