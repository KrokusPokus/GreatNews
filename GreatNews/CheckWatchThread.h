#pragma once

#include <vector>
#include "NGMTsync.h"
#include "GNThread.h"
#include "FeedManagerLib.h"

#define MM_REFRESH_WATCHED_ITEM		(WM_APP + 250)

class CCheckWatchThread :
	public CGNThread
{
public:
	CCheckWatchThread(HWND wnd);
	virtual ~CCheckWatchThread(void);

	virtual DWORD Run();

public:
	HWND m_hwndToNotify;
	NewsItemVector m_itemsToCheck;
	std::vector<ULONG_PTR> m_indexToNotify;
};
