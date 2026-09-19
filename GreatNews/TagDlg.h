#pragma once

#include "resource.h"
#include "FeedManagerLib.h"

class CTagDlg : public CDialogImpl<CTagDlg>, 
				public CWinDataExchange<CTagDlg>
{
public:
	enum { IDD = IDD_LABEL };

	BEGIN_MSG_MAP(CTagDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

    BEGIN_DDX_MAP(CMainDlg)
        DDX_TEXT(IDC_LABEL_NAME, m_tag.m_name)
    END_DDX_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	CTag m_tag;
};

