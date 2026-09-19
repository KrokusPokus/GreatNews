// aboutdlg.h : interface of the CDinosaurDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>

class CDinosaurDlg : public CDialogImpl<CDinosaurDlg>
{
public:
	enum { IDD = IDD_DINOSAURCHANNELS };
	
	CDinosaurDlg(std::set<ULONG_PTR>& dinosaurChannels);

	BEGIN_MSG_MAP(CDinosaurDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_BTNKEEPINACTIVE, OnKeepInactive)
		COMMAND_ID_HANDLER(IDC_BTNDELETEINACTIVE, OnDeleteInactive)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnKeepInactive(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDeleteInactive(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
public:
	std::set<ULONG_PTR>& m_dinosaurChannels;
	bool m_bChannelChanged;

protected:
	CCheckListViewCtrl m_listCtrl;
	size_t GetSelectedChannels(std::set<ULONG_PTR>& selectedChannels);
	void RemoveChannelFromList(ULONG_PTR id);
};
