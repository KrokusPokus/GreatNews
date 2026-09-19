#include "StdAfx.h"
#include "RootGroupFeedTreeItem.h"

CRootGroupFeedTreeItem::CRootGroupFeedTreeItem(FeedGroupPtr feedGroup):
	CFeedGroupTreeItem(feedGroup)
{
	m_ContextMenuID = IDR_TREEMENU_ROOT;
}

CRootGroupFeedTreeItem::~CRootGroupFeedTreeItem(void)
{
}

CString CRootGroupFeedTreeItem::GetName()
{
	return (m_feedGroup!=NULL ? m_feedGroup->m_name : _T(""));
}

NewsSourcePtr CRootGroupFeedTreeItem::GetNewsSource()
{
	return m_feedGroup;
}
BatchContentGeneratorPtr CRootGroupFeedTreeItem::GetContentGenerator()
{
	return NULL;
}

int CRootGroupFeedTreeItem::GetIcon()
{
	return ChannelRootIcon;
}

bool CRootGroupFeedTreeItem::Delete()
{
	::MessageBox(NULL, _T("No channel or channel group was selected"), _T("GreatNews"), MB_OK);
	return false;
}
