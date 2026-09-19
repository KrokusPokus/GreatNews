#include "stdafx.h"
#include "resource.h"
#include "DinosaurDlg.h"
#include "FeedManagerLib.h"
#include "NewsFeedCache.h"
#include "GMTimeLib.h"
#include "GNResourceManager.h"

CDinosaurDlg::CDinosaurDlg(std::set<ULONG_PTR>& dinosaurChannels)
	: m_dinosaurChannels(dinosaurChannels),
	  m_bChannelChanged(false)
{
}

LRESULT CDinosaurDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	m_listCtrl.SubclassWindow(GetDlgItem(IDC_DINOSAURLIST));
	m_listCtrl.InsertColumn(0,_T("Channel"), LVCFMT_LEFT, 400, 0 );
	m_listCtrl.InsertColumn(1,_T("Last Modified"), LVCFMT_LEFT, 120, 0 );

	for(std::set<ULONG_PTR>::iterator it = m_dinosaurChannels.begin(); it != m_dinosaurChannels.end(); ++it)
	{
		NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(*it);

		int item = m_listCtrl.InsertItem(0, feed->m_title);
		m_listCtrl.SetItemText(item, 1, CGMTimeHelper::FormatDisplayDate(feed->m_lastModified));
		m_listCtrl.SetItemData(item, feed->m_id);
		m_listCtrl.SetCheckState(item, true);
	}

	// apply language pack
	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("InactiveChannelDialog"));

	return TRUE;
}

LRESULT CDinosaurDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}


size_t CDinosaurDlg::GetSelectedChannels(std::set<ULONG_PTR>& selectedChannels)
{
	selectedChannels.clear();
	for(int i = 0; i < m_listCtrl.GetItemCount(); ++i)
		if(m_listCtrl.GetCheckState(i))
			selectedChannels.insert((ULONG_PTR)m_listCtrl.GetItemData(i));

	return selectedChannels.size();
}

void CDinosaurDlg::RemoveChannelFromList(ULONG_PTR id)
{
	for(int i = 0; i < m_listCtrl.GetItemCount(); ++i)
	{
		ULONG_PTR channelID = (ULONG_PTR)m_listCtrl.GetItemData(i);
		if(id == channelID)
		{
			m_listCtrl.DeleteItem(i);
			return;
		}
	}
}

LRESULT CDinosaurDlg::OnKeepInactive(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::set<ULONG_PTR> selectedChannels;
	GetSelectedChannels(selectedChannels);

	for(std::set<ULONG_PTR>::iterator it = selectedChannels.begin(); it != selectedChannels.end(); ++it)
	{
		NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(*it);
		if(feed)
		{
			feed->KeepEvenInactive();
		}
		RemoveChannelFromList(*it);
	}

	return 0;
}

LRESULT CDinosaurDlg::OnDeleteInactive(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::set<ULONG_PTR> selectedChannels;
	GetSelectedChannels(selectedChannels);

	for(std::set<ULONG_PTR>::iterator it = selectedChannels.begin(); it != selectedChannels.end(); ++it)
	{
		NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(*it);
		if(feed)
		{
			feed->DeleteOrDisable();
			m_bChannelChanged = true;
		}
		RemoveChannelFromList(*it);
	}

	return 0;
}

