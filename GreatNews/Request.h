// Request.h: interface for the CRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__REQUEST_H__)
#define __REQUEST_H__

#include "NGMTSync.h"

///////////////////////////////////////////////////////
class CRequest  
{
public:
	CRequest();
	virtual ~CRequest();

public:
	virtual HRESULT HandleRequest();

	void static Shutdown();
	void Submit(bool bHighPriority=false);

	BOOL WaitUntilFinished(long nTimeOut = INFINITE);
	BOOL SetFinished();

	void SetOwnedByPool(bool bOwn = true);
	bool IsOwnedByPool();

protected:
	CNGEvent	m_eventFinished; 
	bool		m_bOwnedByPool;
};

#endif // !defined(__REQUEST_H__)
