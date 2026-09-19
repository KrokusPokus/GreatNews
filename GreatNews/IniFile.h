#pragma once

/////////////////////////////////////////////////////////////////////////////
// CIniFile
#include <vector>
#include <strsafe.h>

class CIniFile
{
public:
	TCHAR m_szFilename[MAX_PATH];

	CIniFile()
	{
		m_szFilename[0] = '\0';
	}
	void SetFilename(LPCTSTR pstrFilename)
	{
		ATLASSERT(!::IsBadStringPtr(pstrFilename,-1));
		::StringCchCopy(m_szFilename, sizeof(m_szFilename)/sizeof(TCHAR), pstrFilename);
	}
	BOOL GetString(LPCTSTR pstrSection, LPCTSTR pstrKey, LPTSTR pstrValue, UINT cchMax, LPCTSTR pstrDefault = NULL) const
	{
		ATLASSERT(m_szFilename[0]);
		ATLASSERT(!::IsBadStringPtr(pstrSection,-1));
		ATLASSERT(!::IsBadStringPtr(pstrKey,-1));
		ATLASSERT(!::IsBadWritePtr(pstrValue,cchMax));
		return ::GetPrivateProfileString(pstrSection, pstrKey, pstrDefault, pstrValue, cchMax,  m_szFilename) > 0;
	}
#if defined(__ATLSTR_H__) || defined(_WTL_USE_CSTRING)
	BOOL GetString(LPCTSTR pstrSection, LPCTSTR pstrKey, CString& sValue, LPCTSTR pstrDefault = NULL) const
	{      
		enum { MAX_INIVALUE_LEN = 1024 };
		BOOL bRes = GetString(pstrSection, pstrKey, sValue.GetBuffer(MAX_INIVALUE_LEN), MAX_INIVALUE_LEN, pstrDefault);
		sValue.ReleaseBuffer(bRes ? -1 : 0);
		return bRes;
	}
#endif // __ATLSTR_H__ _WTL_USE_CSTRING
	BOOL GetInt(LPCTSTR pstrSection, LPCTSTR pstrKey, int& iValue, int iDefault = 0) const
	{
		ATLASSERT(m_szFilename[0]);
		ATLASSERT(!::IsBadStringPtr(pstrSection,-1));
		ATLASSERT(!::IsBadStringPtr(pstrKey,-1));
		iValue = ::GetPrivateProfileInt(pstrSection, pstrKey, iDefault, m_szFilename);
		return TRUE;
	}
	BOOL GetBool(LPCTSTR pstrSection, LPCTSTR pstrKey, bool& bValue, bool bDefault = true) const
	{
		TCHAR szValue[2] = { 0 };
		GetString(pstrSection, pstrKey, szValue, sizeof(szValue)/sizeof(TCHAR), bDefault ? _T("Y") : _T("N"));
		switch( szValue[0] ) {
case 'y': // Yes
case 'Y':
case 't': // True
case 'T':
case '1': // 1
	bValue = true;
	break;
case 'n': // No
case 'N':
case 'f': // False
case 'F':
case '0': // 0
	bValue = false;
	break;
default:
	bValue = bDefault;
		}
		return TRUE;
	}
	BOOL PutString(LPCTSTR pstrSection, LPCTSTR pstrKey, LPCTSTR pstrValue)
	{
		ATLASSERT(m_szFilename[0]);
		ATLASSERT(!::IsBadStringPtr(pstrSection,-1));
		ATLASSERT(!::IsBadStringPtr(pstrKey,-1));
		return ::WritePrivateProfileString(pstrSection, pstrKey, pstrValue, m_szFilename);
	}
	BOOL PutInt(LPCTSTR pstrSection, LPCTSTR pstrKey, int iValue)
	{
		TCHAR szValue[32];
		::StringCchPrintf(szValue, sizeof(szValue)/sizeof(TCHAR),  _T("%d"), iValue);
		return PutString(pstrSection, pstrKey, szValue);
	}
	BOOL PutBool(LPCTSTR pstrSection, LPCTSTR pstrKey, bool bValue)
	{
		TCHAR szValue[2] = { 0 };
		szValue[0] = bValue ? 'Y' : 'N'; // TODO: What about localization? Use 0/1?
		return PutString(pstrSection, pstrKey, szValue);
	}

	void DeleteKey(LPCTSTR pstrSection, LPCTSTR pstrKey)
	{
		ATLASSERT(m_szFilename[0]);
		ATLASSERT(!::IsBadStringPtr(pstrSection,-1));
		ATLASSERT(!::IsBadStringPtr(pstrKey,-1));
		::WritePrivateProfileString(pstrSection, pstrKey, NULL, m_szFilename);
	}
	void DeleteSection(LPCTSTR pstrSection)
	{
		ATLASSERT(m_szFilename[0]);
		ATLASSERT(!::IsBadStringPtr(pstrSection,-1));
		::WritePrivateProfileString(pstrSection, NULL, NULL, m_szFilename);
	}

	void Flush()
	{
		ATLASSERT(m_szFilename[0]);
		::WritePrivateProfileString(NULL, NULL, NULL, m_szFilename);
	}

	void ClearSection(LPCTSTR pstrSection)
	{
		std::vector<CString> keys;
		EnumSections(keys, pstrSection);
		for(std::vector<CString>::iterator it = keys.begin(); it != keys.end(); ++it)
		{
			DeleteKey(pstrSection, *it);
		}
	}

	BOOL EnumSections(std::vector<CString>& sections, LPCTSTR pstrSection)
	{
		sections.clear();
		ATLASSERT(m_szFilename[0]);
		TCHAR buffer[2048];
		::GetPrivateProfileString(pstrSection, NULL, NULL, buffer, sizeof(buffer)/sizeof(TCHAR),  m_szFilename);

		TCHAR* p = buffer;
		while(*p)
		{
			sections.push_back(p);
			p+=_tcslen(p)+1;
		}

		return TRUE;
	}
};
