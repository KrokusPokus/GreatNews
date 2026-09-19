#include "StdAfx.h"
#include "FeedGroupTreeItem.h"
#include "FeedGroupDlg.h"
#include "NewsFeedTreeItem.h"

CFeedGroupTreeItem::CFeedGroupTreeItem(FeedGroupPtr feedGroup):
	m_feedGroup(feedGroup)
{
	m_ContextMenuID = IDR_TREEMENU_GROUP;
}

CFeedGroupTreeItem::~CFeedGroupTreeItem(void)
{
}

NewsSourcePtr CFeedGroupTreeItem::GetNewsSource()
{
	return m_feedGroup;
}

BatchContentGeneratorPtr CFeedGroupTreeItem::GetContentGenerator()
{
	return m_feedGroup;
}

CString CFeedGroupTreeItem::GetName()
{
	return (m_feedGroup ? m_feedGroup->m_name : _T(""));
}

int CFeedGroupTreeItem::GetIcon()
{
	return ChannelGroupIcon;
}

INT_PTR CFeedGroupTreeItem::OnProperties(HWND hWndParent)
{
	CFeedGroupDlg dlg;
	dlg.m_feedGroup = *m_feedGroup;

	INT_PTR cmd;
	if((cmd=dlg.DoModal()) == IDOK)
	{ 
		*m_feedGroup = dlg.m_feedGroup;
	}
	
	return cmd;
}

bool CFeedGroupTreeItem::IsItem(INT_PTR id, Type t)
{
	return (t == Group && m_feedGroup->m_id == id);
}

bool CFeedGroupTreeItem::CanDrop(CFeedTreeItem* pItem)
{
	CNewsFeedTreeItem* pFeedItem = dynamic_cast<CNewsFeedTreeItem*>(pItem);
	return (pFeedItem!=NULL);
}

bool CFeedGroupTreeItem::Delete()
{
	CString msg;
	msg.Format(ResManagerPtr->GetString(IDS_CONFIRMDELGROUP), (LPCTSTR)m_feedGroup->m_name);
	if(::MessageBox(NULL, (LPCTSTR)msg, _T("GreatNews"), MB_YESNO|MB_ICONQUESTION) != IDYES)
		return false;

	try
	{
		int n = m_feedGroup->GetNumOfLabelled();
		if(n>0)
		{
			CString str;
			str.Format(_T("This group has %d labelled news items. Are you really sure to delete the group?"), n);
			if(::MessageBox(NULL, str, _T("GreatNews"), MB_YESNO|MB_ICONSTOP) != IDYES)
				return false;
		}

		m_feedGroup->Delete();
		return true;
	}
	catch(...)
	{
	}

	return false;
}


COLORREF CFeedGroupTreeItem::GetColor()
{
	if(m_feedGroup && m_feedGroup->m_bDisabled)
		return RGB(192,192,192);
	else
		return CFeedTreeItem::GetColor();
}

bool CFeedGroupTreeItem::Rename(LPCTSTR newName)
{
	if(m_feedGroup->m_name != newName)
		m_feedGroup->Rename(newName);

	return true;
}
