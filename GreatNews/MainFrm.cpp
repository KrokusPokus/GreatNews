#include "stdafx.h"
#include "resource.h"
#include <shlwapi.h>
#include "AboutDlg.h"
#include "MainFrm.h"
#include "FeedGroupDlg.h"
#include "SearchChannelDlg.h"
#include "GNLockWindowUpdate.h"
#include "FeedManagerLib.h"
#include "MinimizeToTray.h"
#include "FeedGroupTreeItem.h"
#include "NewsFeedTreeItem.h"
#include "WatchPropSheet.h"
#include "CleanupWizard.h"
#include "ExportWizard.h"
#include "ImportWizard.h"
#include "OptionsSheet.h"
#include "FeedPropSheet.h"
#include "SearchingDlg.h"
#include "WinPlacement.h"
#include "TagDlg.h"
#include "GNClipBoard.h"
#include "GNUtil.h"
#include "ExportRssDlg.h"
#include "FindChannelDlg.h"
#include "OrgChannelDlg.h"
#include "UIUtil.h"
#include "GMTimeLib.h"
#include "SyncAccountWizard.h"
#include "ConfigBlogThisDlg.h"
#include "DinosaurDlg.h"
#include "StatConfigDlg.h"
#include "UpdateUsageRequest.h"
#include "BlogToolType.h"
#include "GNResourceManager.h"

#import "msxml3.dll"

#define COMBO_SIZE_DAY	15
#define COMBO_SIZE_STATUS 13
#define COMBO_SIZE_FIND 30
#define TOOLBAR_COMBO_DROPLINES 10

#define KEYDOWN_MASK   0x1000 

#define MY_TOOLBAR_STYLE \
	(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST)

// Skaliert einen Pixelwert basierend auf den DPI
inline int ScaleForDPI(int pixelValue, int dpi)
{
    return ::MulDiv(pixelValue, dpi, 96);
}

CMainFrame::CMainFrame() : 
	m_channelUpdate(g_GreatNewsConfig.GetNumOfConcurrentUpdate()),
	m_bTrayShowingNewsArrived(false),
	m_bReadFlagManuallyChanged(false),
	m_remindToUpgrade(0),
	m_TaskbarCreatedMsg(0)
{
	m_nNewHeadlines=0;
	m_nChannelSucceeded=0;
	m_nChannelFailed=0;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// for CTRL+PgUp/PgDn, we want handle by mainframe
		if(GetAsyncKeyState(VK_CONTROL) < 0 && 
			( pMsg->wParam == VK_PRIOR || pMsg->wParam == VK_NEXT))
		{
			return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
		}

		if((pMsg->wParam == VK_F4 || pMsg->wParam == 'W')
			&& (GetKeyState( VK_CONTROL ) & KEYDOWN_MASK))
		{
			// CTRL+F4 or CTRL+W to close tab
			m_host.RemoveTab();
			return TRUE;
		}

		if(pMsg->wParam == 'N'	&& (GetKeyState( VK_CONTROL ) & KEYDOWN_MASK))
		{
			// CTRL+N to open new browser window
			CString url = m_host.GetInternetURL();
			CGNUtil::OpenUrlExternally(url, false, true);
				
			return TRUE;
		}

		if(pMsg->wParam == 'S'	&& (GetKeyState( VK_CONTROL ) & KEYDOWN_MASK))
		{
			// CTRL+S to open Save As dialog
			m_host.ShowSaveAsDialog();	
			return TRUE;
		}

		if(pMsg->wParam == VK_TAB) // TAB key?
		{
			if(GetKeyState( VK_CONTROL ) & KEYDOWN_MASK) // with CTRL key?
			{
				if(GetKeyState( VK_SHIFT ) & KEYDOWN_MASK) // with SHIFT key?
					m_host.PreviousTab();
				else
					m_host.NextTab();
			}
			else
			{
				if(HandleTabKey(pMsg))
					return TRUE;
			}
		}
	}

	if(m_host != NULL && m_host.PreTranslateMessage(pMsg))
		return TRUE;

	// then let mainframe check the msg
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	// if it's a mouse wheel event, we want to send to window under the mouse directly
	if(pMsg->message == WM_MOUSEWHEEL)
	{
		CWindow child = WindowFromPoint(CPoint(pMsg->lParam));
		if(child && 
			(IsChild(child) ||		// is child of main window?
			m_wndNewsPopup && m_wndNewsPopup.IsChild(child))) // or child of new item popup?
		{
			child.SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnVisitGreatNews(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_host.Navigate(_T("http://www.curiostudio.com/"), 0, CBrowserHost::NewTabForeground);
	return 0;
}

LRESULT CMainFrame::OnVisitForum(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_host.Navigate(_T("http://www.curiostudio.com/forum"), 0, CBrowserHost::NewTabForeground);
	return 0;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	//m_defaultFont.CreateFont(
	//						13,                        // nHeight
	//						0,                         // nWidth
	//						0,                         // nEscapement
	//						0,                         // nOrientation
	//						FW_NORMAL,                 // nWeight
	//						FALSE,                     // bItalic
	//						FALSE,                     // bUnderline
	//						0,                         // cStrikeOut
	//						DEFAULT_CHARSET,              // nCharSet
	//						OUT_DEFAULT_PRECIS,        // nOutPrecision
	//						CLIP_DEFAULT_PRECIS,       // nClipPrecision
	//						DEFAULT_QUALITY,           // nQuality
	//						DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
	//						_T("Tahoma"));			   // lpszFacename

#define REBAR_STYLE \
	(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_AUTOSIZE | CCS_NODIVIDER | RBS_DBLCLKTOGGLE)

	m_nUpdateTotal=0;
	m_nUpdateFinished=0;
	m_nLastSearchResult = -1;

	m_dpi = GetWindowDPI(m_hWnd);

	// ************************************************************
	// * Create the main UI
	// ************************************************************

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.SetImageMaskColor(RGB(192, 192, 192));

	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	//m_CmdBar.LoadImages(IDR_MAINFRAME);
	m_CmdBar.LoadImages(IDB_STANDARDBAR);
	// remove old menu
	SetMenu(NULL);

#define PN_REBAR_STYLE \
	(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_AUTOSIZE | CCS_NODIVIDER | RBS_DBLCLKTOGGLE)

	HWND standardBar = CreateStandardBar();
	HWND filterBar = CreateFilterBar();
	HWND findBar = CreateFindBar();
	CreateSimpleReBar(PN_REBAR_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(standardBar, NULL, TRUE);
	AddSimpleReBarBandCtrl(m_hWndToolBar, filterBar, 200, _T("News"));
	AddSimpleReBarBandCtrl(m_hWndToolBar, findBar, 201, _T("Search"));
	SizeSimpleReBarBands();

	// CreateSimpleStatusBar();
	InitStatusBar();

	UIAddToolBar(standardBar);
	UIAddToolBar(findBar);
	UIEnable(ID_EDIT_FINDNEXT, false);
	UIEnable(ID_FIND_GOOGLE, false);
	UIEnable(ID_CHANNEL_STOPALLUPDATES, false);

	// register object for message filtering and idle updates
	CMessageLoop* pMessageLoop = _Module.GetMessageLoop();
	ATLASSERT(pMessageLoop != NULL);
	pMessageLoop->AddMessageFilter(this);
	pMessageLoop->AddIdleHandler(this);

    // Create the splitter windows.
	const DWORD dwSplitStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  m_wndVertSplit.Create ( *this, rcDefault, NULL, dwSplitStyle);
	m_wndVertSplit.SetSplitterExtendedStyle(SPLIT_VERTICAL, SPLIT_PROPORTIONAL | SPLIT_VERTICAL);
  m_wndHorzSplit.Create ( m_wndVertSplit, rcDefault, NULL, dwSplitStyle);
	m_wndHorzSplit.SetSplitterExtendedStyle(
    g_GreatNewsConfig.m_nReadingPanePos == CGreatNewsConfig::Right ?
    SPLIT_VERTICAL : 0, SPLIT_PROPORTIONAL | SPLIT_VERTICAL);
  m_hWndClient = m_wndVertSplit; // Set the vertical splitter as the client area window.

    // Create the pane containers.
  m_wndTreeContainer.Create ( m_wndVertSplit );

	// create left pane: feed tree
	const DWORD dwTreeStyle = WS_CHILD | WS_CLIPCHILDREN | WS_HSCROLL | TVS_INFOTIP | TVS_EDITLABELS;
	const DWORD dwTreeExStyle = WS_EX_CLIENTEDGE;
	m_wndTreeFeeds.SetFaviconRequest(&m_faviconRequest);
	m_wndTreeFeeds.Create(m_wndTreeContainer, rcDefault,NULL,dwTreeStyle, dwTreeExStyle, IDC_TREE);

	// DPI-Scaling
	int baseHeight = (int)::SendMessage(m_wndTreeFeeds.m_hWnd, TVM_GETITEMHEIGHT, 0, 0);
	::SendMessage(m_wndTreeFeeds.m_hWnd, TVM_SETITEMHEIGHT, ScaleForDPI(baseHeight, m_dpi), 0);

	// create top right pane: news list
	const DWORD dwListStyle = WS_CHILD | WS_CLIPCHILDREN | WS_HSCROLL | LVS_REPORT ;
    const DWORD dwListExStyle = WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT;
	m_wndListNews.Create(m_wndHorzSplit, rcDefault,NULL,dwListStyle, dwListExStyle, IDC_LIST);
	pMessageLoop->AddIdleHandler(&m_wndListNews);

	// Create the bottom pane
	m_host.Create(m_wndHorzSplit, rcDefault);
	// UIAddChildWindowContainer(m_host);
	pMessageLoop->AddIdleHandler(&m_host);
	m_host.SetWorkOffline(g_GreatNewsConfig.m_bWorkOffline);

	// Set up the pane containers
  m_wndTreeContainer.SetClient ( m_wndTreeFeeds );
  // Set up the splitter panes
  m_wndHorzSplit.SetSplitterPanes ( m_wndListNews, m_host );
  m_wndVertSplit.SetSplitterPanes ( m_wndTreeContainer, m_wndHorzSplit );

  // Set the splitter as the client area window, and resize the splitter to match the frame size.
  UpdateLayout();
    
	m_wndVertSplit.SetBarSize(6, 0);
	m_wndHorzSplit.SetBarSize(6);
	
	m_wndVertSplit.SetCollapsePos(g_GreatNewsConfig.GetChannelTreePos(), false);
	m_wndHorzSplit.SetCollapsePos(g_GreatNewsConfig.GetNewsListPos(), false);
	if(g_GreatNewsConfig.GetChannelTreeClosed())
		CloseChannelBar(true);
	m_bLastNewsListIsOpen = !g_GreatNewsConfig.GetNewsListClosed();

	CFontHandle defaultFont = m_wndListNews.GetFont();
	LOGFONT lf;
	defaultFont.GetLogFont(&lf);
	StringCchCopy(lf.lfFaceName,LF_FACESIZE, _T("Tahoma"));
	lf.lfWeight = FW_NORMAL; // reset to normal 
	if(m_defaultFont.CreateFontIndirect(&lf))
	{
		m_wndTreeContainer.SetFont(m_defaultFont);
		m_wndTreeFeeds.SetFont(m_defaultFont);
		m_wndListNews.SetFont(m_defaultFont);
	}

	// ************************************************************
	// * Load init data
	// ************************************************************
	LoadInitData(LoadDbCount);
	
	InitTrayIcon(false, _T("GreatNews"));

	InitUpdateTimer();

	GetNewsFilter();

	m_channelUpdate.Start(m_hWnd);

	CloseNewsList(true);
	m_bLastNodeIsRootGroup = true;

	ApplyLanguage();

	SetTimer(IDT_UPDATEUI, 2000); // Update UI look every 1 sec
	SetTimer(IDT_POSTINIT, 10000);
	SetTimer(IDT_FASTPOSTINIT, 1000);

	// setup new item notifications
	CNewsFeed::m_notifyWnd = m_hWnd;
	m_wndNewsPopup.Create(m_hWnd);

	m_wndTreeFeeds.SetFocus();
	
	if(!g_GreatNewsConfig.m_bWorkOffline)
		m_faviconRequest.Submit();

	// show welcome page or user specified page
	m_wndTreeFeeds.SelectItem(m_wndTreeFeeds.GetRootItem());
	if(g_GreatNewsConfig.GetStartupScreen() != CGreatNewsConfig::Blank)
		PostMessage(WM_COMMAND, CMD_ID_REFRESH);

	m_TaskbarCreatedMsg = RegisterWindowMessage(_T("TaskbarCreated"));
	return 0;
}

void CMainFrame::DisplayChannelCount()
{
	CString treeHeader;
	treeHeader.Format(ResManagerPtr->GetString(IDS_CHANNELCOUNT), m_wndTreeFeeds.GetChannelCount());

	m_wndTreeContainer.SetTitle(treeHeader);
}

void CMainFrame::LoadInitData(LoadOption loadOption)
{
	try
	{
		if(loadOption == SaveThenLoad)
			SaveUnreadCounts();

		m_wndTreeFeeds.LoadTree((LoadTreeType)(LoadAll | LoadSearch));
		CTreeItem channelRoot = m_wndTreeFeeds.GetRootItem();
		CTreeItem childNode = channelRoot.GetChild();
		while(childNode) // only check the first level as we only support one level folder
		{
			CFeedTreeItem* pItem = (CFeedTreeItem*)childNode.GetData();
			if(pItem->GetType() == CFeedTreeItem::Group)
			{
				CFeedGroupTreeItem* pGroupItem = (CFeedGroupTreeItem*)pItem;
				if(pGroupItem->m_feedGroup->m_bExpanded)
					childNode.Expand();
			}
			childNode = childNode.GetNextSibling();
		}

		if(loadOption == RecountUnRead)
		{
			CCheckUnreadRequest* pRequest = new CCheckUnreadRequest(m_wndTreeFeeds.m_hWnd);
			pRequest->Submit();
		}
		else if(loadOption == LoadDbCount || loadOption == SaveThenLoad)
		{
			// load unread count from feed/group etc themselves
			m_wndTreeFeeds.UpdateUIUnreadCount();
		}
		// m_wndTreeFeeds.UpdateWindow();

		DisplayChannelCount();
	}
	catch(const CExceptionBase& e)
	{
		MessageBox(e.GetErrorMsg(),_T("GreatNews"));
	}
	catch(...)
	{
		MessageBox(_T("Cannot initialize newsfeed data"),_T("GreatNews"));
	}
}

void CMainFrame::GetNewsFilter()
{
	static CNewsFilter::NewsStatus statusArray[] = {CNewsFilter::Unread, CNewsFilter::Flagged, CNewsFilter::All};
	static CNewsFilter::DateRange dateRangeArray[] = {CNewsFilter::Today, CNewsFilter::SinceYesterday, CNewsFilter::Last7Days, CNewsFilter::Last30Days, CNewsFilter::AllDates};
	
	if(m_cboStatusFilter.GetCurSel()>=0)
		m_currentNewsFilter.m_newsStatus = statusArray[m_cboStatusFilter.GetCurSel()];

	if(m_cboDateFilter.GetCurSel()>=0)
		m_currentNewsFilter.m_dateRange = dateRangeArray[m_cboDateFilter.GetCurSel()];
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// m_wndNewsPopup.Show(NULL);
	CAboutDlg dlg;
	dlg.DoModal();

	return 0;
}

LRESULT CMainFrame::OnChannelUpdate(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		CFeedTreeItem* pSelectedItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(pSelectedItem == NULL)
			return 0;

		bool bIncludeDisabled = ((wNotifyCode & 8)==0);
		CFeedGroupTreeItem* pGroupItem = dynamic_cast<CFeedGroupTreeItem*>(pSelectedItem);
		if(pGroupItem)
		{
			StartUpdating(pGroupItem->m_feedGroup, bIncludeDisabled);
		}
		else
		{
			CNewsFeedTreeItem* pFeedItem = dynamic_cast<CNewsFeedTreeItem*>(pSelectedItem);
			if(pFeedItem)
				StartUpdating(pFeedItem->m_newsFeed, bIncludeDisabled);
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnChannelWorkOffline(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if(g_GreatNewsConfig.m_bWorkOffline)
	{
		g_GreatNewsConfig.m_bWorkOffline = false;
	}
	else
	{
		g_GreatNewsConfig.m_bWorkOffline = true;
		OnChannelStopAll(0, 0, 0);
	}

	m_host.SetWorkOffline(g_GreatNewsConfig.m_bWorkOffline);

	UISetCheck(ID_CHANNEL_WORKOFFLINE, g_GreatNewsConfig.m_bWorkOffline);
	UIEnable(ID_CHANNEL_UPDATE, !g_GreatNewsConfig.m_bWorkOffline);
	UIEnable(ID_CHANNEL_UPDATEALLCHANNEL, !g_GreatNewsConfig.m_bWorkOffline);

	return 0;
}

LRESULT CMainFrame::OnChannelUnlabelAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		CTagTreeItem* pTagTreeItem = dynamic_cast<CTagTreeItem*>(m_wndTreeFeeds.GetSelectedFeedItem());
		if(!pTagTreeItem)
			return 0;

		CWaitCursor wc;
		INT_PTR labelCount = pTagTreeItem->GetUnreadCount();
		if(labelCount)
		{
			pTagTreeItem->m_tag->UnlabelAll();
			m_wndTreeFeeds.AdjustUnreadCounts(m_wndTreeFeeds.GetSelectedItem(), -labelCount);
			SyncUIContent();
		}
	}
	CATCH_ALL_ERROR()

	return 0;

}

LRESULT CMainFrame::OnChannelStopAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if(IsUpdating())
	{
		CWaitCursor wc;
		m_channelUpdate.Stop();
		m_channelUpdate.Start(m_hWnd);

		CleanupUpdate();
		m_wndTreeFeeds.ClearAllChannelStatus();

		NewsFeedVector feeds;
		m_wndTreeFeeds.GetAllChannels(feeds);
		for(NewsFeedVector::iterator it = feeds.begin(); it!=feeds.end(); ++it)
		{
			NewsFeedPtr& feed = *it;
			// we want to timestamp all channels so they will not start until next scheduled time
			feed->m_lastChecked = CGMTimeHelper::GetCurrentSysTime(); 
		}
	}

	return 0;
}

LRESULT CMainFrame::OnChannelUpdateAll(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/)
{
	StartUpdating();

	return 0;
}

LRESULT CMainFrame::OnChannelRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	// check if anything selected in feed tree
	CFeedTreeItem* pFeedTreeItem = m_wndTreeFeeds.GetSelectedFeedItem();
	if(!pFeedTreeItem)
		return 0;

	if(pFeedTreeItem->CanRename())
	{
		m_wndTreeFeeds.SetFocus();
		m_wndTreeFeeds.m_bAllowLabelEditing = true;
		CEdit edit = m_wndTreeFeeds.EditLabel(m_wndTreeFeeds.GetSelectedItem());
		if(edit)
			edit.SetWindowText(pFeedTreeItem->GetName());
	}

	return 0;
}
LRESULT CMainFrame::OnChannelDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		CFeedTreeItem* pFeedTreeItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(!pFeedTreeItem)
			return 0;

		CWaitCursor wc;
		if(pFeedTreeItem->Delete())
		{
			CTreeItem item = m_wndTreeFeeds.GetSelectedItem();
			
      // we need to use this command to adjust channel unread and label totals
			PostMessage(WM_COMMAND, ID_CHANNEL_REFRESH);

			item.Delete();
			DisplayChannelCount();
			SyncUIContent();
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}

void CMainFrame::SubmitUpdate(NewsFeedPtr feed, bool bHighPriority)
{
	if(g_GreatNewsConfig.m_bWorkOffline)
		return;

	if(!g_GreatNewsConfig.m_bBypassTTLChecking
		&& feed->SkipUpdate())
	{
		// this feed's TTL is not expired yet. Don't update
		return;
	}

	if(!feed->IsCommentChannel())
	{
		IncUpdating();
		m_wndTreeFeeds.ShowChannelStatus(feed->m_id, ChannelQueued);
		m_channelUpdate.EnqueueUpdate(feed, bHighPriority);

		// auto submit comment feeds from this channel
		NewsFeedVector commentFeeds;
		if(CNewsFeedCache::GetCommentFeeds(commentFeeds, feed->m_id))
		{
			for(NewsFeedVector::iterator it = commentFeeds.begin(); it!=commentFeeds.end(); ++it)
			{
				SubmitUpdate(*it, false);
			}
		}
	}
	else
	{
		m_channelUpdate.EnqueueUpdate(feed, bHighPriority);
	}
}

void CMainFrame::StartUpdating(NewsFeedPtr feed, bool bIncludeDisabled)
{
	CheckVersionTime();

	if(m_wndTreeFeeds.IsChannelUpdating(feed->m_id))
		return;

	feed->m_bBypassCache = true;
	SubmitUpdate(feed, true);
	InitUpdateProgress();
}

void CMainFrame::StartUpdating(FeedGroupPtr group, bool bIncludeDisabled)
{
	CheckVersionTime();

	NewsFeedVector feeds;
	m_wndTreeFeeds.GetChildChannels(group->m_id, feeds, bIncludeDisabled); 
	for(NewsFeedVector::reverse_iterator it = feeds.rbegin(); it!=feeds.rend(); ++it)
	{
		NewsFeedPtr& feed = *it;
		if(m_wndTreeFeeds.IsChannelUpdating(feed->m_id))
			continue;

		feed->m_bBypassCache = true;
		SubmitUpdate(feed, true);
	}
	InitUpdateProgress();
}

void CMainFrame::StartUpdating()
{
	CheckVersionTime();

	NewsFeedVector feeds;
	m_wndTreeFeeds.GetAllChannels(feeds);

	for(NewsFeedVector::iterator it = feeds.begin(); it!=feeds.end(); ++it)
	{
		NewsFeedPtr& feed = *it;
		
		if(m_wndTreeFeeds.IsChannelUpdating(feed->m_id) || m_wndTreeFeeds.IsFeedDisabled(feed))
			continue;

		feed->m_bBypassCache = true;
		SubmitUpdate(feed, false);
	}

	InitUpdateProgress();
}

LRESULT CMainFrame::OnChannelOrganize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if(!CheckUpdateInProgress(_T("GreatNews is updating channels. Stop updating and organize channels?")))
		return 0;

	COrgChannelDlg dlg;
	dlg.DoModal();

	if(dlg.m_bReload)
		LoadInitData(SaveThenLoad);

	return 0;

}

LRESULT CMainFrame::OnChannelProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		CFeedTreeItem* pFeedTreeItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(!pFeedTreeItem)
			return 0;

		if(pFeedTreeItem->OnProperties(m_hWnd) == IDOK)
		{
			// check if user modified channel's group
			if(pFeedTreeItem->GetType()== CFeedTreeItem::Channel)
			{
				CTreeItem channelNode = m_wndTreeFeeds.GetSelectedItem();
				CTreeItem groupNode = channelNode.GetParent();
				
				CFeedGroupTreeItem* pGroupItem = dynamic_cast<CFeedGroupTreeItem*>((CFeedTreeItem*)groupNode.GetData());
				CNewsFeedTreeItem* pFeedItem = dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)channelNode.GetData());

				if(pGroupItem->m_feedGroup->m_id != pFeedItem->m_newsFeed->m_groupID)
				{
					LONG_PTR nChannelID = pFeedItem->m_newsFeed->m_id;
					LoadInitData(SaveThenLoad);

					CTreeItem newNode = m_wndTreeFeeds.FindNodeByIdType(nChannelID, CFeedTreeItem::Channel);
					if(newNode)
					{
						newNode.Select();
						SyncUIContent();
					}
				}
			}

			m_wndTreeFeeds.RefreshSelectedNode();
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}


LRESULT CMainFrame::OnChannelRefreshUnread(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_wndTreeFeeds.MarkAllRead();

	CCheckUnreadRequest* pRequest = new CCheckUnreadRequest(m_wndTreeFeeds.m_hWnd);
	pRequest->Submit();

	return 0;
}

LRESULT CMainFrame::OnChannelBloglines(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CSyncAccountWizard wizard;
	if(wizard.DoModal() == IDOK)
		LoadInitData(SaveThenLoad);

	return 0;
}


LRESULT CMainFrame::OnChannelAddSearchChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		FeedGroupPtr group = NULL;
		CFeedTreeItem* pItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(pItem)
			group = pItem->GetFeedGroup();
		if(group == NULL)
			group = CFeedGroup::GetRootFeedGroup();
		
		CSearchChannelDlg dlg;
		if(group)
			dlg.m_newsFeed.m_groupID = group->m_id;

		if(dlg.DoModal() == IDOK)
		{ 
			NewsFeedPtr newsFeed = new CNewsFeed(dlg.m_newsFeed);
			CTreeItem item = m_wndTreeFeeds.FindNodeByIdType(newsFeed->m_groupID, CFeedTreeItem::Group);
			CTreeItem newItem = m_wndTreeFeeds.InsertItem(item, newsFeed, TRUE);
			newItem.Select();
			DisplayChannelCount();

			SyncUIContent();
			OnChannelUpdate(0, 0, 0);
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}


LRESULT CMainFrame::OnChannelAddChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		// try to get a default URL
		CString url;
		CGNClipBoard clipboard;
		if(clipboard.Open(m_hWnd) && clipboard.GetTextData(url))
		{
			url.Trim();
			if(CGNUtil::IsFeedUrl(url)
				|| url.Find(_T("http:"))== 0
				|| url.Find(_T("https:"))== 0
				|| url.Find(_T("www.")) == 0 )
			{
				url = CGNUtil::NormalizeURL(url);
			}
			else
			{
				url.Empty();
			}
		}
		clipboard.Close();

		FeedGroupPtr feedGroup = m_wndTreeFeeds.GetDefaultFeedGroup();
		AddChannel((UINT)IDS_ADDCHANNELWIZARD, url, feedGroup!=NULL ? feedGroup->m_id : 0);
	}
	CATCH_ALL_ERROR()

	return 0;
}

void CMainFrame::AddChannel(_U_STRINGorID title, const CString& defaultURL, ULONG_PTR groupId)
{
	// CNewsFeedDlg dlg;
	CFeedPropSheet dlg(title);
	dlg.m_newsFeed.m_url = defaultURL;
	dlg.m_newsFeed.m_groupID = groupId;

	if(dlg.DoModal() == IDOK)
	{ 
		NewsFeedPtr newsFeed = CNewsFeedCache::GetNewsFeed(dlg.m_newsFeed.m_id);
		CTreeItem groupItem = m_wndTreeFeeds.FindNodeByIdType(newsFeed->m_groupID, CFeedTreeItem::Group);
		if(groupItem == NULL)
		{
			// user added a new group
			LoadInitData(SaveThenLoad);
			CTreeItem newItem = m_wndTreeFeeds.FindNodeByIdType(dlg.m_newsFeed.m_id, CFeedTreeItem::Channel);
			newItem.Select();
		}
		else
		{
			CTreeItem newItem = m_wndTreeFeeds.InsertItem(groupItem, newsFeed, TRUE);
			newItem.Select();
		}
		DisplayChannelCount();

		SyncUIContent();
		OnChannelUpdate(0, 0, 0);
	}
}

LRESULT CMainFrame::OnChannelAddGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		CFeedGroupDlg dlg;
		FeedGroupPtr root = CFeedGroup::GetRootFeedGroup();
		dlg.m_feedGroup.m_parentID = root->m_id;

		if(dlg.DoModal() == IDOK)
		{ 
			CFeedGroup* pFeedGroup = new CFeedGroup(dlg.m_feedGroup);
			CTreeItem item = m_wndTreeFeeds.InsertItem(m_wndTreeFeeds.GetRootItem(), pFeedGroup, TRUE);
			item.Select();
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnCommandFlagChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	NewsItemPtr pItem = m_wndListNews.GetHotNewsItem();
	if(pItem)
	{
		m_wndTreeFeeds.AdjustUnreadCounts(pItem, (pItem->IsUnread()? 1 : -1));
		m_bReadFlagManuallyChanged = true;
		m_host.RefreshItem(pItem->m_id);
	}

	return 0;
}

LRESULT CMainFrame::OnNewsItemSelected(WORD flag, WORD, HWND)
{
	NewsItemPtr pItem = m_wndListNews.GetSelectedNewsItem();
	if(pItem)
	{
		if(flag == CNewsListCtrl::OpenNormal)
		{
			m_host.SetContent(pItem);
		}
		else if(flag == CNewsListCtrl::OpenWeb)
		{
			CString url = pItem->GetLink();
			if(url.GetLength())
				m_host.Navigate(url);
		}
		else if(flag == CNewsListCtrl::OpenWebInNewWindow)
		{
			CString url = pItem->GetLink();
			if(url.GetLength())
				CGNUtil::OpenUrlExternally(url, false, true);
		}
		else if(flag == CNewsListCtrl::OpenWebInNewTab)
		{
			CString url = pItem->GetLink();
			if(url.GetLength())
				m_host.Navigate(url, 0, CBrowserHost::NewTabBackground);
		}
		else if(flag == CNewsListCtrl::OpenWebInNewTabForeground)
		{
			CString url = pItem->GetLink();
			if(url.GetLength())
			{
				m_host.Navigate(url, 0, CBrowserHost::NewTabForeground);
			}
		}
	}

	m_wndListNews.SetFocus();

	return 0;
}

LRESULT CMainFrame::OnLabelAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		CTagDlg dlg;

		if(dlg.DoModal() == IDOK)
		{ 
			CTag* pTag = new CTag(dlg.m_tag);
			CTreeItem item = m_wndTreeFeeds.InsertItem(m_wndTreeFeeds.GetTagRootItem(), pTag, TRUE);
			item.Select();
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}

// Synchonize content display in NewsList/BrowserHost from Channel Tree selection
void CMainFrame::SyncUIContent()
{
	m_wndListNews.SetLastViewedItem(-1);
	m_wndListNews.GetOrderBy(&m_currentNewsFilter);

	CFeedTreeItem* pSelectedTreeItem = m_wndTreeFeeds.GetSelectedFeedItem();
  m_contentGen = pSelectedTreeItem ? pSelectedTreeItem->GetContentGenerator() : 0;

	if(m_contentGen)
	{
		m_contentGen->SetNewsFilter(&m_currentNewsFilter);
		m_contentGen->InitGenerator(g_GreatNewsConfig.m_nPageSize);
	}		

	m_nLastSearchResult = -1;
	m_wndListNews.SetNewsSource(m_contentGen);
	m_host.SetContent(m_contentGen);

	if(m_contentGen)
	{
		CUpdateUsageRequest* pRequest = new CUpdateUsageRequest(m_contentGen);
		pRequest->Submit();
	}
}

LRESULT CMainFrame::OnCommandRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		m_host.StopMarkReadTimer();

		CGNLockWindowUpdate lockWnd(m_hWnd);

		CFeedTreeItem* pSelectedTreeItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(!pSelectedTreeItem)
			return 0;

		// auto mark previous channel as read, 
		if(m_bReadFlagManuallyChanged  // if user manually changed read flag, 
			|| GetKeyState( VK_CONTROL ) & KEYDOWN_MASK) // or user holding CTRL key
		{
			// don't mark read automaticlaly when switching channel

			m_bReadFlagManuallyChanged = false;
		}
		else if(m_wndTreeFeeds.m_hPreviousNode != NULL
			&& m_wndTreeFeeds.m_hPreviousNode != m_wndTreeFeeds.GetSelectedItem()) // user indeed selected different tree node
		{
			CFeedTreeItem* pPreviousItem = (CFeedTreeItem*)m_wndTreeFeeds.GetItemData(m_wndTreeFeeds.m_hPreviousNode);
			CFeedTreeItem* pCurrentItem = (CFeedTreeItem*)m_wndTreeFeeds.GetItemData(m_wndTreeFeeds.GetSelectedItem());

			if (g_GreatNewsConfig.m_bMarkChannelReadWhenSwitching)
			{
				if(pPreviousItem->GetType() == CFeedTreeItem::Channel && pPreviousItem->GetUnreadCount())
				{
					NewsSourcePtr newsSource = pPreviousItem->GetNewsSource();
					newsSource->MarkRead();
					m_wndTreeFeeds.MarkRead(m_wndTreeFeeds.m_hPreviousNode);
					ClearTrayTooltip();
					m_host.RefreshContent();
				}
			}
		}
		m_wndTreeFeeds.m_hPreviousNode = m_wndTreeFeeds.GetSelectedItem(); 

		CWatchRootTreeItem* pWatchRootItem = dynamic_cast<CWatchRootTreeItem*>(pSelectedTreeItem);
		CTagRootTreeItem * pTagRootItem = dynamic_cast<CTagRootTreeItem*>(pSelectedTreeItem);
		if((pWatchRootItem || pTagRootItem)
			&& !g_GreatNewsConfig.m_bNoAutoGroupCollapse)
		{
			m_wndTreeFeeds.CollapseToGroup();
			m_wndTreeFeeds.Expand(m_wndTreeFeeds.GetSelectedItem());
			return 0;
		}

		if(g_GreatNewsConfig.m_bUpdateWhenClick)
		{
			bool bUpdate = false;

			switch(pSelectedTreeItem->GetType())
			{
			case CFeedTreeItem::Group:
				if(!pSelectedTreeItem->Disabled())
					bUpdate = true;
				break;
			case CFeedTreeItem::Channel:
				if(!m_wndTreeFeeds.IsFeedDisabled((CNewsFeedTreeItem*)pSelectedTreeItem))
					bUpdate = true;
				break;
			}

			if(bUpdate)
			{
				PostMessage(WM_COMMAND, MAKELONG(ID_CHANNEL_UPDATE,8), 0); // set the 4th bit to 1 so we will bypass disabled channels
			}
		}

		CRootGroupFeedTreeItem* pRootItem = dynamic_cast<CRootGroupFeedTreeItem*>(pSelectedTreeItem);
		if(pRootItem)
		{
			m_contentGen = NULL;

			if(!m_bLastNodeIsRootGroup) // don't save twice if user click root group twice
			{
				m_bLastNewsListIsOpen = !m_wndHorzSplit.IsCollapsed();
			}

			CloseNewsList(true);
			if(g_GreatNewsConfig.GetStartupScreen() == CGreatNewsConfig::Top10Visited)
			{
				PostMessage(WM_COMMAND, ID_TOOLS_MOST10);
			}
			else if(g_GreatNewsConfig.GetStartupScreen() == CGreatNewsConfig::Welcome)
			{
				m_host.Navigate(g_GreatNewsConfig.GetMediaDir()+_T("\\Home.htm"));
			}
			else if(g_GreatNewsConfig.GetStartupScreen() == CGreatNewsConfig::Label)
			{
				CTreeItem tagNode = m_wndTreeFeeds.FindNodeByIdType(g_GreatNewsConfig.m_nStartupLabel, CFeedTreeItem::Tag);
				if(tagNode)
				{
					tagNode.Select();
					SyncUIContent();
				}
			}
			else if(g_GreatNewsConfig.GetStartupScreen() == CGreatNewsConfig::Watch)
			{
				CTreeItem watchNode = m_wndTreeFeeds.FindNodeByIdType(g_GreatNewsConfig.m_nStartupLabel, CFeedTreeItem::Watch);
				if(watchNode)
				{
					watchNode.Select();
					SyncUIContent();
				}
			}

			m_bLastNodeIsRootGroup = true;
			return 0;
		}

		//
		// Auto collapse group
		//
		if(pSelectedTreeItem->GetType() == CFeedTreeItem::Group)
		{
			CTreeItem selectedItem = m_wndTreeFeeds.GetSelectedItem();
			if(!g_GreatNewsConfig.m_bNoAutoGroupCollapse)
			{
				m_wndTreeFeeds.CollapseToGroup(selectedItem);
				selectedItem.Expand();
			}
		}

		// all other items
		if(m_bLastNodeIsRootGroup)
		{
			m_bLastNodeIsRootGroup = false;
			if(m_bLastNewsListIsOpen)
				CloseNewsList(false);
		}

		// Now display news in selected item
		if(g_GreatNewsConfig.m_bNoChannelGroupView &&
			pSelectedTreeItem->GetType() == CFeedTreeItem::Group)
		{
			// do nothing
		}
		else
		{
			SyncUIContent();
		}

		lockWnd.Release();
		m_wndTreeFeeds.UpdateWindow();
	}
	CATCH_ALL_ERROR()
	
	return 0;
}

LRESULT CMainFrame::OnChannelMarkAllAsRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		// KillTimer(IDT_UPDATECHANNEL);

		if(MessageBox(ResManagerPtr->GetString(IDS_SUREMARKALLASREAD), _T("GreatNews"), MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) != IDYES)
			return 0;

		CWaitCursor wc;

		CNewsItem::MarkAllAsRead();
		m_wndListNews.Refresh();
		m_wndTreeFeeds.MarkAllRead();

		ClearTrayTooltip();
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnChannelMarkAsRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		CFeedTreeItem* pFeedTreeItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(!pFeedTreeItem)
			return 0;

		m_host.StopMarkReadTimer();

		// if clicked on root item, translate to Mark All As Read message
		CRootGroupFeedTreeItem* pRootItem = dynamic_cast<CRootGroupFeedTreeItem*>(pFeedTreeItem);
		if(pRootItem != NULL)
		{
			PostMessage(WM_COMMAND, ID_CHANNEL_MARKALLASREAD, 0);
			return 0;
		}

		// start marking read
		CWaitCursor wc;

		NewsSourcePtr newsSource = pFeedTreeItem->GetNewsSource();
		if(newsSource)
		{
			newsSource->MarkRead();
			m_wndListNews.Refresh();
			m_wndTreeFeeds.MarkRead(m_wndTreeFeeds.GetSelectedItem());
			ClearTrayTooltip();
			m_host.RefreshContent();
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnChannelMarkPageRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	OnCommandMarkPageRead(0, 0, 0);
	m_host.RefreshContent();

	return 0;
}
LRESULT CMainFrame::OnChannelMarkAsUnRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		CFeedTreeItem* pFeedTreeItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(!pFeedTreeItem)
			return 0;

		// KillTimer(IDT_UPDATECHANNEL);

		NewsSourcePtr newsSource = pFeedTreeItem->GetNewsSource();
		newsSource->MarkUnread();
		m_wndListNews.Refresh();
		m_host.RefreshContent();
		PostMessage(WM_COMMAND, ID_CHANNEL_REFRESH); // we need to use this command to adjust channel unread and label totals
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnDblClickTreeItem(LPNMHDR)
{
	// if user doesn't update channel by single click, he's using double click
	if(!g_GreatNewsConfig.m_bUpdateWhenClick)
	{
		CFeedTreeItem* pItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(pItem->GetType() == CFeedTreeItem::Channel)
			OnChannelUpdate(0, 0, 0);
	}

	return 0;
}

LRESULT CMainFrame::OnUpdateDataReady(UINT /*uMsg*/, WPARAM channelID, LPARAM numOfNewItems, BOOL& /*bHandled*/)
{
	NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(channelID);
	if(feed == NULL)
	{
		AtlTrace(_T("!!! Data consistency error !!!\n"));
		return 0; 
	}

	if(feed->IsCommentChannel())
	{
		// for comment channels, the numOfNewItems has two parts, highword is the number of changed existing items
		// lowword is the number of downloaded comments
		if(numOfNewItems)
		{
			m_host.RefreshItem(feed->m_parent_item_id);

			// now check if we changed read item to unread
			numOfNewItems /= 65536; 
			ATLASSERT(numOfNewItems <=1);
			if(numOfNewItems)
			{
				// if we do have changed item's read/unread status, set these parameters to the parent channel
				channelID = feed->m_parent_feed_id;
			}
			else
			{
				// no changed items, we are done
				return 0;
			}
		}
		else
		{
			// nothing to do
			return 0;
		}
	}
	else
	{
		if(!IsUpdating())
			return 0;

		IncFinished();
		++m_nChannelSucceeded;
	}


	//
	// Show update status on feed tree
	//
	m_wndTreeFeeds.ShowChannelStatus(channelID, ChannelIdle);

	CTreeItem node = m_wndTreeFeeds.FindNodeByIdType(channelID, CFeedTreeItem::Channel);
	if(node)
	{
		CNewsFeedTreeItem* pFeedTreeItem = (CNewsFeedTreeItem*)node.GetData();
		pFeedTreeItem->m_bUpdateFailed = false;
		NewsFeedPtr feed = pFeedTreeItem->m_newsFeed;

		if(numOfNewItems)
		{
			m_wndTreeFeeds.AdjustUnreadCounts(node, numOfNewItems);

			m_nNewHeadlines += numOfNewItems;

			CFeedTreeItem* pSelectedItem = m_wndTreeFeeds.GetSelectedFeedItem();
			if(pSelectedItem && pSelectedItem->IsItem(channelID, CFeedTreeItem::Channel))
			{
				SyncUIContent();
				m_host.StopMarkReadTimer(); // do not mark as read for this one
			}
		}
		else
		{
			// no new post, check for dinosour channels
			if(g_GreatNewsConfig.m_bNotifyDinosaurChannels && !feed->m_bKeepInactive)
			{
				CTime dinoDays = CGMTimeHelper::DaysAgo(CGMTimeHelper::GetCurrentSysTime(), g_GreatNewsConfig.m_nDinosaurDays);
				if(feed->m_lastModified < dinoDays)
					m_dinosaurChannels.insert(feed->m_id);
			}
		}
	}

	return 0;
}

LRESULT CMainFrame::OnUpdateDone(UINT /*uMsg*/, WPARAM channelID, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_statusBar.EndProgressBar();

	//set statusbar msg
	CString tip;
	tip.Format(ResManagerPtr->GetString(IDS_UPDATESUMMARY)
				, m_nNewHeadlines, m_nChannelSucceeded);
	if(m_nChannelFailed)
	{
		tip.Format(ResManagerPtr->GetString(IDS_UPDATEFAILEDCOUNT), m_nChannelFailed);
	}
	m_statusBar.SetText( 0, tip);

	// optionally show ballon tip
	if(m_nNewHeadlines > 0)
	{
		if(g_GreatNewsConfig.m_bShowTooltipAfterUpdate && IsIconic())
		{
			CString tip = FormatTrayTooltipMessage();
			ShowBalloonTip(tip, ResManagerPtr->GetString(IDS_FINISHUPDATE));
		}
	}

	if(IsActiveFrame() 
		&& g_GreatNewsConfig.m_bNotifyDinosaurChannels
		&& !m_dinosaurChannels.empty())
	{
		CDinosaurDlg dlg(m_dinosaurChannels);
		dlg.DoModal();
		m_dinosaurChannels.clear();

		if(dlg.m_bChannelChanged) // some channels were deleted/disabled
			LoadInitData(SaveThenLoad);
	}

	return 0;
}

LRESULT CMainFrame::OnUpdateProgress(UINT /*uMsg*/, WPARAM channelID, LPARAM lpText, BOOL& /*bHandled*/)
{
	m_wndTreeFeeds.ShowChannelStatus(channelID, ChannelDownloading);

	if(lpText != NULL)
	{
		LPTSTR p = (LPTSTR)lpText;
		m_statusBar.SetText(0, p);
		delete[] p;
	}

	return 0;
}

LRESULT CMainFrame::OnUpdateError(UINT /*uMsg*/, WPARAM channelID, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(channelID);
	if(feed == NULL)
	{
		AtlTrace(_T("!!! Data consistency error !!!\n"));
		return 0; 
	}

	if(feed->IsCommentChannel())
	{
		// nothing to do
	}
	else
	{
		if(!IsUpdating())
			return 0;

		m_nChannelFailed++;
		IncFinished();
		m_wndTreeFeeds.ShowChannelStatus(channelID, ChannelIdle);

		CTreeItem item = m_wndTreeFeeds.FindNodeByIdType(channelID, CFeedTreeItem::Channel);
		if(item)
		{
			CFeedTreeItem* pFeedTreeItem = (CFeedTreeItem*)item.GetData();
			pFeedTreeItem->m_bUpdateFailed = true;
			m_wndTreeFeeds.RefreshNode(item);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnTaskbarCreatedMsg(UINT /*uMsg*/, WPARAM channelID, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// reinstall tray icon
	InitTrayIcon(false, _T("GreatNews"), true);

	return 0;
}

bool CMainFrame::InitTrayIcon(bool bNewsArrived,LPCTSTR tip, bool bReinstall)
{
	if (!g_GreatNewsConfig.GetCloseToTray() && !g_GreatNewsConfig.GetRemoveFromTaskBar()) {
		return false;
	}
	
	// Load a small icon
	HICON hIconSmall = NULL;
	if(!bNewsArrived)
	{
		hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), 
							MAKEINTRESOURCE(IDR_MAINFRAME),
							IMAGE_ICON, 
							::GetSystemMetrics(SM_CXSMICON), 
							::GetSystemMetrics(SM_CYSMICON), 
							LR_DEFAULTCOLOR);
	}
	else
	{
		hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), 
							MAKEINTRESOURCE(IDR_UPDATENEWMSG),
							IMAGE_ICON, 
							::GetSystemMetrics(SM_CXSMICON), 
							::GetSystemMetrics(SM_CYSMICON), 
							LR_DEFAULTCOLOR);
	}

	// Install tray icon
	return InstallIcon(tip, hIconSmall, IDR_TRAYPOPUP, bReinstall);
}

void CMainFrame::InitStatusBar()
{
	m_hWndStatusBar = m_statusBar.Create(*this);
	UIAddStatusBar(m_hWndStatusBar);
}

void CMainFrame::InitUpdateProgress()
{
	if(m_nUpdateTotal == 0)
		return;

	// create the progress bar
	m_statusBar.ShowProgressBar();
	m_statusBar.SetProgressPos(100/m_nUpdateTotal/2); // give progress bar a head start
}

bool CMainFrame::IsUpdating()
{
	return (m_nUpdateFinished<m_nUpdateTotal);
}

void CMainFrame::IncUpdating()
{
	m_nUpdateTotal += 1;
	UIEnable(ID_CHANNEL_STOPALLUPDATES, true);
}
void CMainFrame::CleanupUpdate()
{
	UIEnable(ID_CHANNEL_STOPALLUPDATES, false);

	m_nUpdateTotal = m_nUpdateFinished = 0;
	PostMessage(MM_UPDATEDONE, 0, 0);
	// SetTooltipText(_T("GreatNews"));
}
void CMainFrame::IncFinished()
{
	m_nUpdateFinished++;
	m_statusBar.SetProgressPos(m_nUpdateFinished*100/m_nUpdateTotal);

	if(m_nUpdateFinished == m_nUpdateTotal)
	{
		CleanupUpdate();
	}
	else
	{
		CString trayText;
		trayText.Format(ResManagerPtr->GetString(IDS_UPDATESTATUS),m_nUpdateFinished,m_nUpdateTotal);
		SetTooltipText(trayText);
	}
}

LRESULT CMainFrame::OnOpenGreatNews(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CUIUtil::ShowMyWindow(m_hWnd);
	return 0;
}

bool CMainFrame::IsActiveFrame()
{
	HWND hwndFocus = GetFocus();
	return (IsChild(hwndFocus)==TRUE);
}

void CMainFrame::InitUpdateTimer()
{
	KillTimer(IDT_UPDATECHANNEL);

	// The timer ticks every minute. Then we figure out how what channels's update interval is
	SetTimer(IDT_UPDATECHANNEL, 60*1000); 
}

void CMainFrame::OnTimer(UINT_PTR wParam)
{
	if(wParam == IDT_UPDATEUI) // the timer to update UIs
	{
		UISetCheck(ID_VIEW_CHANNELBAR, !m_wndVertSplit.IsCollapsed());
		UISetCheck(ID_VIEW_NEWSLIST, !m_wndHorzSplit.IsCollapsed());
		UISetCheck(ID_CHANNEL_WORKOFFLINE, g_GreatNewsConfig.m_bWorkOffline);
		UISetCheck(ID_READINGPANE_BOTTOM, g_GreatNewsConfig.m_nReadingPanePos == CGreatNewsConfig::Bottom);
		UISetCheck(ID_READINGPANE_RIGHT, g_GreatNewsConfig.m_nReadingPanePos == CGreatNewsConfig::Right);
		UIEnable(ID_CHANNEL_UPDATE, !g_GreatNewsConfig.m_bWorkOffline);
		UIEnable(ID_CHANNEL_UPDATEALLCHANNEL, !g_GreatNewsConfig.m_bWorkOffline);
		UIEnable(ID_TOOLS_EXPORTNEWSLIST, (m_contentGen!=NULL));

		if(m_wndTreeFeeds != NULL)
		{
			CFeedTreeItem* pItem = (CFeedTreeItem*)m_wndTreeFeeds.GetRootItem().GetData();
			bool bNewMsgIcon = (m_nNewHeadlines>0);
			if(bNewMsgIcon != m_bTrayShowingNewsArrived)
			{
				if(InitTrayIcon(bNewMsgIcon))
					m_bTrayShowingNewsArrived = bNewMsgIcon;
			}

			if(!IsUpdating())
			{
				CString tip = FormatTrayTooltipMessage();
				if(tip != GetTooltipText())
				{
					SetTooltipText(tip);
				}
			}
		}
	}
	else if(wParam == IDT_UPDATECHANNEL)
	{
		AutoUpdate();
	}
	else if(wParam == IDT_FASTPOSTINIT)
	{
		KillTimer(IDT_FASTPOSTINIT);

		if(g_GreatNewsConfig.m_newFeedUrl.GetLength())
		{
			CString title(MAKEINTRESOURCE(IDS_ADDCHANNELWIZARD));
			AddChannel((LPCTSTR)title, g_GreatNewsConfig.m_newFeedUrl, 0);
		}

		PostMessage(WM_COMMAND, ID_CHANNEL_REFRESH);
		if(g_GreatNewsConfig.GetAutoUpdateAtStartup())
			PostMessage(WM_COMMAND, ID_CHANNEL_UPDATEALLCHANNEL, 0L);

		FeedManagerLib::MarkTime(FeedManagerLib::StartupTime);
		PostMessage(WM_COMMAND, IDT_UPDATECHANNEL);

	}
	else if(wParam == IDT_POSTINIT)
	{
		KillTimer(IDT_POSTINIT);

		CTime lastCleanup = g_GreatNewsConfig.GetCleanupLastRun();
		CTimeSpan cleanSpan = CGMTimeHelper::GetCurrentSysTime() - lastCleanup;
		CTimeSpan remindSpan = CTimeSpan(g_GreatNewsConfig.GetCleanupReminderDays(), 0, 0, 0);
		if(remindSpan >= 0 && remindSpan < cleanSpan)
		{
			CString msg;
			msg.Format(ResManagerPtr->GetString(IDS_CONFIRMCLEANUP), cleanSpan.GetDays());
			if(IDYES == MessageBox(msg, _T("GreatNews"), MB_YESNO|MB_ICONQUESTION))
			{
				PostMessage(WM_COMMAND, ID_TOOLS_CLEANUP, 0);
			}
		}
	}
	else
	{
		SetMsgHandled(FALSE);
	}

}

void CMainFrame::OnEndSession(BOOL Ending, UINT lParam)
{
	SaveUnreadCounts();
	SaveUserEnvironment();
	SetMsgHandled(FALSE);
	return;
}
void CMainFrame::OnDestroy()
{
	try
	{
		if(m_wndNewsPopup.m_hWnd)
			m_wndNewsPopup.DestroyWindow();

		KillTimer(IDT_UPDATEUI);
		KillTimer(IDT_UPDATECHANNEL);
		KillTimer(IDT_POSTINIT);

		// stop all threads
		m_faviconRequest.SignalShutdown();
		m_channelUpdate.Stop();
		CRequest::Shutdown();

		// Save user status
		SaveUnreadCounts();

		// save user environment
		SaveUserEnvironment();

		CNewsWatchCache::Empty();
		CNewsItemCache::Uninit();
		CNewsFeedCache::Empty();

		if(g_GreatNewsConfig.m_bCleanTempFilesUponExit)
			CGNUtil::CleanupGNTempFiles();
	}
	catch(...)
	{
	}

	SetMsgHandled(FALSE);
}

void CMainFrame::SaveUserEnvironment()
{
	try
	{
		// save channel tree states
		std::vector<std::pair<ULONG_PTR,long> > treeStates;
		CTreeItem channelRoot = m_wndTreeFeeds.GetRootItem();
		CTreeItem childNode = channelRoot.GetChild();
		while(childNode) // only check the first level as we only support one level folder
		{
			CFeedTreeItem* pItem = (CFeedTreeItem*)childNode.GetData();
			if(pItem->GetType() == CFeedTreeItem::Group)
			{
				treeStates.push_back(std::pair<ULONG_PTR,long>(
					pItem->GetId(),
					(childNode.GetState(TVIS_EXPANDED) & TVIS_EXPANDED) ? 1 : 0));
			}
			childNode = childNode.GetNextSibling();
		}
		CFeedGroup::SaveTreeState(treeStates);

		// save other environment
		CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
		g_GreatNewsConfig.SaveLanguage(pResMngr->GetCurrentLanguagePackFileName());

		g_GreatNewsConfig.SetChannelTreePos(m_wndVertSplit.GetCollapsePos());
		g_GreatNewsConfig.SetNewsListPos(m_wndHorzSplit.GetCollapsePos());
		g_GreatNewsConfig.SetChannelTreeClosed(m_wndVertSplit.IsCollapsed());
		g_GreatNewsConfig.SetNewsListClosed(
			m_bLastNodeIsRootGroup ? !m_bLastNewsListIsOpen : m_wndHorzSplit.IsCollapsed());
		
		g_GreatNewsConfig.SetDateFilterSelection(m_cboDateFilter.GetCurSel());
		g_GreatNewsConfig.SetStatusFilterSelection(m_cboStatusFilter.GetCurSel());

		CWindowPlacement place;
		if(place.GetPosData(m_hWnd))
		{
			CString placement;
			place.SetPosData(placement);
			g_GreatNewsConfig.SetWindowPlacement(placement);
		}

		g_GreatNewsConfig.SaveConfig();
	}
	catch(...)
	{
		ATLTRACE(_T("!!! Failed to save user environment !!!"));
	}
}

void CMainFrame::SaveUnreadCounts()
{
	std::vector<std::pair<ULONG_PTR,INT_PTR>> counts;
	if(m_wndTreeFeeds.RetrieveUIUnreadCount(counts, CFeedTreeItem::Channel))
		CNewsFeed::SaveUnreadCount(counts);
	if(m_wndTreeFeeds.RetrieveUIUnreadCount(counts, CFeedTreeItem::Group))
		CFeedGroup::SaveUnreadCount(counts);
	if(m_wndTreeFeeds.RetrieveUIUnreadCount(counts, CFeedTreeItem::Watch))
		CNewsWatch::SaveUnreadCount(counts);
	if(m_wndTreeFeeds.RetrieveUIUnreadCount(counts, CFeedTreeItem::Tag))
		CTag::SaveUnreadCount(counts);
	FeedManagerLib::MarkTime(FeedManagerLib::ShutdownTime);
}

void CMainFrame::OnSysCommand(UINT wCommand,const CPoint& pt)
{
	if(wCommand == SC_MINIMIZE && g_GreatNewsConfig.GetRemoveFromTaskBar())
	{
		MinimizeWndToTray(m_hWnd);
		DefWindowProc();
		ShowWindow(SW_HIDE);
	}
	else if(wCommand == SC_CLOSE && g_GreatNewsConfig.GetCloseToTray())
	{
		MinimizeWndToTray(m_hWnd);
		ShowWindow(SW_HIDE);
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}


LRESULT CMainFrame::OnFilterChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_wndListNews.GetOrderBy(&m_currentNewsFilter);
	SyncUIContent();
	return 0;
}

void CMainFrame::OnFilterSelChange(UINT code, int id, HWND)
{
	GetNewsFilter();
	SyncUIContent();
}

LRESULT CMainFrame::OnAddNewsWatch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CWatchPropSheet watchDlg;
	if(IDOK == watchDlg.DoModal(m_hWnd))
	{
		CNewsWatch* pWatch = new CNewsWatch(watchDlg.m_NewsWatch);
		CTreeItem item = m_wndTreeFeeds.InsertItem(
			m_wndTreeFeeds.GetWatchRootItem(), pWatch);
		item.Select();
	}

	return 0;
}

LRESULT CMainFrame::OnStatMost10(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		NewsFeedVector most10;
		CNewsFeed::GetStatistics(most10, CNewsFeed::Most10Visited
									,g_GreatNewsConfig.m_nStatsNumOfChannels
									,g_GreatNewsConfig.m_bStatsExcludeDisabled );
		ShowStatistics(_T("Most Visited Feeds"), most10);
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnStatLeast10(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	try
	{
		NewsFeedVector least10;
		CNewsFeed::GetStatistics(least10, CNewsFeed::Least10Visited
								,g_GreatNewsConfig.m_nStatsNumOfChannels
								,g_GreatNewsConfig.m_bStatsExcludeDisabled );

		ShowStatistics(_T("Least Visited Feeds"), least10);
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnStatMostActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CWaitCursor wc;

	try
	{
		NewsFeedVector mostActive;
		CNewsFeed::GetActivityStatistics(mostActive, CNewsFeed::MostActive, 60
										,g_GreatNewsConfig.m_nStatsNumOfChannels
										,g_GreatNewsConfig.m_bStatsExcludeDisabled );
		ShowActivityStatistics(_T("Most Active Feeds In Last 60 Days"), mostActive);
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnStatLeastActive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CWaitCursor wc;

	try
	{
		NewsFeedVector leastActive;
		CNewsFeed::GetActivityStatistics(leastActive, CNewsFeed::LeastActive, 60
										, g_GreatNewsConfig.m_nStatsNumOfChannels
										, g_GreatNewsConfig.m_bStatsExcludeDisabled);
		ShowActivityStatistics(_T("Least Active Feeds In Last 60 Days"), leastActive);
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnStatConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CStatConfigDlg dlg;
	dlg.DoModal();

	return 0;
}


LRESULT CMainFrame::OnToolsExportRss(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if(m_contentGen != NULL)
	{
		CExportRssDlg dlg;
		dlg.m_batchContent = m_contentGen;
		dlg.DoModal(m_hWnd);
	}

	return 0;
}

LRESULT CMainFrame::OnToolsExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CExportWizard dlg;
	dlg.DoModal(m_hWnd);

	return 0;
}

LRESULT CMainFrame::OnToolsOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	COptionsSheet dlg;
	if(IDOK == dlg.DoModal(m_hWnd))
		InitUpdateTimer();
	return 0;
}

LRESULT CMainFrame::OnToolsImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CImportWizard dlg;
	if(IDOK == dlg.DoModal(m_hWnd))
	{
		LoadInitData(SaveThenLoad);
	}

	return 0;
}

bool CMainFrame::CheckUpdateInProgress(LPCTSTR msg)
{
	if(IsUpdating())
	{
		if(IDOK == MessageBox(msg, _T("GreatNews"), MB_OKCANCEL | MB_ICONQUESTION))
		{
			CWaitCursor wc;
			OnChannelStopAll(0, 0, 0);
		}
		else
		{
			return false;
		}
	}

	return true;
}

LRESULT CMainFrame::OnToolsCleanup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if(!CheckUpdateInProgress(_T("GreatNews is updating channels. Stop updating and run Cleanup Wizard?")))
		return 0;

	CCleanupWizard wiz;
	if(IDOK == wiz.DoModal(m_hWnd))
	{
		SyncUIContent();

		// reload unread count
		m_wndTreeFeeds.MarkAllRead();
		CCheckUnreadRequest* pRequest = new CCheckUnreadRequest(m_wndTreeFeeds.m_hWnd);
		pRequest->Submit();
	}

	return 0;
}

HWND CMainFrame::CreateStandardBar()
{
	HWND wndStandardBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, MY_TOOLBAR_STYLE);
	if(!wndStandardBar)
		return 0;

	CToolBarCtrl standardBar(wndStandardBar);
	CImageList imageList;
	imageList.CreateFromImage( IDB_STANDARDBAR, 16, 1, RGB(192,192,192), IMAGE_BITMAP, LR_CREATEDIBSECTION );
	standardBar.SetImageList(imageList);

	standardBar.SetExtendedStyle(
		standardBar.GetExtendedStyle() |TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);

	TBBUTTONINFO info;
	info.cbSize = sizeof(TBBUTTONINFO);
	info.dwMask = TBIF_STYLE;
	standardBar.GetButtonInfo(ID_CHANNEL_UPDATEALLCHANNEL, &info);
	info.fsStyle |= BTNS_SHOWTEXT | BTNS_AUTOSIZE;
	info.dwMask |= TBIF_TEXT;
	info.pszText = _T("Update All");
	standardBar.SetButtonInfo(ID_CHANNEL_UPDATEALLCHANNEL, &info);

	info.dwMask = TBIF_STYLE;
	standardBar.GetButtonInfo(ID_CHANNEL_MARKASREAD, &info);
	info.fsStyle |= BTNS_SHOWTEXT | BTNS_AUTOSIZE;
	info.dwMask |= TBIF_TEXT;
	info.pszText = _T("Mark Read");
	standardBar.SetButtonInfo(ID_CHANNEL_MARKASREAD, &info);

	return wndStandardBar;
}

HWND CMainFrame::CreateFindBar()
{
	HWND wndFindBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_FINDBAR, FALSE, MY_TOOLBAR_STYLE);
	if(!wndFindBar)
		return 0;

	CToolBarCtrl findBar(wndFindBar);
	findBar.SetExtendedStyle(
		findBar.GetExtendedStyle() |TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);

	CSize sizeChar = GetGUIFontSize();
	int cx = COMBO_SIZE_FIND * sizeChar.cx;

	TBBUTTONINFO tbi;
	RECT rc;
	tbi.cbSize = sizeof TBBUTTONINFO;
	tbi.dwMask = TBIF_STYLE | TBIF_SIZE;
	tbi.fsStyle = TBSTYLE_SEP;
	tbi.cx = (unsigned short)cx;
	findBar.SetButtonInfo(ID_FIND_COMBO, &tbi);
	findBar.GetItemRect(0, &rc);
	rc.bottom = TOOLBAR_COMBO_DROPLINES * sizeChar.cy;
	rc.left += 1; // slight offset from previous and next buttons.
	rc.right -= 1;
	
	// create date combobox
	DWORD dwStyle = CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL;
	m_cboFind.Create(m_hWnd, rc, _T(""), dwStyle, 0, IDC_COMBOFIND,
		g_GreatNewsConfig.GetConfigFileName(), _T("FindAutoComplete"));
	m_cboFind.SetParent(wndFindBar);
	m_cboFind.SetFont(AtlGetDefaultGuiFont());

	TBBUTTONINFO info;
	info.cbSize = sizeof(TBBUTTONINFO);
	info.dwMask = TBIF_STYLE;
	findBar.GetButtonInfo(ID_FIND_GOOGLE, &info);
	info.fsStyle |= TBSTYLE_DROPDOWN;
	findBar.SetButtonInfo(ID_FIND_GOOGLE, &info);

	return wndFindBar;
}

HWND CMainFrame::CreateFilterBar()
{
	HWND wndFilterBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_FILTERBAR, FALSE, MY_TOOLBAR_STYLE);
	if(!wndFilterBar)
		return 0;

	CToolBarCtrl filterBar(wndFilterBar);
	CImageList imageList;
	imageList.CreateFromImage( IDR_FILTERBAR, 16, 1, RGB(192,192,192), IMAGE_BITMAP, LR_CREATEDIBSECTION );
	filterBar.SetImageList(imageList);
	filterBar.SetExtendedStyle( filterBar.GetExtendedStyle() | TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_MIXEDBUTTONS);

	CSize sizeChar = GetGUIFontSize();
	int cx = COMBO_SIZE_DAY * sizeChar.cx;

	TBBUTTONINFO tbi;
	RECT rc;
	tbi.cbSize = sizeof TBBUTTONINFO;
	tbi.dwMask = TBIF_STYLE | TBIF_SIZE;
	tbi.fsStyle = TBSTYLE_SEP;
	tbi.cx = (unsigned short)cx;
	filterBar.SetButtonInfo(ID_FILTER_PLACEHOLDER_DAY, &tbi);
	filterBar.GetItemRect(0, &rc);
	rc.bottom = TOOLBAR_COMBO_DROPLINES * sizeChar.cy;
	rc.left += 1; // slight offset from previous and next buttons.
	rc.right -= 1;
	
	// create date combobox
	DWORD dwStyle = CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL;
	m_cboDateFilter.Create(m_hWnd, rc, _T(""), dwStyle, 0, IDC_COMBODATE);
	m_cboDateFilter.SetParent(wndFilterBar);
	m_cboDateFilter.SetFont(AtlGetDefaultGuiFont());

	m_cboDateFilter.AddString(_T("Today's"));
	m_cboDateFilter.AddString(_T("Since yesterday"));
	m_cboDateFilter.AddString(_T("Last 7 Days'"));
	m_cboDateFilter.AddString(_T("Last 30 Days'"));
	m_cboDateFilter.AddString(_T("All"));
	m_cboDateFilter.SetCurSel(g_GreatNewsConfig.GetDateFilterSelection());
	
	// create status combobox
	cx = COMBO_SIZE_STATUS * sizeChar.cx;
	tbi.cx = (unsigned short)cx;
	filterBar.SetButtonInfo(ID_FILTER_PLACEHOLDER_STATUS, &tbi);
	filterBar.GetItemRect(1, &rc);
	rc.bottom = TOOLBAR_COMBO_DROPLINES * sizeChar.cy;
	rc.left += 1; // slight offset from previous and next buttons.
	rc.right -= 1;
	m_cboStatusFilter.Create(m_hWnd, rc, _T(""), dwStyle, 0, IDC_COMBOSTATUS);
	m_cboStatusFilter.SetParent(wndFilterBar);
	m_cboStatusFilter.SetFont(AtlGetDefaultGuiFont());

	m_cboStatusFilter.AddString(_T("Unread"));
	m_cboStatusFilter.AddString(_T("Labelled"));
	m_cboStatusFilter.AddString(_T("All"));
	m_cboStatusFilter.SetCurSel(g_GreatNewsConfig.GetStatusFilterSelection());

	TBBUTTONINFO info;
	info.cbSize = sizeof(TBBUTTONINFO);
	info.dwMask = TBIF_STYLE;
	filterBar.GetButtonInfo(ID_FILTER_SHOWALL, &info);
	info.fsStyle |= BTNS_SHOWTEXT | BTNS_AUTOSIZE;
	info.dwMask |= TBIF_TEXT;
	info.pszText = _T("Show All");
	filterBar.SetButtonInfo(ID_FILTER_SHOWALL, &info);

	return wndFilterBar;
}

CSize CMainFrame::GetGUIFontSize()
{
	CClientDC dc(m_hWnd);
	dc.SelectFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));		
	TEXTMETRIC tm;
	dc.GetTextMetrics( &tm );

	return CSize( tm.tmAveCharWidth, tm.tmHeight + tm.tmExternalLeading);
}

LRESULT CMainFrame::OnGotoPage(WORD wNotifyCode, WORD /*wID*/, HWND)
{
	CWaitCursor wc;

	long pageNo = (short)wNotifyCode;
	if(g_GreatNewsConfig.m_nMarkReadAutomatically==0 &&  // mark read interactively?
		(pageNo == CContentGenerator::NextPage || pageNo == CContentGenerator::LastPage)) // clicked Next Page/Last Page link?
	{
		OnCommandMarkPageRead(0, 0, 0);
	}

	if(m_contentGen)
	{
		if(pageNo == CContentGenerator::NextPage	 // clicked on Next> link?
			&& m_contentGen->DisplayingLastPage()) // on the last page?
		{
			// if user clicked Next on the last page, jump to next unread channel or group if so configured
			if(g_GreatNewsConfig.m_nNextPageOpenUnread) // want to move to next unread?
			{
				CFeedTreeItem* pSelectedItem = m_wndTreeFeeds.GetSelectedFeedItem();
				if(pSelectedItem && pSelectedItem->GetType() == CFeedTreeItem::Group)
					PostMessage(WM_COMMAND, MAKELONG(ID_VIEW_NEXTUNREADGROUP, 8));
				else
					PostMessage(WM_COMMAND, MAKELONG(ID_VIEW_NEXTUNREADNEWS, 8));
			}
			else
			{
				::MessageBeep(MB_OK);
			}
		}
		else
		{
			m_host.GotoPage(pageNo);
			m_wndListNews.SetTopItem(m_contentGen->GetFirstItemOnPage());
		}
	}

	return 0;
}

LRESULT CMainFrame::OnOpenChannel(WORD wNotifyCode, WORD /*wID*/, HWND h)
{
	ULONG_PTR channelID = (ULONG_PTR)h;
	if(channelID != 0)
		GotoChannel(channelID, (wNotifyCode != 0));

	return 0;
}

void CMainFrame::ProcessStatusBarIcon(std::set<CString>& items, UINT baseCmdID, CPoint& pt)
{
	CMenu menu;
	menu.CreatePopupMenu();

	int iMenu=0;
	for(std::set<CString>::iterator it=items.begin(); iMenu<15 && it!=items.end();++it)
	{
		menu.AppendMenu(MF_STRING | MF_BYPOSITION,
			baseCmdID+iMenu,
			(LPCTSTR)(*it));
		iMenu++;
	}
	menu.TrackPopupMenuEx(TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RIGHTBUTTON|TPM_HORIZONTAL, pt.x, pt.y, m_hWnd);
}

LRESULT CMainFrame::OnRssIconClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	std::map<CString, CString> discoveredFeeds;
	m_host.GetDiscoveredFeeds(discoveredFeeds);
	CPoint rssPt = m_statusBar.GetMenuPos(ID_RSS_INDICATOR);
	std::set<CString> feedTitles;
	for(std::map<CString, CString>::iterator it=discoveredFeeds.begin();
		it!=discoveredFeeds.end(); ++it)
	{
		feedTitles.insert(it->first);
	}

	ProcessStatusBarIcon(feedTitles, ID_IE_RSSFEED_FIRST, rssPt);

	return 0;
}

LRESULT CMainFrame::OnPopupIconClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	std::set<CString>& urls = m_host.GetBlockedURLs();
	CPoint pt = m_statusBar.GetMenuPos(ID_POPUP_INDICATOR);
	ProcessStatusBarIcon(urls, ID_IE_BLOCKEDPOPUP_FIRST, pt);

	return 0;
}

void CMainFrame::OnGotoBlockedUrl(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	int n = wID-ID_IE_BLOCKEDPOPUP_FIRST;
	std::set<CString>& urls = m_host.GetBlockedURLs();
	if(n>=0 && n<(int)urls.size())
	{
		std::set<CString>::iterator it = urls.begin();
		std::advance(it, n);

		CString url = *it;
		m_host.Navigate(url);
	}
}


void CMainFrame::OnAddDiscoveredFeed(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	int n = wID-ID_IE_RSSFEED_FIRST;
	std::map<CString, CString> discoveredFeeds;
	m_host.GetDiscoveredFeeds(discoveredFeeds);
	if(n>=0 && n<(int)discoveredFeeds.size())
	{
		std::map<CString, CString>::iterator it = discoveredFeeds.begin();
		std::advance(it, n);
		CString url = it->second;

		if(CGNUtil::IsOpmlUrl(url))
		{
			CGNClipBoard clipboard;
			if(clipboard.Open(m_hWnd))
			{
				USES_CONVERSION;
				clipboard.SetTextData(T2A((LPTSTR)(LPCTSTR)url));
				clipboard.SetUnicodeTextData(T2W((LPTSTR)(LPCTSTR)url));

				PostMessage(WM_COMMAND, ID_TOOLS_IMPORT, 0);
			}
		}
		else
		{
			ULONG_PTR nChannelID = CNewsFeed::GetIdFromXmlUrl(url);
			if(nChannelID>0)
			{
				// new channel
				if(IDYES == MessageBox(ResManagerPtr->GetString(IDS_OPENEXISTINGCHANNEL),
					_T("GreatNews"), MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1))
				{
					PostMessage(WM_COMMAND, ID_IE_OPENCHANNEL, (LPARAM)nChannelID);
				}
			}
			else
			{
				// new channel
				FeedGroupPtr feedGroup = m_wndTreeFeeds.GetDefaultFeedGroup();
				AddChannel((UINT)IDS_ADDCHANNELWIZARD, url, feedGroup!=NULL ? feedGroup->m_id : 0);
			}
		}
	}
}


LRESULT CMainFrame::OnPopupBlocked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	std::set<CString>& v = m_host.GetBlockedURLs();

	m_statusBar.EnablePopupIndicator(!v.empty());

	return 0;
}

LRESULT CMainFrame::OnFoundFeed(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	std::map<CString, CString> v;
	m_host.GetDiscoveredFeeds(v);
	m_statusBar.EnableRssIndicator(!v.empty());
	return 0;
}

LRESULT CMainFrame::OnSetStatusText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_statusBar.SetText(0, m_host.m_statusText);
	
	return 0;
}

LRESULT CMainFrame::OnNavigateXML(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if(m_host.m_navigateXmlURL.GetLength()>0)
	{
		CString title(MAKEINTRESOURCE(IDS_ADDCHANNELWIZARD));
		AddChannel((LPCTSTR)title, m_host.m_navigateXmlURL, 0);
	}

	return 0;
}


LRESULT CMainFrame::OnItemPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CloseNewsList(false);
	m_wndListNews.PrevItem();

	return 0;
}

LRESULT CMainFrame::OnItemNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CloseNewsList(false);
	m_wndListNews.NextItem();

	return 0;
}

LRESULT CMainFrame::OnPreviousPage(WORD wNotifyCode, WORD , HWND /*hWndCtl*/)
{
	PostMessage(WM_COMMAND, MAKELONG(ID_IE_GOTO_PAGE, CContentGenerator::PrevPage), 0);
	return 0;
}

LRESULT CMainFrame::OnNextPage(WORD wNotifyCode, WORD , HWND /*hWndCtl*/)
{
	PostMessage(WM_COMMAND, MAKELONG(ID_IE_GOTO_PAGE, CContentGenerator::NextPage), 0);
	return 0;
}


LRESULT CMainFrame::OnCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::PostMessage(::GetFocus(), WM_CUT, 0, 0);
	return TRUE;
}

LRESULT CMainFrame::OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::PostMessage(::GetFocus(), WM_COPY, 0, 0);
	return TRUE;
}

LRESULT CMainFrame::OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	::PostMessage(::GetFocus(), WM_PASTE, 0, 0);
	return TRUE;
}

LRESULT CMainFrame::OnFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_host.ShowFindDialog();
	return TRUE;
}

LRESULT CMainFrame::OnFilterShowAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CFeedTreeItem* pTreeItem = m_wndTreeFeeds.GetSelectedFeedItem();
	if(pTreeItem)
	{
		BatchContentGeneratorPtr gen = pTreeItem->GetContentGenerator();
		if(gen)
		{
			m_wndListNews.GetOrderBy(&m_showAllNewsFilter);
			gen->SetNewsFilter(&m_showAllNewsFilter);
			gen->InitGenerator(g_GreatNewsConfig.m_nPageSize);

			m_wndListNews.SetNewsSource(gen);
			m_host.SetContent(gen);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnFindEndEdit(int idCtrl, LPNMHDR pnmh, BOOL &bHandled)
{
	PNMCBEENDEDIT pInfo = (PNMCBEENDEDIT)pnmh;
	if(pInfo->iWhy == CBENF_RETURN)
		PostMessage(WM_COMMAND, ID_EDIT_FINDNEXT, 0);
	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnToolbarButtonDropdown(int idCtrl, LPNMHDR pnmh, BOOL &bHandled) 
{
	NMTOOLBAR *pnmTB = (NMTOOLBAR *) pnmh;

	if (pnmTB ->iItem == ID_FIND_GOOGLE)
	{
		HWND hWndCtl = pnmh->hwndFrom;

		// Create the menu
		HMENU hMenu = ::CreatePopupMenu ();
		for(int i=1; i<g_GreatNewsConfig.m_searchTargets.GetSize();++i) // i starts from 1 because the first one is google
		{
			::InsertMenu (hMenu, i-1, MF_STRING | MF_BYPOSITION, 
				ID_FIND_GOOGLE + i, 
				g_GreatNewsConfig.m_searchTargets[i]);
		}

		CPoint pt(pnmTB->rcButton.left, pnmTB->rcButton.bottom);
		::ClientToScreen(pnmh->hwndFrom, &pt);

		::TrackPopupMenuEx (hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, 
			pt.x, pt.y, m_hWnd, NULL);

		::DestroyMenu (hMenu);

		return TBDDRET_DEFAULT;
	}
	else
		return TBDDRET_TREATPRESSED;
}


void CMainFrame::OnInternetSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	int n = wID-ID_FIND_GOOGLE;
	if(n>=0 && n<g_GreatNewsConfig.m_searchTargetURLs.GetSize())
	{
		CString searchText;
		m_cboFind.GetWindowText(searchText);
		searchText.Replace(_T(' '), _T('+'));

		CString searchURL;
		searchURL.Format(g_GreatNewsConfig.m_searchTargetURLs[n], (LPCTSTR)searchText);

		m_host.Navigate(searchURL, 0, CBrowserHost::NewTabForeground);
	}
}


LRESULT CMainFrame::OnFindChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFindChannelDlg dlg;
	m_wndTreeFeeds.GetAllChannels(dlg.m_newsfeeds);
	if(dlg.DoModal() == IDOK)
		GotoChannel(dlg.m_selectedFeedId);				
	return 1;
}

LRESULT CMainFrame::OnFindNewsItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_cboFind.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(g_GreatNewsConfig.m_bSearchAutoOpenNewsList)
		CloseNewsList(false);

	DoFindNext();
	return 0;
}

void CMainFrame::GotoChannel(ULONG_PTR nChannelId, bool bAutoUpdate)
{
	CTreeItem channelNode = m_wndTreeFeeds.FindNodeByIdType(nChannelId, CFeedTreeItem::Channel);
	if(channelNode)
	{
		m_wndTreeFeeds.SelectItem(channelNode);
		SyncUIContent();

		if(bAutoUpdate)
			PostMessage(WM_COMMAND, ID_CHANNEL_UPDATE, 0);
	}
}

void CMainFrame::DoFindNext()
{
	if(m_contentGen == NULL || m_contentGen->m_vectNewsIDs.empty())
	{
		MessageBox(ResManagerPtr->GetString(IDS_EMPTYNEWSLIST),
			_T("GreatNews"), MB_OK|MB_ICONINFORMATION);	
		return;
	}

	CString findText;
	m_cboFind.GetWindowText(findText);
	findText.Trim();
	m_cboFind.AddString(findText);

	// start from the selection
	int nStart = m_wndListNews.GetSelectedIndex();

	CSearchingDlg dlg;
	dlg.SetSearchParam(m_contentGen, findText, 
		m_nLastSearchResult==nStart ? nStart+1 : nStart);
	if(IDOK == dlg.DoModal())
	{
		m_nLastSearchResult = dlg.m_itemFound;
		m_host.SetHighlightAfterNavigate(dlg.m_matchedString);
		m_wndListNews.SelectItem(m_nLastSearchResult);
	}

	m_cboFind.SetFocus();
}

LRESULT CMainFrame::OnViewNewTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_host.NewTab(_T("about:blank"), false);
	m_host.FocusOnAddress();
	return 0; 
}

LRESULT CMainFrame::OnViewChannelBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_wndVertSplit.ToggleCollapse();
	return 0; 
}

void CMainFrame::CloseChannelBar(bool bClose)
{
	m_wndVertSplit.Collapse(bClose);
}

void CMainFrame::CloseNewsList(bool bClose)
{
	m_wndHorzSplit.Collapse(bClose);
}

LRESULT CMainFrame::OnViewNewsList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_wndHorzSplit.ToggleCollapse();

	return 0;
}

void CMainFrame::openNextMarkRead(CFeedTreeItem::Type nodeType)
{
	// mark channel/group as read
	CFeedTreeItem* pSelectedItem = m_wndTreeFeeds.GetSelectedFeedItem();
	if(pSelectedItem
		&& g_GreatNewsConfig.m_bOpenNextAutoMarkLastChannelAsRead  // user wants to mark unread
		&& m_wndTreeFeeds.GetSelectedItem() != m_wndTreeFeeds.GetRootItem() // not selected root
		&& pSelectedItem->GetUnreadCount()) // and we do have unread news in current selection
	{
		if(pSelectedItem->GetType() == nodeType)
			OnChannelMarkAsRead(0, 0, 0);
	}
}

void CMainFrame::internalOpenNextUnread(CTreeItem unreadNode)
{
	if(unreadNode == NULL)
		return;

	// move to next channel/group
	if(g_GreatNewsConfig.m_bAutoCloseNewsList)
		CloseNewsList(true);
	if(g_GreatNewsConfig.m_bAutoSetFilterToUnread)
		SetUnreadFilter();

	if(!g_GreatNewsConfig.m_bNoAutoGroupCollapse)
		m_wndTreeFeeds.CollapseToGroup();

	m_wndTreeFeeds.SelectItem(unreadNode);

	SyncUIContent();

	return;
}

LRESULT CMainFrame::OnNextUnreadGroup(WORD wNotifyCode, WORD , HWND /*hWndCtl*/)
{
	CGNLockWindowUpdate lockWnd(m_hWnd);

	// try move up to goup level before update
	CFeedTreeItem* pItem = m_wndTreeFeeds.GetSelectedFeedItem();
	if(pItem && pItem->GetType() == CFeedTreeItem::Channel)
	{
		CTreeItem parent = m_wndTreeFeeds.GetSelectedItem().GetParent();
		if(parent && parent !=m_wndTreeFeeds.GetRootItem())
			parent.Select();
	}
	
	if((wNotifyCode & 8) != 0)
	{
		 // when 4th bit of wNotifyCode is 1, don't mark read no matter what settings are
	}
	else
	{
		openNextMarkRead(CFeedTreeItem::Group);
	}

	CTreeItem unreadGroup = m_wndTreeFeeds.GetUnreadNode(CFeedTreeItem::Group);
	if(unreadGroup)
	{
		internalOpenNextUnread(unreadGroup);
		if(!g_GreatNewsConfig.m_bNoAutoGroupCollapse)
			unreadGroup.Expand();
	}
	else
	{
		PostMessage(WM_COMMAND, ID_VIEW_NEXTUNREADNEWS); // if there's no unread group, try jump to unread channel at root level
	}

	return 0;
}

LRESULT CMainFrame::OnNextUnreadNewsChannel(WORD wNotifyCode, WORD, HWND /*hWndCtl*/)
{
	CGNLockWindowUpdate lockWnd(m_hWnd);

	if((wNotifyCode & 8) != 0)
	{
		// when 4th bit of wNotifyCode is 1, don't mark read no matter what settings are
	}
	else
	{
		openNextMarkRead(CFeedTreeItem::Channel);
	}

	CTreeItem unreadChannel = m_wndTreeFeeds.GetUnreadNode(CFeedTreeItem::Channel);
	if(unreadChannel)
	{
		internalOpenNextUnread(unreadChannel);
	}
	else
	{
		// no unread channel found, jump to welcome page
		CTreeItem item = m_wndTreeFeeds.GetRootItem();
		item.Select();
		PostMessage(WM_COMMAND, CMD_ID_REFRESH);
	}

	return 0;
}

LRESULT CMainFrame::OnViewMaxBrowser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	// determine we want to max brwoser or restore it
	bool bMax = (!m_wndHorzSplit.IsCollapsed() || 
				 !m_wndVertSplit.IsCollapsed());

	CloseChannelBar(bMax);
	CloseNewsList(bMax);

	return 0;
}

void CMainFrame::SetUnreadFilter()
{
	m_cboStatusFilter.SelectString(-1, _T("Unread"));
	GetNewsFilter();
}


void CMainFrame::OnEditFindComboBox(UINT code, int id, HWND)
{
	if(m_cboFind.GetWindowTextLength())
	{
		UIEnable(ID_EDIT_FINDNEXT,true);
		UIEnable(ID_FIND_GOOGLE, true);
	}
	else
	{
		UIEnable(ID_EDIT_FINDNEXT,false);
		UIEnable(ID_FIND_GOOGLE, false);
	}
}


void CMainFrame::ShowStatistics(LPCTSTR statTitle, NewsFeedVector& statFeeds)
{
	CString htmlFrag;

	// display top 10 (fill in blanks if less than 10)
	NewsFeedVector::iterator it=statFeeds.begin();
	size_t cnt = statFeeds.size() > 10 ? statFeeds.size() : 10;
	for(size_t i=1; i<=cnt; ++i)
	{
		if(it!=statFeeds.end())
		{
			NewsFeedPtr feed = *it;
			htmlFrag.AppendFormat(_T("<tr><td>%d</td><td><a href=\"http://localhost/#GreatNewsTag_GotoChannel_%d\" onMouseOver=\"window.status='Goto feed'; return true;\">%s</a></td><td>%d</td><td>%s</td></tr>\n"),
				i,
				feed->m_id,
				(LPCTSTR)feed->m_title,
				feed->m_usage,
				feed->m_lastModified.GetTime() ? (LPCTSTR)CGMTimeHelper::FormatDisplayDate(feed->m_lastModified) : _T("&nbsp;")
				);

			it++;
		}
		else
		{
			htmlFrag.Append(_T("<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n"));
		}
	}

	CString statTemplateFileName = g_GreatNewsConfig.GetMediaDir()+"\\Statistics.html";
	CString statTemplate;
	if(!CGNUtil::ReadStringFromFile(statTemplateFileName, statTemplate))
	{
		MessageBox(_T("Cannot load statistics templates"),_T("GreatNews"));
		return;
	}

	statTemplate.Replace(_T("%StatisticsTitle%"),statTitle);
	statTemplate.Replace(_T("%StatisticsContent%"),htmlFrag);

	CString statFileName = CGNUtil::GetTempPathName()+_T("~statistics.html");
	if(!CGNUtil::WriteToFile(statFileName, statTemplate))
	{
		MessageBox(_T("Cannot write statistics"),_T("GreatNews"));
		return;
	}

	m_host.Navigate(statFileName);
}

void CMainFrame::ShowActivityStatistics(LPCTSTR statTitle, NewsFeedVector& statFeeds)
{
	CString htmlFrag;

	// display top 10 (fill in blanks if less than 10)
	NewsFeedVector::iterator it=statFeeds.begin();
	size_t cnt = statFeeds.size() > 10 ? statFeeds.size() : 10;
	for(size_t i=1; i<=cnt; ++i)
	{
		if(it!=statFeeds.end())
		{
			NewsFeedPtr feed = *it;
			htmlFrag.AppendFormat(_T("<tr><td>%d</td><td><a href=\"http://localhost/#GreatNewsTag_GotoChannel_%d\" onMouseOver=\"window.status='Goto Channel [%s]'; return true;\">%s</a></td><td>%d</td><td>%s</td></tr>\n"),
				i,
				feed->m_id,
				(LPCTSTR)feed->m_title,
				(LPCTSTR)feed->m_title, 
				feed->m_numOfNewItemsInDays,
				feed->m_lastModified.GetTime() ? (LPCTSTR)CGMTimeHelper::FormatDisplayDate(feed->m_lastModified) : _T("&nbsp;")
				);
			it++;
		}
		else
		{
			htmlFrag.Append(_T("<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n"));
		}
	}

	CString statTemplateFileName = g_GreatNewsConfig.GetMediaDir()+"\\Activity.html";
	CString statTemplate;
	if(!CGNUtil::ReadStringFromFile(statTemplateFileName, statTemplate))
	{
		MessageBox(_T("Cannot load statistics templates"),_T("GreatNews"));
		return;
	}

	statTemplate.Replace(_T("%StatisticsTitle%"),statTitle);
	statTemplate.Replace(_T("%StatisticsContent%"),htmlFrag);

	CString statFileName = CGNUtil::GetTempPathName()+_T("~statistics.html");
	if(!CGNUtil::WriteToFile(statFileName, statTemplate))
	{
		MessageBox(_T("Cannot write statistics"),_T("GreatNews"));
		return;
	}

	m_host.Navigate(statFileName);
}

LRESULT CMainFrame::OnCommandMarkItemRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	int itemIndex = (int)h;
	NewsItemPtr item = m_wndListNews.GetNewsItemByIndex(itemIndex);
	if(item != NULL && item->IsUnread())
	{
		item->SetRead();
		CTreeItem channelNode = m_wndTreeFeeds.FindNodeByIdType(item->m_feedID, CFeedTreeItem::Channel);
		if(channelNode)
			m_wndTreeFeeds.AdjustUnreadCounts(channelNode, -1);
		ClearTrayTooltip();
	}

	return 0;
}

LRESULT CMainFrame::OnCommandMarkPageRead(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	ContentGeneratorPtr spGen = m_host.GetContentGenerator();
	if(spGen == NULL || !spGen->HasUnreadItem())
		return 0;

	// mark unread item read
	CNewsItem::MarkItemsRead(spGen->m_unreadItemIDs);
	ClearTrayTooltip();

	// update channel counts
	std::map<ULONG_PTR, int>& mapsChannelsUnreadCounts = m_contentGen->m_channelUnreadCounts;
	for(std::map<ULONG_PTR, int>::iterator it = mapsChannelsUnreadCounts.begin();
		it != mapsChannelsUnreadCounts.end() ; ++it)
	{
		CTreeItem item = m_wndTreeFeeds.FindNodeByIdType(it->first, CFeedTreeItem::Channel);
		if(item)
			m_wndTreeFeeds.AdjustUnreadCounts(item, -it->second);
	}

	m_wndListNews.Refresh();

	return 0;
}

void CMainFrame::CheckVersionTime()
{
	return;
	CTime time = CTime::GetCurrentTime();

	if(time > CTime(2017,6,30,23,59,59) && m_remindToUpgrade < 3)
	{
		m_remindToUpgrade++;
		MessageBox(ResManagerPtr->GetString(IDS_UPDATETONEWVERSION),
			_T("GreatNews"),
			MB_OK|MB_ICONINFORMATION);
	}
}

void CMainFrame::OnViewLanguage(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	int n = wID-ID_VIEW_LANGUAGE0;
	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->SetLanguage(n);

	ApplyLanguage();
}

void CMainFrame::OnReadingPane(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	int n = wID-ID_READINGPANE_BOTTOM;
	if(n != g_GreatNewsConfig.m_nReadingPanePos)
	{
		g_GreatNewsConfig.m_nReadingPanePos = n;
		m_wndHorzSplit.ToggleSplitOrientation();
	}
}

void CMainFrame::ApplyLanguage()
{
	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();

	HMENU hMenu = ::LoadMenu(ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
	pResMngr->ApplyLanguage(hMenu, 2, ID_VIEW_LANGUAGE0);
	m_CmdBar.AttachMenu(hMenu);

	pResMngr->ApplyLanguageToCombo(m_cboDateFilter, _T("DateFilter"));
	pResMngr->ApplyLanguageToCombo(m_cboStatusFilter, _T("StatusFilter"));
	pResMngr->ApplyLanguageToRebar(m_hWndToolBar);
	SizeSimpleReBarBands();

	DisplayChannelCount();

	m_host.ApplyLanguage();
}

void CMainFrame::AutoUpdate()
{
	NewsFeedVector feeds;
	m_wndTreeFeeds.GetAllChannels(feeds);
	bool bAutoUpdateChannel = g_GreatNewsConfig.m_bAutoUpdateChannels;

	for(NewsFeedVector::iterator it = feeds.begin(); it != feeds.end(); ++it)
	{
		NewsFeedPtr& feed = *it;

		INT_PTR channelUpdateFreq = feed->m_updateFreq;

		// if auto update channel was turned off, and the feed has no channel level setting, we can skip this one
		if(!bAutoUpdateChannel && !channelUpdateFreq)
			continue;

		int updateFreqSeconds = (channelUpdateFreq ?	// does the channel has special update freq?
				CNewsFeed::GetChannelAutoUpdateFreqSec(channelUpdateFreq) // yes, use per channel setting
			:	g_GreatNewsConfig.GetAutoUpdateFreqSec()); // no, use default setting

		if(!feed->UpdateDue(updateFreqSeconds)	// update not due yet?
			|| m_wndTreeFeeds.IsChannelUpdating(feed->m_id) // already updating?
			|| m_wndTreeFeeds.IsFeedDisabled(feed)) // feed disabled?
			continue; // skip this feed

		feed->m_bBypassCache = true; // never use cache when auto updating
		SubmitUpdate(feed, false);
	}

	InitUpdateProgress();
}

LRESULT CMainFrame::OnToggleReadItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	try
	{
		int itemID = (int)h;
		NewsItemPtr pItem = CNewsItemCache::GetNewsItem(itemID);

		// toggle read/unread
		if(pItem->IsUnread())
			pItem->SetRead();
		else
			pItem->SetUnread();

		m_wndTreeFeeds.AdjustUnreadCounts(pItem, (pItem->IsUnread()? 1 : -1));

		// refresh the UI
		if(m_wndListNews.IsWindowVisible())
			m_wndListNews.Refresh();
		m_host.RefreshItem(itemID);
	}
	CATCH_ALL_ERROR()

	return 0;

}
LRESULT CMainFrame::OnCommandLabelItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	try
	{
		DWORD dwPos = GetMessagePos();
		CPoint spt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );

		int itemID = (int)h;
		NewsItemPtr pItem = CNewsItemCache::GetNewsItem(itemID);
		std::vector<ULONG_PTR> itemTags;
		pItem->GetItemTags(itemTags);
		
		TagVector allTags;
		CTag::GetAllTags(allTags);

		// build the label popup menu
		CMenu menu;
		menu.CreatePopupMenu();
		int iMenu=0;
		for(TagVector::iterator it = allTags.begin(); it != allTags.end(); ++it)
		{
			TagPtr& tag = *it;
			CString menuTitle = _T("Label: ")+tag->m_name;
			menu.AppendMenu(MF_STRING | MF_BYPOSITION,
				ID_LABEL_TAGGLE_FIRST + iMenu,
				(LPCTSTR)menuTitle);

			if(std::find(itemTags.begin(), itemTags.end(), (ULONG_PTR)tag->m_id)!=itemTags.end())
				menu.CheckMenuItem(ID_LABEL_TAGGLE_FIRST + iMenu, MF_CHECKED);

			iMenu++;
		}
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_NEWSLIST_ADDNEWLABEL, _T("New Label..."));
		
		int nCmd = menu.TrackPopupMenuEx(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, 
			spt.x, spt.y, m_hWnd, NULL);

		if(!nCmd)  // something's wrong?
			return 0;


		if(nCmd == ID_NEWSLIST_ADDNEWLABEL)
		{
			PostMessage(WM_COMMAND, ID_LABEL_ADD);
			return 0;
		}

		int index = nCmd - ID_LABEL_TAGGLE_FIRST;
		TagPtr tag = allTags[index];
		int count = 0;
		if(std::find(itemTags.begin(), itemTags.end(), (ULONG_PTR)tag->m_id)!=itemTags.end())
		{
			pItem->RemoveTag(tag->m_id);
			count = -1;
		}
		else
		{
			pItem->AddTag(tag->m_id);
			count = 1;
		}
		
		// adjust tree node counter
		CTreeItem treeNode = m_wndTreeFeeds.FindNodeByIdType(tag->m_id, CFeedTreeItem::Tag);
		if(treeNode)
			m_wndTreeFeeds.AdjustUnreadCounts(treeNode, count);

		// try to modify HTML page to reflect tags
		m_host.ChangeItemLabelStyle(pItem);

		m_wndListNews.Refresh();
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnCommandBlogThis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	try
	{
		DWORD dwPos = GetMessagePos();
		CPoint spt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );


		// build the label popup menu
		CMenu menu;
		m_wndListNews.BuildBlogThisPopup(menu);

		int nCmd = menu.TrackPopupMenuEx(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, 
			spt.x, spt.y, m_hWnd, NULL);

		if(!nCmd)
			return 0;

		if(nCmd == CMD_ID_CONFIGBLOGTHIS)
		{
			PostMessage(WM_COMMAND, CMD_ID_CONFIGBLOGTHIS);
			return 0;
		}
		else
		{
			int itemID = (int)h;
			PostMessage(WM_COMMAND, nCmd, itemID);
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnCommandEmailItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	try
	{
		int itemID = (int)h;
		NewsItemPtr pItem = CNewsItemCache::GetNewsItem(itemID);

		CString mailto;
		mailto.Format(_T("mailto:?subject=%s&body=%s"),
			(LPCTSTR)CGNUtil::EscapeUrl(pItem->GetTitle()),
			(LPCTSTR)CGNUtil::EscapeUrl(pItem->GetLink()));

		::ShellExecute(m_hWnd, _T("open"), mailto, NULL, NULL, SW_SHOW);

	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnCommandAddDelicious(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	try
	{
		int itemID = (int)h;
		NewsItemPtr pItem = CNewsItemCache::GetNewsItem(itemID);

		CString featureUrl;
		featureUrl.Format(_T("http://del.icio.us/post?title=%s&url=%s"),
			(LPCTSTR)CGNUtil::EscapeUrl(pItem->GetTitle()),
			(LPCTSTR)CGNUtil::EscapeUrl(pItem->GetLink()));

		m_host.NewTab(featureUrl, false);

	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnCommandAddFurl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	try
	{
		int itemID = (int)h;
		NewsItemPtr pItem = CNewsItemCache::GetNewsItem(itemID);

		CString featureUrl;
		featureUrl.Format(_T("http://www.furl.net/storeIt.jsp?t=%s&u=%s"),
			(LPCTSTR)CGNUtil::EscapeUrl(pItem->GetTitle()),
			(LPCTSTR)CGNUtil::EscapeUrl(pItem->GetLink()));

		m_host.NewTab(featureUrl, false);

	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnCommandTrackComments(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	try
	{
		CWaitCursor wc;

		int itemID = (int)h;
		NewsItemPtr pItem = CNewsItemCache::GetNewsItem(itemID);

		if(!pItem->m_bTrackComments)
		{
			if(pItem->m_commentFeedURL.IsEmpty())
				return 0; // cannot track w/o comment feed

			// enable comment tracking for this item
			NewsFeedPtr commentFeed = CNewsFeed::AddCommentFeed(pItem);
			if(commentFeed == NULL)
				return 0; // something is wrong

			pItem->SetTrackComment(true);
			SubmitUpdate(commentFeed, true);

		}
		else
		{
			NewsFeedPtr commentFeed = CNewsFeedCache::GetCommentFeed(itemID);
			if(commentFeed != NULL)
			{
				commentFeed->Delete();
			}
			pItem->SetTrackComment(false);
			m_host.RefreshItem(itemID);
		}

		// now try to refresh UI
		m_host.ChangeItemTrackCommentText(pItem);

	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnConfigBlogThis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CConfigBlogThisDlg dlg;
	dlg.DoModal();

	return 0;
}


void CMainFrame::OnBlogThisTool(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	try
	{
		int itemID = (int)hWndCtl;
		NewsItemPtr newsItem = CNewsItemCache::GetNewsItem(itemID);

		// get user selection
		int index = wID - CMD_ID_BLOGTHISTOOL0;
		BlogToolVector allBlogTools;
		CBlogTool::GetAllBlogTools(allBlogTools);
		BlogToolPtr tool = allBlogTools[index];

		// find the tool type
		BlogToolTypePtr toolType = NULL;
		BlogToolTypeVector allBlogToolTypes;
		CBlogToolType::GetAllBlogToolTyes(allBlogToolTypes, g_GreatNewsConfig.GetPluginDir());
		for(BlogToolTypeVector::iterator it = allBlogToolTypes.begin(); it != allBlogToolTypes.end(); ++it)
		{
			BlogToolTypePtr t = *it;
			if(t->m_name.CompareNoCase(tool->m_type)==0)
			{
				toolType = *it;
				break;
			}
		}
		if(toolType == NULL)
		{
			MessageBox(ResManagerPtr->GetString(IDS_CANTFINDBLOGTOOL),_T("GreatNews"));
			return;
		}

		// invoke blog this tool
		if(toolType->m_type == CBlogToolType::Plugin || toolType->m_type == CBlogToolType::GenericExternal)
		{
			toolType->BlogThis(newsItem, tool->m_url); // the url is the parameter sent to plugin
		}
		else
		{
			CString url = tool->m_url;
			if(url.GetLength())
			{
				url.Replace(_T("%TITLE%"), CGNUtil::EscapeUrl(newsItem->GetTitle()));
				url.Replace(_T("%AUTHOR%"),CGNUtil::EscapeUrl(newsItem->m_author)); 
				url.Replace(_T("%LINK%"),CGNUtil::EscapeUrl(newsItem->GetLink())); 
				url.Replace(_T("%TEXT%"),CGNUtil::EscapeUrl(newsItem->m_description)); 
				m_host.Navigate(url, 0, CBrowserHost::NewTabForeground);
			}
		}
	}
	CATCH_ALL_ERROR()
}


LRESULT CMainFrame::OnNewItemArrived(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	NewsItemVector* pItems = (NewsItemVector*)lParam;
	if(m_wndNewsPopup)
	{
		m_wndNewsPopup.Show(pItems, g_GreatNewsConfig.m_nNewItemNotifySeconds);
	}
	else
	{
		delete pItems;
	}

	return 0;
}

LRESULT CMainFrame::OnGlobalSearchItemFound(UINT /*uMsg*/, WPARAM itemID, LPARAM lParam, BOOL& /*bHandled*/)
{
	NewsItemPtr pItem = CNewsItemCache::GetNewsItem(itemID);
	if(pItem)
	{
		GotoChannel(pItem->m_feedID);

		LPTSTR p = (LPTSTR)lParam;
		if(p)
		{
			m_host.SetHighlightAfterNavigate(p);
			delete p;
		}

		m_host.SetContent(pItem);
	}

	return 0;
}

LRESULT CMainFrame::OnShowNewsItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CUIUtil::ShowMyWindow(m_hWnd);

	NewsItemPtr pItem = CNewsItemCache::GetNewsItem(lParam);
	if(pItem)
	{
		GotoChannel(pItem->m_feedID);
		m_host.SetContent(pItem);
	}

	return 0;
}

void CMainFrame::ClearTrayTooltip()
{
	m_nNewHeadlines=0;
	m_nChannelSucceeded=0;
	m_nChannelFailed=0;

	CString tip;
	tip.Format(_T("No new message retrieved since you last read."));
	SetTooltipText(tip);
}

bool CMainFrame::HasFocus()
{
	HWND child = GetFocus();
	return (child && (child==m_hWnd || IsChild(child)));
}

CString CMainFrame::FormatTrayTooltipMessage()
{
	CString tip;

	if(m_nNewHeadlines)
		tip.Format(_T("%d new messages retrieved since you last read."), m_nNewHeadlines);
	else
		tip.Format(_T("No new messages retrieved since you last read."), m_nNewHeadlines);

	INT_PTR totalUnread = GetTotalUnread();
	if(totalUnread)
		tip.AppendFormat(_T(" (%d unread in total)"), totalUnread);

	if(m_nChannelFailed)
	{
		tip.Append(_T("\n"));
		tip.AppendFormat(ResManagerPtr->GetString(IDS_UPDATEFAILEDCOUNT), m_nChannelFailed);
	}

	return tip;
}

INT_PTR CMainFrame::GetTotalUnread()
{
	CTreeItem item = m_wndTreeFeeds.GetRootItem();
	if(!item)
		return 0;

	CRootGroupFeedTreeItem* pRootItem = dynamic_cast<CRootGroupFeedTreeItem*>((CFeedTreeItem*)item.GetData());
  return pRootItem ? pRootItem->GetUnreadCount() : 0;
}

BOOL CMainFrame::HandleTabKey(MSG* pMsg)
{
	HWND hWndFocus = GetFocus();

	if(GetKeyState( VK_SHIFT ) & KEYDOWN_MASK)
	{
		if(m_wndTreeContainer.IsChild(hWndFocus))
		{
			m_host.SetFocusToReadingPane();
			return TRUE;
		}
		else if(m_host.ReadingPaneHasFocus(hWndFocus))
		{
			if(m_host.CanTabOff())
			{
				m_host.SetFocusToAddress();
				return TRUE;
			}
			else
			{
				m_host.PreTranslateMessage(pMsg);
				return TRUE;
			}
		}		
		else if(m_host.AddressHasFocus(hWndFocus))
		{
			if(m_wndListNews.IsWindowVisible())
			{
				m_wndListNews.SetFocus();
			}
			else
			{
				m_wndTreeFeeds.SetFocus();
			}
			return TRUE;
		}
		else if(m_wndListNews==hWndFocus || m_wndListNews.IsChild(hWndFocus))
		{
			m_wndTreeFeeds.SetFocus();
			return TRUE;
		}
		else if(m_cboFind.IsChild(hWndFocus))
		{
			m_wndTreeFeeds.SetFocus();
			return TRUE;
		}
		else // we don't know who got the focus, set to tree anyway
		{
			m_wndTreeFeeds.SetFocus();
			return TRUE;
		}
	}
	else
	{
		if(m_wndTreeContainer.IsChild(hWndFocus))
		{
			m_wndListNews.SetFocus();
			return TRUE;
		}
		else if(m_cboFind.IsChild(hWndFocus))
		{
			if(m_wndListNews.IsWindowVisible())
			{
				m_wndListNews.SetFocus();
			}
			else
			{
				m_host.SetFocusToAddress();
			}
			return TRUE;
		}
		else if(m_wndListNews==hWndFocus || m_wndListNews.IsChild(hWndFocus))
		{
			m_host.SetFocusToAddress();
			return TRUE;
		}
		else if(m_host.AddressHasFocus(hWndFocus))
		{
			m_host.SetFocusToReadingPane();
			return TRUE;
		}
		else if(m_host.ReadingPaneHasFocus(hWndFocus))
		{
			if(m_host.CanTabOff())
			{
				m_wndTreeFeeds.SetFocus();
				return TRUE;
			}
			else
			{
				m_host.PreTranslateMessage(pMsg);
				return TRUE;
			}
		}
		else // we don't know who got the focus, set to tree anyway
		{
			m_wndTreeFeeds.SetFocus();
			return TRUE;
		}
	}

	return FALSE;
}


LRESULT CMainFrame::OnGlobalSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_wndGlobalSearch.IsWindow())
	{
		if(!m_wndGlobalSearch.IsWindowVisible())
			m_wndGlobalSearch.ShowWindow(SW_SHOW);
		m_wndGlobalSearch.SetFocus();
	}
	else
	{
		m_wndGlobalSearch.Create(m_hWnd, rcDefault);
		m_wndGlobalSearch.ShowWindow(SW_SHOW);
	}
	return 0;
}

LRESULT CMainFrame::OnCommandSubscribeChannel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	CDragDropData* pData = (CDragDropData*)h;
	if(pData == NULL)
		return 0;

	CString url = CGNUtil::NormalizeURL(pData->m_url);
	AddChannel((UINT)IDS_ADDCHANNELWIZARD, url, pData->m_defaultGroupId);

	delete pData;

	return 0;
}

LRESULT CMainFrame::OnCommandSetFocusFeedTree(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	if(m_wndVertSplit.IsCollapsed())
		m_wndVertSplit.Collapse(false); // show feed tree

	m_wndTreeFeeds.SetFocus();

	return 0;
}

LRESULT CMainFrame::OnCommandSetFocusNewsList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	if(m_wndHorzSplit.IsCollapsed())
		m_wndHorzSplit.Collapse(false); // show news list

	m_wndListNews.SetFocus();

	return 0;
}

LRESULT CMainFrame::OnCommandShowChannelWebSite(WORD /*wNotifyCode*/, WORD /*wID*/, HWND h)
{
	try
	{
		CFeedTreeItem* pSelectedItem = m_wndTreeFeeds.GetSelectedFeedItem();
		if(pSelectedItem == NULL)
			return 0;

		CNewsFeedTreeItem* pFeedItem = dynamic_cast<CNewsFeedTreeItem*>(pSelectedItem);
		if(pFeedItem == NULL)
			return 0;

		if(pFeedItem->m_newsFeed->m_website.GetLength())
			m_host.Navigate(pFeedItem->m_newsFeed->m_website, 0, CBrowserHost::NewTabForeground);

	}
	CATCH_ALL_ERROR()

	return 0;
}

// Gibt die aktuellen DPI für ein Fenster zurück
int CMainFrame::GetWindowDPI(HWND hWnd)
{
    // Für Windows 10 (1607+)
    HMODULE hUser32 = ::GetModuleHandle(_T("user32.dll"));
    typedef UINT (WINAPI *PFN_GetDpiForWindow)(HWND);
    PFN_GetDpiForWindow pGetDpiForWindow = (PFN_GetDpiForWindow)::GetProcAddress(hUser32, "GetDpiForWindow");
    
    if (pGetDpiForWindow != NULL)
    {
        return pGetDpiForWindow(hWnd);
    }

    // Fallback für ältere Windows-Versionen
    HDC hdc = ::GetDC(hWnd);
    int dpi = ::GetDeviceCaps(hdc, LOGPIXELSX);
	if (hdc) {
		::ReleaseDC(hWnd, hdc);
	}
    return dpi ? dpi : 96;
}
