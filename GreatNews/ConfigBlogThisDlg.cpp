// aboutdlg.cpp : implementation of the CConfigBlogThisDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "ConfigBlogThisDlg.h"
#include "FeedManagerLib.h"
#include "GreatNewsConfig.h"

#include "GNResourceManager.h"

LRESULT CConfigBlogThisDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	CListBox list(GetDlgItem(IDC_TOOLSLIST));

	CBlogTool::GetAllBlogTools(m_allBlogTools);
	for(BlogToolVector::iterator it = m_allBlogTools.begin(); it != m_allBlogTools.end(); ++it)
	{
		BlogToolPtr tool = *it;
		int n = list.AddString(tool->m_name);
		list.SetItemData(n, tool->m_id);
	}

	CBlogToolType::GetAllBlogToolTyes(m_allBlogToolTypes, g_GreatNewsConfig.GetPluginDir());
	CComboBox box(GetDlgItem(IDC_TOOLTYPES));
	for(BlogToolTypeVector::iterator it2 = m_allBlogToolTypes.begin(); it2 != m_allBlogToolTypes.end(); ++it2)
	{
		BlogToolTypePtr type = *it2;
		box.AddString(type->m_name);
	}

	if(list.GetCount())
		list.SetCurSel(0);

	SetupControls();

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ConfigBlogToolsDialog"));

	return TRUE;
}

LRESULT CConfigBlogThisDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

void CConfigBlogThisDlg::OnTypeSelChange(UINT code, int id, HWND)
{
	CString str;
	GetDlgItemText(IDC_TOOLTYPES, str);

	for(BlogToolTypeVector::iterator it = m_allBlogToolTypes.begin(); it != m_allBlogToolTypes.end(); ++it)
	{
		BlogToolTypePtr type = *it;
		if(type->m_name == str)
		{
			SetDlgItemText(IDC_TOOLURL, type->m_defaultURL);
			break;
		}
	}
}

void CConfigBlogThisDlg::OnToolSelChange(UINT code, int id, HWND)
{
	SetupControls();
}

void CConfigBlogThisDlg::SetupControls()
{
	CListBox list(GetDlgItem(IDC_TOOLSLIST));
	size_t toolID = list.GetItemData(list.GetCurSel());
	
	BlogToolPtr tool;
	for(BlogToolVector::iterator it = m_allBlogTools.begin(); it != m_allBlogTools.end(); ++it)
	{
		tool = *it;
		if(tool->m_id == toolID)
			break;
	}

	if(tool != NULL)
	{
		::EnableWindow(GetDlgItem(IDC_TOOLNAME), TRUE);
		::EnableWindow(GetDlgItem(IDC_TOOLTYPES), TRUE);
		::EnableWindow(GetDlgItem(IDC_TOOLURL), TRUE);
		::EnableWindow(GetDlgItem(IDC_DELETE), TRUE);
		::EnableWindow(GetDlgItem(IDC_SAVE), TRUE);
		
		m_selectedTool = tool;

		CComboBox box(GetDlgItem(IDC_TOOLTYPES));
		SetDlgItemText(IDC_TOOLNAME, m_selectedTool->m_name);
		box.SetCurSel(-1);
		box.SelectString(-1, m_selectedTool->m_type);
		SetDlgItemText(IDC_TOOLURL, m_selectedTool->m_url);
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_TOOLNAME), FALSE);
		::EnableWindow(GetDlgItem(IDC_TOOLTYPES), FALSE);
		::EnableWindow(GetDlgItem(IDC_TOOLURL), FALSE);
		::EnableWindow(GetDlgItem(IDC_DELETE), FALSE);
		::EnableWindow(GetDlgItem(IDC_SAVE), FALSE);

	}
}
LRESULT CConfigBlogThisDlg::OnBnClickedNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static int n = 1;

	try
	{
		BlogToolPtr newTool = new CBlogTool();
		// create a unique name
		while(newTool->m_name.IsEmpty() || std::find(m_allBlogTools.begin(), m_allBlogTools.end(), newTool->m_name) != m_allBlogTools.end()) 
			newTool->m_name.Format(_T("New Tool %d"), n++);
		newTool->Save();
		m_allBlogTools.push_back(newTool);

		CListBox list(GetDlgItem(IDC_TOOLSLIST));
		int n = list.AddString(newTool->m_name);
		list.SetItemData(n, newTool->m_id);
		list.SetCurSel(n);
		SetupControls();
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CConfigBlogThisDlg::OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	try
	{
		if(m_selectedTool != NULL)
		{
			CString msg;
			msg.Format(_T("Are you sure to delete '%s'?"), (LPCTSTR)m_selectedTool->m_name);
			if(IDYES!=MessageBox(msg, _T("GreatNews"), MB_YESNO|MB_ICONQUESTION))
				return 0;

			CListBox list(GetDlgItem(IDC_TOOLSLIST));
			for(int n = 0; n < list.GetCount(); ++n)
			{
				if(list.GetItemData(n) == m_selectedTool->m_id)
				{
					m_selectedTool->Delete();
					list.DeleteString(n);

					if(n<list.GetCount())
						list.SetCurSel(n);
					else if(n>0)
						list.SetCurSel(n-1);

					break;
				}
			}
		}

		SetupControls();
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CConfigBlogThisDlg::OnBnClickedSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	try
	{
		if(m_selectedTool != NULL)
		{
			// verify
			CString url;
			GetDlgItemText(IDC_TOOLURL, url);
			if(url.Find(_T("<yoursite.com>")) != -1)
			{
				MessageBox(ResManagerPtr->GetString(IDS_REPLACEYOURSITE), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
				::SetFocus(GetDlgItem(IDC_TOOLURL));
				return 0;
			}

			//save
			GetDlgItemText(IDC_TOOLNAME, m_selectedTool->m_name);
			GetDlgItemText(IDC_TOOLTYPES, m_selectedTool->m_type);
			GetDlgItemText(IDC_TOOLURL, m_selectedTool->m_url);
			m_selectedTool->Save();

			//update list display
			CListBox list(GetDlgItem(IDC_TOOLSLIST));
      for(int n = 0; n < list.GetCount(); ++n)
			{
				if(list.GetItemData(n) == m_selectedTool->m_id)
				{
					list.DeleteString(n);
					list.InsertString(n, m_selectedTool->m_name);
					list.SetItemData(n, m_selectedTool->m_id);
					list.SetCurSel(n);
					break;
				}
			}
		}
	}
	CATCH_ALL_ERROR()
	
	return 0;
}
