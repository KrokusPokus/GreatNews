#pragma once


class CGNStatusBar : public CMultiPaneStatusBarCtrlImpl<CGNStatusBar>
{
public:
	#define IDC_TIMER_RSS 100
	#define IDC_TIMER_POPUP 101
	#define IDC_RSS_ICON  200
	#define IDC_POPUP_ICON 201

	typedef CMultiPaneStatusBarCtrlImpl<CGNStatusBar> baseClass;

	DECLARE_WND_SUPERCLASS(_T("GN_StatusBar"), GetWndClassName())
	CGNStatusBar();
	~CGNStatusBar();

	BEGIN_MSG_MAP(CGNStatusBar)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_CODE_HANDLER_EX(STN_CLICKED, OnRssIconClicked)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnRssIconClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void OnDestroy();
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	void OnTimer(UINT_PTR idEvent);
	HWND Create(HWND hWndParent, UINT nTextID = ATL_IDS_IDLEMESSAGE, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP, UINT nID = ATL_IDW_STATUS_BAR);

public:
	void ShowProgressBar();
	void SetProgressPos(INT_PTR percent);
	void EndProgressBar();
	void EnableRssIndicator(bool bEnable = true);
	void EnablePopupIndicator(bool bEnable = true);
	CPoint GetMenuPos(UINT panelID);

protected:
	CProgressBarCtrl	m_progressBar;
	CStatic				m_rssIndicator;
	CStatic				m_popupIndicator;

	bool m_bRssIndicatorEnabled;
	bool m_bPopupIndicatorEnabled;
	int m_nRssFlashCount;
	int m_nPopupFlashCount;
};
