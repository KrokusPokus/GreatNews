#pragma once

#include "GlobalSearchThread.h"

class CGlobalSearchDlg : public CDialogImpl<CGlobalSearchDlg>,
					     public CWinDataExchange<CGlobalSearchDlg>,
						 public CMessageFilter
{
public:
	enum { IDD = IDD_GLOBALSEARCH };
    BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CGlobalSearchDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(MM_GLOBALSEARCH_ITEMFOUND, OnItemFound)
		MESSAGE_HANDLER(MM_GLOBALSEARCH_PROGRESS, OnProgress)
		MESSAGE_HANDLER(MM_GLOBALSEARCH_DONE, OnDone)

		COMMAND_ID_HANDLER(IDOK, OnOkCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

    BEGIN_DDX_MAP(CGlobalSearchDlg)
		DDX_TEXT(IDC_KEYWORDS, m_keywords)
		DDX_CHECK(IDC_CHKCASE, m_bCaseSensitive)
		DDX_CHECK(IDC_CHKWHOLE, m_bWholeWord)
    END_DDX_MAP()


	LRESULT OnItemFound(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnProgress(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDone(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOkCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CProgressBarCtrl m_progressBar;
	CString m_keywords;
	bool m_bCaseSensitive;
	bool m_bWholeWord;


	bool m_bItemFound;
	bool m_bSearching;
	CGlobalSearchThread m_searchThread;
};
