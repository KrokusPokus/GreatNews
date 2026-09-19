#pragma once

#include "Request.h"
#include "FeedManagerLib.h"
#include "FeedTreeItem.h"

#define MM_SETUNREAD		(WM_APP + 210)	

enum CheckUnreadLevel
{
	CheckChannel = 1,
	CheckWatch = 2,
	CheckLabel = 4,
	CheckAll = CheckChannel + CheckWatch + CheckLabel
};

class CCheckUnreadResult
{
public:
	CCheckUnreadResult(CFeedTreeItem::Type type, ULONG_PTR id, int unread)
		: m_type(type), m_id(id), m_unread(unread) {}

public:
	CFeedTreeItem::Type m_type;
	ULONG_PTR m_id;
	int m_unread;
};

class CCheckUnreadRequest :
	public CRequest
{
public:
	CCheckUnreadRequest(HWND notifyWnd, CheckUnreadLevel level=CheckAll);
	virtual ~CCheckUnreadRequest(void);

	virtual HRESULT HandleRequest();
	virtual void Submit(bool bHighPriority=false) {CRequest::Submit(true);}

protected:
	HWND m_hwndToNotify;
	CheckUnreadLevel m_level;

private: // STL helper:
	class NotifyWnd
	{
	public:
		NotifyWnd(HWND hWnd, CFeedTreeItem::Type type) : m_hwndToNotify(hWnd), m_type(type) {}
		void operator ()(const std::pair<ULONG_PTR, int>& t);
	private:
		CFeedTreeItem::Type m_type;
		HWND m_hwndToNotify;
	};
};
