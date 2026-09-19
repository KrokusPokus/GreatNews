#pragma once

#define MM_CENTER_SHEET WM_APP + 111

template <class T>
class CGNPropertySheet: public CPropertySheetImpl<T>
{
public:
	CGNPropertySheet(_U_STRINGorID title = (LPCTSTR) NULL, UINT uStartPage = 0, HWND hWndParent = NULL )
	               : CPropertySheetImpl<T>(title, uStartPage, hWndParent),
				   m_bCentered(false)
	{
	}

	BEGIN_MSG_MAP(CGNPropertySheet)
		MESSAGE_HANDLER_EX(MM_CENTER_SHEET, OnPageInit)
		CHAIN_MSG_MAP(CPropertySheetImpl<T>)
	END_MSG_MAP()

	LRESULT OnPageInit ( UINT, WPARAM, LPARAM )
	{
		if ( !m_bCentered )
		{
			m_bCentered = true;
			CenterWindow ( m_psh.hwndParent );
		}

		return 0;
	}

private:
    bool m_bCentered;  // set to false in the ctor
};