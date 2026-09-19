#pragma once

#include "resource.h"
#include "FeedManagerLib.h"
#include "BevelLine.h"
#include "CheckedFeedTree.h"
#include "FeedTreeCtrlBase.h"

class COrgChannelDlg : public CDialogImpl<COrgChannelDlg>, 
					  public CWinDataExchange<COrgChannelDlg>
{
public:
	enum { IDD = IDD_ORGANIZE_CHANNELS };

	BEGIN_MSG_MAP(COrgChannelDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_BTNSELECTALL2, OnSelectAll)
		COMMAND_ID_HANDLER(IDC_BTNUNSELECTALL, OnUnselectAll)
		COMMAND_ID_HANDLER(IDC_ASSIGNGROUP, OnMove)
		COMMAND_ID_HANDLER(IDC_DELETECHANNELS, OnDeleteChannels)
		COMMAND_ID_HANDLER(IDC_ADD_GROUP, OnAddGroup)
		COMMAND_ID_HANDLER(IDC_DETACHBLOGLINES, OnDetachBloglines)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

    BEGIN_DDX_MAP(CMainDlg)
    END_DDX_MAP()



	LRESULT OnDetachBloglines(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAddGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDeleteChannels(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	bool m_bReload;
	CBevelLine m_line1;
	CBevelLine m_line2;
	CBevelLine m_line3;

protected:
	CCheckedFeedTreeCtrl m_feedTree;
	CComboBoxEx	m_comboGroup;

	void LoadGroup(CComboBoxEx& comboGroup);

};

