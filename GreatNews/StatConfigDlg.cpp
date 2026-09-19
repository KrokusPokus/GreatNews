#include "stdafx.h"
#include "resource.h"
#include "StatConfigDlg.h"

LRESULT CStatConfigDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	DoDataExchange(false);

	return TRUE;
}

LRESULT CStatConfigDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID == IDOK)
	{
		if(!DoDataExchange(true))
			return 0;

		g_GreatNewsConfig.SaveConfig();
	}

	EndDialog(wID);
	return 0;
}
