#pragma once

#include "FeedManagerLib.h"

class CExportRssDlg: public CDialogImpl<CExportRssDlg>
                , public CWinDataExchange<CExportRssDlg> //  DDX implementation, call DoDataExchange() where relevant.
{
	public:

		enum {IDD = IDD_EXPORTRSS};

		BEGIN_MSG_MAP(CExportRssDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDCANCEL, OnIdcancel)
			COMMAND_ID_HANDLER(IDC_BTNBROWSE, OnBtnbrowse)
			COMMAND_ID_HANDLER(IDOK, OnIdok)
		END_MSG_MAP()

		BEGIN_DDX_MAP(CExportRssDlg)
			DDX_TEXT(IDC_FILENAME, m_fileName)
		END_DDX_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnIdcancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBtnbrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnIdok(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	    CButton  m_idcBtnbrowse;
	    CString  m_fileName;

public:
	BatchContentGeneratorPtr m_batchContent;
}; // class CExportRssDlg

