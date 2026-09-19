#include "StdAfx.h"
#include <atlmisc.h>
#include <wininet.h>
#include <Shobjidl.h>
#include "resource.h"
#include "BrowserHost.h"
#include "GreatNewsConfig.h"
#include "GNUtil.h"
#include "GNClipBoard.h"
#include "GNResourceManager.h"

#define ADDR_COMBO_SIZE			10
#define TOOLBAR_COMBO_DROPLINES 10

#define HTMLID_FIND 1
#define HTMLID_VIEWSOURCE 2
#define HTMLID_OPTIONS 3 

#define KEYDOWN_MASK   0x1000 

#define MY_TOOLBAR_STYLE \
	(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST)

#define REBAR_STYLE \
	(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_AUTOSIZE |RBS_DBLCLKTOGGLE )

CBrowserHost::CBrowserHost() :
	m_popupBrowser(*this),
	m_bNewWindow3InUse(false),
	m_bOpenInDefaultBrowser(false),
	m_bUserClickedLink(false),
	m_bWorkOffline(false)
{
}

BOOL CBrowserHost::PreTranslateMessage(MSG* pMsg)
{
	if(m_addrBar.IsChild(::GetFocus()))
		return FALSE;

	if(pMsg->message == WM_KEYDOWN && m_wndIE && m_wndIE->m_hWnd)
	{
		if((GetKeyState( VK_CONTROL ) & KEYDOWN_MASK || GetKeyState( VK_SHIFT ) & KEYDOWN_MASK))
			return m_wndIE->PreTranslateMessage(pMsg);
	
		if(ReadingPaneHasFocus(::GetFocus()))
		{
			if(m_wndIE->PreTranslateMessage(pMsg))
				return TRUE;

			if(!g_GreatNewsConfig.m_bUseMozillaEngine)
			{
				if(pMsg->wParam == VK_SPACE && IsFeedTabActive())
				{
					if(m_wndIE->IsAtPageBottom(g_GreatNewsConfig.m_bLegacyBrowser))
					{
						// hit SPACE at page end will turn page
						ReadNext();
						return TRUE;
					}
				}
			}
		}

		return FALSE;
	}

	//
	// Now check mouse messages
	//
	DWORD dwPos = GetMessagePos();
	CPoint pt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );
	HWND hWndMouse = WindowFromPoint(pt);
	if(hWndMouse == NULL  || !IsChild(hWndMouse))
		return FALSE; // mouse is not over reading pane

	if(m_wndIE && m_wndIE->m_hWnd)
	{
		if(pMsg->message == WM_XBUTTONDOWN || pMsg->message == WM_XBUTTONUP)
		{
			WORD w = LOWORD(pMsg->wParam);
			if(w & MK_XBUTTON1)
			{
				//PostMessage(WM_COMMAND, MAKELONG(ID_IE_GOTO_PAGE, CContentGenerator::PrevPage));
				m_wndIE->GoBack();
				return TRUE;
			}
			else if(w & MK_XBUTTON2)
			{
				//PostMessage(WM_COMMAND, MAKELONG(ID_IE_GOTO_PAGE, CContentGenerator::NextPage));
				m_wndIE->GoForward();
				return TRUE;
			}
			return m_wndIE->PreTranslateMessage(pMsg);
			//return FALSE;
		}
		else if(pMsg->message == WM_RBUTTONDOWN)
		{
			m_bEatRightButtonUp = false; // reset
			return FALSE; // let other program handles it
		}
		else if(pMsg->message == WM_RBUTTONUP)
		{
			return m_bEatRightButtonUp;
		}
		else if(pMsg->message == WM_MOUSEWHEEL)		// scrolling wheel
		{
			short scroll = HIWORD(pMsg->wParam);
			AtlTrace(_T("Wheel time [%d], Scroll [%d]\n"), pMsg->time, scroll);

			m_bEatRightButtonUp = true;
			if(!IsFeedTabActive())
				return FALSE;

			static DWORD s_lastWheelMsgHandledTime; // last time wheel msg was handled here

			if(pMsg->time - s_lastWheelMsgHandledTime < 1000)
				return TRUE; // if we just handled a wheel msg,we will eat all remaining ones in 1sec

			// Forced page turning
			if((LOWORD(pMsg->wParam)&MK_RBUTTON))	// and holding right button ?
			{
				PostMessage(WM_COMMAND,
					MAKELONG(ID_IE_GOTO_PAGE, (scroll<0 ? CContentGenerator::NextPage : CContentGenerator::PrevPage)));
				s_lastWheelMsgHandledTime = pMsg->time;
				return TRUE;
			}

			// Handle automatic page turning
			if(!g_GreatNewsConfig.m_bUseMozillaEngine)
			{
				static DWORD s_lastWheelMsgTime;	// last time wheel turned more than 1 click
				static int s_wheelCount;
				
				DWORD lastWheelMsgTime = s_lastWheelMsgTime;
				s_lastWheelMsgTime = pMsg->time;

				// if we are at the bottom of a page
				if(m_wndIE->IsAtPageBottom(g_GreatNewsConfig.m_bLegacyBrowser) && scroll<0)
				{
					if(s_wheelCount>=0 && pMsg->time - lastWheelMsgTime < 600) // we only process wheel msgs that are well apart
					{
						s_wheelCount = 0;
						return TRUE; // eat it
					}
					else if(s_wheelCount<0 && pMsg->time - lastWheelMsgTime > 100)
					{
						s_wheelCount = 0; // if page turning wheel started, they should finish in very short time
						return TRUE;
					}
					
					s_wheelCount+=scroll;
					if(s_wheelCount <= -g_GreatNewsConfig.m_nMouseWheelSensitivity*WHEEL_DELTA)
					{
						s_wheelCount = 0;
						s_lastWheelMsgHandledTime = pMsg->time;
						ReadNext();
					}

					return TRUE;
				}

				// if we are at the top of a page
				if(m_wndIE->IsAtPageTop(g_GreatNewsConfig.m_bLegacyBrowser) && scroll>0)
				{
					if(s_wheelCount<=0 && pMsg->time - lastWheelMsgTime < 600) // we only process wheel msgs that are well apart
					{
						s_wheelCount = 0;
						return TRUE; // eat it
					}
					else if(s_wheelCount>0 && pMsg->time - lastWheelMsgTime > 100)
					{
						s_wheelCount = 0; // if page turning wheel started, they should finish in very short time
						return TRUE;
					}
					
					s_wheelCount+=scroll;
					if(s_wheelCount >= g_GreatNewsConfig.m_nMouseWheelSensitivity*WHEEL_DELTA)
					{
						s_wheelCount = 0;
						s_lastWheelMsgHandledTime = pMsg->time;
						ReadPrevious();
					}

					return TRUE;
				}

				return FALSE;
			}
		}
		else if(pMsg->message == WM_LBUTTONUP)
		{
			CString strUrl = m_wndIE->GetUrlUnderMouse();

//* Debugging */	CString msg;
//* Debugging */	msg.Format(_T("URL: %s"), (LPCTSTR)strUrl);
//* Debugging */	::MessageBox(NULL, msg, _T("PreTranslateMessage"), MB_OK | MB_ICONINFORMATION);

			if(!strUrl.IsEmpty() && !IsGreatNewsSpecialUrl(strUrl))
			{
				m_bUserClickedLink = true;

				if(GetKeyState( VK_CONTROL ) & KEYDOWN_MASK)
				{
					NewTab(strUrl);
					return TRUE;
				}
				else if(GetKeyState( VK_SHIFT ) & KEYDOWN_MASK)
				{
					CGNUtil::OpenUrlExternally(strUrl, true, true);
					return TRUE;
				}
				else if(g_GreatNewsConfig.m_nOpenRssLinkInDefaultBrowser == 1	// default browser
							&& IsFeedTabActive())
				{
					CGNUtil::OpenUrlExternally(strUrl, true, true);
					return TRUE;
				}
				else if(g_GreatNewsConfig.m_nOpenRssLinkInDefaultBrowser == 2 // new tab
							&& IsFeedTabActive())
				{
					NewTab(strUrl,false);
					return TRUE;
				}
			}
		}
		else if(pMsg->message == WM_MBUTTONDOWN)
		{
			CString strUrl = m_wndIE->GetUrlUnderMouse();
			if(!strUrl.IsEmpty())
				return TRUE; // eat the link if it's middle clicked
		}
		else if(pMsg->message == WM_MBUTTONUP)
		{
			CString strUrl = m_wndIE->GetUrlUnderMouse();
			if(!strUrl.IsEmpty() && !IsGreatNewsSpecialUrl(strUrl))
			{
				NewTab(strUrl);
				return TRUE;
			}
		}
		
		if(m_wndIE->PreTranslateMessage(pMsg))
			return TRUE;
	}
    
	return FALSE;
}

BOOL CBrowserHost::OnIdle()
{
	UIEnable(ID_IE_BACK, m_wndIE->CanBack());
	UIEnable(ID_IE_FORWARD, m_wndIE->CanForward());

	UIUpdateToolBar();

	return FALSE;
}

HWND CBrowserHost::Create(HWND hWndParent, ATL::_U_RECT rect)
{
	return CWindowImpl<CBrowserHost>::Create(hWndParent, rect, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
}

LRESULT CBrowserHost::OnTBDropDown (int idCtrl, LPNMHDR pnmh, BOOL &bHandled) 
{
	NMTOOLBAR *pnmTB = (NMTOOLBAR *) pnmh;

	CPoint pt(pnmTB->rcButton.left, pnmTB->rcButton.bottom);
	::ClientToScreen(pnmh->hwndFrom, &pt);

	if (pnmTB ->iItem == ID_IE_STYLE)
	{
		std::vector<CString> styleNames;
		CString selectedStyle = g_GreatNewsConfig.GetCurrentStyle();

		HWND hWndCtl = pnmh->hwndFrom;

		CFindFile finder;
		BOOL bFound = finder.FindFile(g_GreatNewsConfig.GetMediaDir()+"\\*.css");

		// Collect all style sheets
		while(bFound)
		{
			CString fileName = finder.GetFileName();
			styleNames.push_back(fileName);
			bFound = finder.FindNextFile();
		}
		sort(styleNames.begin(), styleNames.end());

		// Create the menu
		CMenu menu;
		menu.CreatePopupMenu();
		for (size_t iMenu = 0; iMenu < styleNames.size(); ++iMenu)
		{
			menu.AppendMenu(MF_STRING | MF_BYPOSITION, ID_IE_STYLE_FIRST + iMenu, styleNames[iMenu].Left(styleNames[iMenu].Delete(0, 0) - 4) );
			if (styleNames[iMenu].CompareNoCase(selectedStyle) == 0)
				menu.CheckMenuItem(ID_IE_STYLE_FIRST + (UINT)iMenu, MF_CHECKED);
		}

		int nCmd = menu.TrackPopupMenuEx(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, 
			pt.x, pt.y, m_hWnd, NULL);

		if(nCmd != 0)
		{
			int index = nCmd - ID_IE_STYLE_FIRST;
			g_GreatNewsConfig.SetCurrentStyle(styleNames[index]);
			BuildHtmlPage(CContentGenerator::CurrentPage);
		}

		return TBDDRET_DEFAULT;
	}
	else if (pnmTB ->iItem == ID_IE_FAVI)
	{
		CMenu menu;
		menu.LoadMenu(IDR_FAVI_POPUP);
		CMenu popupMenu = menu.GetSubMenu(0);
		if(!GetInternetURL().GetLength())
		{
			// disable Add Fav
			popupMenu.EnableMenuItem(ID_FAVI_ADDTOFAVORITES, MF_BYCOMMAND|MF_GRAYED);
		}

		// find out from the registry where the favorites are located.
		TCHAR           sz[MAX_PATH];
		TCHAR           szPath[MAX_PATH];
		HKEY            hKey;
		DWORD           dwSize;
		if(RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"), &hKey) != ERROR_SUCCESS)
		{
			ATLTRACE("Favorites folder not found\n");
			return TBDDRET_DEFAULT;
		}
		dwSize = sizeof(sz);
		RegQueryValueEx(hKey, _T("Favorites"), NULL, NULL, (LPBYTE)sz, &dwSize);
		ExpandEnvironmentStrings(sz, szPath, MAX_PATH);
		RegCloseKey(hKey);

		std::vector<CString> favorites;
		BuildFavoritesMenu(szPath, 0, popupMenu, favorites);

		int nCmd = popupMenu.TrackPopupMenuEx(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, 
			pt.x, pt.y, m_hWnd, NULL);
		if(nCmd == ID_FAVI_ADDTOFAVORITES || nCmd == ID_FAVI_ORGANIZEFAVORITES)
			PostMessage(WM_COMMAND, nCmd);
		else if(nCmd >= ID_FAVI_FIRST)
		{
			int index = nCmd - ID_FAVI_FIRST;
			m_wndIE->Navigate2(favorites[index]);
		}
		
		return TBDDRET_DEFAULT;
	}

	return TBDDRET_TREATPRESSED;
}


void CBrowserHost::SetHighlightAfterNavigate(const CString& str)
{
	m_wndIE->SetHighlightString(str);
}

void CBrowserHost::ResizeAddressCombo()
{
#define ADDRESSBAR_MARGIN 25

	// resize address combobox
	RECT rcAddress = {0};
	::GetWindowRect(m_addrBar, &rcAddress);
	if(::IsWindow(m_cboAddress) && rcAddress.right-rcAddress.left > m_minAddressWidth + ADDRESSBAR_MARGIN)
	{
		TBBUTTONINFO tbi;
		RECT rc;
		tbi.cbSize = sizeof TBBUTTONINFO;
		tbi.dwMask = TBIF_SIZE;
		tbi.cx = (unsigned short)(rcAddress.right-rcAddress.left-ADDRESSBAR_MARGIN);
		m_addrBar.SetButtonInfo(ID_ADDRESS_PLACEHOLDER_COMBO, &tbi);
		if(m_addrBar.GetItemRect(0, &rc))
		{
			::SetWindowPos(m_cboAddress, NULL, rc.left, rc.top,
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}


LRESULT CBrowserHost::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CRect rect;
	GetClientRect(&rect);

	if(m_rebar != NULL && ((DWORD)::GetWindowLong(m_navigateBar, GWL_STYLE) & WS_VISIBLE))
	{
		::SendMessage(m_rebar, WM_SIZE, 0, 0);
		RECT rectTB = { 0 };
		::GetWindowRect(m_rebar, &rectTB);

		rect.top += rectTB.bottom - rectTB.top;
	}

#define TAB_HEIGHT 24

	if(m_wndBrowserTabBar != NULL && ((DWORD)::GetWindowLong(m_wndBrowserTabBar, GWL_STYLE) & WS_VISIBLE))
	{
		CRect rc = rect;
		rc.bottom = rc.top + TAB_HEIGHT;
		::SetWindowPos(m_wndBrowserTabBar, NULL, rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOZORDER | SWP_NOACTIVATE);
		m_wndBrowserTabBar.UpdateLayout();

		rect.top += TAB_HEIGHT;
	}


	// resize client window
	// rect.InflateRect(2,2);
	if(m_wndIE != NULL && m_wndIE->m_hWnd != NULL)
		::SetWindowPos(m_wndIE->m_hWnd, NULL, rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			SWP_NOZORDER | SWP_NOACTIVATE);

	return 0;
}

LRESULT CBrowserHost::OnBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_wndIE->GoBack();
	return 0;
}

LRESULT CBrowserHost::OnForward(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_wndIE->GoForward();
	return 0;
}

LRESULT CBrowserHost::OnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_wndIE->Stop();
	return 0;
}

LRESULT CBrowserHost::OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_wndIE->Refresh();
	return 0;
}

LRESULT CBrowserHost::OnHome(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if(m_contentGenerator)
	{
		CString homeURL = m_contentGenerator->GetHomeURL();
		if(homeURL.GetLength()>0)
			m_wndIE->Navigate2(homeURL);
	}

	return 0;
}

void CBrowserHost::Navigate(LPCTSTR url, int navOption, TabOptions tabOption)
{
	switch(tabOption)
	{
	case CurrentTab:
		m_wndIE->Navigate2(url, navOption);
		break;
	case NewTabBackground:
		NewTab(url);
		break;
	case NewTabForeground:
		NewTab(url,false);
		break;
	}
}

LRESULT CBrowserHost::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	ATLASSERT(::IsWindow(m_hWnd));
	m_commandParser.SetNotifyWnd(m_hWnd);

	HWND wndNaviBar = CreateNaviBar();
	HWND wndAddressBar = CreateAddressBar();
	m_rebar = CFrameWindowImplBase<>::CreateSimpleReBarCtrl(m_hWnd, REBAR_STYLE, 200);
	CFrameWindowImplBase<>::AddSimpleReBarBandCtrl(m_rebar, wndNaviBar,100);
	CFrameWindowImplBase<>::AddSimpleReBarBandCtrl(m_rebar, wndAddressBar,101,_T("Address"),0,0,TRUE);
	SizeSimpleReBarBands();

	DWORD tabStyle = WS_CHILD|CTCS_FLATEDGE|CTCS_HOTTRACK|CTCS_CLOSEBUTTON|CTCS_BOLDSELECTEDTAB|CTCS_TOOLTIPS|CTCS_FLATEDGE;
	if(!g_GreatNewsConfig.m_bDisableTabbedBrowsing)
		tabStyle |= WS_VISIBLE;
	m_wndBrowserTabBar.Create(m_hWnd, rcDefault,NULL,tabStyle, 0, CHILD_TAB);
	CImageList imageList;
	imageList.CreateFromImage( IDB_BROWSERSTATUS, 16, 1, RGB(192,192,192), IMAGE_BITMAP, LR_CREATEDIBSECTION );
	m_wndBrowserTabBar.SetImageList(imageList);

	// create a tab
	NewTab(BLANK_PAGE);

	UIAddToolBar(wndNaviBar);

	return 0;
}

HWND CBrowserHost::CreateAddressBar()
{
	HWND wndAddressBar = CFrameWindowImplBase<>::CreateSimpleToolBarCtrl(m_hWnd, IDR_ADDRESSBAR, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	if(!wndAddressBar)
		return 0;
	m_addrBar = wndAddressBar;

	CImageList imageList;
	imageList.CreateFromImage( IDR_ADDRESSBAR, 16, 1, RGB(192,192,192), IMAGE_BITMAP, LR_CREATEDIBSECTION );
	m_addrBar.SetImageList(imageList);

	CSize sizeChar = GetGUIFontSize();
	int cx = ADDR_COMBO_SIZE * sizeChar.cx;
	m_minAddressWidth = cx;

	TBBUTTONINFO tbi;
	RECT rc;
	tbi.cbSize = sizeof TBBUTTONINFO;
	tbi.dwMask = TBIF_STYLE | TBIF_SIZE;
	tbi.fsStyle = TBSTYLE_SEP;
	tbi.cx = (unsigned short)cx;
	m_addrBar.SetButtonInfo(ID_ADDRESS_PLACEHOLDER_COMBO, &tbi);
	m_addrBar.GetItemRect(0, &rc);

	rc.bottom = TOOLBAR_COMBO_DROPLINES * sizeChar.cy;
	rc.left += 1; // slight offset from previous and next buttons.
	rc.right -= 1;
	DWORD dwStyle = CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL;
	// DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL;
	m_cboAddress.Create(m_hWnd, rc, _T(""), dwStyle, 0, 210);
	m_cboAddress.SetParent(wndAddressBar);
	m_cboAddress.SetFont(AtlGetDefaultGuiFont());

	SHAutoComplete(m_cboAddress.GetEditCtrl(),SHACF_URLMRU|SHACF_USETAB);

	return wndAddressBar;
}

HWND CBrowserHost::CreateNaviBar()
{
	m_navigateBar = CFrameWindowImplBase<>::CreateSimpleToolBarCtrl(m_hWnd, IDR_NAVIBAR,FALSE, MY_TOOLBAR_STYLE);
	ATLASSERT(::IsWindow(m_navigateBar));

	CImageList imageList;
	imageList.CreateFromImage( IDR_NAVIBAR, 16, 1, RGB(192,192,192), IMAGE_BITMAP, LR_CREATEDIBSECTION );
	m_navigateBar.SetImageList(imageList);

	m_navigateBar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	m_navigateBar.SetExtendedStyle(m_navigateBar.GetExtendedStyle() | /*TBSTYLE_EX_HIDECLIPPEDBUTTONS |*/ TBSTYLE_EX_MIXEDBUTTONS);

	// style button
	TBBUTTONINFO info = {0};
	info.cbSize = sizeof(TBBUTTONINFO);
	info.dwMask = TBIF_STYLE;
	m_navigateBar.GetButtonInfo(ID_IE_STYLE, &info);
	// info.iImage = I_IMAGENONE;
	info.fsStyle |= BTNS_SHOWTEXT | BTNS_AUTOSIZE;
	info.fsStyle |= TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN;
	info.dwMask |= TBIF_TEXT;
	info.pszText = _T("Style");
	m_navigateBar.SetButtonInfo(ID_IE_STYLE, &info);

	// favi menu
	info.dwMask = TBIF_STYLE;
	m_navigateBar.GetButtonInfo(ID_IE_FAVI, &info);
	info.fsStyle |= TBSTYLE_DROPDOWN | BTNS_WHOLEDROPDOWN;
	m_navigateBar.SetButtonInfo(ID_IE_FAVI, &info);

	return m_navigateBar;
}

void CBrowserHost::SizeSimpleReBarBands()
{
	ATLASSERT(::IsWindow(m_rebar));   // must be an existing rebar

	int nCount = (int)::SendMessage(m_rebar, RB_GETBANDCOUNT, 0, 0L);

	for(int i = 0; i < nCount; ++i)
	{
		REBARBANDINFO rbBand = { 0 };
		rbBand.cbSize = sizeof(REBARBANDINFO);
		rbBand.fMask = RBBIM_SIZE;
		BOOL bRet = (BOOL)::SendMessage(m_rebar, RB_GETBANDINFO, i, (LPARAM)&rbBand);
		ATLASSERT(bRet);
		RECT rect = { 0, 0, 0, 0 };
		::SendMessage(m_rebar, RB_GETBANDBORDERS, i, (LPARAM)&rect);
		rbBand.cx += rect.left + rect.right;
		bRet = (BOOL)::SendMessage(m_rebar, RB_SETBANDINFO, i, (LPARAM)&rbBand);
		ATLASSERT(bRet);
	}
}

CSize CBrowserHost::GetGUIFontSize()
{
	CClientDC dc(m_hWnd);
	dc.SelectFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));		
	TEXTMETRIC tm;
	dc.GetTextMetrics( &tm );
	return CSize( tm.tmAveCharWidth, tm.tmHeight + tm.tmExternalLeading);
}

LRESULT CBrowserHost::OnGo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CWaitCursor wc;

	try
	{
		CString url;
		m_cboAddress.GetWindowText(url);
		if(url.GetLength()==0)
			return 0;

#ifdef _DEBUG
		if(url.Find(_T("sql:")) == 0) // is this a sql hack?
		{
			CString result = FeedManagerLib::DebugSql(url.Mid(4));

			CString fileName = CGNUtil::GetTempPathName();
			fileName.Append(_T("~greatnewsDebug.htm"));
			CGNUtil::WriteToFile(fileName, result);
			Navigate(_T("file:")+fileName, navNoHistory);

		}
		else
#endif
			m_wndIE->Navigate2(url);

	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CBrowserHost::OnAddressSelectOK(int, int, HWND, BOOL &bHandled)
{
	PostMessage(WM_COMMAND, ID_ADDRESS_GO,0);
	return 0;
}


LRESULT CBrowserHost::OnAddressDropdown(int, int, HWND, BOOL &bHandled)
{
	CString key = "Software\\Microsoft\\Internet Explorer\\TypedURLs";
	CRegKey reg;
	if(ERROR_SUCCESS != reg.Open(HKEY_CURRENT_USER, key))
		return 0; // something's wrong

	m_cboAddress.ResetContent();

	CString valueName;
	TCHAR value[255];
	COMBOBOXEXITEM cbei = {0};
	cbei.mask = CBEIF_TEXT;
	cbei.iItem = -1;

	for(int i = 1; i <= 20; ++i)
	{
		ULONG size = sizeof(value)/sizeof(TCHAR);
		valueName.Format(_T("url%d"), i);
		if(reg.QueryStringValue(valueName, value, &size) != ERROR_SUCCESS)
      break;
		cbei.pszText = value;
		cbei.cchTextMax = size;
		m_cboAddress.InsertItem(&cbei);
	}

	return 0;
}


LRESULT CBrowserHost::OnAddressEndEdit(int idCtrl, LPNMHDR pnmh, BOOL &bHandled)
{
	PNMCBEENDEDIT pInfo = (PNMCBEENDEDIT)pnmh;
	if(pInfo->iWhy == CBENF_RETURN)
	{
		if(GetKeyState( VK_CONTROL ) & KEYDOWN_MASK)
		{
			CString text;
			m_cboAddress.GetWindowText(text);
			text = _T("http://www.")+text+_T(".com");
			m_cboAddress.SetWindowText(text);
		}

		OnGo(0,0,0);
	}

	return 0;
}

void CBrowserHost::BuildHtmlPage(int nPage)
{
	StopMarkReadTimer();

	if(!m_contentGenerator)
		return;

	CString html = m_contentGenerator->GeneratePageHTML(nPage);
	if(html.GetLength())
	{
		CString fileName = CGNUtil::GetTempPathName();
		fileName.Append(_T("~")+m_contentGenerator->GetContentID()+_T(".htm"));
		//Navigate(BLANK_PAGE, navNoHistory);
		WriteToFile(fileName, html);
		Navigate(_T("file:")+fileName, navNoHistory);
	}
	else
		Navigate(BLANK_PAGE);

	if(g_GreatNewsConfig.m_nSecondsToMarkItemRead && m_contentGenerator->HasUnreadItem())
		SetTimer(IDT_MARKPAGEREAD, g_GreatNewsConfig.m_nSecondsToMarkItemRead*1000);
}
void CBrowserHost::SetContent(ContentGeneratorPtr pGen)
{
	ResetFlags();

	if(!pGen)
	{
		Navigate(BLANK_PAGE);
		return;
	}

	ActivateFeedTab();

	CGNTabItem* pItem = (CGNTabItem*)m_wndBrowserTabBar.GetItem(
						m_wndBrowserTabBar.GetCurSel());
	if(pItem)
		pItem->SetContentGenerator(pGen);
	m_contentGenerator = pGen;

	if(pGen->GetNumOfItems())
		BuildHtmlPage(0);
	else
	{
		CString fileName = g_GreatNewsConfig.m_appDir+_T("\\Media\\NoItem.htm");
		Navigate(fileName);
	}
}

void CBrowserHost::RefreshContent()
{
	if(m_contentGenerator)
		BuildHtmlPage(CContentGenerator::CurrentPage);
}

void CBrowserHost::GotoPage(int pageNo)
{
	if(!m_contentGenerator)
		Navigate(BLANK_PAGE);
	else
		BuildHtmlPage(pageNo);
}

bool CBrowserHost::WriteToFile(const CString& fileName, const CString& content)
{
	CString baseTag;
	CString style = g_GreatNewsConfig.GetCurrentStyleString();
	if(m_contentGenerator)
	{
		CString homeURL = m_contentGenerator->GetHomeURL();
		if(homeURL.GetLength()>0)
			baseTag.Format(_T("<base href=\"%s\">"), (LPCTSTR)homeURL);

		CString styleName = m_contentGenerator->GetStyle();
		if(styleName.GetLength())
			style = g_GreatNewsConfig.GetStyleFileContent(styleName);
	}

	CString html;
	if(!g_GreatNewsConfig.m_bLegacyBrowser)
		html.Append(_T("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Strict//EN\">\r\n"));

	if(g_GreatNewsConfig.m_bInternetZone)
		html.Append(_T("<!-- saved from url=(0014)about:internet -->\r\n"));

	html.AppendFormat(
    _T("<html>\n<head>\n")
    _T("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n")
    _T("<title>%s</title>\n")
    _T("<style type=\"text/css\">\n%s\n%s\n</style>\n")
    _T("%s</head>\n<body>\n%s\n</body>\n</html>\n"),
    m_contentGenerator->GetTitle(),
    g_GreatNewsConfig.GetBuiltInStyle(),
    style,
    baseTag,
    content);

	return CGNUtil::WriteToFile(fileName, html);
}

void CBrowserHost::OnUrl(const CString& url)
{
	m_cboAddress.SetWindowText(url);
	UIEnable(ID_IE_STYLE,(url.GetLength()==0));
}

void CBrowserHost::OnFoundFeed(size_t nNumOfFeeds)
{
	GetParent().PostMessage(WM_COMMAND, ID_IE_FOUNDFEEDS, 0);
}

void CBrowserHost::OnStatusTextChange(const CString& szText)
{
	if(m_statusText == szText)
    return; // nothing to do
	m_statusText = szText;
	GetParent().PostMessage(WM_COMMAND, ID_IE_SETSTATUSTEXT, NULL);
}

BOOL CBrowserHost::OnNaviOpml(const CString& url)
{
	CGNClipBoard clipboard;
	if(clipboard.Open(m_hWnd))
	{
		USES_CONVERSION;
		clipboard.SetTextData(T2A((LPTSTR)(LPCTSTR)url));
		clipboard.SetUnicodeTextData(T2W((LPTSTR)(LPCTSTR)url));

		GetParent().PostMessage(WM_COMMAND, ID_TOOLS_IMPORT,0);
	}

	return TRUE; // cannel navigation
}

BOOL CBrowserHost::OnNaviXML(const CString& url)
{
	ULONG_PTR nChannelId = CNewsFeed::GetIdFromXmlUrl(url);
	if(nChannelId != 0)
	{
		if(IDYES == MessageBox(ResManagerPtr->GetString(IDS_OPENEXISTINGCHANNEL),
			_T("GreatNews"), MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1))
		{
			GetParent().PostMessage(WM_COMMAND, ID_IE_OPENCHANNEL, (LPARAM)nChannelId);
		}

		return TRUE; // cannel navigation
	}

	if(IDYES == MessageBox(ResManagerPtr->GetString(IDS_SUBSCRIIBECHANNEL),
		_T("GreatNews"), MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1))
	{
		m_navigateXmlURL = url;
		GetParent().PostMessage(WM_COMMAND, ID_IE_NAVIGATEXML, NULL);

		return TRUE; // cannel navigation
	}
	else
	{
		return FALSE;
	}
}

LRESULT CBrowserHost::OnRebarChildSize(int idCtrl, LPNMHDR pnmh, BOOL &bHandled)
{
	LPNMREBARCHILDSIZE lprbcs = (LPNMREBARCHILDSIZE) pnmh;
	if(lprbcs->wID == 101)
		ResizeAddressCombo();
	return 0;
}

CString CBrowserHost::GetInternetURL()
{
	if(m_wndIE == NULL)
		return "";

	CString url = m_wndIE->GetLocationURL();
	if(url.Find(_T("http://"))==0 || url.Find(_T("https://"))==0)
		return url;

	return "";
}

void CBrowserHost::OnSetFocus(HWND hOldWnd)
{
	return;
}

void CBrowserHost::ShowFindDialog()
{
	if(m_wndIE != NULL)
		m_wndIE->ExecCmdTarget(HTMLID_FIND);
}

void CBrowserHost::ShowSaveAsDialog()
{
	if(m_wndIE != NULL)
		m_wndIE->ExecWB(OLECMDID_SAVEAS, OLECMDEXECOPT_DODEFAULT);
}


LRESULT CBrowserHost::OnToolTipTextA(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	LPNMTTDISPINFOA pDispInfo = (LPNMTTDISPINFOA)pnmh;
	pDispInfo->szText[0] = 0;

	if((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))
	{
		const int cchBuff = 256;
		char szBuff[cchBuff];
		szBuff[0] = 0;
#if (_ATL_VER >= 0x0700)
		int nRet = ::LoadStringA(ATL::_AtlBaseModule.GetResourceInstance(), idCtrl, szBuff, cchBuff);
#else //!(_ATL_VER >= 0x0700)
		int nRet = ::LoadStringA(_Module.GetResourceInstance(), idCtrl, szBuff, cchBuff);
#endif //!(_ATL_VER >= 0x0700)
		for(int i = 0; i < nRet; ++i)
		{
			if(szBuff[i] == '\n')
			{
				lstrcpynA(pDispInfo->szText, &szBuff[i + 1], sizeof(pDispInfo->szText) / sizeof(pDispInfo->szText[0]));
				break;
			}
		}
#if (_WIN32_IE >= 0x0300)
		if(nRet > 0)   // string was loaded, save it
			pDispInfo->uFlags |= TTF_DI_SETITEM;
#endif //(_WIN32_IE >= 0x0300)
	}

	return 0;
}

LRESULT CBrowserHost::OnToolTipTextW(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	LPNMTTDISPINFOW pDispInfo = (LPNMTTDISPINFOW)pnmh;
	pDispInfo->szText[0] = 0;

	if((idCtrl != 0) && !(pDispInfo->uFlags & TTF_IDISHWND))
	{
		const int cchBuff = 256;
		wchar_t szBuff[cchBuff];
		szBuff[0] = 0;
#if (_ATL_VER >= 0x0700)
		int nRet = ::LoadStringW(ATL::_AtlBaseModule.GetResourceInstance(), idCtrl, szBuff, cchBuff);
#else //!(_ATL_VER >= 0x0700)
		int nRet = ::LoadStringW(_Module.GetResourceInstance(), idCtrl, szBuff, cchBuff);
#endif //!(_ATL_VER >= 0x0700)
		for(int i = 0; i < nRet; ++i)
		{
			if(szBuff[i] == L'\n')
			{
				lstrcpynW(pDispInfo->szText, &szBuff[i + 1], sizeof(pDispInfo->szText) / sizeof(pDispInfo->szText[0]));
				break;
			}
		}
#if (_WIN32_IE >= 0x0300)
		if(nRet > 0)   // string was loaded, save it
			pDispInfo->uFlags |= TTF_DI_SETITEM;
#endif //(_WIN32_IE >= 0x0300)
	}

	return 0;
}

size_t CBrowserHost::GetDiscoveredFeeds(std::map<CString, CString>& discoveredFeeds)
{
	discoveredFeeds.clear();
	if(m_wndIE)
		discoveredFeeds = m_wndIE->m_discoveredFeeds;

	return discoveredFeeds.size();
}

int CBrowserHost::BuildFavoritesMenu(LPCTSTR pszPath, int nStartPos, CMenu& menu, std::vector<CString>& favorites)
{
	CString         strPath(pszPath);
	CString         strPath2;
	CString         str;
	std::map<CString, CString> mapTitleUrl;

	int             nEndPos = nStartPos;
	TCHAR           buf[INTERNET_MAX_PATH_LENGTH];

	// make sure there's a trailing backslash
	if(strPath[strPath.GetLength() - 1] != _T('\\'))
		strPath += _T('\\');
	strPath2 = strPath;
	strPath += "*.*";

	// now scan the directory, first for .URL files and then
	// for subdirectories that may also contain .URL files
	CFindFile finder;
	BOOL bFound = finder.FindFile(strPath);
	if(bFound)
	{
		// get all directories first
		do
		{
			if(finder.IsDirectory())
			{
				CString fileName = finder.GetFileName();

				// ignore the current and parent directory entries
				if(fileName == _T(".") || fileName == _T(".."))
					continue;

				mapTitleUrl.insert(std::map<CString, CString>::value_type(fileName, finder.GetFilePath()));
			}
		} while(finder.FindNextFile());

		for(std::map<CString, CString>::iterator it = mapTitleUrl.begin(); it != mapTitleUrl.end(); ++it)
		{
			CMenu pSubMenu = ::CreatePopupMenu();

			// call this function recursively.
			int nNewEndPos = BuildFavoritesMenu(it->second, nEndPos, pSubMenu, favorites);
			if(nNewEndPos != nEndPos)
			{
				// only intert a submenu if there are in fact .URL files in the subdirectory
				nEndPos = nNewEndPos;
				menu.AppendMenu(MF_POPUP | MF_STRING, (UINT)pSubMenu.m_hMenu, it->first);
				pSubMenu.Detach();
			}
		}

		// now get the root favorites
		mapTitleUrl.clear();
		finder.FindFile(strPath);
		do
		{
			if((finder.m_fd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))==0)
			{
				CString fileName = finder.GetFileName();
				if(fileName.Right(4) == _T(".url"))
				{
					// an .URL file is formatted just like an .INI file, so we can
					// use GetPrivateProfileString() to get the information we want
					::GetPrivateProfileString(_T("InternetShortcut"), _T("URL"),
											  _T(""), buf, INTERNET_MAX_PATH_LENGTH,
											  strPath2 + fileName);
					fileName = fileName.Left(fileName.GetLength() - 4);

					mapTitleUrl.insert(std::map<CString, CString>::value_type(fileName, buf));
				}
			}
		} while(finder.FindNextFile());

		// Now add these items to the menu
		for(std::map<CString, CString>::iterator it = mapTitleUrl.begin(); it != mapTitleUrl.end(); ++it, ++nEndPos)
		{
			menu.AppendMenu(MF_STRING | MF_ENABLED, ID_FAVI_FIRST + nEndPos, it->first);
			favorites.push_back(it->second);
		}

	}
	return nEndPos;
}

LRESULT CBrowserHost::OnAddFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	IShellUIHelper* pShellUIHelper;
	HRESULT hr = ::CoCreateInstance(CLSID_ShellUIHelper, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellUIHelper, (LPVOID*)&pShellUIHelper);
	if (SUCCEEDED(hr))
	{
		CString title = m_wndIE->GetLocationName();
		_variant_t vtTitle((LPCTSTR)title);
		CString strURL = m_wndIE->GetLocationURL();

		_bstr_t bstr = (LPCTSTR)strURL;
		hr = pShellUIHelper->AddFavorite(bstr, &vtTitle);
		pShellUIHelper->Release();
	}

	return 0;
}

LRESULT CBrowserHost::OnOrganizeFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	IShellUIHelper* pShellUIHelper;
	HRESULT hr = ::CoCreateInstance(CLSID_ShellUIHelper, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellUIHelper, (LPVOID*)&pShellUIHelper);
	if (SUCCEEDED(hr))
	{
		hr = pShellUIHelper->ShowBrowserUI(_bstr_t(_T("OrganizeFavorites")), NULL, NULL);
		pShellUIHelper->Release();
	}

	return 0;
}

BOOL CBrowserHost::OnNewWindow2(IDispatch** ppDisp)
{
	if(m_bNewWindow3InUse) // if we have new browser(IE6SP2), NewWindow3() already taken care of popups
		return FALSE; //allow

	// stuff in popup browser, so we can get the url, 
	// then this operation will continue in OnPopBeforeNav
	if(!::IsWindow(m_popupBrowser.m_hWnd))
	{
		const DWORD dwPopupStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_HSCROLL | WS_VSCROLL;
		m_popupBrowser.Create(m_hWnd, rcDefault, BLANK_PAGE, dwPopupStyle, 0);
		ATLASSERT(::IsWindow(m_popupBrowser.m_hWnd));			
	}

	HRESULT hr = m_popupBrowser.GetDispatch(ppDisp);
	ATLASSERT(hr == S_OK && *ppDisp != NULL);
	
	return FALSE; // allow
}

void CBrowserHost::OnPopBeforeNav(const CString& szURL)
{
	bool userInited = m_bUserClickedLink;
	ResetFlags();

	HandlePopup(userInited, szURL);
}

BOOL CBrowserHost::OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
	m_bNewWindow3InUse = true;
	ResetFlags();

	bool userInited = ((dwFlags & NWMF_USERINITED) && (dwFlags & NWMF_FIRST)); 
	CString url = bstrUrl;
	return HandlePopup(userInited, url);
}

BOOL CBrowserHost::HandlePopup(bool bUserInited, CString url)
{
	if(CGNUtil::IsFeedUrl(url) && OnNaviXML(url))
		return TRUE;
	else if(CGNUtil::IsOpmlUrl(url) && OnNaviOpml(url))
		return TRUE;

	if(g_GreatNewsConfig.m_nBlockPopups == 0 || bUserInited)
	{
		if(!g_GreatNewsConfig.m_bDisableTabbedBrowsing)
			NewTab(url,false);
		else
			CGNUtil::OpenUrlExternally(url, false, true);

		return TRUE; //block it because we opened it in new window
	}
	else
	{
		PopupBlocked(url);

		return TRUE; // block it
	}

	return FALSE; // shouldn't come here
}

void CBrowserHost::OnDestroy()
{
	if(::IsWindow(m_popupBrowser.m_hWnd))
		m_popupBrowser.DestroyWindow();

	SetMsgHandled(false);

	return;
}

BOOL CBrowserHost::OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders)
{
	ResetFlags();

	if(m_commandParser.ProcessCommand(szURL))
		return TRUE; // cancel the navigation. It's a GN command

	if(m_bOpenInDefaultBrowser
		&& IsFeedTabActive())	// only open externally on feed tab
	{
		m_bOpenInDefaultBrowser = false;

		if(CGNUtil::IsInternetUrl(szURL)
			&& !IsGreatNewsSpecialUrl(szURL))
		{
			//::ShellExecute(NULL, _T("open"), (LPCTSTR)szURL, NULL, NULL, SW_SHOWNORMAL);
			CGNUtil::OpenUrlExternally(szURL, false, true);
			// NewTab(szURL);

			return TRUE; // cancel it, we opened it in external default browser;
		}
	}

	CString url = szURL;
	if(CGNUtil::IsFeedUrl(url))
		return OnNaviXML(url);
	else if(CGNUtil::IsOpmlUrl(url))
		return OnNaviOpml(url);

	// clear 
	m_wndIE->m_discoveredFeeds.clear();
	OnFoundFeed(0);

	m_blockedURLs.clear();
	GetParent().PostMessage(WM_COMMAND, ID_IE_POPUPBLOCKED, 0);

	return FALSE; // allow
}

void CBrowserHost::PopupBlocked(const CString& url)
{
	m_wndIE->PutSilent(TRUE);

	if(!m_bNewWindow3InUse)
	{
		CString sUrl(url);
		if(CGNUtil::IsFeedUrl(sUrl))
		{
			OnNaviXML(sUrl);
			return;
		}
		else if(CGNUtil::IsOpmlUrl(sUrl))
		{
			OnNaviOpml(sUrl);
			return;
		}
	}
	
	AtlTrace(_T("Popup blocked[%s]\n"), (LPCTSTR)url);
	m_blockedURLs.insert(url);
	GetParent().PostMessage(WM_COMMAND, ID_IE_POPUPBLOCKED);
}

void CBrowserHost::OnTimer(UINT_PTR wParam)
{
	if(wParam == IDT_MARKPAGEREAD)
	{
		StopMarkReadTimer();
		if(g_GreatNewsConfig.m_nMarkReadAutomatically)
			GetParent().PostMessage(WM_COMMAND, CMD_ID_MARKPAGEREAD);
	}
	else
		SetMsgHandled(FALSE);
}

void CBrowserHost::StopMarkReadTimer()
{
	KillTimer(IDT_MARKPAGEREAD);
}

void CBrowserHost::OnMouseDown(long button, bool ctrl, bool bUserClickedLink, const CString& url)
{
}

void CBrowserHost::ApplyLanguage()
{
	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();

	pResMngr->ApplyLanguageToRebar(m_rebar);

}

void CBrowserHost::ResetFlags()
{
	m_bUserClickedLink = false;
	m_bOpenInDefaultBrowser = false;
}	

void CBrowserHost::NextTab()
{
	if(m_wndBrowserTabBar.GetItemCount() <=1)
		return;

	int item = m_wndBrowserTabBar.GetCurSel()+1;
	if(item >= m_wndBrowserTabBar.GetItemCount())
		item = 0;

	m_wndBrowserTabBar.SetCurSel(item);
}

void CBrowserHost::PreviousTab()
{
	if(m_wndBrowserTabBar.GetItemCount() <= 1)
		return;

	int item = m_wndBrowserTabBar.GetCurSel() - 1;
	if (item < 0)
		item = m_wndBrowserTabBar.GetItemCount() - 1;

	m_wndBrowserTabBar.SetCurSel(item);

}

void CBrowserHost::RemoveTab(int item)
{
	if (item < 0)
		item = m_wndBrowserTabBar.GetCurSel();
	
	if (item < 0) // how come?
		return;

	// remove the tab, or if this is the last tab, navigate to blank
	if(m_wndBrowserTabBar.GetItemCount()>1)
		m_wndBrowserTabBar.DeleteItem(item);
	else if(m_wndIE)
		m_wndIE->Navigate2(BLANK_PAGE);
}

void CBrowserHost::NewTab(LPCTSTR url, bool bOnBackGround)
{

	if(m_wndBrowserTabBar.GetItemCount() && g_GreatNewsConfig.m_bDisableTabbedBrowsing)
		CGNUtil::OpenUrlExternally(url, bOnBackGround, true);
	else
	{
		CGNTabItem* pItem = new CGNTabItem();
		pItem->m_hwndNotify = this->m_hWnd;
		pItem->SetForwardAdvisor(this);

		FeedBrowserPtr newBrowser = new CFeedBrowser(*pItem);
		HWND hwndBrowser = newBrowser->Create(m_hWnd, g_GreatNewsConfig.m_bUseMozillaEngine);
		ATLASSERT(::IsWindow(hwndBrowser));
		newBrowser->Offline = m_bWorkOffline;

		pItem->m_browser = newBrowser;
		pItem->SetImageIndex(0);
		int nNewItem = m_wndBrowserTabBar.InsertItem(m_wndBrowserTabBar.GetItemCount(), pItem);

		if (!bOnBackGround)
			m_wndBrowserTabBar.SetCurSel(nNewItem);

		if (url != BLANK_PAGE)
			newBrowser->Navigate(url);
	}
}

LRESULT CBrowserHost::OnTabSelChange (int idCtrl, LPNMHDR pnmh, BOOL &bHandled) 
{
	NMCTC2ITEMS* nmh = (NMCTC2ITEMS*)pnmh;

	// handle old tab
	if(nmh->iItem1 >=0)
	{
		CGNTabItem* pOldItem = (CGNTabItem*)m_wndBrowserTabBar.GetItem(nmh->iItem1);
		pOldItem->SetForwardAdvisor(NULL);
		pOldItem->ShowBrowser(SW_HIDE);
	}

	// handle new tab
	CGNTabItem* pItem = (CGNTabItem*)m_wndBrowserTabBar.GetItem(nmh->iItem2);
	pItem->SetForwardAdvisor(this);
	m_wndIE = pItem->m_browser;
	m_contentGenerator = pItem->GetContentGenerator();
	pItem->ShowBrowser();
	
	std::map<CString, CString> discoveredFeeds;
	size_t nCount = GetDiscoveredFeeds(discoveredFeeds);
	OnFoundFeed(nCount);
	OnUrl(pItem->m_currentURL);

	OnSize(0,0,0, bHandled);

	return 1;
}

LRESULT CBrowserHost::OnTabClose (int idCtrl, LPNMHDR pnmh, BOOL &bHandled) 
{
	if(m_wndBrowserTabBar.GetItemCount()<=1)
		return 0;

	NMCTCITEM* pNM = (NMCTCITEM*)pnmh;
	m_wndBrowserTabBar.DeleteItem(pNM->iItem);

	return 0;
}

LRESULT CBrowserHost::OnTabDeleteItem (int idCtrl, LPNMHDR pnmh, BOOL &bHandled) 
{
	NMCTCITEM* pNM = (NMCTCITEM*)pnmh;
	CGNTabItem* pItem = (CGNTabItem*)m_wndBrowserTabBar.GetItem(pNM->iItem);
	pItem->SetForwardAdvisor(NULL);
	pItem->SetContentGenerator(NULL);
	pItem->m_browser->DestroyWindow();
	pItem->m_browser = NULL;

	return 0;
}

LRESULT CBrowserHost::OnTabMClick (int idCtrl, LPNMHDR pnmh, BOOL &bHandled) 
{
	NMCTCITEM* pNM = (NMCTCITEM*)pnmh;

	if(pNM->iItem >= 0)
		RemoveTab(pNM->iItem);

	return 1;
}

LRESULT CBrowserHost::OnTabDblClick (int idCtrl, LPNMHDR pnmh, BOOL &bHandled) 
{
	NMCTCITEM* pNM = (NMCTCITEM*)pnmh;
	if (pNM->iItem >= 0) // clicked on tab?
		RemoveTab(pNM->iItem);
	else // dbl clicked at empty place?
		GetParent().PostMessage(WM_COMMAND, ID_VIEW_NEWTAB, 0);
	return 1;
}

LRESULT CBrowserHost::OnTabRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if(m_wndBrowserTabBar != NULL && m_wndBrowserTabBar.IsWindowVisible())
	{
		m_wndBrowserTabBar.UpdateLayout();
		m_wndBrowserTabBar.Invalidate();
	}
	return 0;
}

void CBrowserHost::ActivateFeedTab()
{
	for(int i = 0; i < m_wndBrowserTabBar.GetItemCount(); ++i)
	{
		CGNTabItem* pItem = (CGNTabItem*)m_wndBrowserTabBar.GetItem(i);
		ContentGeneratorPtr gen = pItem->GetContentGenerator();
		if(gen != NULL)
		{
			// disable scripts etc
			pItem->m_browser->SetBrowserProperties(SAFE_BROWSER_PROPERTIES);

			// this is the feed tab
			if(m_wndBrowserTabBar.GetCurSel() != i)
				m_wndBrowserTabBar.SetCurSel(i);

			break;
		}
	}
}

bool CBrowserHost::IsFeedTabActive()
{
	CString url = GetInternetURL();
	return !url.GetLength();
}

bool CBrowserHost::IsGreatNewsSpecialUrl(const CString& url)
{
	return (url.Find(_T("#GreatNewsTag")) != -1
			|| url.Find(_T("javascript:external.")) != -1);
}

void CBrowserHost::FocusOnAddress()
{
	m_cboAddress.SetFocus();
}


void CBrowserHost::ReadPrevious()
{
	if(!m_contentGenerator)
		return;

	if(m_contentGenerator->IsBatch())
		PostMessage(WM_COMMAND, MAKELONG(ID_IE_GOTO_PAGE, CContentGenerator::PrevPage));
	else
		PostMessage(WM_COMMAND, ID_IE_PREV);
}

void CBrowserHost::ReadNext()
{
	if(!m_contentGenerator)
		return;

	if(m_contentGenerator->IsBatch())
		PostMessage(WM_COMMAND, MAKELONG(ID_IE_GOTO_PAGE, CContentGenerator::NextPage));
	else
		PostMessage(WM_COMMAND, ID_IE_NEXT);
}

void CBrowserHost::SetFocusToAddress()
{
	m_cboAddress.SetFocus();
}

BOOL CBrowserHost::AddressHasFocus(HWND hwndFocus)
{
	return m_cboAddress.IsChild(hwndFocus);
}

void CBrowserHost::SetFocusToReadingPane()
{
	if(m_wndIE)
		m_wndIE->SetFocus();
}

BOOL CBrowserHost::ReadingPaneHasFocus(HWND hwndFocus)
{
	if(!m_wndIE)
		return FALSE;

	return m_wndIE->m_hWnd==hwndFocus || m_wndIE->IsChild(hwndFocus);
}

BOOL CBrowserHost::CanTabOff()
{
	if(!m_wndIE)
		return TRUE;

	return !m_wndIE->HasActiveElement();
}

bool CBrowserHost::ChangeItemReadFlag(NewsItemPtr& pItem)
{
	return false;
}

bool CBrowserHost::ChangeItemLabelStyle(NewsItemPtr& pItem)
{
	if(!IsFeedTabActive() // feed tab is hidden?
		|| !(g_GreatNewsConfig.m_nSelectedItemFeatures & CNewsItem::LabelThis)) // LabelThis link is not enabled?
		return false;

	CString urlLabelThis(pItem->GetLabelThisUrl());

	MSHTML::IHTMLDocument2Ptr spDoc = m_wndIE->GetDocument();
	if(spDoc == NULL)
		return false;

	// get <a> tags
	MSHTML::IHTMLElementCollectionPtr spElements = spDoc->Getall();
	long count = spElements->Getlength();
	for (long i = 0; i < count; ++i) 
	{
		MSHTML::IHTMLElementPtr spElement = spElements->item(i, i);
		if(spElement == NULL)
			continue;

		_bstr_t tag = spElement->tagName;
		if(_tcsicmp((LPCTSTR)tag,_T("A")) != 0)
			continue;

		_bstr_t href = spElement->getAttribute(_T("href"),0);
		CString url((LPCTSTR)href);
		if(url.Find(urlLabelThis) >=0 )
		{
			//we have found the Lable This link
			//_bstr_t cls0 = spElement->outerHTML;

			spElement->setAttribute(_T("className"), pItem->GetLabelThisUrlClass(), 0);

			// spElement->innerText = pItem->GetLabelThisUrlClass();
			return true;
		}

	}

	return false;
}

bool CBrowserHost::ChangeItemTrackCommentText(NewsItemPtr& pItem)
{
	if(!IsFeedTabActive() // feed tab is hidden?
		|| !(g_GreatNewsConfig.m_nSelectedItemFeatures & CNewsItem::TrackComments)) // LabelThis link is not enabled?
		return false;

	CString urTrackComment(pItem->GetTrackCommentUrl());

	MSHTML::IHTMLDocument2Ptr spDoc = m_wndIE->GetDocument();
	if(spDoc == NULL)
		return false;

	// get <a> tags
	MSHTML::IHTMLElementCollectionPtr spElements = spDoc->Getall();
	long count= spElements->Getlength();
	for (long i = 0; i < count; ++i) 
	{
		MSHTML::IHTMLElementPtr spElement = spElements->item(i, i);
		if(spElement == NULL)
			continue;

		_bstr_t tag = spElement->tagName;
		if(_tcsicmp((LPCTSTR)tag,_T("A")) != 0)
			continue;

		_bstr_t href = spElement->getAttribute(_T("href"),0);
		CString url((LPCTSTR)href);
		if(url.Find(urTrackComment) >=0 )
		{
			//we have found the Lable This link
			//_bstr_t cls0 = spElement->outerHTML;

			spElement->innerText = pItem->GetTrackCommentText();

			// spElement->innerText = pItem->GetLabelThisUrlClass();
			return true;
		}

	}

	return false;
}

void CBrowserHost::RefreshItem(ULONG_PTR itemID)
{
	if(!IsFeedTabActive()) // feed tab is hidden?
		return;

	MSHTML::IHTMLDocument2Ptr spDoc = m_wndIE->GetDocument();
	if (spDoc == NULL)
		return;

	NewsItemPtr item = CNewsItemCache::GetNewsItem(itemID);
	CString itemDivID = item->GetDivID();

	// get <div> tags
	MSHTML::IHTMLElementCollectionPtr spElements = spDoc->Getall();
	long count= spElements->Getlength();
	for (long i = 0; i < count; ++i) 
	{
		MSHTML::IHTMLElementPtr spElement = spElements->item(i, i);
		if(spElement == NULL)
			continue;

		_bstr_t tag = spElement->tagName;
		if(_tcsicmp((LPCTSTR)tag,_T("DIV")) != 0)
			continue;

		_bstr_t divID = spElement->getAttribute(_T("id"),0);
		if(divID.length() && itemDivID == (LPCTSTR)divID)
		{
			spElement->outerHTML = (LPCTSTR)item->internalGenerateHTML(
				m_contentGenerator!=NULL ? !m_contentGenerator->IsBatch() : false);
			return;
		}
	}
}

void CBrowserHost::SetWorkOffline(bool bWorkOffline)
{
	m_bWorkOffline = bWorkOffline;
	for(int i = 0; i < m_wndBrowserTabBar.GetItemCount(); ++i)
	{
		CGNTabItem* pItem = (CGNTabItem*)m_wndBrowserTabBar.GetItem(i);
		if(pItem->m_browser != NULL)
			pItem->m_browser->Offline = bWorkOffline;
	}
}
