#include "StdAfx.h"
#include "NewsListCtrl.h"
#include "resource.h"
#include "FeedManagerLib.h"
#include "GreatNewsConfig.h"
#include "GMTimeLib.h"
#include "GNLockWindowUpdate.h"
#include "BlogTool.h"
#include "NewsFeedCache.h"
#include "GNResourceManager.h"

#include <strsafe.h>

CNewsListCtrl::CNewsListCtrl():
    m_SortedColumn(-1),
    m_SortAscending(false),
    m_lastClickedItemNo(-1),
    m_openFlag(OpenNormal),
    m_bOpenWebsite(false),
    m_manualFlagChange(false),
    m_cc6(false)
{
	REParseError status = reEscape.Parse(_T("&#{\\z};"));
	ATLASSERT(status == REPARSE_ERROR_OK);
}

CNewsListCtrl::~CNewsListCtrl()
{
}

BOOL CNewsListCtrl::OnIdle()
{
	if(m_indexToCheckWatch.size())
	{
		CCheckWatchThread* pThread = new CCheckWatchThread(m_hWnd);
		pThread->m_indexToNotify.assign(m_indexToCheckWatch.begin(), m_indexToCheckWatch.end());
		pThread->m_itemsToCheck.reserve(m_indexToCheckWatch.size());
		for(std::set<ULONG_PTR>::iterator it = m_indexToCheckWatch.begin(); it != m_indexToCheckWatch.end(); ++it)
			pThread->m_itemsToCheck.push_back(GetNewsItemByIndex(*it));

		m_indexToCheckWatch.clear();

		pThread->Start();
	}

	return FALSE;
}

HWND CNewsListCtrl::Create(HWND hWndParent, ATL::_U_RECT rect, LPCTSTR szWindowName,
		DWORD dwStyle, DWORD dwExStyle,	ATL::_U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	dwStyle |= LVS_SHOWSELALWAYS | LVS_OWNERDATA |LVS_SINGLESEL;
	HWND hWnd = CWindowImpl<CNewsListCtrl, CListViewCtrl>::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
	if(!::IsWindow(hWnd))
		return hWnd;
	SetExtendedListViewStyle(LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_UNDERLINEHOT,
							 LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES|LVS_EX_UNDERLINEHOT);

	InsertColumn(0,_T("R"), LVCFMT_LEFT, 20, 0 );
	InsertColumn(1,_T("!"), LVCFMT_LEFT, 20, 0 );
	InsertColumn(2,_T("Title"), LVCFMT_LEFT, g_GreatNewsConfig.GetColWidthTitle(), 0 );
	InsertColumn(3,_T("P"), LVCFMT_LEFT, 20, 0 );
	InsertColumn(4,_T("Date"), LVCFMT_LEFT, g_GreatNewsConfig.GetColWidthDate(), 0 );
	InsertColumn(5,_T("Author"), LVCFMT_LEFT, g_GreatNewsConfig.GetColWidthAuthor(), 0 );
	InsertColumn(6,_T("Channel"), LVCFMT_LEFT, g_GreatNewsConfig.GetColWidthChannel(), 0 );

	m_imageList.CreateFromImage( IDB_LIST, 16, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION );
	SetImageList(m_imageList,LVSIL_SMALL);

	m_link.Create(hWnd, CRect(10,20,10,40),_T(""), 
		WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|SS_SIMPLE|SS_CENTER, 0, IDC_LINK);

	// creat font:
	CFontHandle defaultFont = GetFont();
	LOGFONT lf;
	defaultFont.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_unreadFont.CreateFontIndirect(&lf);

	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	if ( SUCCEEDED(AtlGetCommCtrlVersion(&dwMajor, &dwMinor)) && dwMajor >= 6 )
	{
		m_cc6 = true;
	}

	// setup header
	if(!m_cc6)  // common control 6 has build-in support for sorting icons
	{
		m_headerImageList.CreateFromImage( IDB_HEADER, 11, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION );
		CHeaderCtrl hdr = GetHeader();
		hdr.SetImageList(m_headerImageList);
	}

	LoadConfig();

	return hWnd;
}

LRESULT CNewsListCtrl::OnLvnColumnclick(LPNMHDR pnmh)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pnmh);


    HDITEM HeaderItem;
	HeaderItem.mask = (m_cc6 ? 0 : HDI_IMAGE) | HDI_FORMAT;
	CHeaderCtrl hdr = GetHeader();
	if(m_SortedColumn>=0)
	{
		hdr.GetItem(m_SortedColumn, &HeaderItem);
	}
	
	if(pNMLV->iSubItem <= (int)ColTag || pNMLV->iSubItem == ColPod) // cannot sort Unread/Tag/Podcasting columns
	{
		return 0;
	}

    if (m_SortedColumn == pNMLV->iSubItem)
    {
		// remove previous sort icon
		if(m_cc6)
		{
			HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
		}
		
		// set sorting direction: Descending->Ascending->No sort
		if(!m_SortAscending)
		{
			m_SortAscending = true;
			if(m_cc6)
			{
				HeaderItem.fmt |= HDF_SORTUP | HDF_BITMAP_ON_RIGHT;
			}
			else
			{
				HeaderItem.iImage = 1;
				HeaderItem.fmt |= HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
			}

			hdr.SetItem(m_SortedColumn, &HeaderItem);
		}
		else
		{
			// remove sorting
			if(m_cc6)
			{
				HeaderItem.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP|HDF_BITMAP_ON_RIGHT);
			}
			else
			{
				HeaderItem.fmt &= ~(HDF_IMAGE | HDF_BITMAP_ON_RIGHT);
			}
	        hdr.SetItem(m_SortedColumn, &HeaderItem);

			m_SortedColumn = -1; 
		}
    }
    else // clicked on a new sortable column
    {
		// remove sort icon on previous column
		if(m_SortedColumn >0)
		{
			if(m_cc6)
			{
				HeaderItem.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP | HDF_BITMAP_ON_RIGHT);
			}
			else
			{
				HeaderItem.fmt &= ~(HDF_IMAGE | HDF_BITMAP_ON_RIGHT);
			}
			hdr.SetItem(m_SortedColumn, &HeaderItem);
		}

		// setup the new sort column
        m_SortedColumn = pNMLV->iSubItem;
        m_SortAscending = false;

		ShowSortColumn();
    }

	// tell the main window to redisplay news items
	GetParent().PostMessage(WM_COMMAND, CMD_ID_FILTERCHANGE);

	return 0;
}

void CNewsListCtrl::ShowSortColumn()
{
	if(m_SortedColumn<0)
		return;

    HDITEM HeaderItem;
	HeaderItem.mask = (m_cc6 ? 0 : HDI_IMAGE) | HDI_FORMAT;
	CHeaderCtrl hdr = GetHeader();
	hdr.GetItem(m_SortedColumn, &HeaderItem);

	if(m_cc6)
	{
		HeaderItem.fmt |= m_SortAscending? HDF_SORTUP : HDF_SORTDOWN;
		HeaderItem.fmt |= HDF_BITMAP_ON_RIGHT;
	}
	else
	{
		HeaderItem.iImage = m_SortAscending? 1 : 0;
		HeaderItem.fmt |= HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
	}
	hdr.SetItem(m_SortedColumn, &HeaderItem);
}

void CNewsListCtrl::GetOrderBy(CNewsFilter* pFilter)
{
	pFilter->m_bSortDesc = !m_SortAscending;
	switch(m_SortedColumn)
	{
	case ColTitle:
		pFilter->m_sort = CNewsFilter::SortByTitle;
		break;
	case ColAuthor:
		pFilter->m_sort = CNewsFilter::SortByAuthor;
		break;
	case ColChannel:
		pFilter->m_sort = CNewsFilter::SortByChannel;
		break;
	case ColDate:
		pFilter->m_sort = CNewsFilter::SortByDate;
		break;
	default:
		pFilter->m_sort = CNewsFilter::SortNone;
	}
}

void CNewsListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	//LVHITTESTINFO hti = { 0 };
	//hti.pt = point;
	//// int n = HitTest(&hti);
	//int itemNo = (int)SendMessage(LVM_SUBITEMHITTEST, 0, (LPARAM)&hti);
	//NewsItemPtr pItem = GetNewsItemByIndex(itemNo);
	//
	//if(pItem != NULL)
	//{
	//	//SetLastViewedItem(itemNo);
	//	ListView_SetHotItem(m_hWnd, itemNo);

	//	CMenu menu;
	//	menu.LoadMenu(IDR_NEWSLIST_POPUP);
	//	CMenu popupMenu = menu.GetSubMenu(0);
	//	
	//	// read/unread
	//	if(pItem->IsUnread())
	//	{
	//		popupMenu.DeleteMenu(1, MF_BYPOSITION);
	//	}
	//	else
	//	{
	//		popupMenu.DeleteMenu(0, MF_BYPOSITION);
	//	}

	//	// labels
	//	PopulateLabelMenu(popupMenu, pItem);

	//	// blog this tools
	//	CMenu blogThisMenu;
	//	BuildBlogThisPopup(blogThisMenu);
	//	popupMenu.ModifyMenu(CMD_ID_BLOGTHIS_POPUP, MF_POPUP, (UINT_PTR)blogThisMenu.Detach(), _T("Blog This"));

	//	// localize
	//	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	//	pResMngr->ApplyLanguageToMenu(popupMenu);

	//	CPoint spt = point;
	//	ClientToScreen(&spt);
	//	popupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, spt.x, spt.y, m_hWnd);
	//}

	return;
}
//LRESULT CNewsListCtrl::OnContextMenu(LPNMHDR pnmh)
LRESULT CNewsListCtrl::OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt( GET_X_LPARAM( lParam ), GET_Y_LPARAM ( lParam ) );
	if(pt.x == -1 && pt.y==-1)
	{
		// this message is generated by SHIFT+F10 or Context Meny key
		int selected = GetSelectedIndex();
		if(selected < 0)
			selected = 0;
		RECT rc;
		GetItemRect(selected, &rc, LVIR_BOUNDS);
		pt.x = rc.left+5;
		pt.y = rc.top+5;
		ClientToScreen(&pt);
	}
	CPoint spt = pt;
	ScreenToClient( &pt );
	LVHITTESTINFO hti = { 0 };
	hti.pt = pt;
	// int n = HitTest(&hti);
	int itemNo = (int)SendMessage(LVM_SUBITEMHITTEST, 0, (LPARAM)&hti);
	NewsItemPtr pItem = GetNewsItemByIndex(itemNo);
	
	if(pItem != NULL)
	{
		//SetLastViewedItem(itemNo);
		ListView_SetHotItem(m_hWnd, itemNo);

		CMenu menu;
		menu.LoadMenu(IDR_NEWSLIST_POPUP);
		CMenu popupMenu = menu.GetSubMenu(0);
		
		// read/unread
		if(pItem->IsUnread())
		{
			popupMenu.DeleteMenu(1, MF_BYPOSITION);
		}
		else
		{
			popupMenu.DeleteMenu(0, MF_BYPOSITION);
		}

		// labels
		PopulateLabelMenu(popupMenu, pItem);

		// blog this tools
		CMenu blogThisMenu;
		BuildBlogThisPopup(blogThisMenu);
		popupMenu.ModifyMenu(CMD_ID_BLOGTHIS_POPUP, MF_POPUP, (UINT_PTR)blogThisMenu.Detach(), _T("Blog This"));

		// localize
		CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
		pResMngr->ApplyLanguageToMenu(popupMenu);

		popupMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, spt.x, spt.y, m_hWnd);
	}

	return 0;
}

void CNewsListCtrl::SetNewsSource(BatchContentGeneratorPtr gen)
{
	m_manualFlagChange = false;
	m_contentGen = gen;
	m_lastClickedItemNo = -1;
	ListView_SetHotItem(m_hWnd, -1); 
	
	m_indexToCheckWatch.clear();

	Refresh(Reload);
}

LRESULT CNewsListCtrl::OnDblClick(LPNMHDR pnmh)
{
	DWORD dwPos = GetMessagePos();
	CPoint pt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );
	CPoint spt = pt;
	ScreenToClient( &pt );
	LVHITTESTINFO hti = { 0 };
	hti.pt = pt;
	int itemNo = (int)SendMessage(LVM_SUBITEMHITTEST, 0, (LPARAM)&hti);

	#define KEYDOWN_MASK   0x1000 

	if (itemNo >= 0 && (hti.flags & LVHT_ONITEM))
	{
		SetLastViewedItem(itemNo);
		m_bOpenWebsite = true;

		// determine where to open the web page
		if(GetKeyState( VK_CONTROL ) & KEYDOWN_MASK)
		{
				m_openFlag = OpenWebInNewTab;
		}
		else if(GetKeyState( VK_SHIFT ) & KEYDOWN_MASK)
		{
			m_openFlag = OpenWebInNewWindow;
		}
		else
		{
			switch(g_GreatNewsConfig.m_nDoubleClickingNewsList)
			{
			case 0:
				m_openFlag = OpenWebInNewWindow;
				break;
			case 1:
				m_openFlag = OpenWeb;
				break;
			default:
				m_openFlag = OpenWebInNewTabForeground;
			}
		}

		int nPrevious = GetSelectedIndex();
		if(nPrevious != itemNo)
		{
			SelectItem(itemNo);
		}
		else
		{
			// select the same item won't trigger ItemSelectedChange msg, so we have to do by ourselves
			OpenRssLink();
		}

	}
	return 0;
}


LRESULT CNewsListCtrl::OnClick(LPNMHDR pnmh)
{
	DWORD dwPos = GetMessagePos();
	CPoint pt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );
	CPoint spt = pt;
	ScreenToClient( &pt );
	LVHITTESTINFO hti = { 0 };
	hti.pt = pt;
	int itemNo = (int)SendMessage(LVM_SUBITEMHITTEST, 0, (LPARAM)&hti);

	if (itemNo >= 0 && (hti.flags & LVHT_ONITEM))
	{
		NewsItemPtr pItem = GetNewsItemByIndex(itemNo);
		SetLastViewedItem(itemNo);

		ListView_SetHotItem(m_hWnd, itemNo);

		if(hti.iSubItem == ColReadFlag) // read flag
		{
			OnToggleReadFlag(0,0,0);
		}
		else if(hti.iSubItem == ColTag) // label flag
		{
			CMenu menu;
			menu.CreatePopupMenu();
			PopulateLabelMenu(menu, pItem);
			menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN, 
				spt.x, spt.y, m_hWnd, NULL);
		}
		else if(hti.iSubItem == ColPod && pItem->m_podCastingURL.GetLength()) 
		{
			::ShellExecute(m_hWnd, _T("open"), pItem->m_podCastingURL, NULL, g_GreatNewsConfig.GetHomeDir(), SW_SHOW);
		}
	}

	return 0;
}

LRESULT CNewsListCtrl::OnCacheHint(LPNMHDR pnmh)
{
	NMLVCACHEHINT* pCachehint = (NMLVCACHEHINT *)pnmh;
	std::vector<ULONG_PTR> IDsToLoad(m_contentGen->m_vectNewsIDs.begin()+pCachehint->iFrom,
								 m_contentGen->m_vectNewsIDs.begin()+pCachehint->iTo);
	CNewsItemCache::FillCache(IDsToLoad, false);
	return 0;
}

LRESULT CNewsListCtrl::OnGetDispInfo(LPNMHDR pnmhdr)
{
	try
	{
        NMLVDISPINFO* plvdi = (NMLVDISPINFO*) pnmhdr;
        if (-1 == plvdi->item.iItem)
		{
            AtlTrace("LVOWNER: Request for -1 item?\n");
			return 0;
        }
        // Retrieve information for item at index iItem.
		NewsItemPtr newsItem = GetNewsItemByIndex(plvdi->item.iItem);
		if(newsItem == NULL)
		{
			AtlTrace("Failed in GetDispInfo: cannot find requested item \n");
			return 0;
		}
		
        if(plvdi->item.mask & LVIF_IMAGE){
            // Fill in the image information.
            switch (plvdi->item.iSubItem){
                case ColReadFlag:
					if(newsItem->IsUnread())
						plvdi->item.iImage = 0;
					break;
				case ColTag:
					if(newsItem->m_marked>0)
						plvdi->item.iImage = 1;
					break;
				case ColPod:
					if(newsItem->m_podCastingURL.GetLength())
						plvdi->item.iImage = 2;
					break;
                default:
                    break;
            }
        }

        if(plvdi->item.mask & LVIF_TEXT){
			CString cleanTitle;
      // Fill in the text information.
      switch (plvdi->item.iSubItem){
        case ColTitle:
          // Copy the main item text.
					cleanTitle = newsItem->GetTitle();
					StripHTML(cleanTitle);
					StringCchCopy (plvdi->item.pszText, plvdi->item.cchTextMax, (LPCTSTR)cleanTitle);
					// A2W(plvdi->item.pszText);
					break;
				case ColAuthor: // author:
					StringCchCopy(plvdi->item.pszText,plvdi->item.cchTextMax,
						(LPCTSTR)newsItem->m_author);
					break;
				case ColChannel: // channel name:
					StringCchCopy(plvdi->item.pszText,plvdi->item.cchTextMax,
						(LPCTSTR)GetChannelName(newsItem->m_feedID));
					break;
				case ColDate:
					StringCchCopy(plvdi->item.pszText, 
						plvdi->item.cchTextMax,
						(LPCTSTR)CGMTimeHelper::FormatDisplayDate(newsItem->m_date));
					break;
        default:
            break;
            }
        }
	}
	catch(...)
	{
		AtlTrace(_T("GetDispInfo failed"));
	}

	return 0;
}

void CNewsListCtrl::SetItemRead(int itemNo, bool bSetToRead)
{
	NewsItemPtr pItem = GetNewsItemByIndex(itemNo);
	if(pItem == NULL)
		return;

	if(bSetToRead && pItem->IsUnread())
		pItem->SetRead();
	else if(!bSetToRead && !pItem->IsUnread())
		pItem->SetUnread();

	ListView_Update(m_hWnd, itemNo);
	// CListViewCtrl::SetItemText(itemNo, 0, pItem->IsUnread() ? _T("N") : _T(""));
}

void CNewsListCtrl::Refresh(RefreshMode mode)
{
	if(mode >= Reload)
	{
		// show/hide the "Display All" link
		if(m_contentGen && m_contentGen->m_vectNewsIDs.empty())
		{
			if(m_contentGen->HasFilter())
			{
				m_link.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON);
				m_link.SetLabel(_T("No news item fits the filter. Click here to display all."));
			}
			else
			{
				m_link.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON|HLINK_NOTUNDERLINED);
				m_link.SetLabel(_T("No news item is available."));
			}
			m_link.Invalidate();
			m_link.ShowWindow(SW_SHOW);
		}
		else
			m_link.ShowWindow(SW_HIDE);

		DeleteAllItems();
		SetItemCount(m_contentGen ? m_contentGen->m_vectNewsIDs.size() : 0);
	}

	if(mode >= ClearCache)
		CNewsItemCache::Empty();

	Invalidate();
}

LRESULT CNewsListCtrl::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	int cx = LOWORD(lParam);
	int cy = HIWORD(lParam);

	if(cy<=2)
	{
		if(IsWindowVisible())
			ShowWindow(SW_HIDE);

		return 0;
	}

	if(cy>2 && !IsWindowVisible())
	{
		ShowWindow(SW_SHOW);
	}

	if(m_link)
	{
		::SetWindowPos(m_link, NULL, 0, 25,LOWORD(lParam), 45,SWP_NOZORDER | SWP_NOACTIVATE);
	}

	return 0;
}

LRESULT CNewsListCtrl::OnListItemSelected(LPNMHDR lParam)
{
	try
	{
		ListView_SetHotItem(m_hWnd, -1); 

		LPNMLISTVIEW lpnmia = (LPNMLISTVIEW)lParam;
		if(lpnmia->iItem>=0 && lpnmia->uOldState == 0)
		{
			AtlTrace("reading %d...\n",lpnmia->iItem);
			
			SetLastViewedItem(lpnmia->iItem);
			OpenRssLink();
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}

void CNewsListCtrl::OpenRssLink()
{
	if(m_bOpenWebsite)
	{
		m_bOpenWebsite = false;
		GetParent().PostMessage(WM_COMMAND,
			MAKELONG(CMD_ID_NEWSITEMSELECTED, m_openFlag),
			0L);
	}
	else
	{
		GetParent().PostMessage(WM_COMMAND,
			MAKELONG(CMD_ID_NEWSITEMSELECTED, OpenNormal),
			0L);
	}
}

DWORD CNewsListCtrl::OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD)
{
	LPNMLVCUSTOMDRAW lpLVCD = (LPNMLVCUSTOMDRAW) lpNMCD;

	NewsItemPtr	item = GetNewsItemByIndex(lpLVCD->nmcd.dwItemSpec);
	if(!item->WatchStatusChecked())
		m_indexToCheckWatch.insert(lpLVCD->nmcd.dwItemSpec);

	COLORREF txtColor = item->GetTextColor();
	if(txtColor!=CNewsItem::DefaultColor)
		lpLVCD->clrText = txtColor;

	COLORREF bkColor = item->GetBkColor();
	if(bkColor!=CNewsItem::DefaultColor)
		lpLVCD->clrTextBk = bkColor;

	if(item->IsUnread())
	{
		CDCHandle dc = lpLVCD->nmcd.hdc;
		dc.SelectFont(m_unreadFont);
	}

	return CDRF_DODEFAULT;
}


CString CNewsListCtrl::GetChannelName(ULONG_PTR channelID)
{
	NewsFeedPtr feed = CNewsFeedCache::GetNewsFeed(channelID);
	if(feed)
		return feed->m_title;
	else
		return _T("");
}


LRESULT CNewsListCtrl::OnShowAllLinkClick(UINT, int, HWND)
{
	GetParent().PostMessage(WM_COMMAND, (WPARAM)ID_FILTER_SHOWALL, 0L);
	return 0;
}

void CNewsListCtrl::PrevItem()
{
	int selected = GetSelectedIndex();
	if(selected < 0)
		selected = 0;
	else if (selected == 0)
		selected = GetItemCount()-1;
	else
		selected = selected-1;

	SelectItem(selected);

}

void CNewsListCtrl::NextItem()
{
	int selected = GetSelectedIndex();
	if(selected < 0 || selected == GetItemCount()-1)
		selected = 0;
	else
		selected = selected+1;

	SelectItem(selected);
}

NewsItemPtr CNewsListCtrl::GetNewsItemByIndex(DWORD_PTR index)
{
	if(index < 0 || index >= m_contentGen->m_vectNewsIDs.size())
		return NULL;
	
	return CNewsItemCache::GetNewsItem(m_contentGen->m_vectNewsIDs[index]);
}

NewsItemPtr CNewsListCtrl::GetHotNewsItem()
{
	int n = GetHotItem();
	return GetNewsItemByIndex(n);
}

NewsItemPtr CNewsListCtrl::GetSelectedNewsItem()
{
	int n = GetSelectedIndex();
	return GetNewsItemByIndex(n);
}


LRESULT CNewsListCtrl::OnToggleReadFlag(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	// NewsItemPtr pItem = GetNewsItemByIndex(m_lastClickedItemNo);
	NewsItemPtr pItem = GetHotNewsItem();
	if(pItem == NULL)
		return 0;

	m_manualFlagChange = true;
	if(pItem->IsUnread())
		pItem->SetRead();
	else
		pItem->SetUnread();

	ListView_Update(m_hWnd, GetHotItem());
	GetParent().PostMessage(WM_COMMAND, CMD_ID_READFLAGCHANGED, 0L);

	return 0;
}

void CNewsListCtrl::OnBlogThisTool(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	NewsItemPtr pItem = GetHotNewsItem();
	if(pItem == NULL)
		return;

	// let mainframe handle it
	GetParent().PostMessage(WM_COMMAND, wID, pItem->m_id);
}


void CNewsListCtrl::OnToggleLabel(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	UINT n = wID - ID_LABEL_TAGGLE_FIRST;
	NewsItemPtr pItem = GetHotNewsItem();
	if(pItem == NULL || n>=m_allTags.size())
		return;


	try
	{
		TagPtr tag = m_allTags[n];
		std::vector<ULONG_PTR> itemTags;
		pItem->GetItemTags(itemTags);
		if(std::find(itemTags.begin(), itemTags.end(), (ULONG_PTR)tag->m_id)!=itemTags.end())
		{
			pItem->RemoveTag(tag->m_id);
		}
		else
		{
			pItem->AddTag(tag->m_id);
		}
		ListView_Update(m_hWnd, GetHotItem());
	}
	CATCH_ALL_ERROR()

}

LRESULT CNewsListCtrl::OnForwardCommand(WORD /*wNotifyCode*/, WORD cmdID, HWND /*hWndCtl*/)
{
	NewsItemPtr pItem = GetHotNewsItem();
	if(pItem != NULL)
	{
		GetParent().PostMessage(WM_COMMAND, cmdID, pItem->m_id);
	}

	return 0;
}

void CNewsListCtrl::PopulateLabelMenu(CMenu& menu, NewsItemPtr pItem)
{
	try
	{
		CTag::GetAllTags(m_allTags); // always refresh to get the latest tags
		std::vector<ULONG_PTR> itemTags;
		pItem->GetItemTags(itemTags);

		int iMenu=0;
		for(TagVector::iterator it=m_allTags.begin(); it!=m_allTags.end();++it)
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

	}
	CATCH_ALL_ERROR()
}

LRESULT CNewsListCtrl::OnRefreshWatchedItem(UINT /*uMsg*/, WPARAM index, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	//ListView_Update(m_hWnd, index);
	Invalidate();
	return 0L;
}

void CNewsListCtrl::OnMButtonDown(UINT nFlags, CPoint point)
{
	LVHITTESTINFO hti = { 0 };
	hti.pt = point;
	// int n = HitTest(&hti);
	int itemNo = (int)SendMessage(LVM_SUBITEMHITTEST, 0, (LPARAM)&hti);

	if (itemNo >= 0 && (hti.flags & LVHT_ONITEM))
	{
		// SetLastViewedItem(itemNo);
		m_bOpenWebsite = true;
		m_openFlag = OpenWeb;

		int nPrevious = GetSelectedIndex();
		if(nPrevious != itemNo)
		{
			SelectItem(itemNo);
		}
		else
		{
			// select the same item won't trigger ItemSelectedChange msg, so we have to do by ourselves
			OpenRssLink();
		}
	}
	return;
}

LRESULT CNewsListCtrl::OnDeleteNewsItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	NewsItemPtr pItem = GetHotNewsItem();
	if(pItem)
	{
		try
		{
			int itemNo =  GetHotItem();
			SetLastViewedItem(-1);
			DeleteItem(itemNo);
			m_contentGen->m_vectNewsIDs.erase(m_contentGen->m_vectNewsIDs.begin()+itemNo);
			pItem->Delete();

			// re-select current item so user can keep deleting
			if(itemNo < (int)m_contentGen->m_vectNewsIDs.size())
			{
				m_manualFlagChange = true;
				SelectItem(itemNo);
				SetLastViewedItem(itemNo); // we have to call this one becuase OnItemSelected won't fire
			}

		}
		CATCH_ALL_ERROR()
	}

	return 0;
}

void CNewsListCtrl::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_DELETE)
	{
		// set the hot item to selected item, because ID_NEWSLIST_DELETE works on hot item
		ListView_SetHotItem(m_hWnd, GetSelectedIndex()); 
		PostMessage(WM_COMMAND, ID_NEWSLIST_DELETE, 0);
	}
	else
	{
		SetMsgHandled(FALSE);
	}

	return;
}

void CNewsListCtrl::StripHTML(CString& textWithHTML)
{
	// remove tags
	int nPosLt = textWithHTML.Find(_T('<'));
	int nPosGt = textWithHTML.Find(_T('>'));
	while(nPosLt>=0 && nPosLt<nPosGt)
	{
		textWithHTML.Delete(nPosLt,nPosGt-nPosLt+1);
		nPosLt = textWithHTML.Find(_T('<'));
		nPosGt = textWithHTML.Find(_T('>'));
	}

	// decode &#1234;
	CAtlREMatchContext<CAtlRECharTraitsW> mcTags;
	const CAtlREMatchContext<CAtlRECharTraitsW>::RECHAR* szStart = 0;
	const CAtlREMatchContext<CAtlRECharTraitsW>::RECHAR* szEnd = 0;
	while(reEscape.Match(textWithHTML,&mcTags))
	{
		mcTags.GetMatch(0, &szStart, &szEnd);
		CString code(szStart, int(szEnd-szStart));
		int nCode = _ttol(code);
		TCHAR tchar = nCode > 0 ? nCode : _T(' ');
		textWithHTML = textWithHTML.Left(textWithHTML.GetLength()- (int)_tcslen(mcTags.m_Match.szStart))
									+ tchar
									+ mcTags.m_Match.szEnd;
	}
}

void CNewsListCtrl::SetTopItem(int itemNo)
{
	int total = GetItemCount();
	if(itemNo < total)
	{
		CGNLockWindowUpdate lockWnd(m_hWnd);

		if(total>1)
		{
			EnsureVisible(total-1, FALSE);
		}
		EnsureVisible(itemNo, FALSE);
	}
}


LRESULT CNewsListCtrl::OnAddNewLabel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	GetParent().PostMessage(WM_COMMAND, ID_LABEL_ADD);
	return 0;
}

void CNewsListCtrl::SetLastViewedItem(int itemNo)
{
	if(m_lastClickedItemNo>=0 && m_lastClickedItemNo != itemNo)
	{
		if(m_manualFlagChange) // if user manually changed read flag, we shouldn't mark it read
		{
			m_manualFlagChange = false;
		}
		else if(g_GreatNewsConfig.m_nMarkReadAutomatically==0)  // mark read interactively?
		{
			NewsItemPtr pItem = GetNewsItemByIndex(m_lastClickedItemNo);
			if(pItem != NULL && pItem->IsUnread())
			{
				// mark this item read before moving onto next item
				GetParent().SendMessage(WM_COMMAND, CMD_ID_MARKITEMREAD, m_lastClickedItemNo);
				ListView_Update(m_hWnd, m_lastClickedItemNo);
			}
		}
	}

	m_lastClickedItemNo = itemNo;
}


LRESULT CNewsListCtrl::OnSetSursor(HWND hWnd, UINT nHitTest, UINT message)
{
	if(hWnd == m_hWnd)
	{
		DWORD dwPos = GetMessagePos();
		CPoint pt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );
		CPoint spt = pt;
		ScreenToClient( &pt );
		LVHITTESTINFO hti = { 0 };
		hti.pt = pt;
		int itemNo = (int)SendMessage(LVM_SUBITEMHITTEST, 0, (LPARAM)&hti);

		if (itemNo >= 0 && (hti.flags & LVHT_ONITEM))
		{
			NewsItemPtr pItem = GetNewsItemByIndex(itemNo);

			if(hti.iSubItem == ColReadFlag // read flag
				|| hti.iSubItem == ColTag // label flag
				|| (hti.iSubItem == ColPod && pItem->m_podCastingURL.GetLength()) )
			{
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return TRUE;
			}
		}

		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		return TRUE;

	}
	return FALSE;
}

void CNewsListCtrl::OnDestroy()
{
	SaveConfig();

	SetMsgHandled(FALSE);
}

LRESULT CNewsListCtrl::OnEnterKey(LPNMHDR pnmh)
{
	int itemNo = GetSelectedIndex();
	if (itemNo >= 0)
	{
		// SetLastViewedItem(itemNo);
		m_bOpenWebsite = true;

		#define KEYDOWN_MASK   0x1000 
		if(GetKeyState( VK_CONTROL ) & KEYDOWN_MASK)
		{
			m_openFlag = OpenWebInNewTab;
		}
		else if(GetKeyState( VK_SHIFT ) & KEYDOWN_MASK)
		{
			m_openFlag = OpenWebInNewWindow;
		}
		else
			m_openFlag = OpenWeb;

		int nPrevious = GetSelectedIndex();
		if(nPrevious != itemNo)
		{
			SelectItem(itemNo);
		}
		else
		{
			// select the same item won't trigger ItemSelectedChange msg, so we have to do by ourselves
			OpenRssLink();
		}
	}

	return 0L;
}

void CNewsListCtrl::BuildBlogThisPopup(CMenu& blogThisMenu)
{
	BlogToolVector allBlogTools;
	CBlogTool::GetAllBlogTools(allBlogTools);

	blogThisMenu.CreatePopupMenu();
	int i=0;
	for(BlogToolVector::iterator it=allBlogTools.begin(); it!=allBlogTools.end(); ++it, ++i)
	{
		BlogToolPtr& tool = *it;
		blogThisMenu.AppendMenu(MF_STRING | MF_BYPOSITION, CMD_ID_BLOGTHISTOOL0+i, tool->m_name );
	}
	blogThisMenu.AppendMenu(MF_SEPARATOR);
	blogThisMenu.AppendMenu(MF_STRING | MF_BYPOSITION, CMD_ID_CONFIGBLOGTHIS, _T("Configure...") );
}


void CNewsListCtrl::SaveConfig()
{
	g_GreatNewsConfig.SetColWidthTitle(GetColumnWidth(ColTitle));
	g_GreatNewsConfig.SetColWidthDate(GetColumnWidth(ColDate));
	g_GreatNewsConfig.SetColWidthAuthor(GetColumnWidth(ColAuthor));
	g_GreatNewsConfig.SetColWidthChannel(GetColumnWidth(ColChannel));

	// only save Date and Channel sorting
	if(m_SortedColumn == ColDate || m_SortedColumn == ColChannel)
		g_GreatNewsConfig.SetNewsListSortColumn( m_SortedColumn );
	else
		g_GreatNewsConfig.SetNewsListSortColumn( -1 );
	g_GreatNewsConfig.SetNewsListSortAscending( m_SortAscending );
}

void CNewsListCtrl::LoadConfig()
{
	m_SortedColumn = g_GreatNewsConfig.GetNewsListSortColumn();
	m_SortAscending = g_GreatNewsConfig.GetNewsListSortAscending();

	ShowSortColumn();
}


