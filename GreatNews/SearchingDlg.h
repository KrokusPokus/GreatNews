#pragma once

#include "SearchThread.h"

#define IDT_CHECKPROGRESS	100

class CSearchingDlg : public CDialogImpl<CSearchingDlg>
{
public:
	enum { IDD = IDD_SEARCHING };
	CSearchingDlg();

	BEGIN_MSG_MAP(CSearchingDlg)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(MM_SEARCH_FOUND, OnSearchFound)
		MESSAGE_HANDLER(MM_SEARCH_NOTFOUND, OnSearchNotFound)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	void OnTimer(UINT_PTR wParam);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSearchFound(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSearchNotFound(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void SetSearchParam(BatchContentGeneratorPtr m_content, const CString& searchText, int lastSearchResult);

public:
	CSearchThread m_searchThread;
	CProgressBarCtrl m_progressBar;
	CStatic m_text;

	CString m_matchedString;
	int	m_itemFound;

private:
	bool m_bAutoGenerateTitle;
};
