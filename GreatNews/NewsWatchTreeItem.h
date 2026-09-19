#pragma once

#include "FeedTreeItem.h"
#include "NewsWatch.h"

class CNewsWatchTreeItem :
	public CFeedTreeItem
{
public:
	CNewsWatchTreeItem(NewsWatchPtr newsWatch);
	~CNewsWatchTreeItem(void);

public:
	virtual Type GetType() {return Watch;}
	virtual ULONG_PTR GetId() { return m_NewsWatch ? m_NewsWatch->m_id : 0;}
	virtual void UpdateUIUnreadCount() {m_unread = (m_NewsWatch ? m_NewsWatch->m_unreadCount : 0);};
	virtual bool IsUnreadCountChanged() {return m_NewsWatch ? m_unread != m_NewsWatch->m_unreadCount : false;}

	virtual bool Delete();
	virtual bool Rename(LPCTSTR newName);
	virtual CString GetName();
	virtual NewsSourcePtr GetNewsSource();
	virtual BatchContentGeneratorPtr GetContentGenerator();
	virtual bool IsItem(INT_PTR id, Type t);
	virtual int GetIcon();
	virtual INT_PTR OnProperties(HWND hWndParent);
	virtual COLORREF GetColor();
	virtual COLORREF GetBkColor();

public:
	virtual bool CanDrag() {return true;}
	virtual bool CanDrop(CFeedTreeItem* pItem);
	virtual bool CanRename() {return true;}

public:
	NewsWatchPtr m_NewsWatch;
};
