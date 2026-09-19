#include "StdAfx.h"
#include "WatchRootTreeItem.h"

CWatchRootTreeItem::CWatchRootTreeItem(void)
{
	m_ContextMenuID = IDR_TREEMENU_WATCHROOT;
}

CWatchRootTreeItem::~CWatchRootTreeItem(void)
{
}

CString CWatchRootTreeItem::GetName()
{
	return "All News Watches";
}

NewsSourcePtr CWatchRootTreeItem::GetNewsSource()
{
	return NULL;
}
BatchContentGeneratorPtr CWatchRootTreeItem::GetContentGenerator()
{
	return NULL;
}

int CWatchRootTreeItem::GetIcon()
{
	return WatchIcon;
}
