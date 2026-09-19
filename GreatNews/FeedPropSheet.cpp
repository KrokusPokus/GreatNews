#include "stdafx.h"
#include "resource.h"
#include "FeedPropSheet.h"
#include "NewsFeedParser.h"
#include "GNUtil.h"
#include "FeedGroupDlg.h"
#include "GMTimeLib.h"
#include "GNResourceManager.h"
#include "GreatNewsConfig.h"
#include "StyleComboHelper.h"
#include "DetectFeedBrowser.h"
#include "SelectChannelDlg.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// implementation of class cfeedpageurl
///////////////////////////////////////////////////////////////////////////////////////////
LRESULT CFeedPropSheet::CFeedPageUrl::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	GetPropertySheet().SendMessage( MM_CENTER_SHEET );

	m_bQuery = (m_feed.m_id == 0);
	DoDataExchange(false);

	// display number of items if this is not a new feed
	if(m_feed.m_id != 0)
	{
		CString text;
		text.Format(ResManagerPtr->GetString(IDS_CHANNELITEMS),
			m_feed.GetNumOfNewsItems(),m_feed.GetNumOfNewsItems(true));
		GetDlgItem(IDC_NUMOFITEM).SetWindowText(text);
		GetDlgItem(IDC_NUMOFITEM).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_HINT).ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_CHKDISABLE).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_HINT).ShowWindow(SW_SHOW);
	}

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ChannelPropertiesDialog_URL"));

	return 0;
} // OnInitDialog

void CFeedPropSheet::CFeedPageUrl::SetupControls()
{
	bool bIsSyncAccount = (m_feed.m_bloglineSubId != 0);

	GetDlgItem(IDC_EDTURL).EnableWindow(!bIsSyncAccount);
	GetDlgItem(IDC_CHKQUERYCHANNEL).ShowWindow(bIsSyncAccount ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_VALIDATE).ShowWindow(bIsSyncAccount ? SW_HIDE : SW_SHOW);
	
	GetDlgItem(IDC_SYNCBLOGLINES).ShowWindow(!bIsSyncAccount ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DETACHBLOGLINES).ShowWindow(!bIsSyncAccount ? SW_HIDE : SW_SHOW);

	GetDlgItem(IDC_CHKREQUIRELOGIN).ShowWindow(bIsSyncAccount ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_LOGINNAME).ShowWindow(bIsSyncAccount ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_TXTLOGINNAME).ShowWindow(bIsSyncAccount ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_LOGINPWD).ShowWindow(bIsSyncAccount ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_TXTLOGINPWD).ShowWindow(bIsSyncAccount ? SW_HIDE : SW_SHOW);

	if(!bIsSyncAccount)
	{
		bool bChecked = (IsDlgButtonChecked(IDC_CHKREQUIRELOGIN) != 0);
		GetDlgItem(IDC_TXTLOGINNAME).EnableWindow(bChecked);
		GetDlgItem(IDC_TXTLOGINPWD).EnableWindow(bChecked);
	}
}

LRESULT CFeedPropSheet::CFeedPageUrl::OnRequireLogin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SetupControls();
	return 0;
}

LRESULT CFeedPropSheet::CFeedPageUrl::OnDetach(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_feed.m_bloglineSubId = 0;
	SetupControls();
	return 0L;
}

LRESULT CFeedPropSheet::CFeedPageUrl::OnValidate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoDataExchange(true);

	if(GetKeyState( VK_CONTROL ) & 0x1000)
	{
		// if user is holding the ctrl key, show him the raw xml
		CGNUtil::OpenUrlExternally(m_feed.m_url, false, true);
	}
	else
	{
		CString validateURL = _T("http://www.feedvalidator.org/check.cgi?url=")+m_feed.m_url;
		CGNUtil::OpenUrlExternally(validateURL, false, true);
	}

	return 0L;
}


CFeedPropSheet::CFeedPageUrl::CFeedPageUrl(CNewsFeed& feed, bool& bReload, bool& bWizardMode) 
	: CPropertyPageImpl<CFeedPageUrl>( _T("General") ),
	CData(feed, bReload, bWizardMode)
{
	ATLTRACE("CFeedPageUrl::CFeedPageUrl()\n");
}

int CFeedPropSheet::CFeedPageUrl::OnSetActive(void)
{
	if(m_bWizardMode)
		SetWizardButtons(PSWIZB_NEXT );

	SetupControls();

	return 1; // return 0 to avoid setting this page as active
}
int CFeedPropSheet::CFeedPageUrl::OnKillActive(void)
{
	if(!DoDataExchange(true))
		return 0;

	if(m_feed.m_url.GetLength() == 0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_FILLINURL),_T("GreatNews"),MB_OK|MB_ICONINFORMATION);
		GetDlgItem(IDC_EDTURL).SetFocus();
		return 0;
	}

	CString normalizedUrl = CGNUtil::NormalizeURL(m_feed.m_url);
	if(normalizedUrl != m_feed.m_url)
	{
		m_feed.m_url = normalizedUrl;
		SetDlgItemText(IDC_EDTURL, m_feed.m_url);
	}

	ULONG_PTR nChannelID = 0;
	if(m_feed.m_id == 0 && 0 !=(nChannelID=CNewsFeed::GetIdFromXmlUrl(m_feed.m_url)))
	{
		if(IDYES == MessageBox(ResManagerPtr->GetString(IDS_OPENEXISTINGCHANNEL),
			_T("GreatNews"), MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1))
		{
			GetParent().GetParent().PostMessage(WM_COMMAND, ID_IE_OPENCHANNEL, (LPARAM)nChannelID);
			GetParent().DestroyWindow();
			return 0;
		}
	}

	if(m_bQuery)
	{
		CWaitCursor wc;
		try
		{
			NewsFeedParserPtr spParser = CNewsFeedParser::CreateParserFromURL(m_feed.m_url);
			if(spParser != NULL)
			{
				m_feed.CopyProperties(spParser->m_newsFeed, CNewsFeed::Web);
			}
		}
		catch(const CExceptionBase& e)
		{
			if(CGNUtil::IsFeedUrl(m_feed.m_url))
			{
				MessageBox(e.GetErrorMsg(),_T("GreatNews"));
				return 0;
			}
			else
			{
				// try to auto detect feed url
				CString newUrl = AutoDetectFeed(m_feed.m_url);
				if(!newUrl.GetLength())
				{
					MessageBox(e.GetErrorMsg(),_T("GreatNews"));
					return 0;
				}

				// we found feed url, try query feed property again
				m_feed.m_url = newUrl;
				SetDlgItemText(IDC_EDTURL, newUrl);

				return OnKillActive(); // try this step again
			}
		}
	}

	return 1; // 0 to stay in the same page.
}

CString CFeedPropSheet::CFeedPageUrl::AutoDetectFeed(const CString& url)
{
	CNGEvent finishedEvent;
	CWaitCursor wc;

	CDetectFeedBrowser feedBrowser(finishedEvent.m_hObject);
	const DWORD dwPopupStyle = WS_CHILD;
	if(!feedBrowser.Create(m_hWnd, rcDefault, url, dwPopupStyle, 0))
		return _T("");
	feedBrowser.PutSilent(TRUE);

	bool bContinue = true;
	while(bContinue)
	{
		switch (::MsgWaitForMultipleObjects(1,&finishedEvent.m_hObject,FALSE,30*1000,QS_ALLEVENTS))
		{
		case WAIT_OBJECT_0:  // process
			bContinue = false;
			break;

		case WAIT_TIMEOUT:
			feedBrowser.DestroyWindow();
			return _T("");

		case WAIT_OBJECT_0+1:
			{
				// There is a windows message, handle it...
				MSG msg;
				while (::PeekMessage(&msg,0,0,0,PM_REMOVE))
				{
					// Abort on a WM_QUIT message
					if (msg.message == WM_QUIT)
					{
						feedBrowser.DestroyWindow();
						return _T("");
					}

					// Translate and dispatch the message
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
		}
	}

	std::map<CString,CString> rssFeeds;
	size_t feedCount = feedBrowser.DiscoverFeeds(rssFeeds);
	feedBrowser.DestroyWindow();

	if(feedCount <= 0)
		return _T("");

	if(feedCount == 1)
		return rssFeeds.begin()->second;

	wc.Restore();

	// we found more than one feed, ask user to pick one
	CSelectChannelDlg dlg(rssFeeds);
	if(dlg.DoModal() == IDOK)
		return dlg.m_selectedChannelUrl;
	else
		return _T("");
}

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of class CFeedPageProperties
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CFeedPropSheet::CFeedPageProperties::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CFeedPageProperties::OnInitDialog()\n");

	// apply language pack
	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ChannelPropertiesDialog_Properties"));

	return 0;
} // OnInitDialog


CFeedPropSheet::CFeedPageProperties::CFeedPageProperties(CNewsFeed& feed, bool& bReload, bool& bWizardMode)
	: CPropertyPageImpl<CFeedPageProperties>( _T("Properties") ),
	CData(feed, bReload, bWizardMode)
{
	ATLTRACE("CFeedPageProperties::CFeedPageProperties()\n");
}

int CFeedPropSheet::CFeedPageProperties::OnSetActive(void)
{
	m_strTTL.Empty();
	if(m_feed.m_ttl > 1)
		m_strTTL.Format(_T("%d minutes"), m_feed.m_ttl);

	if(m_bWizardMode)
		SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT );

	DoDataExchange(false);

	if(m_feed.m_format.GetLength())
		GetDlgItem(IDC_FORMAT).SetWindowText(m_feed.m_format);
	if(m_feed.m_lastChecked != 0)
		GetDlgItem(IDC_LASTUPDATE).SetWindowText(CGMTimeHelper::FormatDisplayDate(m_feed.m_lastChecked));
	if(m_feed.m_lastModified != 0)
		GetDlgItem(IDC_LASTMODIFIED).SetWindowText(CGMTimeHelper::FormatDisplayDate(m_feed.m_lastModified));

	return 1; // return 0 to avoid setting this page as active
}
int CFeedPropSheet::CFeedPageProperties::OnKillActive(void)
{
	if(!DoDataExchange(true))
		return 0;

	if(m_feed.m_title.GetLength()==0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_EMPTYCHANNELTITLE), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		return 0;
	}
	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

CFeedPropSheet::CFeedPageSettings::CFeedPageSettings(CNewsFeed& feed, bool& bReload, bool& bWizardMode)
	: CPropertyPageImpl<CFeedPageSettings>( _T("Settings") ),
	CData(feed, bReload, bWizardMode)
{
	ATLTRACE("CFeedPageSettings::CFeedPageSettings()\n");
}

LRESULT CFeedPropSheet::CFeedPageSettings::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// fill the update freq combo
	CComboBox cbo2 = GetDlgItem(IDC_UPDATE_FREQ);
	for(int i = 0; CNewsFeed::ChannelAutoUpdateString[i]; ++i)
		cbo2.AddString(CNewsFeed::ChannelAutoUpdateString[i]);

	// fill the cleanup setting combo
	CComboBox cbo1 = GetDlgItem(IDC_CBOCLEANUP);
	for(int i=0;CNewsFeed::ChannelCleanupMaxAgeString[i]; ++i)
		cbo1.AddString(CNewsFeed::ChannelCleanupMaxAgeString[i]);

	// fill in style combo
	CStyleComboHelper::FillInCombo(GetDlgItem(IDC_CBOSTYLES));

	// apply language pack
	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ChannelPropertiesDialog_Settings"));
	pResMngr->ApplyLanguageToCombo(cbo2, _T("ChannelAutoUpdateFrequency"));
	pResMngr->ApplyLanguageToCombo(cbo1, _T("ChannelCleanupMaxAge"));

	return 0;
} // OnInitDialog

int CFeedPropSheet::CFeedPageSettings::OnSetActive(void)
{
	DoDataExchange(false);

	CStyleComboHelper::SetStyleComboSelection(GetDlgItem(IDC_CBOSTYLES),m_feed.m_channelStyle);
	
	SetupControls();

	return 1; // return 0 to avoid setting this page as active
}

int CFeedPropSheet::CFeedPageSettings::OnKillActive(void)
{
	if(!DoDataExchange(true))
		return 0;
	CStyleComboHelper::GetStyleComboSelection(GetDlgItem(IDC_CBOSTYLES),m_feed.m_channelStyle);
	return 1; // 0 to stay in the same page.
}

void CFeedPropSheet::CFeedPageSettings::SetupControls()
{
	GetDlgItem(IDC_EDTCLEANUPITEM).EnableWindow(IsDlgButtonChecked(IDC_CHKCLEANUPITEM));
}

LRESULT CFeedPropSheet::CFeedPageSettings::OnLimitItems(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SetupControls();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CFeedPageGroup
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CFeedPropSheet::CFeedPageGroup::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CFeedPageGroup::OnInitDialog()\n");

	m_treeGroup.SubclassWindow(GetDlgItem(IDC_GROUPTREE));
	m_treeGroup.SetTreeImageList();
	m_treeGroup.LoadTree(LoadGroup);
	m_treeGroup.ShowEverything();

	CTreeItem treeItem = m_treeGroup.GetRootItem();
    if(m_feed.m_groupID > 0)
	{
		treeItem = m_treeGroup.FindNodeByIdType(
			m_feed.m_groupID, CFeedTreeItem::Group);
	}

	if(treeItem != NULL)
		treeItem.Select();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ChannelPropertiesDialog_Group"));

	return 0;
} // OnInitDialog

LRESULT CFeedPropSheet::CFeedPageGroup::OnNewGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	try
	{
		CFeedGroupDlg dlg;
		FeedGroupPtr root = CFeedGroup::GetRootFeedGroup();
		dlg.m_feedGroup.m_parentID = root->m_id;

		if(dlg.DoModal() == IDOK)
		{ 
			CFeedGroup* pFeedGroup = new CFeedGroup(dlg.m_feedGroup);
			CTreeItem item = m_treeGroup.InsertItem(m_treeGroup.GetRootItem(), pFeedGroup, TRUE);
			item.Select();
			m_treeGroup.SetFocus();
			m_bReloadTree = true;
		}
	}
	CATCH_ALL_ERROR()
	
	return 0;
}

CFeedPropSheet::CFeedPageGroup::CFeedPageGroup(CNewsFeed& feed, bool& bReload, bool& bWizardMode) 
	: CPropertyPageImpl<CFeedPageGroup>( _T("Group") ),
	CData(feed, bReload, bWizardMode)
{
}
int CFeedPropSheet::CFeedPageGroup::OnSetActive(void)
{
	if(m_bWizardMode)
		SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH /*PSWIZB_DISABLEDFINISH*/ );

	return 1; // return 0 to avoid setting this page as active
}

int CFeedPropSheet::CFeedPageGroup::OnKillActive(void)
{
	CTreeItem item = m_treeGroup.GetSelectedItem();
	if(item)
	{
		CFeedGroupTreeItem* pItem = (CFeedGroupTreeItem*)item.GetData();
		m_feed.m_groupID = pItem->m_feedGroup->m_id;
	}
	else
	{
		MessageBox(ResManagerPtr->GetString(IDS_PLSSELECTGROUP), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		return 0;
	}

	return 1;
}

INT_PTR CFeedPropSheet::CFeedPageGroup::OnWizardFinish(void)
{
	try
	{
		if(!OnKillActive())
			return 0;

		m_feed.Save();
		m_feed.m_lastChecked = CTime(); // clear last updated timestamp because feed properties were changed
	}
	catch(const CExceptionBase& e)
	{
		MessageBox(e.GetErrorMsg(),_T("GreatNews"));
		return 0;
	}

	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

BOOL CFeedPropSheet::OnOK()
{
	try
	{
		// collect data on the last page. Other pages OnKillActive() was already called
		if(m_pageUrl.IsWindowVisible())
		{
			if(!m_pageUrl.OnKillActive())
				return FALSE;
		}
		else if(m_pageProperties.IsWindow() && m_pageProperties.IsWindowVisible())
		{
			if(!m_pageProperties.OnKillActive())
				return FALSE;
		}
		else if(m_pageGroup.IsWindow() && m_pageGroup.IsWindowVisible())
		{
			if(!m_pageGroup.OnKillActive())
				return FALSE;
		}
		else if(m_pageSettings.IsWindow() && m_pageSettings.IsWindowVisible())
		{
			if(!m_pageSettings.OnKillActive())
				return FALSE;
		}

		// save
		m_newsFeed.Save();
		m_newsFeed.m_lastChecked = CTime(); // clear last updated timestamp because feed properties were changed
	}
	catch(const CExceptionBase& e)
	{
		MessageBox(e.GetErrorMsg(),_T("GreatNews"));
		return FALSE;
	}

	return TRUE;
}
