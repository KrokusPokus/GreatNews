#pragma once

// Bring in the GUID hack and the shell stuff
#include <initguid.h>
#include <shldisp.h>
#include <shlguid.h>
#include <list>
#include <algorithm>
#include "IniFile.h"

class CCustomAutoComplete : public IEnumString
{
public:
	// Constructors/destructors

	CCustomAutoComplete()
	{
		InternalInit();
	}

	CCustomAutoComplete(LPCTSTR szIniFileName, LPCTSTR szSectionName)
	{
		SetStorage(szIniFileName, szSectionName);
	}

	~CCustomAutoComplete()
	{
		SaveToStorage();

		m_stringItems.clear();

		if (m_pac)
			m_pac.Release();
	}

public:
	// Implementation
	BOOL SetStorage(LPCTSTR szIniFileName, LPCTSTR szSectionName)
	{
		InternalInit();

		m_iniFileName = szIniFileName;
		m_sectionName = szSectionName;

		return LoadFromStorage();
	}
	
	BOOL Bind(HWND p_hWndEdit, DWORD p_dwOptions = 0, LPCTSTR p_lpszFormatString = NULL)
	{

		ATLASSERT(::IsWindow(p_hWndEdit));

		LPOLESTR pFormatString = NULL;

		if ((m_fBound) || (m_pac))
			return FALSE;


		HRESULT hr = S_OK;

		hr = m_pac.CoCreateInstance(CLSID_AutoComplete);

		if (SUCCEEDED(hr))
		{

			if (p_dwOptions)
			{
				CComQIPtr<IAutoComplete2> pAC2(m_pac);

				ATLASSERT(pAC2);

				hr = pAC2->SetOptions(p_dwOptions);			// This never fails?
				pAC2.Release();

			}

			USES_CONVERSION;
			
			hr = m_pac->Init(p_hWndEdit, this, NULL, NULL);

			if (SUCCEEDED(hr))
			{
				m_fBound = TRUE;
				return TRUE;
			}

		}
			
		
		return FALSE;


	}

	VOID Unbind()
	{

		if (!m_fBound)
			return;

		if (m_pac)
		{
			m_pac.Release();
			m_fBound = FALSE;

		}
	}

	BOOL HasItem(const CString& p_sItem)
	{
		return (std::find(m_stringItems.begin(), m_stringItems.end(), p_sItem) != m_stringItems.end());
	}
	BOOL AddItem(const CString& p_sItem)
	{
		if (p_sItem.GetLength() != 0 && !HasItem(p_sItem))
		{
			if((int)m_stringItems.size()>=m_maxSize)
			{
				m_stringItems.pop_back();
			}
			m_stringItems.push_front(p_sItem);
		}

		return TRUE;
	}

	INT GetItemCount()
	{
		return (INT)m_stringItems.size();
	}

	std::list<CString>& GetList()
	{
		return m_stringItems;
	}

	int GetMaxSize()
	{
		return m_maxSize;
	}

	BOOL RemoveItem(const CString& p_sItem)
	{
		if (p_sItem.GetLength() != 0)
		{
			std::list<CString>::iterator it = std::find(
				m_stringItems.begin(), m_stringItems.end(), p_sItem);
			if(it!=m_stringItems.end())
			{
				m_stringItems.erase(it);
			}
		}
		return TRUE;
	}
	
	BOOL Disable()
	{
		if ((!m_pac) || (!m_fBound))
			return FALSE;

		return SUCCEEDED(EnDisable(FALSE));

	}

	BOOL Enable(VOID)
	{

		if ((!m_pac) || (m_fBound))
			return FALSE;

		return SUCCEEDED(EnDisable(TRUE));

	}

public:

	//
	//	IUnknown implementation
	//
	STDMETHODIMP_(ULONG) AddRef()
	{
		return ::InterlockedIncrement(reinterpret_cast<LONG*>(&m_nRefCount));
	}

	STDMETHODIMP_(ULONG) Release()
	{
		LONG nCount = 0;
		nCount = ::InterlockedDecrement(reinterpret_cast<LONG*>(&m_nRefCount));

		if (nCount == 0)
			delete this;

		return nCount;

	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
	{

		HRESULT hr = E_NOINTERFACE;
		
		if (ppvObject != NULL)
		{
			*ppvObject = NULL;

			if (IID_IUnknown == riid)
				*ppvObject = static_cast<IUnknown*>(this);

			if (IID_IEnumString == riid)
				*ppvObject = static_cast<IEnumString*>(this);

			if (*ppvObject != NULL)
			{
				hr = S_OK;
				((LPUNKNOWN)*ppvObject)->AddRef();
			}

		}
		else
		{
			hr = E_POINTER;
		}
		
		return hr;

	}

public:

	//
	//	IEnumString implementation
	//
	STDMETHODIMP Next(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched)
	{

		HRESULT hr = S_FALSE;

		if (!celt)
			celt = 1;
		if (pceltFetched)
			*pceltFetched = 0;

		ULONG i;
		for (i = 0; i < celt; ++i)
		{
			USES_CONVERSION;

			if (m_nCurrentElement == (ULONG)m_stringItems.size())
				break;

			std::list<CString>::iterator it = m_stringItems.begin();
			std::advance(it, m_nCurrentElement);
			int size = (ULONG) sizeof(WCHAR) * (it->GetLength() + 1);
			rgelt[i] = (LPWSTR)::CoTaskMemAlloc(size);
			StringCbCopyW(rgelt[i], size, T2OLE((LPTSTR)(LPCTSTR)(*it)));

			if (pceltFetched)
				*pceltFetched++;
			
			m_nCurrentElement++;
		}

		if (i == celt)
			hr = S_OK;

		return hr;
	}
 
	STDMETHODIMP Skip(ULONG celt)
	{
		
		m_nCurrentElement += celt;
		
		if (m_nCurrentElement >= (ULONG)m_stringItems.size())
			m_nCurrentElement = 0;

		return S_OK;
	}
 
	STDMETHODIMP Reset(void)
	{
		
		m_nCurrentElement = 0;
		return S_OK;
	}
 
	STDMETHODIMP Clone(IEnumString** ppenum)
	{

		if (!ppenum)
			return E_POINTER;
		
		CCustomAutoComplete* pnew = new CCustomAutoComplete();

		pnew->AddRef();
		*ppenum = pnew;

		return S_OK;

	}

private:

	// Internal implementation

	void InternalInit()
	{
		m_maxSize = 20;
		m_nCurrentElement = 0;
		m_nRefCount = 0;
		m_fBound = FALSE;
	}

	HRESULT EnDisable(BOOL p_fEnable)
	{

		HRESULT hr = S_OK;

		ATLASSERT(m_pac);

		hr = m_pac->Enable(p_fEnable);

		if (SUCCEEDED(hr))
			m_fBound = p_fEnable;

		return hr;


	}

	bool LoadFromStorage()
	{
		m_stringItems.clear();

		if(m_iniFileName.IsEmpty() || m_sectionName.IsEmpty())
			return false;

		CIniFile iniFile;
		iniFile.SetFilename(m_iniFileName);

		std::vector<CString> keys;
		iniFile.EnumSections(keys, m_sectionName);

		for(std::vector<CString>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			if((int)m_stringItems.size()>=m_maxSize)
				break;

			CString value;
			iniFile.GetString(m_sectionName, *it, value);
			m_stringItems.push_back(value);
		}

		return true;
	}

	BOOL SaveToStorage()
	{
		if(m_iniFileName.IsEmpty() || m_sectionName.IsEmpty())
			return FALSE;

		CIniFile iniFile;
		iniFile.SetFilename(m_iniFileName);

		int i=0;
		for(std::list<CString>::iterator it = m_stringItems.begin(); it != m_stringItems.end(); ++it)
		{
			CString key;
			key.Format(_T("S%d"), i);
			iniFile.PutString(m_sectionName, key, *it);
			i++;
		}
		iniFile.Flush();

		return TRUE;
	}

	BOOL ClearStorage()
	{
		CIniFile iniFile;
		iniFile.SetFilename(m_iniFileName);

		iniFile.ClearSection(m_sectionName);

		return TRUE;
	}


private:
	CString m_iniFileName;
	CString m_sectionName;

	std::list<CString> m_stringItems;
	CComPtr<IAutoComplete> m_pac;

	ULONG m_nCurrentElement;
	ULONG m_nRefCount;
	BOOL m_fBound;

	int m_maxSize;
};

