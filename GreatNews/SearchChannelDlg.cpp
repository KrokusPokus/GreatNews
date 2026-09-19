#include "StdAfx.h"
#include "SearchChanneldlg.h"
#include "FeedManagerLib.h"
#include "GNResourceManager.h"
#include "StyleComboHelper.h"

LRESULT CSearchChannelDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_bAutoGenerateTitle = false;

	CenterWindow(GetParent());

	CComboBox cb = GetDlgItem(IDC_SEARCHCHANNEL);
	SearchChannelDefVector& searchDef = CSearchChannelDef::m_searchChannelDefs;
	for(SearchChannelDefVector::iterator it = searchDef.begin(); it!=searchDef.end(); ++it)
	{
		CSearchChannelDef& def = *it;
		cb.AddString(def.m_name);
	}

	if(m_newsFeed.m_id != 0)
	{
		// display number of items if this is not a new feed
		CString text;
		text.Format(ResManagerPtr->GetString(IDS_CHANNELITEMS),
			m_newsFeed.GetNumOfNewsItems(),m_newsFeed.GetNumOfNewsItems(true));
		GetDlgItem(IDC_NUMOFITEM).SetWindowText(text);
		GetDlgItem(IDC_NUMOFITEM).ShowWindow(SW_SHOW);

		// parse search channels
		CSearchChannelDef::CrackSearchUrl(m_newsFeed.m_url, m_searchChannel, m_keyword);
		cb.SelectString(-1, m_searchChannel);
	}
	else
	{
		cb.SetCurSel(0);
	}

	// fill the update freq combo
	CComboBox cbo2 = GetDlgItem(IDC_UPDATE_FREQ2);
	for(int i=0;CNewsFeed::ChannelAutoUpdateString[i]; ++i)
	{
		cbo2.AddString(CNewsFeed::ChannelAutoUpdateString[i]);
	}

	// fill the cleanup setting combo
	CComboBox cbo1 = GetDlgItem(IDC_CBOCLEANUP);
	for(int i=0;CNewsFeed::ChannelCleanupMaxAgeString[i]; ++i)
	{
		cbo1.AddString(CNewsFeed::ChannelCleanupMaxAgeString[i]);
	}

	DoDataExchange(FALSE);

	// fill in style combo
	CStyleComboHelper::FillInCombo(GetDlgItem(IDC_CBOSTYLES));
	CStyleComboHelper::SetStyleComboSelection(GetDlgItem(IDC_CBOSTYLES),m_newsFeed.m_channelStyle);

	// apply language pack
	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("SearchChannelPropertiesDialog"));

	pResMngr->ApplyLanguageToCombo(cbo2, _T("ChannelAutoUpdateFrequency"));
	pResMngr->ApplyLanguageToCombo(cbo1, _T("ChannelCleanupMaxAge"));

	// check if we should auto update channel title
	if(m_newsFeed.m_id<=0 // new search channel?
		|| m_newsFeed.m_title == GenerateTitle(m_keyword,m_searchChannel)) // auto generated?
	{
		m_bAutoGenerateTitle = true;
	}

	return TRUE;
}

LRESULT CSearchChannelDlg::OnKeywordsChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(!m_bAutoGenerateTitle)
		return 0;

	CString keyword;
	GetDlgItemText(IDC_WATCH_WORDS, keyword);
	CString searchChannel;
	GetDlgItemText(IDC_SEARCHCHANNEL, searchChannel);
	CString title = GenerateTitle(keyword, searchChannel);
	SetDlgItemText(IDC_NAME, title);

	return 1;
}

CString CSearchChannelDlg::GenerateTitle(CString keyword, CString searchChannel)
{
	if(keyword.IsEmpty() || searchChannel.IsEmpty())
		return _T("");

	return keyword+_T(" on ")+searchChannel;
}


LRESULT CSearchChannelDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID == IDOK)
	{
		CStyleComboHelper::GetStyleComboSelection(GetDlgItem(IDC_CBOSTYLES),m_newsFeed.m_channelStyle);
		DoDataExchange(TRUE);
		if(m_keyword.GetLength()==0)
		{
			MessageBox(ResManagerPtr->GetString(IDS_FILLINSEARCHKEYWORD));
			return 0;
		}
		if(m_newsFeed.m_title.GetLength() == 0)
		{
			MessageBox(ResManagerPtr->GetString(IDS_NAMESEARCH));
			return 0;
		}
		GetDlgItem(IDC_SEARCHCHANNEL).GetWindowText(m_searchChannel);

		// make the url string:
		CSearchChannelDef::AssembleSearchUrl(m_newsFeed.m_url, m_searchChannel, m_keyword);

		try
		{
            m_newsFeed.Save();
		}
		catch(const CExceptionBase& e)
		{
			MessageBox(e.GetErrorMsg(),_T("GreatNews"));
			return 0;
		}
	}

	EndDialog(wID);
	return 0;
}

