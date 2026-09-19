#include "stdafx.h"
#include "resource.h"
#include "SearchingDlg.h"
#include "GNResourceManager.h"

CSearchingDlg::CSearchingDlg()
{
}

LRESULT CSearchingDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	m_searchThread.m_hwndToNotify = m_hWnd;
	m_searchThread.m_bStop = false;

	m_text = GetDlgItem(IDC_SEARCH);
	m_progressBar = GetDlgItem(IDC_PROGRESS);
	m_progressBar.SetRange(0,100);
	m_progressBar.SetPos(m_searchThread.m_nSearching * 100 / m_searchThread.m_content->m_vectNewsIDs.size());

	m_searchThread.Start();
	SetTimer(IDT_CHECKPROGRESS, 1000);
	return TRUE;
}

LRESULT CSearchingDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	KillTimer(IDT_CHECKPROGRESS);

	// notify thread to stop
	m_searchThread.m_bStop = true;
	// wait the search thread to stop
	m_searchThread.m_stopEvent.Lock();
	EndDialog(wID);
	return 0;
}

void CSearchingDlg::SetSearchParam(BatchContentGeneratorPtr content, const CString& searchText,  int lastSearchResult)
{
	m_searchThread.m_content = content;
	m_searchThread.m_searchText = searchText;
	m_searchThread.m_nSearching = lastSearchResult;
}

void CSearchingDlg::OnTimer(UINT_PTR wParam)
{
	if(wParam != IDT_CHECKPROGRESS)
	{
		SetMsgHandled(FALSE);
	}
	else
	{
		CString text;
		text.Format(ResManagerPtr->GetString(IDS_SEARCHSTATUS), m_searchThread.m_nSearching, m_searchThread.m_content->m_vectNewsIDs.size());
		m_text.SetWindowText(text);
		m_text.UpdateWindow();
		m_progressBar.SetPos(m_searchThread.m_nSearching * 100 / m_searchThread.m_content->m_vectNewsIDs.size());
	}
}

LRESULT CSearchingDlg::OnSearchFound(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	KillTimer(IDT_CHECKPROGRESS);

	m_itemFound = m_searchThread.m_nSearching;
	m_matchedString = m_searchThread.m_matchedString;

	// wait the search thread to stop
	m_searchThread.m_stopEvent.Lock();
	EndDialog(IDOK);
	return 0;
}

LRESULT CSearchingDlg::OnSearchNotFound(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	KillTimer(IDT_CHECKPROGRESS);
	m_itemFound = -1;
	m_progressBar.SetPos(m_searchThread.m_nSearching * 100 / m_searchThread.m_content->m_vectNewsIDs.size());
	GetDlgItem(IDCANCEL).SetWindowText(_T("Close"));
	m_text.SetWindowText(ResManagerPtr->GetString(IDS_SEARCHNOTFOUND));
	return 0;
}

