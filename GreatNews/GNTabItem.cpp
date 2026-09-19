#include "StdAfx.h"
#include <atlcoll.h>
#include "resource.h"
#include "GNTabItem.h"
#include "DotNetTabCtrl.h"

CGNTabItem::CGNTabItem(void) :
	m_pForwardBrowserAdvisor(NULL)
{
}

CGNTabItem::~CGNTabItem(void)
{
}

void CGNTabItem::SetForwardAdvisor(CFeedBrowserAdvisor* pAdvisor)
{
	m_pForwardBrowserAdvisor = pAdvisor;
}

void CGNTabItem::ShowBrowser(int cmd)
{
	if(m_browser && ::IsWindow(m_browser->m_hWnd))
	{
		m_browser->ShowWindow(cmd);
	}
}

void CGNTabItem::RefreshTab()
{
	if(::IsWindow(m_hwndNotify))
	{
		::PostMessage(m_hwndNotify, WM_COMMAND, CMD_ID_REFRESHTAB,0L);
	}
}

void CGNTabItem::OnUrl(const CString& url)
{
	m_currentURL = url;

	if(m_pForwardBrowserAdvisor)
		m_pForwardBrowserAdvisor->OnUrl(url);
}

void CGNTabItem::OnStatusTextChange(const CString& szText)
{
	if(m_pForwardBrowserAdvisor)
		m_pForwardBrowserAdvisor->OnStatusTextChange(szText);
}

void CGNTabItem::OnTitleChange(const CString& szText)
{
	SetText(szText);
	RefreshTab();
}

void CGNTabItem::OnFoundFeed(size_t nNumOfFeeds)
{
	if(m_pForwardBrowserAdvisor)
		m_pForwardBrowserAdvisor->OnFoundFeed(nNumOfFeeds);
}

BOOL CGNTabItem::OnNewWindow2(IDispatch** ppDisp)
{
	if(m_pForwardBrowserAdvisor)
		return m_pForwardBrowserAdvisor->OnNewWindow2(ppDisp);
	else
		return TRUE; // cancel it
}

BOOL CGNTabItem::OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
	if(m_pForwardBrowserAdvisor)
		return m_pForwardBrowserAdvisor->OnNewWindow3(ppDisp, dwFlags, bstrUrlContext, bstrUrl);
	else
		return TRUE; // cancel it
}

BOOL CGNTabItem::OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders)
{
	if(m_pForwardBrowserAdvisor)
		return m_pForwardBrowserAdvisor->OnBeforeNavigate2(pDisp, szURL, dwFlags,szTargetFrameName, pPostedData, szHeaders);
	else
		return FALSE; // allow
}

void CGNTabItem::OnMouseDown(long button, bool ctrl, bool bUserClickedLink, const CString& url)
{
	if(m_pForwardBrowserAdvisor)
		m_pForwardBrowserAdvisor->OnMouseDown(button, ctrl, bUserClickedLink, url);
}

void CGNTabItem::OnProgressChange(long nProgress, long nProgressMax)
{
	if(nProgress < 0)
		return ;

	int imgIndex = ( (nProgress == nProgressMax || nProgressMax == 0) ? -1  : nProgress/(nProgressMax/4));
	if(GetImageIndex() != imgIndex)
	{
		SetImageIndex(imgIndex);
		RefreshTab();
	}
}

void CGNTabItem::SetContentGenerator(ContentGeneratorPtr gen)
{
	m_contentGenerator = gen;
}

ContentGeneratorPtr CGNTabItem::GetContentGenerator()
{
	return m_contentGenerator;
}

