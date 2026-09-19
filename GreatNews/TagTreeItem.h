#pragma once

#include "FeedTreeItem.h"
#include "Tag.h"

class CTagTreeItem :
	public CFeedTreeItem
{
public:
	CTagTreeItem(TagPtr tag);
	virtual ~CTagTreeItem(void);

public:
	virtual Type GetType() {return Tag;}
	virtual ULONG_PTR GetId() { return m_tag ? m_tag->m_id : 0;}
	virtual void UpdateUIUnreadCount() {m_unread = (m_tag ? m_tag->m_unreadCount : 0);};
	virtual bool IsUnreadCountChanged() {return m_tag ? m_unread != m_tag->m_unreadCount : false;}

	virtual CString GetTreeNodeText();

	virtual bool Delete();
	virtual bool Rename(LPCTSTR newName);
	virtual CString GetName();
	virtual NewsSourcePtr GetNewsSource();
	virtual BatchContentGeneratorPtr GetContentGenerator();
	virtual bool IsItem(INT_PTR id, Type t);
	virtual INT_PTR OnProperties(HWND hWndParent);
	virtual int GetIcon();

public:
	virtual bool CanRename() {return true;}
	virtual bool ShowInBold() {return false;}

public:
	TagPtr m_tag;
};
