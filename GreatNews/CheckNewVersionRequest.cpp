#include "StdAfx.h"
#include ".\checknewversionrequest.h"
#include "resource.h"

CCheckNewVersionRequest::CCheckNewVersionRequest(HWND hwnd) : m_wndToNotify(hwnd)
{
}

CCheckNewVersionRequest::~CCheckNewVersionRequest(void)
{
}

HRESULT CCheckNewVersionRequest::HandleRequest()
{
	CString newVersion;
	VersionCheckStatus status;
	status = CGNCheckUpdate::IsUpdateAvailable(newVersion);
	if(status == NewVersionAvailable)
	{
		::PostMessage(m_wndToNotify, WM_COMMAND, ID_HELP_CHECKNEWVERSION,0);
	}

	return CRequest::HandleRequest();
}
