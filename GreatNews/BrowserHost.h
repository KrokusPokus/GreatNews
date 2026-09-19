#pragma once

#include <vector>
#include "ContentGenerator.h"
#include "FeedBrowser.h"
#include "PopupBrowser.h"
#include "GNTabItem.h"
#include "DotNetTabCtrl.h"
#include "GNCommandParser.h"

#define BLANK_PAGE _T("about:blank")

#define IDT_MARKPAGEREAD		204

class CBrowserHost : public CWindowImpl<CBrowserHost>, 
					 public CUpdateUI<CBrowserHost>,
					 public CMessageFilter,
					 public CIdleHandler,
					 public CPopupBrowserAdvisor,
					 public CFeedBrowserAdvisor
{
public:
	DECLARE_WND_CLASS(NULL)
	CBrowserHost();

#define CHILD_TAB	200

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_IE_BACK, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_IE_FORWARD, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_IE_STYLE, UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CBrowserHost)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SETFOCUS(OnSetFocus)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_ID_HANDLER_EX(ID_FAVI_ADDTOFAVORITES, OnAddFavorites)
		COMMAND_ID_HANDLER_EX(ID_FAVI_ORGANIZEFAVORITES, OnOrganizeFavorites)
		COMMAND_ID_HANDLER_EX(ID_IE_BACK, OnBack)
		COMMAND_ID_HANDLER_EX(ID_IE_FORWARD, OnForward)
		COMMAND_ID_HANDLER_EX(ID_IE_STOP, OnStop)
		COMMAND_ID_HANDLER_EX(ID_IE_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER_EX(ID_IE_HOME, OnHome)
		COMMAND_ID_HANDLER_EX(ID_ADDRESS_GO, OnGo)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFOW, OnToolTipTextW)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFOA, OnToolTipTextA)
	    NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnTBDropDown)
		NOTIFY_CODE_HANDLER(RBN_CHILDSIZE, OnRebarChildSize)
	    NOTIFY_CODE_HANDLER(CBEN_ENDEDIT, OnAddressEndEdit)
		
		// notifications from tab
		NOTIFY_CODE_HANDLER(CTCN_SELCHANGE, OnTabSelChange)
	    NOTIFY_CODE_HANDLER(CTCN_CLOSE, OnTabClose)
	    NOTIFY_CODE_HANDLER(CTCN_MCLICK, OnTabMClick)
		NOTIFY_HANDLER(CHILD_TAB, NM_DBLCLK, OnTabDblClick)
	    NOTIFY_CODE_HANDLER(CTCN_DELETEITEM, OnTabDeleteItem)
		COMMAND_ID_HANDLER_EX(CMD_ID_REFRESHTAB, OnTabRefresh)

		COMMAND_CODE_HANDLER(CBN_DROPDOWN, OnAddressDropdown)
		COMMAND_CODE_HANDLER(CBN_SELENDOK, OnAddressSelectOK)
		FORWARD_COMMAND_ID(ID_VIEW_MAXIMIZEBROWSER)
		FORWARD_COMMAND_ID(ID_FILTER_SHOWALL)
		FORWARD_COMMAND_ID(ID_CHANNEL_UPDATEALLCHANNEL)
		FORWARD_COMMAND_ID(ID_TOOLS_IMPORT)
		FORWARD_COMMAND_ID(ID_CHANNEL_ADDCHANNEL)
		FORWARD_COMMAND_ID(ID_VIEW_NEXTUNREADNEWS)
		FORWARD_COMMAND_ID(ID_IE_OPENCHANNEL)
		FORWARD_COMMAND_ID(ID_IE_GOTO_PAGE)
		FORWARD_COMMAND_ID(ID_IE_PREV)
		FORWARD_COMMAND_ID(ID_IE_NEXT)
		FORWARD_COMMAND_ID(CMD_ID_LABELITEM)
		FORWARD_COMMAND_ID(CMD_ID_EMAILITEM)
		FORWARD_COMMAND_ID(CMD_ID_ADDDELICIOUS)
		FORWARD_COMMAND_ID(CMD_ID_ADDFURL)
		FORWARD_COMMAND_ID(CMD_ID_BLOGTHIS)
		FORWARD_COMMAND_ID(CMD_ID_TRACKCOMMENTS)
		FORWARD_COMMAND_ID(CMD_ID_TOGGLEREADUNREAD)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnToolTipTextA(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT OnToolTipTextW(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/);

	void OnTimer(UINT_PTR wParam);
	void OnDestroy();
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void OnSetFocus(HWND hOldWnd);
	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	
	LRESULT OnOrganizeFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnAddFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnForward(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnHome(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnGo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnTBDropDown (int idCtrl, LPNMHDR pnmh, BOOL &bHandled);
	LRESULT OnAddressDropdown(int, int, HWND, BOOL &bHandled);
	LRESULT OnAddressSelectOK(int, int, HWND, BOOL &bHandled);
	LRESULT OnAddressEndEdit(int idCtrl, LPNMHDR pnmh, BOOL &bHandled);
	LRESULT OnRebarChildSize(int idCtrl, LPNMHDR pnmh, BOOL &bHandled);

	LRESULT OnTabSelChange (int idCtrl, LPNMHDR pnmh, BOOL &bHandled);
	LRESULT OnTabClose (int idCtrl, LPNMHDR pnmh, BOOL &bHandled);
	LRESULT OnTabMClick (int idCtrl, LPNMHDR pnmh, BOOL &bHandled);
	LRESULT OnTabDblClick (int idCtrl, LPNMHDR pnmh, BOOL &bHandled);
	LRESULT OnTabDeleteItem (int idCtrl, LPNMHDR pnmh, BOOL &bHandled);
	LRESULT OnTabRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);


	void GotoPage(int pageNo);
	void StopMarkReadTimer();

public: // from CFeedBrowserAdvisor
	virtual void OnUrl(const CString& url);
	virtual void OnStatusTextChange(const CString& szText);
	virtual void OnTitleChange(const CString& szText) {};
	virtual void OnFoundFeed(size_t nNumOfFeeds);
	virtual BOOL OnNewWindow2(IDispatch** ppDisp);
	virtual BOOL OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
	virtual BOOL OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders);
	virtual void OnMouseDown(long button, bool ctrl, bool bUserClickedLink, const CString& url);
	virtual void OnProgressChange(long nProgress, long nProgressMax){}

public:
	enum TabOptions {CurrentTab, NewTabBackground, NewTabForeground};

	HWND Create(HWND hWndParent, ATL::_U_RECT rect);
	void Navigate(LPCTSTR url, int navOption=0, TabOptions tabOption = CurrentTab);
	void SetHighlightAfterNavigate(const CString& str);
	void ShowFindDialog();
	void ShowSaveAsDialog();

	void RefreshItem(ULONG_PTR itemID);
	bool ChangeItemReadFlag(NewsItemPtr& pItem);
	bool ChangeItemLabelStyle(NewsItemPtr& pItem);
	bool ChangeItemTrackCommentText(NewsItemPtr& pItem);
	void FocusOnAddress();
	void NewTab(LPCTSTR url, bool bOnBackGround = true);
	void RemoveTab(int item=-1);
	void NextTab();
	void PreviousTab();
	void SetContent(ContentGeneratorPtr gen);
	void RefreshContent();
	void SetWorkOffline(bool bWorkOffline=true);

	ContentGeneratorPtr& GetContentGenerator() { return m_contentGenerator; }
	CString GetInternetURL();
	size_t GetDiscoveredFeeds(std::map<CString, CString>& discoveredFeeds);
	std::set<CString>& GetBlockedURLs() {return m_blockedURLs;}
	BOOL OnNaviXML(const CString& url);
	BOOL OnNaviOpml(const CString& url);

	void ApplyLanguage();

public: // focus related
	void SetFocusToAddress();
	BOOL AddressHasFocus(HWND hwndFocus);
	void SetFocusToReadingPane();
	BOOL ReadingPaneHasFocus(HWND hwndFocus);
	BOOL CanTabOff();

public: // from CPopupBrowserAdvisor
	virtual void OnPopBeforeNav(const CString& szURL);

public:
	CString m_navigateXmlURL;
	CString m_statusText;

	CDotNetTabCtrl<CGNTabItem> m_wndBrowserTabBar;
	CToolBarCtrl	m_navigateBar;
	FeedBrowserPtr	m_wndIE;
	HWND			m_rebar;
	CComboBoxEx		m_cboAddress;
	CToolBarCtrl	m_addrBar;

private:
	bool WriteToFile(const CString& fileName, const CString& content);
	void ResizeAddressCombo();
	void BuildHtmlPage(int nPage);
	void ResetFlags();
	BOOL HandlePopup(bool bUserInited, CString url);

	int BuildFavoritesMenu(LPCTSTR pszPath, int nStartPos, CMenu& pMenu, std::vector<CString>& favorites);
	void PopupBlocked(const CString& url);
	bool IsGreatNewsSpecialUrl(const CString& url);

	void ReadPrevious();
	void ReadNext();
	
private:
	std::set<CString> m_blockedURLs;

	HWND CreateNaviBar();
	HWND CreateAddressBar();
	void SizeSimpleReBarBands();
	CSize GetGUIFontSize();
	void ActivateFeedTab();
	bool IsFeedTabActive();

	int m_minAddressWidth;

private:
	ContentGeneratorPtr m_contentGenerator;

	// CGNDispatchCommand m_command;
	CGNCommandParser m_commandParser;

private:
	CPopupBrowser m_popupBrowser; // hidden browser to detect popup url
	bool m_bNewWindow3InUse;
	bool m_bWorkOffline;

private:
	bool m_bOpenInDefaultBrowser;
	bool m_bUserClickedLink;
	bool m_bEatRightButtonUp;
};
