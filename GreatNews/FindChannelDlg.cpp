#include "StdAfx.h"
#include "FindChannelDlg.h"
#include "FeedTreeItem.h"
#include "GNResourceManager.h"

LRESULT CFindChannelDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_selectedFeedId = 0;

	CenterWindow(GetParent());
	
	m_listview = GetDlgItem(IDC_LSTCHANNELS);
	CImageList imageList;
	imageList.CreateFromImage( IDB_TREE, 16, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION );
	m_listview.SetImageList(imageList,LVSIL_SMALL);
	
	int n=0;
	for(NewsFeedVector::iterator it = m_newsfeeds.begin(); it != m_newsfeeds.end(); ++it)
	{
		NewsFeedPtr& newsFeed = *it;
		int nItem = m_listview.InsertItem(
					LVIF_IMAGE|LVIF_PARAM|LVIF_TEXT,
					n++,
					(LPCTSTR)newsFeed->m_title,
					0,
					0,
					ChannelIcon,
					newsFeed->m_id);
	}

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("FindSubscribedChannelDialog"));

	return TRUE;
}

LRESULT CFindChannelDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID == IDOK)
	{
		int nItem = m_listview.GetSelectedIndex();
		if(nItem>=0)
		{
			m_selectedFeedId = m_listview.GetItemData(nItem);
		}
	}

	EndDialog(wID);
	return 0;
}

LRESULT CFindChannelDlg::OnDblClickListItem(LPNMHDR pnmh)
{
	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pnmh;
	m_selectedFeedId = m_listview.GetItemData(lpnmitem->iItem);
	EndDialog(IDOK);

	return 0;
}
