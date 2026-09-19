#include "StdAfx.h"
#include "UpdateUsageRequest.h"

CUpdateUsageRequest::CUpdateUsageRequest(ContentGeneratorPtr gen)
{
	m_gen = gen;
	m_bOwnedByPool = true;
}

CUpdateUsageRequest::~CUpdateUsageRequest(void)
{
}

HRESULT CUpdateUsageRequest::HandleRequest()
{
	try
	{
		if(m_gen)
			m_gen->UpdateUsage();
	}
	catch(...)
	{
		ATLTRACE(_T("**** Failed to update usage...\n"));
	}

	return CRequest::HandleRequest();
}