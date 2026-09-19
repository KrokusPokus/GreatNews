#include "stdafx.h"
#include "resource.h"
#include "GlobalSearchDlg.h"
#include "FeedManagerLib.h"
#include "GNResourceManager.h"

BOOL CGlobalSearchDlg::PreTranslateMessage(MSG* pMsg)
{
    return IsDialogMessage(pMsg);
}

LRESULT CGlobalSearchDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	m_bSearching = false;
	m_progressBar = GetDlgItem(IDC_PROGRESS);
	m_progressBar.SetRange(0,100);

	m_searchThread.Reset();
	m_searchThread.m_wndToNotify = m_hWnd;

	m_bCaseSensitive = false;
	m_bWholeWord = true;
	DoDataExchange(FALSE);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("GlobalSearchDialog"));

	return TRUE;
}

LRESULT CGlobalSearchDlg::OnOkCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(!m_bSearching)
	{
		DoDataExchange(TRUE);
		// setup news watch
		m_searchThread.m_watch.m_watchFlag = WATCH_TITLE | WATCH_CONTENT | WATCH_AUTHOR;
		m_searchThread.m_watch.m_allWords = CNewsWatch::MatchExact;
		m_searchThread.m_watch.m_matchCase = m_bCaseSensitive;
		m_searchThread.m_watch.m_wholeWord = m_bWholeWord;
		m_searchThread.m_watch.m_matchCriteria = m_keywords;
		m_searchThread.m_watch.SetupSearchFunc();
		m_searchThread.m_watch.Split();

		m_bItemFound = false;
		m_progressBar.ShowWindow(SW_SHOW);
		SetDlgItemText(IDOK, _T("Stop"));
		m_bSearching = true;
		m_searchThread.Start();
	}
	else // stop searching;
	{
		m_searchThread.m_bStop = true;
		CWaitCursor wc;
		m_searchThread.m_stopEvent.Lock(5*1000); // wait 5 seconds
	}

	return 0;

}

LRESULT CGlobalSearchDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_bSearching)
	{
		m_searchThread.m_bStop = true;
		CWaitCursor wc;
		m_searchThread.m_stopEvent.Lock(5*1000); // wait 5 seconds,then close anyway
	}

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);

	DestroyWindow();

	return 0;
}


LRESULT CGlobalSearchDlg::OnItemFound(UINT /*uMsg*/, WPARAM itemID, LPARAM lParam, BOOL& /*bHandled*/)
{
	m_bItemFound = true;

	// forward to parent
	GetParent().PostMessage(MM_GLOBALSEARCH_ITEMFOUND, itemID, lParam);

	return 0;
}

LRESULT CGlobalSearchDlg::OnProgress(UINT /*uMsg*/, WPARAM progress, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_progressBar.SetPos((int)progress);
	return 0;
}

LRESULT CGlobalSearchDlg::OnDone(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(!m_bItemFound
		&& !m_searchThread.m_bStop) // not forced to stop
	{
		MessageBox(ResManagerPtr->GetString(IDS_NOITEMFOUNDINSEARCH),_T("GreatNews"), MB_OK|MB_ICONINFORMATION);
	}

	m_bSearching = false;
	SetDlgItemText(IDOK, _T("Start"));

	return 0;
}

