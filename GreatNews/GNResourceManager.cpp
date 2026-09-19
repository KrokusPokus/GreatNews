#include "StdAfx.h"
#include "GNResourceManager.h"
#include "accel.h"
#include "resource.h"

CGNResourceManager::CGNResourceManager(void)
{
}

CGNResourceManager::~CGNResourceManager(void)
{
}

bool CGNResourceManager::Init(LPCTSTR szLanguagePackPath)
{
	m_languagePackPath = szLanguagePackPath;

	return true;
}

bool CGNResourceManager::SetLanguage(int languagePackIndex)
{
	std::vector<CString> files;
	int n = GetLanguagePackFiles(files);
	if(languagePackIndex >= 0 && languagePackIndex < n)
		return SetLanguage(files[languagePackIndex]);
	else
		return false;
}

bool CGNResourceManager::SetLanguage(LPCTSTR szLanguagePackFileName)
{
	CString fullName = m_languagePackPath + "\\" + szLanguagePackFileName;
	if(!PathFileExists(fullName))
	{
		return false;
	}
	else
	{
		m_languagePackFileName = szLanguagePackFileName;
		m_languagePackFileName.Trim();
		m_languagePack.SetFilename(fullName);
		return true;
	}
}

CString CGNResourceManager::GetCurrentLanguagePackFileName()
{
	return m_languagePackFileName;
}

int CGNResourceManager::GetLanguagePackFiles(std::vector<CString>& files)
{
	files.clear();

	CFindFile finder;
	BOOL bFound = finder.FindFile(m_languagePackPath + "\\" + _T("gn_*.ini"));
	while(bFound)
	{
		files.push_back(finder.GetFileName());
		bFound = finder.FindNextFile();
	}

	return (int)files.size();
}

void CGNResourceManager::ApplyLanguage(HMENU hMainMenu, int nViewMenu, int firstLanguageID)
{
	CMenuHandle mainMenu;
	mainMenu.Attach(hMainMenu);

	if(nViewMenu>=0)
	{
		CMenu languageMenu;
		languageMenu.CreatePopupMenu();

		CFindFile finder;
		BOOL bFound = finder.FindFile(m_languagePackPath + "\\" + _T("gn_*.ini"));
		int i=0;
		while(bFound)
		{
			CIniFile languagePack;
			languagePack.SetFilename(finder.GetFilePath());
			CString languageName;
			languagePack.GetString(_T("Info"), _T("Language"), languageName);
			languageMenu.AppendMenu(MF_STRING | MF_BYPOSITION, firstLanguageID + i, languageName );

			if(finder.GetFileName().CompareNoCase(m_languagePackFileName) == 0)
			{
				languageMenu.CheckMenuItem(firstLanguageID + i, MF_CHECKED);
			}

			i++;
			bFound = finder.FindNextFile();
		}

		CMenuHandle viewMenu = mainMenu.GetSubMenu(nViewMenu);
		viewMenu.AppendMenu( MF_POPUP, (UINT_PTR)languageMenu.Detach(), _T("Language")); 
	}

	
	//
	// start modify menu string
	//
	ApplyLanguageToMenu(hMainMenu);
}

void CGNResourceManager::ApplyLanguageToMenu(HMENU hMenu)
{
	if(!CheckLanguagePack())
		return;  // no language pack specified

	std::map<UINT, CString> mapId2AccelText;
	common::GetAcceleratorTexts(ATL::_AtlBaseModule.GetResourceInstance(), 
								IDR_MAINFRAME,
								mapId2AccelText);

	CMenuHandle menu;
	menu.Attach(hMenu);

	int nMenuItemCount = menu.GetMenuItemCount();
	for(int i = 0; i < nMenuItemCount; ++i)
	{
		CMenuItemInfo  mii;

		mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_SUBMENU;
		BOOL bRet = menu.GetMenuItemInfo(i, TRUE, &mii);
		ATLASSERT(bRet);

		if((mii.fType & MFT_SEPARATOR) !=0)
			continue; // skip separator

		CString menuKey;
		CString menuText;
		if(mii.hSubMenu && ::IsMenu(mii.hSubMenu))
		{
			// recursively apply language to sub menus
			ApplyLanguageToMenu(mii.hSubMenu);

			// we use the id of first time in popup menu as key
			CMenuHandle popupMenu;
			popupMenu.Attach(mii.hSubMenu);
			BOOL bRet = popupMenu.GetMenuItemInfo(0, TRUE, &mii);
			menuKey.Format(_T("Title%d"),mii.wID);
			m_languagePack.GetString(_T("Menu"), menuKey, menuText);
		}
		else
		{
			menuKey.Format(_T("Item%d"),mii.wID);
			m_languagePack.GetString(_T("Menu"), menuKey, menuText);
			if(menuText.GetLength()>0)
			{
				// apply accelerator
				std::map<UINT, CString>::iterator it = mapId2AccelText.find(mii.wID);
				if(it != mapId2AccelText.end())
				{
					menuText += "\t";
					menuText += it->second;
				}
			}
		}

		if(menuText.GetLength()>0)
		{
			mii.dwTypeData = (LPTSTR)(LPCTSTR)menuText;
			mii.cch = menuText.GetLength();
			mii.fMask = MIIM_STRING;
			menu.SetMenuItemInfo(i, TRUE, &mii);
		}
	}
}
bool CGNResourceManager::CheckLanguagePack()
{
	if(m_languagePackFileName.GetLength()==0)
		return false;
	
	return true;
}


BOOL CALLBACK getAllChildren(HWND hwnd, LPARAM lParam)
{
	ATLASSERT(hwnd);
	ATLASSERT(lParam !=0);

	std::vector<HWND>* pChildren = (std::vector<HWND>*)lParam;
	pChildren->push_back(hwnd);

	return true;
}

void CGNResourceManager::ApplyLanguageToWindow(HWND hWnd, LPCTSTR languageSection)
{
	ATLASSERT(::IsWindow(hWnd));

	if(!CheckLanguagePack())
		return;  // no language pack specified

	// apply title
	CString title;
	m_languagePack.GetString(languageSection, _T("Title"), title);
	if(title.GetLength())
		::SetWindowText(hWnd, title);

	// change child controls text
	std::vector<HWND> children;
	EnumChildWindows(hWnd, getAllChildren, (LPARAM)&children);

	for(std::vector<HWND>::iterator it = children.begin(); it != children.end(); ++it)
	{
		HWND hwndChild = *it;
		int wID = ::GetWindowLong(hwndChild, GWL_ID);
		CString key;
		key.Format(_T("C%d"), wID);
		CString text;
		m_languagePack.GetString(languageSection, key, text);
		if(text.GetLength())
		{
			::SetWindowText(hwndChild, text);
		}
	}
}

void CGNResourceManager::ApplyLanguageToRebar(HWND hwndRebar)
{
	ATLASSERT(::IsWindow(hwndRebar));

	if(!CheckLanguagePack())
		return;  // no language pack specified


	CReBarCtrl rebar;
	rebar.Attach(hwndRebar);

	for(UINT i = 0; i < rebar.GetBandCount(); ++i)
	{
		// modify band title text
		REBARBANDINFO rbBand = { 0 };
		rbBand.cbSize = sizeof(REBARBANDINFO);
		rbBand.fMask = RBBIM_ID|RBBIM_CHILD;

		rebar.GetBandInfo(i,&rbBand);
		CToolBarCtrl toolbar;
		toolbar.Attach(rbBand.hwndChild);  // we assume only toolbar controls are put in the rebar

		if(rbBand.wID > 0)
		{
			CString key;
			key.Format(_T("T%d"), rbBand.wID);
			CString text;
			m_languagePack.GetString(_T("Toolbars"), key, text);
			if(text.GetLength())
			{
				rbBand.lpText = (LPTSTR)(LPCTSTR)text;
				rbBand.cch = text.GetLength();
				rbBand.fMask = RBBIM_TEXT;
				rebar.SetBandInfo(i, &rbBand);
			}
		}

		// modify band buttons' text
		for(int y = 0; y<toolbar.GetButtonCount(); ++y)
		{
			TBBUTTON ttb = {0};
			toolbar.GetButton(y, &ttb);
			if(ttb.idCommand == 0 || ttb.iString == 0)
				continue;
		
			TBBUTTONINFO tbbi = {0};
			tbbi.cbSize = sizeof(TBBUTTONINFO);

			CString key;
			key.Format(_T("B%d"), ttb.idCommand);
			CString text;
			m_languagePack.GetString(_T("Toolbars"), key, text);
			if(text.GetLength())
			{
				tbbi.pszText = (LPTSTR)(LPCTSTR)text;
				tbbi.cchText = text.GetLength();
				tbbi.dwMask = TBIF_TEXT;
				toolbar.SetButtonInfo(ttb.idCommand, &tbbi);
			}
		}

		// resize the band
		toolbar.AutoSize();
		RECT rcTmp = {0};
		BOOL bRet = toolbar.GetItemRect(toolbar.GetButtonCount() - 1, &rcTmp);
		ATLASSERT(bRet);

		rbBand.cx = rcTmp.right;
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = rcTmp.bottom - rcTmp.top;

		rbBand.fMask = RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_IDEALSIZE;

		if(rbBand.wID > 150) // hack, fix the address bar
		{
			rebar.SetBandInfo(i, &rbBand);
		}
		//::SendMessage(hwndRebar, RB_MAXIMIZEBAND, i, 1L);
	}
}

void CGNResourceManager::ApplyLanguageToCombo(HWND combobox, LPCTSTR section)
{
	if(!CheckLanguagePack())
		return;  // no language pack specified

	CComboBox combo;
	combo.Attach(combobox);

	int curSel = combo.GetCurSel();
	int count = combo.GetCount();
	combo.ResetContent();

	for(int i=0;i<count;++i)
	{
		CString key, text;
		key.Format(_T("%d"), i);

		m_languagePack.GetString(section, key, text);
		combo.InsertString(-1, text);
	}

	if(curSel>=0 && curSel<combo.GetCount())
		combo.SetCurSel(curSel);
}


CString CGNResourceManager::GetString(UINT id, LPCTSTR section)
{
	CString text;
	if(CheckLanguagePack())
	{
		CString key;
		key.Format(_T("M%d"), id);

		CComCritSecLock<CComAutoCriticalSection> lock(m_csGetString); // we have to lock it because this function may be called in multiple threads
		BOOL b = m_languagePack.GetString((section ? section : _T("Messages")), key, text);
		lock.Unlock();

		if(b)
		{
			text.Replace(_T("\\t"), _T("    "));
			text.Replace(_T("\\n"), _T("\n"));
			return text;
		}

	}

	// not found in language pack, try to load from resource
	text.LoadString(id);

	return text;
}
