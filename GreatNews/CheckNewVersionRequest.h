#pragma once

#include "request.h"
#include "GNCheckUpdate.h"

class CCheckNewVersionRequest :
	public CRequest
{
public:
	CCheckNewVersionRequest(HWND hwnd);
	virtual ~CCheckNewVersionRequest(void);

	virtual HRESULT HandleRequest();

private:
	HWND m_wndToNotify;
};
