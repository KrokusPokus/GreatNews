#pragma once

#include "Request.h"

class CShutdownRequest :
	public CRequest
{
public:

	CShutdownRequest(void)
	{
	}

	virtual ~CShutdownRequest(void)
	{
	}

public:
	virtual HRESULT HandleRequest()
	{
		CRequest::HandleRequest();

		return E_ABORT;
	}
};
