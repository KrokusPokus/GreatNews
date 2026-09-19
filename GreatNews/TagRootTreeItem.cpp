#include "StdAfx.h"
#include "TagRootTreeItem.h"

CTagRootTreeItem::CTagRootTreeItem(void)
{
	m_ContextMenuID = IDR_TREEMENU_LABELROOT;
}

CTagRootTreeItem::~CTagRootTreeItem(void)
{
}

CString CTagRootTreeItem::GetName()
{
	return "All Labels";
}

NewsSourcePtr CTagRootTreeItem::GetNewsSource()
{
	return NULL;
}
BatchContentGeneratorPtr CTagRootTreeItem::GetContentGenerator()
{
	return NULL;
}

int CTagRootTreeItem::GetIcon()
{
	return TagIcon;
}

CString CTagRootTreeItem::GetTreeNodeText()
{
	CString name = GetName();

	if(m_unread) // for label node, m_unread actually stores the total number of items that have the label
	{
#ifndef DEBUG // weird stuff going on here in debug mode... looks like a SEP to me though.
			name.Format(_T("%s [%d]"), (LPCTSTR)name, m_unread);
#endif
	}

	return name;
}