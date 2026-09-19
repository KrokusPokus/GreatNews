#include "stdafx.h"
#include "resource.h"
#include "ImportWizard.h"
#include "GNFileDialog.h"
#include "FeedManagerLib.h"
#include "GreatNewsConfig.h"
#include "GNUtil.h"
#include "GNClipBoard.h"
#include "FeedGroupDlg.h"
#include "AsyncDownloadManager.h"
#include "GNResourceManager.h"

#import "MSXML3.DLL"
/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CPageFileName
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CImportWizard::CPageFileName::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CPageFileName::OnInitDialog()\n");
	GetPropertySheet().SendMessage( MM_CENTER_SHEET );

	CString url;
	CGNClipBoard clipboard;
	if(clipboard.Open(m_hWnd) && clipboard.GetTextData(url))
	{
		url.Trim();
		if(CGNUtil::IsOpmlUrl(url))
		{
			m_pData->m_url = url;
			m_pData->m_nImportType = 2;
			GetDlgItem(IDC_CBOIMPORTURL).EnableWindow(TRUE);
		}
	}

	DoDataExchange(false);		

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ImportDialog_File"));

	return 0;
} // OnInitDialog

LRESULT CImportWizard::CPageFileName::OnBtnbrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CGNFileDialog dlg(true, _T("xml"), _T(""), OFN_HIDEREADONLY, 
		_T("OPML Files (opml, xml)\0*.opml;*.xml\0All Files (*.*)\0*.*\0"),
		NULL, false);

	if(dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_TXTFILENAME, dlg.m_szFileName);
	}

	return 0;
} // OnBtnbrowse

LRESULT CImportWizard::CPageFileName::OnRdoimportwellknown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetDlgItem(IDC_TXTFILENAME).EnableWindow(FALSE);
	GetDlgItem(IDC_BTNBROWSE).EnableWindow(FALSE);

	GetDlgItem(IDC_CBOIMPORTURL).EnableWindow(FALSE);

	return 0;
} // OnRdoimportwellknown

LRESULT CImportWizard::CPageFileName::OnRdoImportSelectFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetDlgItem(IDC_TXTFILENAME).EnableWindow();
	GetDlgItem(IDC_BTNBROWSE).EnableWindow();

	GetDlgItem(IDC_CBOIMPORTURL).EnableWindow(FALSE);

	return 0;
} // OnRdoImportSelectFile

LRESULT CImportWizard::CPageFileName::OnRdoImportInternet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	GetDlgItem(IDC_TXTFILENAME).EnableWindow(FALSE);
	GetDlgItem(IDC_BTNBROWSE).EnableWindow(FALSE);

	GetDlgItem(IDC_CBOIMPORTURL).EnableWindow(TRUE);

	return 0;
}

CImportWizard::CPageFileName::CPageFileName(void) : CPropertyPageImpl<CPageFileName>( _T("Import from opml...") )
{
	ATLTRACE("CPageFileName::CPageFileName()\n");
}

int CImportWizard::CPageFileName::OnSetActive(void)
{
	DoDataExchange(false);
	SetWizardButtons(PSWIZB_NEXT );

	return 1; // return 0 to avoid setting this page as active
}
int CImportWizard::CPageFileName::OnKillActive(void)
{
	if(!DoDataExchange(true))
		return 0;
	
	// validate
	if(m_pData->m_nImportType == 1 && m_pData->m_fileName.GetLength()==0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SPECIFYFILENAME),_T("GreatNews"));
		GetDlgItem(IDC_TXTFILENAME).SetFocus();
		return 0;
	}
	if(m_pData->m_nImportType == 2 && m_pData->m_url.GetLength() == 0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SPECIFYURL),_T("GreatNews"));
		GetDlgItem(IDC_TXTURL).SetFocus();
		return 0;
	}

	// load the DOM
	try
	{
		MSXML2::IXMLDOMDocumentPtr spDoc; 
		if(m_pData->m_nImportType == 2)
		{
			// import from internet repository
			spDoc = CGNSingleton<CAsyncDownloadManager>::Instance()->GetDomFromUrl(m_pData->m_url);
		}
		else
		{
			spDoc.CreateInstance(_T("MSXML2.DOMDocument.3.0"));
			CString fileName = ( m_pData->m_nImportType == 0 ? 
								g_GreatNewsConfig.GetDefaultChannelOPML() 
								: m_pData->m_fileName);
			spDoc->load((LPCTSTR)fileName);
			CGNUtil::CheckDomError(spDoc);
		}
		m_pData->m_spDoc = spDoc;
		m_pData->m_bReloadTree = true;
		return 1;
	}
	CATCH_ALL_ERROR()

	return 0; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CPageSelect
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CImportWizard::CPageSelect::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CPageSelect::OnInitDialog()\n");
	m_pData->m_hwndTree = GetDlgItem(IDC_FEEDTREE);
	ATLASSERT(::IsWindow(m_pData->m_hwndTree));
	m_treeSelect.SubclassWindow(m_pData->m_hwndTree);
	m_treeSelect.SetTreeImageList();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ImportDialog_Select"));

	return 0;
} // OnInitDialog

LRESULT CImportWizard::CPageSelect::OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_treeSelect.CheckAll();
	return 0;
} // OnRdoimportwellknown

LRESULT CImportWizard::CPageSelect::OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_treeSelect.CheckAll(false);
	return 0;
} // OnRdoImportSelectFile

CImportWizard::CPageSelect::CPageSelect(void) : CPropertyPageImpl<CPageSelect>( _T("Import from opml...") )
{
	ATLTRACE("CPageSelect::CPageSelect()\n");
}

int CImportWizard::CPageSelect::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT );
	
	if(!m_pData->m_bReloadTree)
	{
		return 1;
	}

	//
	// Load channels and groups from file
	//
	bool bSucceeded = false;
	CWaitCursor wc;

	try
	{
		MSXML2::IXMLDOMDocumentPtr spDoc = m_pData->m_spDoc; 
		if(spDoc == NULL)
		{
			// something is wrong
			return 1;
		}

		m_treeSelect.DeleteAllItems();

		CFeedGroup* pRootFeedGroup = new CFeedGroup();
		pRootFeedGroup->m_name = _T("Channels/Groups");
		CTreeItem itemRoot = m_treeSelect.InsertItem(TVI_ROOT, pRootFeedGroup, TRUE);
			
		//
		// import groups
		//
		MSXML2::IXMLDOMNodeListPtr spGroups = 
			spDoc->selectNodes(_T("/opml/body//outline[(@text or @title) and not(@xmlUrl)]"));
		MSXML2::IXMLDOMElementPtr spGroup;
		m_pData->m_bShowGroupPage = (spGroups->Getlength()==0);
		while(NULL != (spGroup = spGroups->nextNode()))
		{
			CFeedGroup* pFeedGroup = new CFeedGroup();
			pFeedGroup->LoadFromXml(spGroup);
			CTreeItem itemGroup = m_treeSelect.InsertItem(itemRoot, pFeedGroup, TRUE);

			// import feeds under this group
			MSXML2::IXMLDOMNodeListPtr spFeeds =
				spGroup->selectNodes(_T("outline[(@text or @title) and @xmlUrl]"));
			MSXML2::IXMLDOMElementPtr spFeed;
			while(NULL != (spFeed = spFeeds->nextNode()))
			{
				CNewsFeed* pFeed = new CNewsFeed();
				pFeed->LoadFromXml(spFeed);
	
				m_treeSelect.InsertItem(itemGroup, pFeed, TRUE);
			}
		}
		
		//
		// import feeds directly under root
		//
		MSXML2::IXMLDOMNodeListPtr spFeeds =
			spDoc->selectNodes(_T("/opml/body/outline[(@text or @title) and @xmlUrl]"));
		MSXML2::IXMLDOMElementPtr spFeed;
		while(NULL != (spFeed = spFeeds->nextNode()))
		{
			CNewsFeed* pFeed = new CNewsFeed();
			pFeed->LoadFromXml(spFeed);

			m_treeSelect.InsertItem(itemRoot, pFeed, TRUE);
		}

		//
		// show everything
		//
		m_treeSelect.CheckAll();
		m_treeSelect.ExpandAll();
		itemRoot.SetState(INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);
		itemRoot.EnsureVisible();

		//
		// check if this opml has watch and label nodes
		//
		MSXML2::IXMLDOMNodePtr spWatchList = spDoc->selectSingleNode(_T("/opml/watchList"));
		MSXML2::IXMLDOMNodePtr spLabelList = spDoc->selectSingleNode(_T("/opml/labelList"));
		m_pData->m_importWatchLabel = (spWatchList != NULL || spLabelList!=NULL);
		GetDlgItem(IDC_CHKIMPORTWATCHLABEL).ShowWindow(
			m_pData->m_importWatchLabel ? SW_SHOW : SW_HIDE);
		DoDataExchange(false);

		m_pData->m_bReloadTree = false;
	}
	CATCH_ALL_ERROR()

	return 1; // return 0 to avoid setting this page as active
}
int CImportWizard::CPageSelect::OnKillActive(void)
{
	m_treeSelect.NormalizeSelection();
	if(!DoDataExchange(true))
		return 0;

	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CPageProgress
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CImportWizard::CPageProgress::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CPageProgress::OnInitDialog()\n");

	m_txtImporting = GetDlgItem(IDC_IMPORTING);
	m_lstProgress= GetDlgItem(IDC_LSTPROGRESS);

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ImportDialog_Progress"));

	return 0;
} // OnInitDialog

CImportWizard::CPageProgress::CPageProgress(void) : CPropertyPageImpl<CPageProgress>( _T("Import from opml...") )
{
	ATLTRACE("CPageProgress::CPageProgress()\n");
	m_bImporting = false;
}

int CImportWizard::CPageProgress::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT );
	GetDlgItem(IDC_STOP).EnableWindow(FALSE);

	return 1; // return 0 to avoid setting this page as active
}

BOOL CImportWizard::CPageProgress::OnQueryCancel()
{
	if(IsImporting())
	{
		MessageBox(ResManagerPtr->GetString(IDS_STOPIMPORT),_T("GreatNews"));
		return FALSE;
	}
	else
		return TRUE;
		
}

int CImportWizard::CPageProgress::OnWizardNext()
{
	SetWizardButtons( PSWIZB_DISABLEDFINISH );

	CWaitCursor wc;

	try
	{
		m_lstProgress.ResetContent();
		GetDlgItem(IDC_STOP).EnableWindow();
		ATLASSERT(::IsWindow(m_pData->m_hwndTree));
		m_importThread.m_hwndTree = m_pData->m_hwndTree;
		m_importThread.m_nDefaultGroup = m_pData->m_nDefaultGroup;
		m_importThread.m_hwndToNotify = m_hWnd;
		m_importThread.m_bUpdateAfterImport = (m_pData->m_updateChannel!=0);
		m_importThread.m_importWatchLabel = (m_pData->m_importWatchLabel!=0);
		m_importThread.m_spDoc = m_pData->m_spDoc;
		m_importThread.Start();		
	}
	CATCH_ALL_ERROR()

	return -1;
}

INT_PTR CImportWizard::CPageProgress::OnWizardFinish(void)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////


LRESULT CImportWizard::CPageProgress::OnImportStatus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lpText, BOOL& /*bHandled*/)
{
	if(lpText != 0)
	{
		LPTSTR p = (LPTSTR)lpText;
		m_lstProgress.AddMessage(p);
		m_lstProgress.UpdateWindow();
		delete[] p;
	}
	
	return 0;
}

LRESULT CImportWizard::CPageProgress::OnImportFinished(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	GetDlgItem(IDC_STOP).EnableWindow(FALSE);
	SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH );

	CString msg;
	msg.Format(ResManagerPtr->GetString(IDS_IMPORTSUMMARY),
		m_importThread.m_cntFeedImported, m_importThread.m_cntGroupImported);
	if(m_importThread.m_cntFeedSkipped || m_importThread.m_cntGroupSkipped)
	{
		msg.AppendFormat(ResManagerPtr->GetString(IDS_IMPORTSKIP),
			m_importThread.m_cntFeedSkipped, m_importThread.m_cntGroupSkipped);
	}
	if(m_importThread.m_cntFeedFailed || m_importThread.m_cntGroupFailed)
	{
		msg.AppendFormat(ResManagerPtr->GetString(IDS_IMPORTFAILEDSUMMARY),
			m_importThread.m_cntFeedFailed, m_importThread.m_cntGroupFailed);
	}
	MessageBox(msg,_T("GreatNews"));

	return 0;
}

LRESULT CImportWizard::CPageProgress::OnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CWaitCursor wc;
	m_lstProgress.AddMessage(_T("*** Stopping import process..."));
	m_lstProgress.UpdateWindow();
	m_importThread.m_bStop = true;
	if(m_importThread.m_stopEvent.Lock(5000)==FALSE)
	{
		// cannot stop the import after 5 seconds, let's do it brutually
		m_importThread.Stop();
		
		// simulate thread's finish msg
		PostMessage(MM_IMPORT_FINISHED, 0 ,0);
	}

	m_lstProgress.AddMessage(_T("*** Import process stopped."));

	return 0;
}

BOOL CImportWizard::CPageProgress::IsImporting()
{
	return GetDlgItem(IDC_STOP).IsWindowEnabled();
}

//
// Implementation of class CPageGroup
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CImportWizard::CPageGroup::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CPageGroup::OnInitDialog()\n");
	m_treeGroup.SubclassWindow(GetDlgItem(IDC_GROUPTREE));

	if(m_pData->m_bShowGroupPage)
	{
		m_treeGroup.SetTreeImageList();
		m_treeGroup.LoadTree(LoadGroup);
		m_treeGroup.GetRootItem().Select();
	}

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ImportDialog_Group"));

	return 0;
} // OnInitDialog

CImportWizard::CPageGroup::CPageGroup(void) : CPropertyPageImpl<CPageGroup>( _T("Import from opml...") )
{
	ATLTRACE("CPageGroup::CPageGroup()\n");
}

int CImportWizard::CPageGroup::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT );

	return m_pData->m_bShowGroupPage ? 1 : 0; // return 0 to avoid setting this page as active
}
int CImportWizard::CPageGroup::OnKillActive(void)
{
	CFeedGroupTreeItem* pItem = dynamic_cast<CFeedGroupTreeItem*>(m_treeGroup.GetSelectedFeedItem());
	if(pItem)
	{
		m_pData->m_nDefaultGroup = pItem->m_feedGroup->m_id;
	}
	else
	{
		MessageBox(ResManagerPtr->GetString(IDS_PLSSELECTGROUP), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		return 0;
	}

	return 1; // 0 to stay in the same page.
}


LRESULT CImportWizard::CPageGroup::OnNewGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
