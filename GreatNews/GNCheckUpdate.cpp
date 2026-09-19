#include "StdAfx.h"
#include ".\gncheckupdate.h"

#include "version.h"
#include "IniFile.h"

CGNCheckUpdate::CGNCheckUpdate(void)
{
}

CGNCheckUpdate::~CGNCheckUpdate(void)
{
}


VersionCheckStatus CGNCheckUpdate::IsUpdateAvailable(CString& newVersion)
{
	TCHAR downloadFile[MAX_PATH];
	HRESULT hr = ::URLDownloadToCacheFile(NULL, 
											VERSION_URL,
											downloadFile,
											MAX_PATH,
											0,
											NULL);

	if(hr != S_OK)
		return Failed;

	CIniFile iniFile;
	iniFile.SetFilename(downloadFile);

	CString ver;
	if(!iniFile.GetString(_T("Version"), _T("Latest"),ver,_T("")))
		return Failed;

	if(ver > PRODUCT_VERSION_STR)
	{
		newVersion = ver;
		return NewVersionAvailable;
	}
	else
	{
		return NoNewVersion;
	}

}
