#pragma once

#include "FeedTreeItem.h"

class CNewsFeedTreeItem :
	public CFeedTreeItem
{
public:
	CNewsFeedTreeItem(NewsFeedPtr newsFeed);
	~CNewsFeedTreeItem(void);

	virtual Type GetType() {return Channel;}
	virtual ULONG_PTR GetId() { return m_newsFeed ? m_newsFeed->m_id : 0;}
	virtual void UpdateUIUnreadCount() {m_unread = (m_newsFeed ? m_newsFeed->m_unreadCount : 0);};
	virtual bool IsUnreadCountChanged() {return m_newsFeed ? m_unread != m_newsFeed->m_unreadCount : false;}

	virtual bool Disabled() { return (m_newsFeed ? m_newsFeed->m_bDisabled : false); }  
	virtual bool Delete();
	virtual bool Rename(LPCTSTR newName);
	virtual CString GetName();
	virtual NewsSourcePtr GetNewsSource();
	virtual BatchContentGeneratorPtr GetContentGenerator();
	virtual int GetIcon();
	virtual bool IsItem(INT_PTR id, Type t);
	virtual INT_PTR OnProperties(HWND hWndParent);
	virtual FeedGroupPtr GetFeedGroup();
	virtual COLORREF GetColor();

	virtual UINT GetContextMenuID();

public:
	virtual bool CanDrag() {return true;}
	virtual bool CanDrop(CFeedTreeItem* pItem);
	virtual bool CanRename() {return true;}

public:
	NewsFeedPtr m_newsFeed;
	int m_faviconIndex;
};
