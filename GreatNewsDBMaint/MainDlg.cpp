// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

#include "FeedManagerLib.h"

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	m_dbPath = _T("newsfeed.db");

	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	//
	m_line.SubclassWindow(GetDlgItem(IDC_LINE));

	//
	if(!::PathFileExists(m_dbPath))
	{
		CString err;
		err.Format(_T("Cannot find data file [%s]"), (LPCTSTR)m_dbPath);
		MessageBox(err, _T("GreatNews Data File Maintainence"), MB_OK|MB_ICONSTOP);

		GetDlgItem(ID_UPGRADE).ShowWindow(SW_HIDE);
	}
	else
	{
		FeedManagerLib::Init(m_dbPath, false);
		CString dbVer = FeedManagerLib::GetDbVersion();
		GetDlgItem(IDC_DBVER).SetWindowText(dbVer);

		SetUpgradeBtnText();

		if(m_upgradeManager.GetHighestVersion() == dbVer)
		{
			CString err;
			err.Format(_T("Your data file version is up to date. No upgrade is needed."));
			MessageBox(err, _T("GreatNews Data File Maintainence"), MB_OK|MB_ICONINFORMATION);

			GetDlgItem(ID_UPGRADE).EnableWindow(FALSE);
		}
	}

	((CButton)GetDlgItem(IDC_LAUNCH)).SetCheck(1);

	return TRUE;

}

void CMainDlg::SetUpgradeBtnText()
{
	CString lastVer;
	lastVer.Format(_T("Upgrade to %s"), (LPCTSTR)m_upgradeManager.GetHighestVersion());
	GetDlgItem(ID_UPGRADE).SetWindowText(lastVer);
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnUpgrade(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CWaitCursor wc;
	GetDlgItem(IDC_MSG).ShowWindow(SW_SHOW);
	UpdateWindow();

	CString err;
	bool bSucceed = m_upgradeManager.Upgrade(m_dbPath, err);
	CString dbVer = FeedManagerLib::GetDbVersion();
	GetDlgItem(IDC_DBVER).SetWindowText(dbVer);
	GetDlgItem(IDC_MSG).ShowWindow(SW_HIDE);


	if(!bSucceed)
	{
		CString msg;
		msg.Format(_T("Data file upgrade failed. %s"), err);
		MessageBox(msg, _T("GreatNews Data File Maintainence"), MB_OK|MB_ICONSTOP);
	}
	else
	{

		MessageBox(_T("Data file has been upgraded successfully"), 
			_T("GreatNews Data File Maintainence"), MB_OK|MB_ICONINFORMATION);

		GetDlgItem(ID_UPGRADE).EnableWindow(FALSE);
	}

	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(((CButton)GetDlgItem(IDC_LAUNCH)).GetCheck())
	{
		::ShellExecute(NULL, _T("open"), _T("GreatNews.exe"), NULL, NULL, SW_SHOW);
	}

	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}
