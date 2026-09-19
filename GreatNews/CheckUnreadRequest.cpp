#include "StdAfx.h"
#include "CheckUnreadRequest.h"

CCheckUnreadRequest::CCheckUnreadRequest(HWND notifyWnd, CheckUnreadLevel level)
: m_hwndToNotify(notifyWnd), m_level(level)
{
}

CCheckUnreadRequest::~CCheckUnreadRequest(void)
{
}


void CCheckUnreadRequest::NotifyWnd::operator()(const std::pair<ULONG_PTR, int>& t)
{
	CCheckUnreadResult* pResult = 
		new CCheckUnreadResult(m_type, t.first, t.second);

	::PostMessage(m_hwndToNotify, MM_SETUNREAD,0, (LPARAM)pResult);
}

HRESULT CCheckUnreadRequest::HandleRequest()
{
	//::Sleep(500);

	try
	{
		//
		//
		//
		if(m_level & CheckChannel)
		{
			std::map<ULONG_PTR, int> mapUnreadCount;
			CNewsFeed::CountUnread(mapUnreadCount);
			if(::IsWindow(m_hwndToNotify))
			{
				NotifyWnd oNotify(m_hwndToNotify, CFeedTreeItem::Channel);
				for_each(mapUnreadCount.begin(), mapUnreadCount.end(), oNotify);
			}
		}

		//
		//
		//
		if(m_level & CheckWatch)
		{
			std::map<ULONG_PTR, int> mapUnreadCount;
			CNewsWatch::CountUnread(mapUnreadCount);
			if(::IsWindow(m_hwndToNotify))
			{
				NotifyWnd oNotify(m_hwndToNotify, CFeedTreeItem::Watch);
				for_each(mapUnreadCount.begin(), mapUnreadCount.end(), oNotify);
			}
		}


		//
		//
		//
		if(m_level & CheckLabel)
		{
			std::map<ULONG_PTR, int> mapUnreadCount;
			CTag::CountTotal(mapUnreadCount);  // label node show total number of items
			if(::IsWindow(m_hwndToNotify))
			{
				NotifyWnd oNotify(m_hwndToNotify, CFeedTreeItem::Tag);
				for_each(mapUnreadCount.begin(), mapUnreadCount.end(), oNotify);
			}
		}
	}
	catch(...)
	{
		AtlTrace(_T("CheckUnreadRequest failed...\n"));
	}

	return CRequest::HandleRequest();
}