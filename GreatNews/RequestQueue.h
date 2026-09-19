#if !defined(__REQUESTQUEUE_H__)
#define __REQUESTQUEUE_H__

#include <list>
#include "NGMTSync.h"

//////////////////////////////////////////////////////////////////////////////////
template < class REQUEST_T, class LOCK_T >
class CRequestQueueT
{
protected:
	std::list<ULONG_PTR> m_Q;

    LOCK_T m_oInsertRemoveLock;
    CNGSemaphore* m_psemCanRemove; // Blocks if queue is empty
    // CNGSemaphore* m_psemCanInsert; // Blocks if queue is full

    unsigned long m_nMaxQueueLimit;

public :
    // The maximum queue limit here is m_nMaxQueueLimit by default
    CRequestQueueT( unsigned long maxQ)
    {
        m_nMaxQueueLimit=maxQ;
        m_psemCanRemove = new CNGSemaphore(0, m_nMaxQueueLimit);
        // m_psemCanInsert = new CNGSemaphore(m_nMaxQueueLimit,  m_nMaxQueueLimit);
    }

	virtual ~CRequestQueueT()
	{
		if(m_psemCanRemove) delete m_psemCanRemove;
		// if(m_psemCanInsert) delete m_psemCanInsert;
	}

    int Enqueue( REQUEST_T * t, bool bHighPriority)
    {
        // m_psemCanInsert->Lock();
        m_oInsertRemoveLock.Lock();
		if(bHighPriority)
			m_Q.push_front((ULONG_PTR)t);
		else
			m_Q.push_back((ULONG_PTR)t);
        
        m_psemCanRemove->Unlock(); // Notify consumers that the queue has an object
        m_oInsertRemoveLock.Unlock();

        return TRUE ;
    }

    REQUEST_T * Dequeue()
    {
        m_psemCanRemove->Lock();        
        m_oInsertRemoveLock.Lock();

        REQUEST_T * pRequest = (REQUEST_T*)m_Q.front();
        m_Q.pop_front();
        
        //if ( m_Q.size() < m_nMaxQueueLimit )
        //    m_psemCanInsert->Unlock();

        m_oInsertRemoveLock.Unlock();
        return pRequest;
    }
};

typedef CRequestQueueT<CRequest, CNGCriticalSection> CRequestQueue;

#endif // !defined(__REQUESTQUEUE_H__)
