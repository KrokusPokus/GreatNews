#include "StdAfx.h"
#include "FeedTreeItem.h"

CFeedTreeItem::CFeedTreeItem():
	m_ContextMenuID(0),
  m_bUpdateFailed(false),
  m_unread(0)
{
}

CFeedTreeItem::~CFeedTreeItem()
{
}


CString CFeedTreeItem::GetTreeNodeText()
{
	CString name = GetName();

	if (m_unread)
		name.Format(_T("%s(%d)"), (LPCTSTR)name, m_unread);

	return name;
}

