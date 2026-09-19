#pragma once

#include "CustomAutoComplete.h"

/**
 * @class CAutoComboBoxImpl
 * @brief template class implementing auto completion for combo boxes.
 */
template <class T, class TBase = CComboBoxEx, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CAutoComboBoxImpl : public CWindowImpl< T, TBase, TWinTraits>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

	typedef CWindowImpl< T, TBase, TWinTraits >	baseClass;

	CCustomAutoComplete* m_pAC;

// Constructors
	CAutoComboBoxImpl() : m_pAC(NULL){ }

	CAutoComboBoxImpl< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;

		return *this;
	}

	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName,
			DWORD dwStyle, DWORD dwExStyle, _U_MENUorID MenuOrID,
			LPCTSTR iniFileName, LPCTSTR szSubKey, UINT nDummyId)
	{
		m_hWndOwner = hWndParent;

		HWND hWnd = baseClass::Create(hWndParent, *rect.m_lpRect, szWindowName, dwStyle, dwExStyle, (unsigned int)MenuOrID.m_hMenu);
		if(hWnd)
		{
			Init(iniFileName, szSubKey);
			SetWindowPos(GetDlgItem(nDummyId), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		return hWnd;
	}

	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName,
			DWORD dwStyle, DWORD dwExStyle, _U_MENUorID MenuOrID,
			LPCTSTR iniFileName, LPCTSTR szSubKey)
	{
		HWND hWnd = baseClass::Create(hWndParent, *rect.m_lpRect, szWindowName, dwStyle, dwExStyle, (unsigned int)MenuOrID.m_hMenu);
		if(hWnd)
		{
			Init(iniFileName, szSubKey);
		}
		return hWnd;
	}

// Message map and handlers
	typedef CAutoComboBoxImpl< T, TBase, TWinTraits >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(m_pAC)
		{
			m_pAC->Unbind();
			m_pAC->Release();
			m_pAC = NULL;
		}

		return 0;
	}

// Implementation

	void AddString(const CString& text, bool sendToAC=true)
	{
		if(text.GetLength() > 0)
		{
			COMBOBOXEXITEM cbei = {0};
			cbei.mask = CBEIF_TEXT;
			cbei.iItem = 0;
			cbei.pszText = (LPTSTR)(LPCTSTR)text;
			cbei.cchTextMax = text.GetLength();

			int i = FindStringExact(-1, text);
			// Bring the string on top
			if(i > 0)
			{
				DeleteItem(i);
				int n=InsertItem(&cbei);
			}
			else if(i < 0)
			{
				int n=InsertItem(&cbei);
			}

			if(m_pAC)
				m_pAC->AddItem(text);
		}
	}

	bool Init(LPCTSTR iniFileName, LPCTSTR szSectionName)
	{
		CEdit hWndEdit = GetEditCtrl();
		
		if(hWndEdit)
		{
			m_pAC = new CCustomAutoComplete(iniFileName, szSectionName);
			m_pAC->AddRef();
			m_pAC->Bind(hWndEdit, /*ACO_UPDOWNKEYDROPSLIST |*/ ACO_AUTOSUGGEST | ACO_AUTOAPPEND);
			// Fill combobox with the 20 recent entries, assuming AC stores the
			// strings inorder.
			std::list<CString>& items = m_pAC->GetList();
			int size = min((int)items.size(), m_pAC->GetMaxSize());
			int i = 0;
			for(std::list<CString>::iterator it = items.begin(); it != items.end() && i < size; ++it)
			{
				AddString(*it, false);
				i++;
			}

			return true;
		}

		return false;
	}
};

class CAutoComboBox : public CAutoComboBoxImpl<CAutoComboBox>
{
public:
	DECLARE_WND_SUPERCLASS(_T("GN_AutoComboBox"), GetWndClassName())
};
