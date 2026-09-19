#include "stdafx.h"
#include "resource.h"
#include "SelectChannelDlg.h"

LRESULT CSelectChannelDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	HWND hwnd = GetDlgItem(IDC_LISTCHANNELS);
	CListBox list(hwnd);
	for(std::map<CString, CString>::iterator it = m_rssFeeds.begin(); it!=m_rssFeeds.end(); ++it)
	{
		list.AddString(it->first);		
	}
	if(list.GetCount())
		list.SetCurSel(0);

	return TRUE;
}

LRESULT CSelectChannelDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CListBox list(GetDlgItem(IDC_LISTCHANNELS));
	if(wID == IDOK && list.GetCurSel()>=0)
	{
		CString feedTitle;
		list.GetText(list.GetCurSel(), feedTitle );
		m_selectedChannelUrl = m_rssFeeds[feedTitle];
	}

	EndDialog(wID);
	return 0;
}

LRESULT CSelectChannelDlg::OnDblClickListItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
	OnCloseCmd(0, IDOK, 0, bHandled);

	return 0;
}
