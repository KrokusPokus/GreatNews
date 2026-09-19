#include "StdAfx.h"
#include "NewsFeedTreeItem.h"
#include "FeedPropSheet.h"
#include "SearchChannelDlg.h"
#include "SearchChannelDef.h"

CNewsFeedTreeItem::CNewsFeedTreeItem(NewsFeedPtr newsFeed):
	m_newsFeed(newsFeed), m_faviconIndex(-1)
{
	m_ContextMenuID = IDR_TREEMENU_CHANNEL;
}

CNewsFeedTreeItem::~CNewsFeedTreeItem(void)
{
}

NewsSourcePtr CNewsFeedTreeItem::GetNewsSource()
{
	return m_newsFeed;
}

BatchContentGeneratorPtr CNewsFeedTreeItem::GetContentGenerator()
{
	return m_newsFeed;
}

CString CNewsFeedTreeItem::GetName()
{
	return (m_newsFeed ? m_newsFeed->m_title : _T(""));
}

int CNewsFeedTreeItem::GetIcon()
{
	if(m_newsFeed->IsSearchChannel())
		return SearchChannelIcon;
	else if(m_faviconIndex>=0)
		return m_faviconIndex;
	else if(m_newsFeed->IsBloglinesSyncChannel())
		return BloglinesChannelIcon;
	else
		return ChannelIcon;
}

bool CNewsFeedTreeItem::IsItem(INT_PTR id, Type t)
{
	return (t == Channel && m_newsFeed->m_id == id);
}

INT_PTR CNewsFeedTreeItem::OnProperties(HWND hWndParent)
{
	if(CSearchChannelDef::IsSearchChannelUrl(m_newsFeed->m_url))
	{
		// this is a search channel
		CSearchChannelDlg dlg;
		dlg.m_newsFeed = *m_newsFeed;

		INT_PTR cmd;
		if((cmd = dlg.DoModal()) == IDOK)
		{ 
			*m_newsFeed = dlg.m_newsFeed;
		}

		return cmd;
	}
	else
	{
		CFeedPropSheet dlg(IDS_CHANNELPROPERTIES, m_newsFeed);

		INT_PTR cmd;
		if((cmd = dlg.DoModal()) == IDOK)
		{ 
			*m_newsFeed = dlg.m_newsFeed;
		}

		return cmd;
	}
}

FeedGroupPtr CNewsFeedTreeItem::GetFeedGroup()
{
	try
	{
		if(m_newsFeed->m_groupID > 0)
		{
			CFeedGroup* pGroup = new CFeedGroup;
			pGroup->Init(m_newsFeed->m_groupID);
			return pGroup;
		}
	}
	catch(...)
	{
	}

	return NULL;
}

COLORREF CNewsFeedTreeItem::GetColor()
{
	if(m_bUpdateFailed)
		return	RGB(255,0,0);
	
	if(m_newsFeed->m_bDisabled)
		return RGB(192,192,192);

	return CFeedTreeItem::GetColor();
}

bool CNewsFeedTreeItem::CanDrop(CFeedTreeItem* pItem)
{
	CNewsFeedTreeItem* pFeedItem = dynamic_cast<CNewsFeedTreeItem*>(pItem);
	return (pFeedItem!=NULL);
}

bool CNewsFeedTreeItem::Delete()
{
	CString msg;
	msg.Format(ResManagerPtr->GetString(IDS_CONFIRMDELCHANNEL), (LPCTSTR)m_newsFeed->m_title);
	if(::MessageBox(NULL, (LPCTSTR)msg, _T("GreatNews"), MB_YESNO|MB_ICONQUESTION) != IDYES)
		return false;

	try
	{
		int n = m_newsFeed->GetNumOfLabelled();
		if(n>0)
		{
			CString str;
			str.Format(_T("This channel has %d labelled news items. Are you really sure to delete the channel?"), n);
			if(::MessageBox(NULL, str, _T("GreatNews"), MB_YESNO|MB_ICONSTOP) != IDYES)
				return false;
		}

		m_newsFeed->Delete();
		return true;
	}
	catch(...)
	{
	}

	return false;
}

bool CNewsFeedTreeItem::Rename(LPCTSTR newName)
{
	if(m_newsFeed->m_title != newName)
		m_newsFeed->Rename(newName);

	return true;
}

UINT CNewsFeedTreeItem::GetContextMenuID()
{
	return m_ContextMenuID;
};
