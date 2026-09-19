#include "StdAfx.h"
#include "Request.h"
#include "Singleton.h"
#include "MTPool.h"

CRequest::CRequest()
	: m_eventFinished(FALSE, TRUE),m_bOwnedByPool(false)
{
}

CRequest::~CRequest()
{
}

void CRequest::Shutdown()
{
	CGNSingleton<CMTPool, CNGCriticalSection>::Destroy();
}

void CRequest::Submit(bool bHighPriority)
{
	CGNSingleton<CMTPool, CNGCriticalSection>::Instance()->Submit(this, bHighPriority);
}

void CRequest::SetOwnedByPool(bool bOwn)
{
	m_bOwnedByPool = bOwn;
}

bool CRequest::IsOwnedByPool()
{
	return m_bOwnedByPool;
}

BOOL CRequest::WaitUntilFinished(long nTimeOut)
{
	CNGSingleLock singleLock(&m_eventFinished);
	return singleLock.Lock(nTimeOut);
}

BOOL CRequest::SetFinished()
{
	return m_eventFinished.SetEvent();
}

HRESULT CRequest::HandleRequest()
{
	SetFinished();
	return S_OK;
}
