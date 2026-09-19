#if !defined(__POOLTHREAD_H__)
#define __POOLTHREAD_H__

#include "GNThread.h"
#include "Request.h"
#include "RequestQueue.h"

/////////////////////////////////////////////////////////
class CPooledThread : public CGNThread
{
public:
	CPooledThread(CRequestQueue* pRQ);
	virtual ~CPooledThread();

    virtual DWORD Run();
	
protected:
	CRequestQueue*	m_pRequestQueue;
};

#endif // !defined(__POOLTHREAD_H__)
