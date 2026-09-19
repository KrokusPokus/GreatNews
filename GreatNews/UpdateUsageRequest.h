#pragma once

#include "Request.h"
#include "ContentGenerator.h"

class CUpdateUsageRequest :
	public CRequest
{
public:
	CUpdateUsageRequest(ContentGeneratorPtr gen);
	~CUpdateUsageRequest(void);

	virtual HRESULT HandleRequest();

protected:
	ContentGeneratorPtr m_gen;
};
