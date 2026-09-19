#include "StdAfx.h"
#include "FeedGroupDlg.h"
#include "GNResourceManager.h"

LRESULT CFeedGroupDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	DoDataExchange(FALSE);

	if(m_feedGroup.IsRootGroup())
	{
		GetDlgItem(IDC_CHKDISABLE).EnableWindow(FALSE);
	}

	// display number of items if this is not a new feed
	if(m_feedGroup.m_id != 0)
	{
		CString text;
		text.Format(ResManagerPtr->GetString(IDS_GROUPITEMS),
			m_feedGroup.GetNumOfNewsFeeds(),
			m_feedGroup.GetNumOfNewsItems(),
			m_feedGroup.GetNumOfNewsItems(true));
		GetDlgItem(IDC_NUMOFITEM).SetWindowText(text);
		GetDlgItem(IDC_NUMOFITEM).ShowWindow(SW_SHOW);
	}

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("FeedGroupPropertiesDialog"));

	return TRUE;
}

LRESULT CFeedGroupDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID == IDOK)
	{
		DoDataExchange(TRUE);
		try
		{
            m_feedGroup.Save();
		}
		catch(const CExceptionBase& e)
		{
			MessageBox(e.GetErrorMsg(),_T("GreatNews"), MB_OK|MB_ICONINFORMATION);
			return 0;
		}
	}

	EndDialog(wID);
	return 0;
}
