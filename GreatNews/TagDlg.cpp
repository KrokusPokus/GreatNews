#include "StdAfx.h"
#include "TagDlg.h"
#include "GNResourceManager.h"
#include "StyleComboHelper.h"

LRESULT CTagDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	DoDataExchange(FALSE);

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("LabelDialog"));

	// fill in style combo
	CStyleComboHelper::FillInCombo(GetDlgItem(IDC_CBOSTYLES));

	CStyleComboHelper::SetStyleComboSelection(GetDlgItem(IDC_CBOSTYLES),m_tag.m_tagStyle);

	CString str;
	str.Format(ResManagerPtr->GetString(IDS_NUMOFITEMSWITHLABEL), m_tag.GetNumOfItemsWithLabel());
	SetDlgItemText(IDC_NUMOFITEMS, str);
	return TRUE;
}

LRESULT CTagDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID == IDOK)
	{
		DoDataExchange(TRUE);
		CStyleComboHelper::GetStyleComboSelection(
			GetDlgItem(IDC_CBOSTYLES),m_tag.m_tagStyle);

		try
		{
            m_tag.Save();
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
