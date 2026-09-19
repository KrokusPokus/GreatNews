#pragma once

#include "IniFile.h"
#include "Singleton.h"

class CGNResourceManager
{
public:
	virtual ~CGNResourceManager(void);

public:
	bool Init(LPCTSTR szLanguagePackPath);
	bool SetLanguage(int languagePackIndex);
	bool SetLanguage(LPCTSTR szLanguagePackFileName);
	CString GetCurrentLanguagePackFileName();
	
	void ApplyLanguage(HMENU hMainMenu, int nViewMenu, int firstLanguageID);
	void ApplyLanguageToMenu(HMENU hMenu);
	void ApplyLanguageToWindow(HWND hWnd, LPCTSTR languageSection);
	void ApplyLanguageToRebar(HWND hwndRebar);
	void ApplyLanguageToCombo(HWND combobox, LPCTSTR section);
	CString GetString(UINT id, LPCTSTR section=NULL);
		
protected:
	int GetLanguagePackFiles(std::vector<CString>& files);
	bool CheckLanguagePack();

	CGNResourceManager(void);
	friend class CGNSingleton<CGNResourceManager>;

private:
	CIniFile m_languagePack;
	CString m_languagePackFileName;
	CString m_languagePackPath;

	CComAutoCriticalSection m_csGetString;
};

#define ResManagerPtr (CGNSingleton<CGNResourceManager>::Instance()) 
