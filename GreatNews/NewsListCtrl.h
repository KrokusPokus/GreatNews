#pragma once

#include <set>
#include <map>
#include <atlrx.h>
#include "NewsItem.h"
#include "BatchContentGenerator.h"
#include "NewsFilter.h"
#include "TransparentHyperLink.h"
#include "NewsItemCache.h"
#include "resource.h"
#include "Tag.h"
#include "CheckWatchThread.h"


#define IDC_LINK			201

class CNewsListCtrl : public CWindowImpl<CNewsListCtrl, CListViewCtrl>,
                      public CCustomDraw<CNewsListCtrl>,
					  public CIdleHandler

{
public:
	enum RefreshMode
	{
		Redraw,
		ClearCache,
		Reload
	};
	enum OpenNewsItem
	{
		OpenNormal,
		OpenWeb,
		OpenWebInNewWindow,
		OpenWebInNewTab,OpenWebInNewTabForeground
	};
	enum Columns
	{
		ColReadFlag = 0,
		ColTag = 1,
		ColTitle = 2,
		ColPod = 3,
		ColDate = 4,
		ColAuthor = 5,
		ColChannel = 6
	};
	CNewsListCtrl(void);
	~CNewsListCtrl(void);

	virtual BOOL OnIdle();

	DECLARE_WND_CLASS_EX(_T("FeedListWindow"), CS_DBLCLKS, COLOR_WINDOW)

    BEGIN_MSG_MAP(CNewsListCtrl)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(MM_REFRESH_WATCHED_ITEM, OnRefreshWatchedItem)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MSG_WM_MBUTTONDOWN(OnMButtonDown)
		//MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_SETCURSOR(OnSetSursor)
		MSG_WM_KEYDOWN(OnKeyDown)
		COMMAND_HANDLER_EX(IDC_LINK, BN_CLICKED, OnShowAllLinkClick)
		COMMAND_RANGE_HANDLER_EX(ID_LABEL_TAGGLE_FIRST, ID_LABEL_TAGGLE_LAST, OnToggleLabel)
		COMMAND_RANGE_HANDLER_EX(CMD_ID_BLOGTHISTOOL0, CMD_ID_BLOGTHISTOOL100, OnBlogThisTool)
		COMMAND_ID_HANDLER_EX(ID_NEWSLIST_TOGGLEREAD, OnToggleReadFlag)
		COMMAND_ID_HANDLER_EX(ID_NEWSLIST_TOGGLEUNREAD, OnToggleReadFlag)
		COMMAND_ID_HANDLER_EX(ID_NEWSLIST_DELETE, OnDeleteNewsItem)
		COMMAND_ID_HANDLER_EX(ID_NEWSLIST_ADDNEWLABEL, OnAddNewLabel)
		COMMAND_ID_HANDLER_EX(CMD_ID_EMAILITEM, OnForwardCommand)
		COMMAND_ID_HANDLER_EX(CMD_ID_ADDDELICIOUS, OnForwardCommand)
		COMMAND_ID_HANDLER_EX(CMD_ID_ADDFURL, OnForwardCommand)
		FORWARD_COMMAND_ID(CMD_ID_CONFIGBLOGTHIS)
		FORWARD_COMMAND_ID(CMD_ID_BLOGTHIS)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK , OnDblClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnListItemSelected)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnLvnColumnclick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_ODCACHEHINT, OnCacheHint)
		//REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_RCLICK, OnContextMenu)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_RETURN, OnEnterKey)
        CHAIN_MSG_MAP_ALT(CCustomDraw<CNewsListCtrl>, 1)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

	void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnMButtonDown(UINT nFlags, CPoint point);
	LRESULT OnSetSursor(HWND hWnd, UINT nHitTest, UINT message);
	void OnToggleLabel(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	void OnBlogThisTool(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnDeleteNewsItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnAddNewLabel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnToggleReadFlag(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnRefreshWatchedItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnForwardCommand(WORD /*wNotifyCode*/, WORD cmdID, HWND /*hWndCtl*/);

	void OnDestroy();
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowAllLinkClick(UINT, int, HWND);

	LRESULT OnLvnColumnclick(LPNMHDR pnmh);
	LRESULT OnListItemSelected(LPNMHDR pnmh);
	LRESULT OnGetDispInfo(LPNMHDR pnmh);
	LRESULT OnCacheHint(LPNMHDR pnmh);
	LRESULT OnClick(LPNMHDR pnmh);
	LRESULT OnDblClick(LPNMHDR pnmh);
	//LRESULT OnContextMenu(LPNMHDR pnmh);
	LRESULT OnEnterKey(LPNMHDR pnmh);

	// custom draw
	DWORD OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD);
    DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {        
        return  CDRF_NOTIFYITEMDRAW;
    }

public:
	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);
	// int InsertItem(CNewsItem* pItem);
	// void SetNewsItems(NewsItemVector& newsItems);
	void SetItemRead(int itemNo, bool bSetToRead=true);
	void SetNewsSource(BatchContentGeneratorPtr gen);
	void Refresh(RefreshMode mode = Redraw);
	void PrevItem();
	void NextItem();
	void SetTopItem(int itemNo);
	void GetOrderBy(CNewsFilter* pFilter);
	void SetLastViewedItem(int itemNo);
	void BuildBlogThisPopup(CMenu& blogThisMenu);
	
	void SaveConfig();
	void LoadConfig();

	NewsItemPtr GetHotNewsItem();
	NewsItemPtr GetSelectedNewsItem();
	NewsItemPtr GetNewsItemByIndex(DWORD_PTR index);

private:
	CString GetChannelName(ULONG_PTR channelID);
	void PopulateLabelMenu(CMenu& menu, NewsItemPtr pItem);
	void StripHTML(CString& textWithHTML);
	void OpenRssLink();
	void ShowSortColumn();

private:
	CImageList m_imageList;
	CImageList m_headerImageList;
	CTransparentHyperLink m_link;
	CFont m_unreadFont;
	bool m_cc6; // Common Control 6?

private:
	int m_SortedColumn;
	bool m_SortAscending;
	bool m_manualFlagChange;

	BatchContentGeneratorPtr m_contentGen;

	TagVector m_allTags;
	int m_lastClickedItemNo;

	bool m_bOpenWebsite;
	OpenNewsItem m_openFlag;

	// items need to check watch status
	std::set<ULONG_PTR> m_indexToCheckWatch;

	CAtlRegExp<CAtlRECharTraitsW> reEscape;
};
