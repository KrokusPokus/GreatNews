#include "StdAfx.h"
#include "CheckWatchThread.h"


CCheckWatchThread::CCheckWatchThread(HWND wnd)
	: m_hwndToNotify(wnd)
{
}

CCheckWatchThread::~CCheckWatchThread(void)
{
}


DWORD CCheckWatchThread::Run()
{
	bool bNotify = false;
	try
	{
		if(m_indexToNotify.size() == 0)
			return 0;

		ATLASSERT(m_indexToNotify.size() == m_itemsToCheck.size());
		AtlTrace(_T("Num Of. Watch Updating = %d\n"),m_indexToNotify.size());
		for(int i = 0; i < (int)m_itemsToCheck.size(); ++i)
		{
			NewsItemPtr newsItem = m_itemsToCheck[i];
			if(newsItem == NULL)
				continue;

			if(!newsItem->WatchStatusChecked())
			{
				AtlTrace(_T("checking item = %d\n"),newsItem->m_id);
				if(newsItem->UpdateWatchColor())
				{
					bNotify = true;
				}
			}
		}
	}
	catch(...)
	{
	}

	if(bNotify && ::IsWindow(m_hwndToNotify))
	{
		CWindow wnd(m_hwndToNotify);
		wnd.Invalidate(FALSE);
	}

	return 0;
}

