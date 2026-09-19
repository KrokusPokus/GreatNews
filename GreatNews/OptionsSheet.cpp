#include "stdafx.h"
#include "resource.h"
#include "FeedManagerLib.h"
#include "OptionsSheet.h"
#include "ConfigBlogThisDlg.h"
#include "GreatNewsConfig.h"
#include "GNUtil.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of class COptionsPageGeneral
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT COptionsSheet::COptionsPageGeneral::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("COptionsPageGeneral::OnInitDialog()\n");
	GetPropertySheet().SendMessage( MM_CENTER_SHEET );

	m_bAutoUpdate = g_GreatNewsConfig.GetAutoUpdateAtStartup();
	m_bRemoveFromTaskBar = g_GreatNewsConfig.GetRemoveFromTaskBar();
	m_bCloseToTray = g_GreatNewsConfig.GetCloseToTray();
	m_cleanupReminder = g_GreatNewsConfig.GetCleanupReminder();
	m_nStartupPage = g_GreatNewsConfig.GetStartupScreen();

	CComboBox cbo = GetDlgItem(IDC_AUTOUPDATE_FREQ);
	for(int i=0;g_GreatNewsConfig.AutoUpdateString[i]; ++i)
	{
		cbo.AddString(g_GreatNewsConfig.AutoUpdateString[i]);
	}

	CComboBox cbo2 = GetDlgItem(IDC_CLEANUPREMINDER);
	for (int i = 0; g_GreatNewsConfig.CleanupRemindString[i]; ++i)
	{
		cbo2.AddString(g_GreatNewsConfig.CleanupRemindString[i]);
	}
	
	PopulateLabels();
	PopulateWatches();

	DoDataExchange(false);
	EnableControls();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("OptionsDialog_General"));
	pResMngr->ApplyLanguageToCombo(cbo, _T("AutoUpdateFrequency"));
	pResMngr->ApplyLanguageToCombo(cbo2, _T("CleanupReminderInterval"));

	return 0;
} // OnInitDialog

void COptionsSheet::COptionsPageGeneral::EnableControls()
{
	CButton chk = GetDlgItem(IDS_UPDATECHANNELEVERY);
	CComboBox cbo = GetDlgItem(IDC_AUTOUPDATE_FREQ);
	cbo.EnableWindow(chk.GetCheck());

	CComboBox cbo2 = GetDlgItem(IDC_ALLLABELS);
	cbo2.EnableWindow(IsDlgButtonChecked(IDC_SHOWLABEL)==TRUE);

	CComboBox cbo3 = GetDlgItem(IDC_ALLWATCHES);
	cbo3.EnableWindow(IsDlgButtonChecked(IDC_SHOWWATCH)==TRUE);

}

void COptionsSheet::COptionsPageGeneral::PopulateLabels()
{
	try
	{
		TagVector allTags;
		CTag::GetAllTags(allTags);

		CComboBox cbo = GetDlgItem(IDC_ALLLABELS);
		cbo.Clear();
		for(TagVector::iterator it = allTags.begin(); it!=allTags.end(); ++it)
		{
			TagPtr tag = *it;
			int index = cbo.AddString(tag->m_name);
			cbo.SetItemData(index, tag->m_id);
		}

	}
	CATCH_ALL_ERROR()
}

void COptionsSheet::COptionsPageGeneral::PopulateWatches()
{
	try
	{
		NewsWatchVector watches;
		CNewsWatch::GetAllNewsWatches(watches, CNewsWatch::SortByDisplayOrder);

		CComboBox cbo = GetDlgItem(IDC_ALLWATCHES);
		cbo.Clear();
		for(NewsWatchVector::iterator it=watches.begin();it!=watches.end();++it)
		{
			NewsWatchPtr watch = *it;
			int index = cbo.AddString(watch->m_title);
			cbo.SetItemData(index, watch->m_id);
		}

	}
	CATCH_ALL_ERROR()
}


LRESULT COptionsSheet::COptionsPageGeneral::OnRefreshControls(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EnableControls();

	return 0;
}

LRESULT COptionsSheet::COptionsPageGeneral::OnDefaultReader(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	g_GreatNewsConfig.RegisterFeedProtocolHandler();

	return 0;
}

COptionsSheet::COptionsPageGeneral::COptionsPageGeneral(void) : CPropertyPageImpl<COptionsPageGeneral>( _T("General") )
{
	ATLTRACE(_T("COptionsPageGeneral::COptionsPageGeneral()\n"));
}

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class COptionPagePager
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT COptionsSheet::COptionPagePager::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE(_T("COptionPagePager::OnInitDialog()\n"));

	m_nPageSize = g_GreatNewsConfig.m_nPageSize;
	m_nSecondsToMarkItemRead = g_GreatNewsConfig.m_nSecondsToMarkItemRead;

	DoDataExchange(false);
	SetupControls();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("OptionsDialog_Reading"));

	return 0;
} // OnInitDialog

void COptionsSheet::COptionPagePager::SetupControls()
{
	GetDlgItem(IDC_DINOSOURDAYS).EnableWindow(IsDlgButtonChecked(IDC_NOTIFYINACTIVECHANNELS));
}

LRESULT COptionsSheet::COptionPagePager::OnNotifyInactive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SetupControls();
	return 0;
}

COptionsSheet::COptionPagePager::COptionPagePager(void) : CPropertyPageImpl<COptionPagePager>( _T("Reading") )
{
	ATLTRACE("COptionPagePager::COptionPagePager()\n");
}

BOOL COptionsSheet::OnOK()
{
	// validate user inputs
	if(!m_generalPage.DoDataExchange(true))
	{
		MessageBox(_T("Invalid input found on General page."),_T("GreatNews"));
		// SetActivePage(0);
		return FALSE;
	}

	if(::IsWindow(m_pagerPage))
	{
		if(!m_pagerPage.DoDataExchange(true))
		{
			MessageBox(_T("Invalid input found on Reading page."),_T("GreatNews"));
			// SetActivePage(1);
			return FALSE;
		}
		else
		{
			g_GreatNewsConfig.m_nPageSize = m_pagerPage.m_nPageSize;
			g_GreatNewsConfig.m_nSecondsToMarkItemRead = m_pagerPage.m_nSecondsToMarkItemRead;
		}
	}

	if(::IsWindow(m_usabilityPage) && !m_usabilityPage.DoDataExchange(true))
	{
		MessageBox(_T("Invalid input found on Usability page."),_T("GreatNews"));
		// SetActivePage(2);
		return FALSE;
	}

	if(::IsWindow(m_featurePage))
	{
		m_featurePage.ReadControls();
	}

	if(::IsWindow(m_connectionPage))
	{
		if(!m_connectionPage.DoDataExchange(true))
			return FALSE;

		g_GreatNewsConfig.SetupProxy();
	}


	g_GreatNewsConfig.SetAutoUpdateAtStartup(m_generalPage.m_bAutoUpdate);
	g_GreatNewsConfig.SetRemoveFromTaskBar(m_generalPage.m_bRemoveFromTaskBar);
	g_GreatNewsConfig.SetCloseToTray(m_generalPage.m_bCloseToTray);
	g_GreatNewsConfig.SetCleanupReminder(m_generalPage.m_cleanupReminder);
	g_GreatNewsConfig.SetStartupScreen(m_generalPage.m_nStartupPage);

	g_GreatNewsConfig.SaveConfig();

	CNewsItem::SetItemFeatures(g_GreatNewsConfig.m_nSelectedItemFeatures, g_GreatNewsConfig.m_bHighlightNewsWatch);
	CGNUtil::m_podCastingExtensions = g_GreatNewsConfig.m_podcastingEnclosuresTypes;
	CNewsFeedParser::m_bKeepOriginalLink = g_GreatNewsConfig.m_bKeepOriginalLink;
	CNewsFeed::m_bMarkChangesUnread = g_GreatNewsConfig.m_bMarkChangesUnread;

	return TRUE;

}

COptionsSheet::COptionPageUsability::COptionPageUsability(void) : CPropertyPageImpl<COptionPageUsability>( _T("Usability") )
{
}

LRESULT COptionsSheet::COptionPageUsability::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoDataExchange(false);

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("OptionsDialog_Usability"));

	return 0;
}


COptionsSheet::COptionPageFeatures::COptionPageFeatures(void) : CPropertyPageImpl<COptionPageFeatures>( _T("Features") )
{
}

LRESULT COptionsSheet::COptionPageFeatures::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoDataExchange(false);

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("OptionsDialog_ItemFeatures"));

	SetupControls();

	return 0;
}

LRESULT COptionsSheet::COptionPageFeatures::OnConfigBlogThis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CConfigBlogThisDlg dlg;
	dlg.DoModal();

	return 0;
}

void COptionsSheet::COptionPageFeatures::SetupControls()
{
	long selectedFeatures = g_GreatNewsConfig.m_nSelectedItemFeatures;

	if(selectedFeatures & CNewsItem::LabelThis)
		CheckDlgButton(IDC_LABELTHIS, 1);

	if(selectedFeatures & CNewsItem::EmailThis)
		CheckDlgButton(IDC_EMAILTHIS, 1);

	if(selectedFeatures & CNewsItem::Delicious)
		CheckDlgButton(IDC_DELICIOUS, 1);

	if(selectedFeatures & CNewsItem::Furl)
		CheckDlgButton(IDC_FURL, 1);
	
	if(selectedFeatures & CNewsItem::Arthur)
		CheckDlgButton(IDC_ARTHUR, 1);

	if(selectedFeatures & CNewsItem::BlogThis)
		CheckDlgButton(IDC_BLOGTHIS, 1);

	if(selectedFeatures & CNewsItem::TrackComments)
		CheckDlgButton(IDC_TRACKCOMMENTS, 1);

	if(g_GreatNewsConfig.m_bHighlightNewsWatch)
		CheckDlgButton(IDC_HIGHLIGHTNEWSWATCH, 1);

	SetDlgItemText(IDC_ENCLOSURE, g_GreatNewsConfig.m_podcastingEnclosuresTypes);

	if(g_GreatNewsConfig.m_bKeepOriginalLink)
		CheckDlgButton(IDC_CHKKEEPORIGINALLINK, 1);

}

void COptionsSheet::COptionPageFeatures::ReadControls()
{
	long selectedFeatures = 0;

	if(IsDlgButtonChecked(IDC_LABELTHIS))
		selectedFeatures |= CNewsItem::LabelThis;

	if(IsDlgButtonChecked(IDC_EMAILTHIS))
		selectedFeatures |= CNewsItem::EmailThis;

	if(IsDlgButtonChecked(IDC_DELICIOUS))
		selectedFeatures |= CNewsItem::Delicious;

	if(IsDlgButtonChecked(IDC_FURL))
		selectedFeatures |= CNewsItem::Furl;

	if(IsDlgButtonChecked(IDC_ARTHUR))
		selectedFeatures |= CNewsItem::Arthur;

	if(IsDlgButtonChecked(IDC_BLOGTHIS))
		selectedFeatures |= CNewsItem::BlogThis;

	if(IsDlgButtonChecked(IDC_TRACKCOMMENTS))
		selectedFeatures |= CNewsItem::TrackComments;

	g_GreatNewsConfig.m_nSelectedItemFeatures = selectedFeatures;

	g_GreatNewsConfig.m_bHighlightNewsWatch = (IsDlgButtonChecked(IDC_HIGHLIGHTNEWSWATCH) != 0);

	GetDlgItemText(IDC_ENCLOSURE, g_GreatNewsConfig.m_podcastingEnclosuresTypes);

	g_GreatNewsConfig.m_bKeepOriginalLink = (IsDlgButtonChecked(IDC_CHKKEEPORIGINALLINK) != 0);
}

COptionsSheet::COptionPageConnection::COptionPageConnection(void) : CPropertyPageImpl<COptionPageConnection>( _T("Connection") )
{
}

LRESULT COptionsSheet::COptionPageConnection::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DoDataExchange(false);
	SetupControls();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("OptionsDialog_Connection"));

	return 0;
}

void COptionsSheet::COptionPageConnection::SetupControls()
{
	bool bEnable  = IsDlgButtonChecked(IDC_GN_PROXY)==TRUE;
	GetDlgItem(IDC_PROXY_URL).EnableWindow(bEnable);
	GetDlgItem(IDC_PROXY_PORT).EnableWindow(bEnable);
	GetDlgItem(IDC_PROXY_USER).EnableWindow(bEnable);
	GetDlgItem(IDC_PROXY_PASSWORD).EnableWindow(bEnable);
	GetDlgItem(IDC_PROXY_BYPASS).EnableWindow(bEnable);

}

LRESULT COptionsSheet::COptionPageConnection::OnProxyChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SetupControls();
	return 0;
}
