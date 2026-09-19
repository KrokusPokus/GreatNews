#pragma once

#include "FeedTreeItem.h"

class CFeedGroupTreeItem :
	public CFeedTreeItem
{
public:
	CFeedGroupTreeItem(FeedGroupPtr feedGroup);
	~CFeedGroupTreeItem(void);

	virtual Type GetType() {return Group;}
	virtual ULONG_PTR GetId() { return m_feedGroup ? m_feedGroup->m_id : 0;}
	virtual void UpdateUIUnreadCount() {m_unread = (m_feedGroup ? m_feedGroup->m_unreadCount : 0);};
	virtual bool IsUnreadCountChanged() {return m_feedGroup ? m_unread != m_feedGroup->m_unreadCount : false;}

	virtual bool Disabled() { return (m_feedGroup ? m_feedGroup->m_bDisabled : false); }  
	virtual bool Delete();
	virtual bool Rename(LPCTSTR newName);
	virtual CString GetName();
	virtual NewsSourcePtr GetNewsSource();
	virtual BatchContentGeneratorPtr GetContentGenerator();
	virtual int GetIcon();
	virtual bool IsItem(INT_PTR id, Type t);
	virtual INT_PTR OnProperties(HWND hWndParent);
	virtual FeedGroupPtr GetFeedGroup() {return m_feedGroup;};
	virtual COLORREF GetColor();

public:
	virtual bool CanDrag() {return false;}
	virtual bool CanDrop(CFeedTreeItem* pItem);
	virtual bool CanRename() {return true;}

public:
	FeedGroupPtr m_feedGroup;

};
