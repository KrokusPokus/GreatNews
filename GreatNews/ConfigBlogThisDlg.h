// aboutdlg.h : interface of the CConfigBlogThisDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "BlogTool.h"
#include "BlogToolType.h"

class CConfigBlogThisDlg : public CDialogImpl<CConfigBlogThisDlg>
{
public:
	enum { IDD = IDD_CONFIGBLOGTHIS };

	BEGIN_MSG_MAP(CConfigBlogThisDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_HANDLER_EX(IDC_TOOLTYPES, CBN_SELCHANGE, OnTypeSelChange)
		COMMAND_HANDLER_EX(IDC_TOOLSLIST, LBN_SELCHANGE , OnToolSelChange)
		COMMAND_HANDLER(IDC_NEW, BN_CLICKED, OnBnClickedNew)
		COMMAND_HANDLER(IDC_DELETE, BN_CLICKED, OnBnClickedDelete)
		COMMAND_HANDLER(IDC_SAVE, BN_CLICKED, OnBnClickedSave)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnTypeSelChange(UINT code, int id, HWND);
	void OnToolSelChange(UINT code, int id, HWND);
	
private:
	void SetupControls();

private:
	BlogToolPtr m_selectedTool;
	BlogToolVector m_allBlogTools;
	BlogToolTypeVector m_allBlogToolTypes;

public:
	LRESULT OnBnClickedNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
