#include "stdafx.h"
#include "resource.h"
#include "CleanupWizard.h"
#include "NewsFeed.h"
#include "FeedManagerLib.h"
#include "GreatNewsConfig.h"
#include "GNResourceManager.h"
#include "GNUtil.h"

LRESULT CCleanupWizard::CCleanupDays<CCleanupWizard::CData>::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CCleanupDays::OnInitDialog()\n");
	GetPropertySheet().SendMessage( MM_CENTER_SHEET );

	CComboBox cboDays = GetDlgItem(IDC_CBODAYS);
	for(int i=0;g_GreatNewsConfig.CleanupMaxAgeString[i]; ++i)
	{
		cboDays.AddString(g_GreatNewsConfig.CleanupMaxAgeString[i]);
	}

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("CleanupDialog_Options"));
	pResMngr->ApplyLanguageToCombo(cboDays, _T("Cleanup"));

	m_pData->m_dwCbodays = g_GreatNewsConfig.GetCleanupDefault();
	m_pData->m_chkDeleteUnread = g_GreatNewsConfig.GetCleanupDeleteUnread();
	m_pData->m_chkDeleteMarked = g_GreatNewsConfig.GetCleanupDeleteLabeled();
	m_pData->m_chkCompress = g_GreatNewsConfig.GetCleanupCompress();
	m_pData->m_chkKeepItems = g_GreatNewsConfig.GetCleanupKeepItems();
	m_pData->m_chkDeleteTemp = g_GreatNewsConfig.GetCleanupDeleteTemp();
	m_pData->m_numOfItemsToKeep = g_GreatNewsConfig.GetCleanupNumOfItemsToKeep();
	return 0;
} // OnInitDialog

int CCleanupWizard::CCleanupDays<CCleanupWizard::CData>::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_NEXT );
	
	DoDataExchange(false);

	return 1; // return 0 to avoid setting this page as active
}
int CCleanupWizard::CCleanupDays<CCleanupWizard::CData>::OnKillActive(void)
{
	DoDataExchange(true);

	if(m_pData->m_chkDeleteMarked)
	{
		if(MessageBox(ResManagerPtr->GetString(IDS_CLEANLABELLED),
			_T("GreatNews"), MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) != IDYES)
		{
			return 0;
		}
	}
	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CCleanupChannels
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CCleanupWizard::CCleanupChannels<CCleanupWizard::CData>::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CCleanupChannels::OnInitDialog()\n");

	m_tree.SubclassWindow(GetDlgItem(IDC_TREESELECT));
	m_tree.LoadTree((LoadTreeType)(LoadChannel|LoadSearch));
	m_tree.SetTreeImageList();
	m_tree.ShowEverything();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("CleanupDialog_Channels"));

	return 0;
} // OnInitDialog


LRESULT CCleanupWizard::CCleanupChannels<CCleanupWizard::CData>::OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tree.CheckAll();
	return 0;
} // 

LRESULT CCleanupWizard::CCleanupChannels<CCleanupWizard::CData>::OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_tree.CheckAll(false);
	return 0;
} // 

int CCleanupWizard::CCleanupChannels<CCleanupWizard::CData>::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT );

	return 1; // return 0 to avoid setting this page as active
}
int CCleanupWizard::CCleanupChannels<CCleanupWizard::CData>::OnKillActive(void)
{
	m_tree.NormalizeSelection();

	if(m_tree.GetAllChannels(m_pData->channelIDs, true) == 0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SELECTONECHANNEL),_T("GreatNews"),MB_OK|MB_ICONINFORMATION);
		return 0;
	}

	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CCleanupProgress
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CCleanupWizard::CCleanupProgress<CCleanupWizard::CData>::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CCleanupProgress::OnInitDialog()\n");
	DoDataExchange(false);

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("CleanupDialog_Progress"));

	return 0;
} // OnInitDialog

int CCleanupWizard::CCleanupProgress<CCleanupWizard::CData>::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT );

	CWaitCursor wc;
	int nNumOfItemsBefore = CNewsItem::GetNumOfAllItems();
	CString msg;
	msg.Format(ResManagerPtr->GetString(IDS_ITEMSBEFORECLEANUP), nNumOfItemsBefore);
	GetDlgItem(IDC_BEFORECLEANUP).SetWindowText(msg);

	return 1; // return 0 to avoid setting this page as active
}

int CCleanupWizard::CCleanupProgress<CCleanupWizard::CData>::OnWizardNext(void)
{
	DoDataExchange(true);

	// save user options first
	g_GreatNewsConfig.SetCleanupDefault(m_pData->m_dwCbodays);
	g_GreatNewsConfig.SetCleanupDeleteUnread(m_pData->m_chkDeleteUnread);
	g_GreatNewsConfig.SetCleanupDeleteLabeled(m_pData->m_chkDeleteMarked);
	g_GreatNewsConfig.SetCleanupCompress(m_pData->m_chkCompress);
	g_GreatNewsConfig.SetCleanupKeepItems(m_pData->m_chkKeepItems);
	g_GreatNewsConfig.SetCleanupDeleteTemp(m_pData->m_chkDeleteTemp);
	g_GreatNewsConfig.SetCleanupNumOfItemsToKeep(m_pData->m_numOfItemsToKeep);

	//SetWizardButtons(PSWIZB_DISABLEDFINISH); // doesn't seem to work
	UpdateWindow();

	// cleaning up!
	try
	{
		CWaitCursor wc;

		CString text;
		CStatic progressText = GetDlgItem(IDC_CLEANTEXT);
		CProgressBarCtrl progressBar = GetDlgItem(IDC_CLEANPROGRESS);
		progressBar.SetRange(0,100);
		float done = 0;
		for(NewsFeedVector::iterator it = m_pData->channelIDs.begin(); it != m_pData->channelIDs.end(); ++it)
		{
			NewsFeedPtr feed = *it;
			text.Format(ResManagerPtr->GetString(IDS_CLEANINGCHANNEL), feed->m_title);
			progressText.SetWindowText(text);
			feed->Cleanup(
				g_GreatNewsConfig.CleanupMaxAgeDays[m_pData->m_dwCbodays],
				m_pData->m_chkDeleteUnread,
				m_pData->m_chkDeleteMarked,
				m_pData->m_chkKeepItems,
				m_pData->m_numOfItemsToKeep);

			done += 1;
			progressBar.SetPos(int(done/m_pData->channelIDs.size()*80));
		}

		// compact database
		if(m_pData->m_chkCompress)
		{
			progressText.SetWindowText(ResManagerPtr->GetString(IDS_COMPACTINGDATAFILE));
			FeedManagerLib::CompactDatabase();
		}
		progressBar.SetPos(90);

		if(m_pData->m_chkDeleteTemp)
		{
			// delete temp files
			progressText.SetWindowText(_T("Deleting GreatNews temporary files..."));

			CGNUtil::CleanupGNTempFiles();
		}
		progressBar.SetPos(100);

		// Finish up
		g_GreatNewsConfig.SetCleanupLastRunNow();

		int nNumOfItemsAfter = CNewsItem::GetNumOfAllItems();
		CString msg;
		msg.Format(ResManagerPtr->GetString(IDS_ITEMSAFTERCLEANUP), nNumOfItemsAfter);
		GetDlgItem(IDC_AFTERCLEANUP).SetWindowText(msg);
		GetDlgItem(IDC_AFTERCLEANUP).ShowWindow(SW_SHOW);

		progressText.SetWindowText(ResManagerPtr->GetString(IDS_CLEANUPFINISH));
		SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
	}
	CATCH_ALL_ERROR()

	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////
