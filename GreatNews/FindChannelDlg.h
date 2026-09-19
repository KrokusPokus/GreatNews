#pragma once

#include "resource.h"
#include "FeedManagerLib.h"

class CFindChannelDlg : public CDialogImpl<CFindChannelDlg>, 
					  public CWinDataExchange<CFindChannelDlg>
{
public:
	enum { IDD = IDD_FINDCHANNEL };

	BEGIN_MSG_MAP(CFindChannelDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER_EX(IDC_LSTCHANNELS, NM_DBLCLK, OnDblClickListItem)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

    BEGIN_DDX_MAP(CFindChannelDlg)
    END_DDX_MAP()

	LRESULT OnDblClickListItem(LPNMHDR pnmh);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	CListViewCtrl m_listview;
	NewsFeedVector m_newsfeeds;
	ULONG_PTR m_selectedFeedId;
};

