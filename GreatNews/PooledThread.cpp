#include "stdafx.h"
#include "PooledThread.h"
#include "Request.h"

////////////////////////////////////////////////////////////////////////////////
CPooledThread::CPooledThread(CRequestQueue* pRQ)
{
	m_pRequestQueue = pRQ;
}

CPooledThread::~CPooledThread()
{
}

DWORD CPooledThread::Run()
{
#ifdef MULTITHREADED
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
	HRESULT hRes = ::CoInitialize(NULL);
#endif
	ATLASSERT(SUCCEEDED(hRes));

	try
	{
		while ( TRUE ){
			// get a request, will block here if nothing in Q
			CRequest * pRequest = m_pRequestQueue->Dequeue();
			if(!pRequest)
			{
				break;
			}

			// start execute the request
			HRESULT hr = pRequest->HandleRequest();
				
			// if request returns E_ABORT, system is shuting down. so stop this thread.
			if(E_ABORT == hr)
			{
				break;
			}

			// if the quest is owned by thread pool, delete it after we handled it.
			if(pRequest->IsOwnedByPool())
				delete pRequest;
		}
	}
	catch(...)
	{
		AtlTrace(_T("thread proc failed.\n"));
	}

	::CoUninitialize();

	return 0;
}


