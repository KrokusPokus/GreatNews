// aboutdlg.h : interface of the CSelectChannelDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>


class CSelectChannelDlg : public CDialogImpl<CSelectChannelDlg>
{
public:
	enum { IDD = IDD_SELECTCHANNEL };

	CSelectChannelDlg(std::map<CString, CString>& rssFeeds) : m_rssFeeds(rssFeeds) {};


	BEGIN_MSG_MAP(CSelectChannelDlg)
		COMMAND_CODE_HANDLER(LBN_DBLCLK, OnDblClickListItem)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnDblClickListItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	CString m_selectedChannelUrl;
	std::map<CString, CString>& m_rssFeeds;
};
