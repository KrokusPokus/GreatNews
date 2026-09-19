#pragma once

#include "GNThread.h"
#include "FeedManagerLib.h"
#include "NGMTsync.h"
#include <strsafe.h>

#define MM_GLOBALSEARCH_ITEMFOUND	(WM_USER+330)
#define MM_GLOBALSEARCH_PROGRESS	(WM_USER+331)
#define MM_GLOBALSEARCH_DONE		(WM_USER+332)

class CGlobalSearchThread :
	public CGNThread
{
public:

	CGlobalSearchThread(void) : CGNThread(false),
		m_wndToNotify(NULL), m_currentFeedIdx(0), m_currentItemIdx(0), m_bStop(false)
	{
	}

	virtual ~CGlobalSearchThread(void)
	{
		Reset();
	}

	virtual DWORD Run()
	{
		try
		{
			m_bStop = false;
			m_stopEvent.ResetEvent();

			// load all feeds if we haven't
			if(m_allFeeds.size() == 0)
			{
				CComCritSecLock<CComAutoCriticalSection> g(m_lock);
				CNewsFeedCache::GetAllNewsFeeds(m_allFeeds);
			}
			
			// start search
			Search();
		}
		catch(...)
		{
		}

		if(::IsWindow(m_wndToNotify))
			::PostMessage(m_wndToNotify, MM_GLOBALSEARCH_DONE, 0, 0);
		m_stopEvent.SetEvent();

		return 0;
	}

	void Reset()
	{
		CComCritSecLock<CComAutoCriticalSection> g(m_lock);
		m_allFeeds.clear();
		m_allItems.clear();
		m_currentFeedIdx = 0;
		m_currentItemIdx = 0;
		m_currentFeed = NULL;
	}

private:
	void Search()
	{
		//
		// This function has to be "re-entrant"!!! meaning if it's called again, 
		// it should start from where it stopped last time!
		//

		for(; m_currentFeedIdx < (int)m_allFeeds.size(); ++m_currentFeedIdx)
		{
			if(m_bStop)
				return;

			if(::IsWindow(m_wndToNotify))
				::PostMessage(m_wndToNotify, MM_GLOBALSEARCH_PROGRESS, m_currentFeedIdx*100/m_allFeeds.size(),0);

			if(m_currentFeed == NULL) // if not null, we continue last search
				m_currentFeed = m_allFeeds[m_currentFeedIdx];
			if(m_allItems.empty())
				m_currentFeed->GetNewsItems(m_allItems);

			while(m_currentItemIdx < (int)m_allItems.size())
			{
				if(m_bStop)
					return;
				
				NewsItemPtr pItem = m_allItems[m_currentItemIdx];
				m_currentItemIdx++; // point to next item

				if(m_watch.Match(pItem))
				{
					LPTSTR p = new TCHAR[m_watch.m_matchCriteria.GetLength()+1];
					StringCchCopy(p, m_watch.m_matchCriteria.GetLength()+1, (LPCTSTR)m_watch.m_matchCriteria);

					// found
					if(::IsWindow(m_wndToNotify))
						::PostMessage(m_wndToNotify, MM_GLOBALSEARCH_ITEMFOUND, pItem->m_id, (LPARAM)p);

					return;
				}
			}
			
			// reset channel
			m_currentFeed = NULL;
			m_allItems.clear();
			m_currentItemIdx = 0;
		}

		// found nothing
		Reset();
	}

public:
	CNewsWatch m_watch;
	HWND m_wndToNotify;
	NewsFeedVector m_allFeeds;
	int m_currentFeedIdx;
	int m_currentItemIdx;
	NewsFeedPtr m_currentFeed;
	NewsItemVector m_allItems;

	bool m_bStop;

	CComAutoCriticalSection m_lock;
	CNGEvent m_stopEvent;
};
