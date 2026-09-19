#if !defined(__MTPool_H__)
#define __MTPool_H__


#include "Request.h"
#include "NGMTSync.h"
#include "PooledThread.h"
#include "RequestQueue.h"
#include "ShutdownRequest.h"

///////////////////////////////////////////////////////////////////////
template<class REQUEST_T, class LOCK_T>
class CMTPoolT
{
protected :
    ULONG_PTR m_nMaxQueueLimit;
	  ULONG_PTR m_nMaxThread;
	  CRequestQueueT<REQUEST_T,CNGCriticalSection> * m_pRequestQ;

public :
    CMTPoolT( ULONG_PTR nNumOfThreads = 3, ULONG_PTR maxQ = 50000)
    {
      m_nMaxQueueLimit = maxQ;
      m_nMaxThread = 0;

      m_pRequestQ = new CRequestQueueT<REQUEST_T,CNGCriticalSection>(maxQ);

		  SetMaxThreads(nNumOfThreads);
    }

    virtual ~CMTPoolT()
	{

		// wait for all threads to finish
		for(ULONG_PTR i = 0; i < m_nMaxThread; ++i)
		{
			CShutdownRequest* pRequest = new CShutdownRequest();
			pRequest->Submit(true);

			// wait for at most 5 sec per thread.
			// if failed, thread will be killed in the brutal way
			if(TRUE == pRequest->WaitUntilFinished(5000))
			{
				// delete pRequest;
			}
			else
			{
				// Leave alone the request object, otherwise we may get GPF.
				// We have memory leak here, but we are shuting down and 
				// that thread already hang anyway
			}
		}

		delete m_pRequestQ;
	}


public:
	HRESULT Submit( REQUEST_T * pRequest, bool bHighPriority)
    {
        if ( !pRequest )
            return E_FAIL;

		m_pRequestQ->Enqueue(pRequest, bHighPriority );

        return TRUE;
    }

	void SetMaxThreads(ULONG_PTR nNumOfThreads)
	{
		if(nNumOfThreads > m_nMaxThread)
		{
			// create new threads
			for(ULONG_PTR i = 0; i < nNumOfThreads-m_nMaxThread; ++i)
			{
				CPooledThread* pThread = new CPooledThread(m_pRequestQ);
				pThread->Start();
			}
		}
		m_nMaxThread = nNumOfThreads;
	}


};

///////////////////////////////////////////////////////////////////

typedef CMTPoolT<CRequest, CNGCriticalSection> CMTPool;

/////////////////////////////////////////////////////////////////////////

#endif // !defined(__MTPool_H__)

