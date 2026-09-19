#pragma once

#include "resource.h"
#include "FeedManagerLib.h"
#include "MyDDX.h"

class CSearchChannelDlg : public CDialogImpl<CSearchChannelDlg>, 
					  public CMyWinDataExchange<CSearchChannelDlg>
{
public:
	enum { IDD = IDD_SEARCHCHANNEL };

	BEGIN_MSG_MAP(CSearchChannelDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_KEYWORD, EN_CHANGE, OnKeywordsChange)
		COMMAND_HANDLER(IDC_SEARCHCHANNEL, CBN_SELCHANGE, OnKeywordsChange)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

    BEGIN_DDX_MAP(CMainDlg)
        DDX_TEXT(IDC_NAME, m_newsFeed.m_title)
        DDX_TEXT(IDC_KEYWORD, m_keyword)
		DDX_COMBO_INDEX(IDC_UPDATE_FREQ2,m_newsFeed.m_updateFreq)
		DDX_COMBO_INDEX(IDC_CBOCLEANUP,m_newsFeed.m_cleanupMaxAge)
		DDX_CHECK(IDC_NOTIFYNEWITEMS, m_newsFeed.m_notifyNewItem)
    END_DDX_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnKeywordsChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	CString GenerateTitle(CString keyword, CString searchChannel);

public:
	CNewsFeed m_newsFeed;
	CString m_searchChannel;
	CString m_keyword;
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	bool m_bAutoGenerateTitle;
};

