#include "StdAfx.h"
#include "TagTreeItem.h"
#include "TagDlg.h"

CTagTreeItem::CTagTreeItem(TagPtr tag) : m_tag(tag)
{
	m_ContextMenuID = IDR_TREEMENU_LABEL;
}

CTagTreeItem::~CTagTreeItem(void)
{
}

NewsSourcePtr CTagTreeItem::GetNewsSource()
{
	return m_tag;
}

BatchContentGeneratorPtr CTagTreeItem::GetContentGenerator()
{
	return m_tag;
}

CString CTagTreeItem::GetName()
{
	return (m_tag ? m_tag->m_name : _T(""));
}

int CTagTreeItem::GetIcon()
{
	return TagIcon;
}

bool CTagTreeItem::IsItem(INT_PTR id, Type t)
{
	return (t == Tag && m_tag->m_id == id);
}

INT_PTR CTagTreeItem::OnProperties(HWND hWndParent)
{
	CTagDlg dlg;
	dlg.m_tag = *m_tag;

	INT_PTR cmd;
	if((cmd=dlg.DoModal()) == IDOK)
	{ 
		*m_tag = dlg.m_tag;
	}
	
	return cmd;
}

bool CTagTreeItem::Delete()
{
	CString msg;
	msg.Format(ResManagerPtr->GetString(IDS_CONFIRMDELLABEL), (LPCTSTR)m_tag->m_name);
	if(::MessageBox(NULL, (LPCTSTR)msg, _T("GreatNews"), MB_YESNO) != IDYES)
		return false;

	try
	{
		m_tag->Delete();
		return true;
	}
	catch(...)
	{
	}


	return false;
}

bool CTagTreeItem::Rename(LPCTSTR newName)
{
	if(m_tag->m_name != newName)
		m_tag->Rename(newName);

	return true;
}

CString CTagTreeItem::GetTreeNodeText()
{
	CString name = GetName();

	if(m_unread) // for label node, m_unread actually stores the total number of items have the label
	{
		name.Format(_T("%s [%d]"), (LPCTSTR)name, m_unread);
	}

	return name;
}