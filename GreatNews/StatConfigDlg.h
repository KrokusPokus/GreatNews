#pragma once

#include "GreatNewsConfig.h"
#include "MyDDX.h"

class CStatConfigDlg : public CDialogImpl<CStatConfigDlg>
                     , public CMyWinDataExchange<CStatConfigDlg> //  DDX implementation, call DoDataExchange() where relevant.
{
public:
	enum { IDD = IDD_STATCONFIG };

	BEGIN_MSG_MAP(CStatConfigDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CStatConfigDlg)
		DDX_INT(IDC_EDITNUMOFCHANNELS, g_GreatNewsConfig.m_nStatsNumOfChannels)
		DDX_CHECK(IDC_CHKEXCLUDEDISABLED, g_GreatNewsConfig.m_bStatsExcludeDisabled)
	END_DDX_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

};
