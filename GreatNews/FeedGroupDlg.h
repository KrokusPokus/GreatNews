#pragma once

#include "resource.h"
#include "FeedManagerLib.h"

class CFeedGroupDlg : public CDialogImpl<CFeedGroupDlg>, 
					  public CWinDataExchange<CFeedGroupDlg>
{
public:
	enum { IDD = IDD_FEED_GROUP };

	BEGIN_MSG_MAP(CFeedGroupDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

    BEGIN_DDX_MAP(CMainDlg)
        DDX_TEXT(IDC_GROUP_NAME, m_feedGroup.m_name)
        DDX_TEXT(IDC_GROUP_DESC, m_feedGroup.m_desc)
        DDX_TEXT(IDC_GROUP_WEBSITE, m_feedGroup.m_website)
		DDX_CHECK(IDC_CHKDISABLE, m_feedGroup.m_bDisabled)
    END_DDX_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	CFeedGroup m_feedGroup;
};

