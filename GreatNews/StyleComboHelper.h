#pragma once

#include "GreatNewsConfig.h"

class CStyleComboHelper
{
public:

#define STR_GLOBALSTYLE _T("<use global style>")

	static void FillInCombo(HWND hwndCombo)
	{
		// fill in style combo
		CComboBox cboStyles = hwndCombo;
		cboStyles.AddString(STR_GLOBALSTYLE);
		CFindFile finder;
		BOOL bFound = finder.FindFile(g_GreatNewsConfig.GetMediaDir()+"\\*.css");
		while(bFound)
		{
			cboStyles.AddString(finder.GetFileTitle());
			bFound = finder.FindNextFile();
		}
	}

	static void SetStyleComboSelection(HWND hwndCombo, const CString& styleText)
	{
		CComboBox cboStyles = hwndCombo;
		if(styleText.GetLength())
			cboStyles.SelectString(-1, styleText);
		else
			cboStyles.SelectString(-1, STR_GLOBALSTYLE);
	}

	static void GetStyleComboSelection(HWND hwndCombo, CString& styleText)
	{
		CComboBox cboStyles = hwndCombo;
		cboStyles.GetWindowText(styleText);
		if(styleText == STR_GLOBALSTYLE)
			styleText.Empty();
	}

};