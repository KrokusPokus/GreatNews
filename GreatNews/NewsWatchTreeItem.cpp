#include "StdAfx.h"
#include "NewsWatchTreeItem.h"
#include "WatchPropSheet.h"

CNewsWatchTreeItem::CNewsWatchTreeItem(NewsWatchPtr newsWatch)
	: m_NewsWatch(newsWatch)
{
	m_ContextMenuID = IDR_TREEMENU_WATCH;
}

CNewsWatchTreeItem::~CNewsWatchTreeItem(void)
{
}

NewsSourcePtr CNewsWatchTreeItem::GetNewsSource()
{
	return m_NewsWatch;
}

BatchContentGeneratorPtr CNewsWatchTreeItem::GetContentGenerator()
{
	return m_NewsWatch;
}

CString CNewsWatchTreeItem::GetName()
{
	return (m_NewsWatch ? m_NewsWatch->m_title : _T(""));
}

int CNewsWatchTreeItem::GetIcon()
{
	return WatchIcon;
}

INT_PTR CNewsWatchTreeItem::OnProperties(HWND hWndParent)
{
	CWatchPropSheet watchDlg;
	watchDlg.m_NewsWatch = *m_NewsWatch;
	INT_PTR cmd;
	if(IDOK == (cmd=watchDlg.DoModal(hWndParent)))
	{
		*m_NewsWatch = watchDlg.m_NewsWatch;
	}

	return cmd;
}

bool CNewsWatchTreeItem::IsItem(INT_PTR id, Type t)
{
	return (t == Watch && m_NewsWatch->m_id == id);
}


bool CNewsWatchTreeItem::CanDrop(CFeedTreeItem* pItem)
{
	CNewsWatchTreeItem* pWatchItem = dynamic_cast<CNewsWatchTreeItem*>(pItem);
	return (pWatchItem!=NULL);
}

COLORREF CNewsWatchTreeItem::GetColor()
{
	return m_NewsWatch->m_bDisabled? RGB(192,192,192) : m_NewsWatch->m_txtColor;
}

COLORREF CNewsWatchTreeItem::GetBkColor()
{
	return m_NewsWatch->m_bDisabled? RGB(255,255,255) : m_NewsWatch->m_bkColor;
}

bool CNewsWatchTreeItem::Delete()
{
	CString msg;
	msg.Format(ResManagerPtr->GetString(IDS_CONFIRMDELWATCH), (LPCTSTR)m_NewsWatch->m_title);
	if(::MessageBox(NULL, (LPCTSTR)msg, _T("GreatNews"), MB_YESNO) != IDYES)
		return false;

	try
	{
		m_NewsWatch->Delete();
		return true;
	}
	catch(...)
	{
	}

	return false;
}

bool CNewsWatchTreeItem::Rename(LPCTSTR newName)
{
	if(m_NewsWatch->m_title != newName)
		m_NewsWatch->Rename(newName);

	return true;
}
