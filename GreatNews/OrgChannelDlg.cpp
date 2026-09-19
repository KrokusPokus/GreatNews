#include "StdAfx.h"
#include "OrgChannelDlg.h"
#include "FeedGroupDlg.h"
#include "GNResourceManager.h"

LRESULT COrgChannelDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_bReload = false;

	CenterWindow(GetParent());
	m_line1.SubclassWindow(GetDlgItem(IDC_LINE));
	m_line2.SubclassWindow(GetDlgItem(IDC_LINE2));
	m_line3.SubclassWindow(GetDlgItem(IDC_LINE3));

	// load channels
	m_feedTree.SubclassWindow(GetDlgItem(IDC_FEEDTREE));
	m_feedTree.SetTreeImageList();
	m_feedTree.LoadTree((LoadTreeType)(LoadChannel|LoadSearch));
	m_feedTree.ShowEverything();
	m_feedTree.CheckAll(false);

	// load group
	m_comboGroup = GetDlgItem(IDC_COMBOGROUP);
	CImageList imageList;
	imageList.CreateFromImage( IDB_TREE, 16, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION );
	m_comboGroup.SetImageList(imageList);
	LoadGroup(m_comboGroup);
	
	if(!m_feedTree.HasBloglinesSyncChannel())
	{
		GetDlgItem(IDC_DETACHBLOGLINES).ShowWindow(SW_HIDE);
	}

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("OrganizeChannelsDialog"));

	return TRUE;
}

void COrgChannelDlg::LoadGroup(CComboBoxEx& comboGroup)
{
	comboGroup.ResetContent();

	COMBOBOXEXITEM cbei = {0};
	cbei.mask = CBEIF_TEXT|CBEIF_LPARAM;
	cbei.iItem = -1;
	cbei.lParam= 0;
	cbei.pszText = _T("<select a group>");
	cbei.cchTextMax = _tcslen(cbei.pszText);
	comboGroup.InsertItem(&cbei);

	// insert root group
	cbei.iItem = -1;
	cbei.mask = CBEIF_TEXT|CBEIF_IMAGE|CBEIF_SELECTEDIMAGE|CBEIF_INDENT|CBEIF_LPARAM;
	FeedGroupPtr spRootFeedGroup = CFeedGroup::GetRootFeedGroup();
	cbei.pszText = (LPTSTR)(LPCTSTR)spRootFeedGroup->m_name;
	cbei.lParam = spRootFeedGroup->m_id;
	cbei.iImage = ChannelRootIcon;
	cbei.iSelectedImage = ChannelRootIcon;
	cbei.iIndent = 0;
	comboGroup.InsertItem(&cbei);

	// insert child groups
	cbei.iImage = ChannelGroupIcon;
	cbei.iSelectedImage = ChannelGroupIcon;
	cbei.iIndent = 1;
	FeedGroupVector childGroups;
	spRootFeedGroup->GetChildGroups(childGroups, CFeedGroup::SortByName);
	std::sort(childGroups.begin(), childGroups.end(), LessGroup);
	for(FeedGroupVector::iterator it = childGroups.begin(); it != childGroups.end(); ++it)
	{
		FeedGroupPtr& spGroup = *it;
		cbei.iItem = -1;
		cbei.pszText = (LPTSTR)(LPCTSTR)spGroup->m_name;
		cbei.cchTextMax = _tcslen(cbei.pszText);
		cbei.lParam = spGroup->m_id;
		comboGroup.InsertItem(&cbei);
	}

	comboGroup.SetCurSel(0);
}

LRESULT COrgChannelDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

LRESULT COrgChannelDlg::OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_feedTree.CheckAll();
	return 0;
}

LRESULT COrgChannelDlg::OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_feedTree.CheckAll(false);
	return 0;
}


LRESULT COrgChannelDlg::OnMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	NewsFeedVector channels;
	if(m_feedTree.GetAllChannels(channels, true)==0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SELECTCHANNELTOMOVE),
			_T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		m_feedTree.SetFocus();
		return 0;
	}

	// get selected group
	int nCurrentSelect = m_comboGroup.GetCurSel();
	if(CB_ERR == nCurrentSelect || 2 > nCurrentSelect)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SELECTGRPTOMOVEIN),
			_T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		m_comboGroup.SetFocus();

		return 0;
	}
	COMBOBOXEXITEM cbei = {0};
	cbei.mask = CBEIF_LPARAM;
	cbei.iItem = nCurrentSelect;
	m_comboGroup.GetItem(&cbei);
	LPARAM nNewGroupID = cbei.lParam;
	ATLASSERT(nNewGroupID);

	// modify group
	m_bReload = true;
	CWaitCursor wc;
	try
	{
		for(NewsFeedVector::iterator it = channels.begin(); it!=channels.end(); ++it)
		{
			NewsFeedPtr& feed = *it;
			feed->m_groupID = nNewGroupID;
			feed->Save();
		}

		// reload modified tree
		m_feedTree.LoadTree(LoadChannel);
		m_feedTree.ShowEverything();
		m_feedTree.CheckAll(false);

	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT COrgChannelDlg::OnAddGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	try
	{
		CFeedGroupDlg dlg;
		FeedGroupPtr root = CFeedGroup::GetRootFeedGroup();
		dlg.m_feedGroup.m_parentID = root->m_id;

		if(dlg.DoModal() == IDOK)
		{ 
			m_bReload = true;

			LoadGroup(m_comboGroup);

			CFeedGroup* pFeedGroup = new CFeedGroup(dlg.m_feedGroup);
			CTreeItem item = m_feedTree.InsertItem(m_feedTree.GetRootItem(), pFeedGroup, TRUE);
			item.Select();
			m_feedTree.SetFocus();
		}
	}
	CATCH_ALL_ERROR()
	
	return 0;
}


LRESULT COrgChannelDlg::OnDeleteChannels(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	NewsFeedVector channels;
	if(m_feedTree.GetAllChannels(channels, true)==0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SELECTCHANNELTODEL),
			_T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		m_feedTree.SetFocus();
		return 0;
	}
	else
	{
		CString msg;
		msg.Format(ResManagerPtr->GetString(IDS_CONFIRMDELSELECTEDCHANNEL), channels.size());
		if(IDYES != MessageBox(msg, _T("GreatNews"), MB_YESNO|MB_ICONQUESTION))
			return 0;
	}


	// delete channels
	m_bReload = true;
	CWaitCursor wc;
	try
	{
		for(NewsFeedVector::iterator it = channels.begin(); it!=channels.end(); ++it)
		{
			NewsFeedPtr& feed = *it;
			feed->Delete();
		}

		// reload modified tree
		m_feedTree.LoadTree(LoadChannel);
		m_feedTree.ShowEverything();
		m_feedTree.CheckAll(false);
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT COrgChannelDlg::OnDetachBloglines(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	NewsFeedVector channels;
	if(m_feedTree.GetAllChannels(channels, true)==0)
	{
		MessageBox(ResManagerPtr->GetString(IDS_SELECTCHANNELTODETACH),
			_T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		m_feedTree.SetFocus();
		return 0;
	}

	// delete channels
	m_bReload = true;
	CWaitCursor wc;
	try
	{
		for(NewsFeedVector::iterator it = channels.begin(); it!=channels.end(); ++it)
		{
			NewsFeedPtr& feed = *it;
			if(feed->IsBloglinesSyncChannel())
				feed->DetachFromBloglines();
		}

		// reload modified tree
		m_feedTree.LoadTree(LoadChannel);
		m_feedTree.ShowEverything();
		m_feedTree.CheckAll(false);
	}
	CATCH_ALL_ERROR()

	return 0;
}