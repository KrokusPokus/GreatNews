#include "StdAfx.h"
#include "FaviconRequest.h"
#include "AsyncDownloadManager.h"
#include "GreatNewsConfig.h"
#include "atlwfile.h"

CFaviconRequest::CFaviconRequest() :
	m_semHasRequest(0,65536), m_wndToNotify(NULL)
{
	
}

void CFaviconRequest::SetNotifyWnd(HWND wndToNotify)
{
	m_wndToNotify = wndToNotify;
}

void CFaviconRequest::AddFeed(NewsFeedPtr& feed)
{
	CComCritSecLock<CComAutoCriticalSection> lockToBeUpdated(m_csToBeUpdated);
	m_feedToCheck[feed->m_id] = (!feed->m_website.IsEmpty()? feed->m_website : feed->m_url);
  if(!m_semHasRequest.Unlock())
	{
		ATLASSERT(FALSE);
	}
}

void CFaviconRequest::SignalShutdown()
{
	CComCritSecLock<CComAutoCriticalSection> lockToBeUpdated(m_csToBeUpdated);
	m_feedToCheck.clear();
	m_feedToCheck[-1] = _T(""); // special ID to tell the thread to shutdown
    m_semHasRequest.Unlock();
}

CFaviconRequest::~CFaviconRequest(void)
{
}

HRESULT CFaviconRequest::HandleRequest()
{
	while(m_semHasRequest.Lock())
	{
		LONG_PTR id;
		CString url;

		CComCritSecLock<CComAutoCriticalSection> lockToBeUpdated(m_csToBeUpdated);
		ATLASSERT(!m_feedToCheck.empty());
		id = m_feedToCheck.begin()->first;
		url = m_feedToCheck.begin()->second;
		m_feedToCheck.erase(m_feedToCheck.begin());
		lockToBeUpdated.Unlock();

		if(id < 0)
		{
			break; // stop processing
		}

		if(DownloadFavicon(id, url) && ::IsWindow(m_wndToNotify))
		{
			::PostMessage(m_wndToNotify, MM_FAVICONREADY, id, 0);
		}
	}

	return CRequest::HandleRequest();
}

bool CFaviconRequest::DownloadFavicon(ULONG_PTR feedID, const CString& strUrl)
{
	try
	{
		// use html url if it's not empty
		CUrl url;
		if(!url.CrackUrl(strUrl))
			return false;

		CString faviconFile = g_GreatNewsConfig.GetFaviconDir();
		faviconFile.AppendFormat(_T("\\F%d.ico"), feedID);
		if(CFile::FileExists(faviconFile))
			return true;

		CString faviconUrl = url.GetSchemeName();
		faviconUrl += _T("://");
		faviconUrl += url.GetHostName();
		if(url.GetPortNumber() != 80)
			faviconUrl.AppendFormat(_T(":%d"), url.GetPortNumber());
		faviconUrl += _T("/favicon.ico");
		if(CGNSingleton<CAsyncDownloadManager>::Instance()->DownloadToFile(faviconUrl,faviconFile))
			return true;

	}
	catch(...)
	{
	}

	return false;
}
