#include "stdafx.h"
#include "resource.h"
#include "WatchPropSheet.h"
#include "FeedManagerLib.h"
#include "GNResourceManager.h"
#include "StyleComboHelper.h"

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CWatchPropPageName
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CWatchPropSheet::CWatchPropPageName::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_bAutoGenerateName = false;

	GetPropertySheet().SendMessage( MM_CENTER_SHEET );

	ATLTRACE("CWatchPropPageName::OnInitDialog()\n");

	// Attach member controls to their resources.
	m_chkTitle = GetDlgItem(IDC_CHKTITLE);
	m_chkContent = GetDlgItem(IDC_CHKCONTENT);
	m_chkAuthor = GetDlgItem(IDC_CHKAUTHOR2);
	m_chkURL = GetDlgItem(IDC_CHKWATCHURL);

	m_chkTitle.SetCheck(m_pNewsWatch->m_watchFlag & WATCH_TITLE);
	m_chkContent.SetCheck(m_pNewsWatch->m_watchFlag & WATCH_CONTENT);
	m_chkAuthor.SetCheck(m_pNewsWatch->m_watchFlag & WATCH_AUTHOR);
	m_chkURL.SetCheck(m_pNewsWatch->m_watchFlag & WATCH_URL);

	m_btnFrontColor.SubclassWindow(GetDlgItem(IDC_TEXTCOLOR));
	m_btnFrontColor.SetDefaultText(_T("Text Color"));
	m_btnFrontColor.SetColor(m_pNewsWatch->m_txtColor);

	m_btnBkColor.SubclassWindow(GetDlgItem(IDC_BKCOLOR));
	m_btnBkColor.SetDefaultText(_T("Background Color"));
	m_btnBkColor.SetColor(m_pNewsWatch->m_bkColor);

	// fill in style combo
	CStyleComboHelper::FillInCombo(GetDlgItem(IDC_CBOSTYLES));

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("NewsWatchPropertiesDialog_Properties"));

	// check if we should auto update watch name
	if(m_pNewsWatch->m_id<=0 // new watch?
		|| m_pNewsWatch->m_title == m_pNewsWatch->m_matchCriteria // same?
		|| m_pNewsWatch->m_title == GenerateTitle(m_pNewsWatch->m_matchCriteria)) // auto generated?
	{
		m_bAutoGenerateName = true;
	}

	return 0;
} // OnInitDialog

LRESULT CWatchPropSheet::CWatchPropPageName::OnKeywordsChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(!m_bAutoGenerateName)
		return 0;

	CString keyword;
	GetDlgItemText(IDC_WATCH_WORDS, keyword);
	CString title = GenerateTitle(keyword);
	SetDlgItemText(IDC_WATCH_NAME, title);

	return 1;
}

CString CWatchPropSheet::CWatchPropPageName::GenerateTitle(CString keyword)
{
	return _T("Watch \"")+keyword+_T("\"");
}


int CWatchPropSheet::CWatchPropPageName::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_NEXT );

	DoDataExchange(false);
	CStyleComboHelper::SetStyleComboSelection(GetDlgItem(IDC_CBOSTYLES),
		m_pNewsWatch->m_watchStyle);

	return 1; // return 0 to avoid setting this page as active
}
int CWatchPropSheet::CWatchPropPageName::OnKillActive(void)
{
	if(!DoDataExchange(true))
		return 0;
	CStyleComboHelper::GetStyleComboSelection(GetDlgItem(IDC_CBOSTYLES),
		m_pNewsWatch->m_watchStyle);
	
	if(m_pNewsWatch->m_matchCriteria.GetLength()==0)
	{
		GetDlgItem(IDC_WATCH_WORDS).SetFocus();
		MessageBox(ResManagerPtr->GetString(IDS_FILLINMATCHWORD),_T("GreatNews"));
		return 0;
	}
	if(m_pNewsWatch->m_title.GetLength()==0)
	{
		GetDlgItem(IDC_WATCH_NAME).SetFocus();
		MessageBox(ResManagerPtr->GetString(IDS_FILLINWATCHTITLE),_T("GreatNews"));
		return 0;
	}

	m_pNewsWatch->m_watchFlag = 0;
	m_pNewsWatch->m_watchFlag |= (m_chkTitle.GetCheck() ? WATCH_TITLE : 0);
	m_pNewsWatch->m_watchFlag |= (m_chkContent.GetCheck() ? WATCH_CONTENT : 0);
	m_pNewsWatch->m_watchFlag |= (m_chkAuthor.GetCheck() ? WATCH_AUTHOR : 0);
	m_pNewsWatch->m_watchFlag |= (m_chkURL.GetCheck() ? WATCH_URL : 0);

	if(m_pNewsWatch->m_watchFlag == 0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SELECTONEWATCHOPTION),_T("GreatNews"));
		m_chkTitle.SetFocus();
		return 0;
	}

	m_pNewsWatch->m_txtColor = m_btnFrontColor.GetColor();
	m_pNewsWatch->m_bkColor = m_btnBkColor.GetColor();
	if(m_pNewsWatch->m_txtColor == m_pNewsWatch->m_bkColor)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SAMECOLOR),_T("GreatNews"));
		return 0;
	}

	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation of class CWatchPropPageChannels
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CWatchPropSheet::CWatchPropPageChannels::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CWatchPropPageChannels::OnInitDialog()\n");

	m_tree.SubclassWindow(GetDlgItem(IDC_TREESELECT));
	m_tree.LoadTree((LoadTreeType)(LoadChannel | LoadSearch));
	m_tree.SetTreeImageList();
	m_tree.ShowEverything();

	std::vector<ULONG_PTR>& watchChannels = m_pNewsWatch->LoadWatchChannels();
	m_tree.SetSelectedChannels(watchChannels);

	EnableTreeControls(m_pNewsWatch->m_bWatchSelectiveChannels!=0);


	m_chkCheckExisting = GetDlgItem(IDC_CHECKEXISINGNEWS);

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("NewsWatchPropertiesDialog_Channels"));

	return 0;
} // OnInitDialog

int CWatchPropSheet::CWatchPropPageChannels::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH /*PSWIZB_DISABLEDFINISH*/ );

	DoDataExchange(false);

	return 1; // return 0 to avoid setting this page as active
}

void CWatchPropSheet::CWatchPropPageChannels::EnableTreeControls(BOOL bEnable)
{
	GetDlgItem(IDC_TREESELECT).EnableWindow(bEnable);
	GetDlgItem(IDC_BTNSELECTALL).EnableWindow(bEnable);
	GetDlgItem(IDC_BTNUNSELECTALL).EnableWindow(bEnable);
}

LRESULT CWatchPropSheet::CWatchPropPageChannels::OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tree.CheckAll();
	return 0;
} // 

LRESULT CWatchPropSheet::CWatchPropPageChannels::OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tree.CheckAll(false);
	return 0;
} // 

LRESULT CWatchPropSheet::CWatchPropPageChannels::OnRdoall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EnableTreeControls(FALSE);

	return 0;
} // OnRdoall

LRESULT CWatchPropSheet::CWatchPropPageChannels::OnRdoselective(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EnableTreeControls();
	return 0;
} // OnRdoselective

INT_PTR CWatchPropSheet::CWatchPropPageChannels::OnWizardFinish(void)
{
	int bCheckExisting = m_chkCheckExisting.GetCheck();

	if(!DoDataExchange(true))
		return 0;

	m_tree.NormalizeSelection();

	NewsFeedVector feeds;
	size_t channelCnt = m_tree.GetAllChannels(feeds,true);
	if(m_pNewsWatch->m_bWatchSelectiveChannels == 1 && channelCnt == 0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SELECTONECHANNELTOWATCH),_T("GreatNews"));
		return 0;
	}


	// save to backend
	m_pNewsWatch->AssignChannelSelection(feeds);
	CWaitCursor wc;

	try
	{
		m_pNewsWatch->Save();

		if(bCheckExisting)
		{
			m_pNewsWatch->CheckExistingItems();
		}
	}
	catch(const CExceptionBase& e)
	{
		MessageBox(e.GetErrorMsg(),_T("GreatNews"));
		return 0;
	}

	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

