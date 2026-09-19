#include "stdafx.h"
#include "resource.h"
#include "SyncAccountWizard.h"
#include "ExceptionBase.h"
#include "FeedManagerErrorCode.h"
#include "BloglinesService.h"
#include "GNResourceManager.h"

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CAccountPage
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CSyncAccountWizard::CAccountPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	GetPropertySheet().SendMessage( MM_CENTER_SHEET );

	CGNSingleton<CBloglinesService>::Instance()->GetAccountInfo(m_pData->m_csSyncAccount, m_pData->m_csSyncPassword);
	m_pData->m_csSyncConfirmPassword = m_pData->m_csSyncPassword;

	DoDataExchange();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("BloglinesSyncDialog_Account"));

	return 0;
} // OnInitDialog

CSyncAccountWizard::CAccountPage::CAccountPage(void) : CPropertyPageImpl<CAccountPage>( _T("Bloglines Sync Channels") )
{
	ATLTRACE("CAccountPage::CAccountPage()\n");
}
int CSyncAccountWizard::CAccountPage::OnSetActive(void)
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT );

	return 1; // return 0 to avoid setting this page as active
}
int CSyncAccountWizard::CAccountPage::OnKillActive(void)
{
	if(!DoDataExchange(true))
		return 0;

	if(m_pData->m_csSyncAccount.GetLength() == 0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_FILLINBLOGLINESACNT), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		return 0;
	}

	if(m_pData->m_csSyncConfirmPassword != m_pData->m_csSyncPassword)
	{
		MessageBox(ResManagerPtr->GetString(IDS_PWDMISMATCH), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		return 0;
	}

	CGNSingleton<CBloglinesService>::Instance()->SetAccountInfo(m_pData->m_csSyncAccount, m_pData->m_csSyncPassword);

	{
		CWaitCursor wc;

		m_pData->m_spBloglinesOPML = CGNSingleton<CBloglinesService>::Instance()->ListChannels( m_pData->m_csSyncAccount, m_pData->m_csSyncPassword);
	}

	if(m_pData->m_spBloglinesOPML == NULL)
	{
		MessageBox(CGNSingleton<CBloglinesService>::Instance()->GetErrorMessage(), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		return 0;
	}

	return 1; // 0 to stay in the same page.
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////

//
// Implementation of class CChannelPage
///////////////////////////////////////////////////////////////////////////////////////////

LRESULT CSyncAccountWizard::CChannelPage::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_treeSelect.SubclassWindow(GetDlgItem(IDC_TREESELECT));
	m_treeSelect.SetTreeImageList();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("BloglinesSyncDialog_Channels"));

	return 0;
} // OnInitDialog

LRESULT CSyncAccountWizard::CChannelPage::OnBtnselectall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_treeSelect.CheckAll();
	return 0;
} // OnBtnselectall

LRESULT CSyncAccountWizard::CChannelPage::OnBtnunselectall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_treeSelect.CheckAll(false);
	return 0;
} // OnBtnunselectall

CSyncAccountWizard::CChannelPage::CChannelPage(void) : CPropertyPageImpl<CChannelPage>( _T("Bloglines Sync Channels") )
{
	ATLTRACE("CChannelPage::CChannelPage()\n");
}

int CSyncAccountWizard::CChannelPage::OnSetActive(void)
{
	m_pData->m_bDontMark = 	CGNSingleton<CBloglinesService>::Instance()->GetMarkReadFlag();
	DoDataExchange(false);

	//
	// Load channels and groups from file
	//
	bool bSucceeded = false;
	CWaitCursor wc;

	try
	{
		MSXML2::IXMLDOMDocumentPtr spDoc = m_pData->m_spBloglinesOPML; 

#ifdef DEBUG
		_bstr_t xml = spDoc->xml;
#endif

		if(spDoc == NULL)
		{
			// something is wrong
			return 1;
		}

		m_treeSelect.DeleteAllItems();

		CFeedGroup* pRootFeedGroup = new CFeedGroup();
		pRootFeedGroup->m_name = "Bloglines Subscriptions";
		CTreeItem itemRoot = m_treeSelect.InsertItem(TVI_ROOT, pRootFeedGroup, TRUE);
			
		//
		// import groups
		//
		MSXML2::IXMLDOMNodeListPtr spGroups = 
			spDoc->selectNodes(_T("/opml/body/outline//outline[(@text or @title) and not(@xmlUrl)]"));
		MSXML2::IXMLDOMElementPtr spGroup;
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
				pFeed->m_bloglineSubId = spFeed->getAttribute(_T("BloglinesSubId"));
	
				m_treeSelect.InsertItem(itemGroup, pFeed, TRUE);
			}
		}
		
		//
		// import feeds directly under root
		//
		MSXML2::IXMLDOMNodeListPtr spFeeds =
			spDoc->selectNodes(_T("/opml/body/outline/outline[(@text or @title) and @xmlUrl]"));
		MSXML2::IXMLDOMElementPtr spFeed;
		while(NULL != (spFeed = spFeeds->nextNode()))
		{
			CNewsFeed* pFeed = new CNewsFeed();
			pFeed->LoadFromXml(spFeed);
			pFeed->m_bloglineSubId = spFeed->getAttribute(_T("BloglinesSubId"));

			m_treeSelect.InsertItem(itemRoot, pFeed, TRUE);
		}

		//
		// show everything
		//
		// m_treeSelect.CheckAll();
		m_treeSelect.ExpandAll();
		itemRoot.SetState(INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);
		itemRoot.EnsureVisible();

	}
	CATCH_ALL_ERROR()
	
	SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH /*PSWIZB_DISABLEDFINISH*/ );

	return 1; // return 0 to avoid setting this page as active
}
/////////////////////////////////////////////////////////////////////////////////////////


INT_PTR CSyncAccountWizard::CChannelPage::OnWizardFinish(void)
{
	try
	{
		// save bloglines parameters first
		DoDataExchange(true);
		CGNSingleton<CBloglinesService>::Instance()->SetMarkReadFlag(m_pData->m_bDontMark);
		CGNSingleton<CBloglinesService>::Instance()->SaveParam();

		// save sync channel selections
		CWaitCursor wc;

		m_treeSelect.NormalizeSelection();

		HWND channelTree = (HWND)m_treeSelect;
		FeedGroupPtr rootGroup = CFeedGroup::GetRootFeedGroup();
		CTreeItem rootItem = m_treeSelect.GetRootItem();
		CTreeItem childItem = rootItem.GetChild();

		CNewsFeedTreeItem* pNewsFeedItem = NULL;
		CFeedGroupTreeItem* pFeedGroupItem = NULL;
		while(childItem)
		{
			if(m_treeSelect.GetCheckState(childItem))
			{
				if(NULL != (pNewsFeedItem=dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)childItem.GetData())))
				{
					// this is a channel at root level
					NewsFeedPtr feed = pNewsFeedItem->m_newsFeed;
					feed->m_groupID = rootGroup->m_id;
					ImportNewsFeed(feed);
				}
				else if(NULL != (pFeedGroupItem=dynamic_cast<CFeedGroupTreeItem*>((CFeedTreeItem*)childItem.GetData())))
				{
					ImportFeedGroup(pFeedGroupItem->m_feedGroup);

					if(pFeedGroupItem->m_feedGroup->m_id)
					{
						CTreeItem feedItem = childItem.GetChild();

						while(feedItem)
						{
							if(m_treeSelect.GetCheckState(feedItem))
							{
								pNewsFeedItem=dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)feedItem.GetData());
								ATLASSERT(pNewsFeedItem);
								NewsFeedPtr feed = pNewsFeedItem->m_newsFeed;
								feed->m_groupID = pFeedGroupItem->m_feedGroup->m_id;

								ImportNewsFeed(feed);
							}
		
							feedItem = feedItem.GetNextSibling();
						}
					}
				}
			}
			
			childItem = childItem.GetNextSibling();
		}
	}
	CATCH_ALL_ERROR()

	return 1; // 0 to stay in the same page.

}

void CSyncAccountWizard::CChannelPage::ImportNewsFeed(NewsFeedPtr feed)
{
	CString temp;
	try
	{
		feed->m_url.Trim();
		if(feed->m_url.GetLength()==0)
		{
			throw CExceptionBase(ERR_FM_GENERICERR, _T("due to invalid URL"));
		}

		ULONG_PTR feedID = CNewsFeed::GetIdFromXmlUrl(feed->m_url);
		if(feedID !=0)
		{
			// if the user already subscribed this feed, link the feed to bloglines
			CNewsFeed existingFeed;
			existingFeed.Init(feedID);

			if(existingFeed.m_bloglineSubId != feed->m_bloglineSubId)
			{
				existingFeed.m_bloglineSubId = feed->m_bloglineSubId;
				existingFeed.Save();
				m_pData->m_cntFeedImported++;
			}
			else
			{
				m_pData->m_cntFeedSkipped++;
			}
		}
		else
		{
			int nRetry = 0;
			while(true)
			{
				try
				{
					feed->Save();
					break;
				}
				catch(const CUniqueNameException&)
				{
					nRetry++;
					if(nRetry>=30)
						throw; // insane

					// non unique name, try to solve it by appending -1
					feed->m_title = feed->m_title+_T("-1");
				}
			}

			m_pData->m_cntFeedImported++;
		}
	}
	catch(const CExceptionBase&)
	{
		m_pData->m_cntFeedFailed++;
	}
	catch(...)
	{
		m_pData->m_cntFeedFailed++;
	}
}

void CSyncAccountWizard::CChannelPage::ImportFeedGroup(FeedGroupPtr group)
{
	try
	{
		ULONG_PTR groupID = CFeedGroup::GetIdFromName(group->m_name);
		if(groupID == 0)
		{
			group->Save();
			m_pData->m_cntGroupImported++;
		}
		else
		{
			group->m_id = groupID;
			m_pData->m_cntGroupSkipped++;
		}
	}
	catch(...)
	{
		m_pData->m_cntGroupFailed++;
	}
}
