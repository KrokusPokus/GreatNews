#pragma once

#include <set>
#include "NewsListCtrl.h"
#include "FeedTreeCtrl.h"
#include "FeedBrowser.h"
#include "BrowserHost.h"
#include "GreatNewsConfig.h"
#include "trayiconimpl.h"
#include "ToolbarCombo.h"
#include "AutoCombo.h"
#include "GNStatusBar.h"
#include "ChannelUpdate.h"
#include "GNSplitter.h"
#include "NewsPopupWnd.h"
#include "FaviconRequest.h"
#include "GlobalSearchDlg.h"

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
		public CMessageFilter, public CIdleHandler, public CTrayIconImpl<CMainFrame>
{
public:
	CMainFrame();

	DECLARE_FRAME_WND_CLASS(_T("GreatNews_FrameWindowClass"), IDR_MAINFRAME)

#define IDC_TREE		200
#define IDC_LIST		201
#define	IDC_COMBODATE	202
#define	IDC_COMBOSTATUS	203
#define IDC_COMBOFIND	204


	CCommandBarCtrl m_CmdBar;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_CHANNELBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_NEWSLIST, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_READINGPANE_BOTTOM, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_READINGPANE_RIGHT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_CHANNEL_WORKOFFLINE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TOOLS_EXPORTNEWSLIST, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_CHANNEL_STOPALLUPDATES, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_CHANNEL_RENAME, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_CHANNEL_UPDATE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_CHANNEL_UPDATEALLCHANNEL, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_FINDNEXT, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_FIND_GOOGLE, UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		// MSG_WM_QUERYENDSESSION(OnQueryEndSession)
		MSG_WM_ENDSESSION(OnEndSession)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MSG_WM_TIMER(OnTimer)
		// MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnCtlColorListBox)

		MESSAGE_HANDLER(MM_NEWITEM_ARRIVED, OnNewItemArrived)
		MESSAGE_HANDLER(MM_SHOWNEWSITEM, OnShowNewsItem)
		MESSAGE_HANDLER(MM_GLOBALSEARCH_ITEMFOUND, OnGlobalSearchItemFound)

		MESSAGE_HANDLER(MM_UPDATEDATAREADY, OnUpdateDataReady)
		MESSAGE_HANDLER(MM_UPDATEPROGRESS, OnUpdateProgress)
		MESSAGE_HANDLER(MM_UPDATEDONE, OnUpdateDone)
		MESSAGE_HANDLER(MM_UPDATEERROR, OnUpdateError)
		MESSAGE_HANDLER(m_TaskbarCreatedMsg, OnTaskbarCreatedMsg)

		NOTIFY_HANDLER_EX(IDC_TREE, NM_DBLCLK, OnDblClickTreeItem)

		
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_REFRESH, OnChannelRefreshUnread)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_BLOGLINES, OnChannelBloglines)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_ADDSEARCHCHANNEL, OnChannelAddSearchChannel)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_ADDGROUP, OnChannelAddGroup)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_ADDCHANNEL, OnChannelAddChannel)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_PROPERTIES, OnChannelProperties)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_ORGANIZECHANNELS, OnChannelOrganize)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_DELETE, OnChannelDelete)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_RENAME, OnChannelRename)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_UPDATE, OnChannelUpdate)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_UPDATEALLCHANNEL, OnChannelUpdateAll)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_MARKALLASREAD, OnChannelMarkAllAsRead)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_MARKASREAD, OnChannelMarkAsRead)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_MARKASUNREAD, OnChannelMarkAsUnRead)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_MARKPAGEASREAD, OnChannelMarkPageRead)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_STOPALLUPDATES, OnChannelStopAll)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_WORKOFFLINE, OnChannelWorkOffline)
		COMMAND_ID_HANDLER_EX(ID_CHANNEL_UNLABELALL, OnChannelUnlabelAll)

		COMMAND_ID_HANDLER_EX(ID_LABEL_ADD, OnLabelAdd)

		COMMAND_ID_HANDLER_EX(ID_TREEMENU_EXPANDALL, OnExpandAll)
		COMMAND_ID_HANDLER_EX(ID_TREEMENU_COLLAPSEALL, OnCollapseAll)
		COMMAND_ID_HANDLER_EX(ID_TREEMENU_COLLAPSETOGROUP, OnCollapseToGroup)

		COMMAND_ID_HANDLER_EX(ID_TREEMENU_ADDNEWSWATCH, OnAddNewsWatch)
		COMMAND_ID_HANDLER_EX(ID_OPENGREATNEWS, OnOpenGreatNews)
		COMMAND_ID_HANDLER_EX(CMD_ID_REFRESH, OnCommandRefresh)
		COMMAND_ID_HANDLER_EX(CMD_ID_READFLAGCHANGED, OnCommandFlagChanged)
		COMMAND_ID_HANDLER_EX(CMD_ID_MARKPAGEREAD, OnCommandMarkPageRead)
		COMMAND_ID_HANDLER_EX(CMD_ID_MARKITEMREAD, OnCommandMarkItemRead)
		COMMAND_ID_HANDLER_EX(CMD_ID_LABELITEM, OnCommandLabelItem)
		COMMAND_ID_HANDLER_EX(CMD_ID_EMAILITEM, OnCommandEmailItem)
		COMMAND_ID_HANDLER_EX(CMD_ID_BLOGTHIS, OnCommandBlogThis)
		COMMAND_ID_HANDLER_EX(CMD_ID_TRACKCOMMENTS, OnCommandTrackComments)
		COMMAND_ID_HANDLER_EX(CMD_ID_ADDDELICIOUS, OnCommandAddDelicious)
		COMMAND_ID_HANDLER_EX(CMD_ID_ADDFURL, OnCommandAddFurl)
		COMMAND_ID_HANDLER_EX(CMD_ID_NEWSITEMSELECTED, OnNewsItemSelected)
		COMMAND_ID_HANDLER_EX(CMD_ID_FILTERCHANGE, OnFilterChange)
		COMMAND_ID_HANDLER_EX(CMD_ID_CONFIGBLOGTHIS, OnConfigBlogThis)
		COMMAND_ID_HANDLER_EX(CMD_ID_TOGGLEREADUNREAD, OnToggleReadItem)
		COMMAND_ID_HANDLER_EX(CMD_ID_SUBCHANNEL, OnCommandSubscribeChannel)
		COMMAND_RANGE_HANDLER_EX(CMD_ID_BLOGTHISTOOL0, CMD_ID_BLOGTHISTOOL100, OnBlogThisTool)
		COMMAND_ID_HANDLER_EX(CMD_ID_SETFOCUS_FEEDTREE, OnCommandSetFocusFeedTree)
		COMMAND_ID_HANDLER_EX(CMD_ID_SETFOCUS_NEWSLIST, OnCommandSetFocusNewsList)
		COMMAND_ID_HANDLER_EX(CMD_ID_SHOWCHANNELWEBSITE, OnCommandShowChannelWebSite)

		COMMAND_HANDLER_EX(IDC_COMBOSTATUS, CBN_SELCHANGE, OnFilterSelChange)
		COMMAND_HANDLER_EX(IDC_COMBODATE, CBN_SELCHANGE, OnFilterSelChange)
		COMMAND_HANDLER_EX(IDC_COMBOFIND, CBN_EDITCHANGE, OnEditFindComboBox)
	    NOTIFY_CODE_HANDLER (CBEN_ENDEDIT, OnFindEndEdit)

		
		COMMAND_ID_HANDLER_EX(ID_TOOLS_EXPORTNEWSLIST, OnToolsExportRss)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_CLEANUP, OnToolsCleanup)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_EXPORT, OnToolsExport)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_IMPORT, OnToolsImport)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_OPTIONS, OnToolsOptions)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_MOST10, OnStatMost10)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_LEAST10, OnStatLeast10)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_MOSTACTIVE, OnStatMostActive)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_LEASTACTIVE, OnStatLeastActive)
		COMMAND_ID_HANDLER_EX(ID_TOOLS_CONFIGSTATISTICS, OnStatConfig)

		COMMAND_ID_HANDLER_EX(ID_IE_PREV, OnItemPrev)
		COMMAND_ID_HANDLER_EX(ID_IE_NEXT, OnItemNext)
		COMMAND_ID_HANDLER_EX(ID_IE_NAVIGATEXML, OnNavigateXML)
		COMMAND_ID_HANDLER_EX(ID_IE_SETSTATUSTEXT, OnSetStatusText)
		COMMAND_ID_HANDLER_EX(ID_IE_FOUNDFEEDS, OnFoundFeed)
		COMMAND_ID_HANDLER_EX(ID_IE_POPUPBLOCKED, OnPopupBlocked)
		COMMAND_ID_HANDLER_EX(ID_IE_RSSICON_CLICKED, OnRssIconClicked)
		COMMAND_ID_HANDLER_EX(ID_IE_POPUPICON_CLICKED, OnPopupIconClicked)
		COMMAND_ID_HANDLER_EX(ID_IE_OPENCHANNEL, OnOpenChannel)
		COMMAND_ID_HANDLER_EX(ID_IE_GOTO_PAGE, OnGotoPage)
		COMMAND_RANGE_HANDLER_EX(ID_IE_RSSFEED_FIRST, ID_IE_RSSFEED_LAST, OnAddDiscoveredFeed)
		COMMAND_RANGE_HANDLER_EX(ID_IE_BLOCKEDPOPUP_FIRST, ID_IE_BLOCKEDPOPUP_LAST, OnGotoBlockedUrl)

		COMMAND_ID_HANDLER_EX(ID_FILTER_SHOWALL, OnFilterShowAll)

		// Global edit action handlers - simply to map accelerators to action...
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnCut)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		COMMAND_ID_HANDLER(ID_EDIT_FIND, OnFind)

		COMMAND_ID_HANDLER_EX(ID_PANE_CLOSE, OnViewChannelBar)
		COMMAND_ID_HANDLER_EX(ID_VIEW_NEWTAB, OnViewNewTab)
		COMMAND_ID_HANDLER_EX(ID_VIEW_CHANNELBAR, OnViewChannelBar)
		COMMAND_ID_HANDLER_EX(ID_VIEW_NEWSLIST, OnViewNewsList)
		COMMAND_RANGE_HANDLER_EX(ID_READINGPANE_BOTTOM, ID_READINGPANE_RIGHT, OnReadingPane)
		COMMAND_ID_HANDLER_EX(ID_VIEW_MAXIMIZEBROWSER, OnViewMaxBrowser)
		COMMAND_ID_HANDLER_EX(ID_VIEW_NEXTUNREADNEWS, OnNextUnreadNewsChannel)
		COMMAND_ID_HANDLER_EX(ID_VIEW_NEXTUNREADGROUP, OnNextUnreadGroup)
		COMMAND_ID_HANDLER_EX(ID_VIEW_PREVPAGE, OnPreviousPage)
		COMMAND_ID_HANDLER_EX(ID_VIEW_NEXTPAGE, OnNextPage)
		COMMAND_RANGE_HANDLER_EX(ID_VIEW_LANGUAGE0, ID_VIEW_LANGUAGELAST, OnViewLanguage)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_HELP_VISITGREATNEWS, OnVisitGreatNews)
		COMMAND_ID_HANDLER(ID_HELP_VISITFORUM, OnVisitForum)

		COMMAND_ID_HANDLER(ID_EDIT_FINDSUBSCRIPTEDCHANNEL, OnFindChannel)
		COMMAND_ID_HANDLER(ID_EDIT_FINDNEWSITEM, OnFindNewsItem)
		COMMAND_ID_HANDLER(ID_EDIT_FINDNEXT, OnFindNext)
		COMMAND_RANGE_HANDLER_EX(ID_FIND_GOOGLE, ID_FIND_GOOGLE_LAST, OnInternetSearch)
		COMMAND_ID_HANDLER(ID_EDIT_GLOBALSEARCH, OnGlobalSearch)
	
	    NOTIFY_CODE_HANDLER (TBN_DROPDOWN, OnToolbarButtonDropdown)

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
		CHAIN_MSG_MAP(CTrayIconImpl<CMainFrame>)

		REFLECT_NOTIFY_CODE(NM_RETURN)
		REFLECT_NOTIFY_CODE(NM_CLICK)
		REFLECT_NOTIFY_CODE(NM_DBLCLK)
		REFLECT_NOTIFY_CODE(NM_RCLICK)
		REFLECT_NOTIFY_CODE(NM_CUSTOMDRAW)

		REFLECT_NOTIFY_CODE(TVN_GETINFOTIP)
		REFLECT_NOTIFY_CODE(TVN_BEGINDRAG)
		REFLECT_NOTIFY_CODE(TVN_DELETEITEM)
		REFLECT_NOTIFY_CODE(TVN_SELCHANGED)
		REFLECT_NOTIFY_CODE(TVN_ENDLABELEDIT)
		REFLECT_NOTIFY_CODE(TVN_BEGINLABELEDIT)

		REFLECT_NOTIFY_CODE(LVN_ITEMCHANGING)
		REFLECT_NOTIFY_CODE(LVN_ITEMCHANGED)
		REFLECT_NOTIFY_CODE(LVN_COLUMNCLICK)
		REFLECT_NOTIFY_CODE(LVN_GETDISPINFO)
		REFLECT_NOTIFY_CODE(LVN_ODCACHEHINT)

		// REFLECT_NOTIFICATIONS()

	END_MSG_MAP()

	void OnTimer(UINT_PTR wParam);
	// LRESULT OnQueryEndSession(UINT wParam, UINT lParam);
	void OnEndSession(BOOL Ending, UINT lParam);
	void OnDestroy();
	void OnSysCommand(UINT wCommand,const CPoint& pt);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnVisitGreatNews(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnVisitForum(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnNewItemArrived(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnShowNewsItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnGlobalSearchItemFound(UINT /*uMsg*/, WPARAM itemID, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnUpdateDataReady(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateProgress(UINT /*uMsg*/, WPARAM percent, LPARAM lpText, BOOL& /*bHandled*/);
	LRESULT OnUpdateDone(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateError(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTaskbarCreatedMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnChannelRefreshUnread(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelBloglines(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelAddSearchChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelAddChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelAddGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelOrganize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelUpdateAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelStopAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelMarkAllAsRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelMarkAsRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelMarkAsUnRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelMarkPageRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelWorkOffline(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnChannelUnlabelAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	LRESULT OnLabelAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	LRESULT OnPreviousPage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnNextPage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnNextUnreadGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnNextUnreadNewsChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnViewMaxBrowser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnViewNewsList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnViewChannelBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnViewNewTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void OnViewLanguage(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	void OnReadingPane(WORD wNotifyCode, WORD wID, HWND hWndCtl);

	LRESULT OnToolsCleanup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnToolsExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnToolsExportRss(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnToolsImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnToolsOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnStatMost10(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnStatLeast10(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnStatMostActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnStatLeastActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnStatConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	LRESULT OnFilterShowAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	void OnGotoBlockedUrl(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	void OnAddDiscoveredFeed(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnRssIconClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnPopupIconClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnGotoPage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnOpenChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnFoundFeed(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnPopupBlocked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnNavigateXML(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnSetStatusText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnItemPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnItemNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	LRESULT OnAddNewsWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	LRESULT OnOpenGreatNews(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandFlagChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandMarkPageRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandMarkItemRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandLabelItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnToggleReadItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnNewsItemSelected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnFilterChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandEmailItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandBlogThis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandAddDelicious(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandAddFurl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandTrackComments(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnConfigBlogThis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandSubscribeChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandSetFocusNewsList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandShowChannelWebSite(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	LRESULT OnCommandSetFocusFeedTree(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void OnBlogThisTool(WORD wNotifyCode, WORD wID, HWND hWndCtl);

	// Edit
	LRESULT OnCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	// Find
	LRESULT OnFindChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFindNewsItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGlobalSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnInternetSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnFindEndEdit(int idCtrl, LPNMHDR pnmh, BOOL &bHandled);

	LRESULT OnDblClickTreeItem(LPNMHDR pnmh);

	void OnFilterSelChange(UINT code, int id, HWND);
	void OnEditFindComboBox(UINT code, int id, HWND);

	LRESULT OnToolbarButtonDropdown(int idCtrl, LPNMHDR pnmh, BOOL &bHandled);

protected:
	LRESULT OnExpandAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		m_wndTreeFeeds.OnExpandAll(0,0,0);
		return 0;
	}

	LRESULT OnCollapseToGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		m_wndTreeFeeds.OnCollapseToGroup(0,0,0);
		return 0;
	}

	LRESULT OnCollapseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
	{
		m_wndTreeFeeds.OnCollapseAll(0,0,0);
		return 0;
	}

protected:
	enum LoadOption
	{
		LoadDbCount,
		RecountUnRead,
		EmptyCount,
		SaveThenLoad
	};

	void SaveUnreadCounts();
	void SaveUserEnvironment();
	void DisplayChannelCount();
	void DoFindNext();
	void GotoChannel(ULONG_PTR nChannelId, bool bAutoUpdate=false);

	bool InitTrayIcon(bool bNewsArrived = false, LPCTSTR tip = NULL, bool bReinstall=false);
	void InitStatusBar();
	void InitUpdateTimer();
	void LoadInitData(LoadOption);
	void ShowStatistics(LPCTSTR statTitle, NewsFeedVector& statFeeds);
	void ShowActivityStatistics(LPCTSTR statTitle, NewsFeedVector& statFeeds);
	void SyncUIContent();

	void AutoUpdate();

protected: // update help functions
	bool IsUpdating();
	void IncFinished();
	void IncUpdating();
	void CleanupUpdate();

	void InitUpdateProgress();

	void StartUpdating(NewsFeedPtr feed, bool bIncludeDisabled=false);
	void StartUpdating(FeedGroupPtr group, bool bIncludeDisabled=false);
	void StartUpdating();
	void SubmitUpdate(NewsFeedPtr feed, bool bPriority);

public:  // data

protected:	// UI pieces
	#define IDT_UPDATECHANNEL		200
	#define IDT_UPDATEUI			202
	#define IDT_POSTINIT			203
	#define IDT_FASTPOSTINIT		204

	CGNSplitter     m_wndVertSplit;
    CGNSplitter		m_wndHorzSplit;
    CPaneContainer        m_wndTreeContainer;

	CFeedTreeCtrl		  m_wndTreeFeeds;
	CNewsListCtrl		  m_wndListNews;
	CToolbarCombo		  m_cboDateFilter;
	CToolbarCombo		  m_cboStatusFilter;
	CAutoComboBox		  m_cboFind;
	CBrowserHost		  m_host;
	CGNStatusBar		  m_statusBar;
	CGlobalSearchDlg	  m_wndGlobalSearch;

	bool IsActiveFrame();
	void GetNewsFilter();
	void AddChannel(_U_STRINGorID title, const CString& defaultURL, ULONG_PTR groupId);

protected:  // Data
	INT_PTR m_nUpdateTotal;
	INT_PTR m_nUpdateFinished;
	INT_PTR m_nNewHeadlines;
	INT_PTR m_nChannelSucceeded;
	INT_PTR m_nChannelFailed;

	
	CNewsFilter m_currentNewsFilter;
	CNewsFilter m_showAllNewsFilter;

	int m_nLastSearchResult;
	BatchContentGeneratorPtr m_contentGen;

private:

	CNewsPopupWnd m_wndNewsPopup;

	HWND CreateFindBar();
	HWND CreateFilterBar();
	HWND CreateStandardBar();
	CSize GetGUIFontSize();

	void CloseNewsList(bool bClose);
	void CloseChannelBar(bool bClose);
	void SetUnreadFilter();
	void ProcessStatusBarIcon(std::set<CString>& items, UINT baseCmdID, CPoint& pt);
	bool CheckUpdateInProgress(LPCTSTR msg);
	CString FormatTrayTooltipMessage();
	INT_PTR GetTotalUnread();

	bool m_bLastNodeIsRootGroup;
	bool m_bLastNewsListIsOpen;

	bool m_bTrayShowingNewsArrived;

	CChannelUpdate m_channelUpdate;
	CFaviconRequest m_faviconRequest;

	std::set<ULONG_PTR> m_dinosaurChannels;

private:
	void internalOpenNextUnread(CTreeItem unreadNode);
	void openNextMarkRead(CFeedTreeItem::Type nodeType);
	inline void CheckVersionTime();
	void ApplyLanguage();
	void ClearTrayTooltip();
	bool HasFocus();
	BOOL HandleTabKey(MSG* pMsg);
	int GetWindowDPI(HWND hWnd);
	int m_dpi;

private:
	CFont m_defaultFont;
	bool m_bReadFlagManuallyChanged;
	int m_remindToUpgrade;

	UINT m_TaskbarCreatedMsg;
};
