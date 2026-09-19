#if !defined(__NGMTSYNC_H__)
#define __NGMTSYNC_H__

/////////////////////////////////////////////////////////////////////////////
// Basic synchronization object

class CNGSyncObject
{
// Constructor
public:
	CNGSyncObject(LPCTSTR pstrName)
	{
		m_hObject = NULL;
	}


// Attributes
public:
	operator HANDLE() {return m_hObject;}
	operator HANDLE*() {return &m_hObject;}

	HANDLE  m_hObject;

// Operations
	virtual BOOL Lock(DWORD dwTimeout = INFINITE)
	{
		if (::WaitForSingleObject(m_hObject, dwTimeout) == WAIT_OBJECT_0)
			return TRUE;
		else
			return FALSE;
	}

	virtual BOOL Unlock() = 0;
	virtual BOOL Unlock(LONG /* lCount */, LPLONG /* lpPrevCount=NULL */)
		{ return TRUE; }

// Implementation
public:
	virtual ~CNGSyncObject()
	{
		if (m_hObject != NULL)
		{
			::CloseHandle(m_hObject);
			m_hObject = NULL;
		}
	}
};


/////////////////////////////////////////////////////////////////////////////
// CNGSemaphore
class CNGSemaphore : public CNGSyncObject
{

// Constructor
public:
	CNGSemaphore(LONG lInitialCount = 1, LONG lMaxCount = 1,
		LPCTSTR pstrName=NULL, LPSECURITY_ATTRIBUTES lpsaAttributes = NULL)
		:  CNGSyncObject(pstrName)
	{
		_ASSERT(lMaxCount > 0);
		_ASSERT(lInitialCount <= lMaxCount);

		m_hObject = ::CreateSemaphore(lpsaAttributes, lInitialCount, lMaxCount,
			pstrName);
		if (m_hObject == NULL)
			throw E_FAIL;
	}

// Implementation
public:
	virtual ~CNGSemaphore() {};
	virtual BOOL Unlock()
	{
		return ::ReleaseSemaphore(m_hObject, 1, NULL);
	}

	virtual BOOL Unlock(LONG lCount, LPLONG lpPrevCount = NULL)
	{
		return ::ReleaseSemaphore(m_hObject, lCount, lpPrevCount);
	}

};

/////////////////////////////////////////////////////////////////////////////
// CNGMutex

class CNGMutex : public CNGSyncObject
{
// Constructor
public:
	CNGMutex(BOOL bInitiallyOwn = FALSE, LPCTSTR lpszName = NULL,
		LPSECURITY_ATTRIBUTES lpsaAttribute = NULL)
		: CNGSyncObject(lpszName)
	{
		m_hObject = ::CreateMutex(lpsaAttribute, bInitiallyOwn, lpszName);
		if (m_hObject == NULL)
			throw E_FAIL;
	}


// Implementation
public:
	virtual ~CNGMutex() {}; 
	BOOL Unlock()
	{
		return ::ReleaseMutex(m_hObject);
	}

};

/////////////////////////////////////////////////////////////////////////////
// CNGEvent

class CNGEvent : public CNGSyncObject
{
// Constructor
public:
	CNGEvent(BOOL bInitiallyOwn = FALSE, BOOL bManualReset = FALSE,
		LPCTSTR lpszName = NULL, LPSECURITY_ATTRIBUTES lpsaAttribute = NULL)
		: CNGSyncObject(lpszName)
	{
		m_hObject = ::CreateEvent(lpsaAttribute, bManualReset,
			bInitiallyOwn, lpszName);
		if (m_hObject == NULL)
			throw E_FAIL;
	}


// Operations
public:
	BOOL SetEvent()
	{
		_ASSERT(m_hObject);
		return ::SetEvent(m_hObject);
	}

	BOOL ResetEvent()
	{
		return ::ResetEvent(m_hObject);
	};
	BOOL Unlock()
	{
		return TRUE;
	}

// Implementation
public:
	virtual ~CNGEvent() {};
};

/////////////////////////////////////////////////////////////////////////////
// CNGCriticalSection

class CNGCriticalSection : public CNGSyncObject
{
// Constructor
public:
	CNGCriticalSection(LPCTSTR lpszName = NULL)
		: CNGSyncObject(lpszName)
	{
		::InitializeCriticalSection(&m_sect);
	}

// Attributes
public:
	operator CRITICAL_SECTION*();
	CRITICAL_SECTION m_sect;

// Operations
public:
	BOOL Unlock()
	{
		::LeaveCriticalSection(&m_sect);
		return TRUE;
	}
	
	BOOL Lock()
	{
		::EnterCriticalSection(&m_sect);
		return TRUE;
	}

#ifdef _WIN32_WINNT
	BOOL Lock(DWORD dwTimeout)
	{
		if(TryEnterCriticalSection(&m_sect))
			return TRUE;

		::Sleep(dwTimeout);

		if(TryEnterCriticalSection(&m_sect))
			return TRUE;
		else
			return FALSE;
	}
#endif

// Implementation
public:
	virtual ~CNGCriticalSection()
	{
		::DeleteCriticalSection(&m_sect);
	}

};

/////////////////////////////////////////////////////////////////////////////
// CNGSingleLock

class CNGSingleLock
{
// Constructors
public:
	CNGSingleLock(CNGSyncObject* pObject, BOOL bInitialLock = FALSE)
	{
		_ASSERT(pObject != NULL);

		m_pObject = pObject;
		m_hObject = pObject->m_hObject;
		m_bAcquired = FALSE;

		if (bInitialLock)
			Lock();
	}

// Operations
public:
	BOOL Lock(DWORD dwTimeOut = INFINITE)
	{
		_ASSERT(m_pObject != NULL || m_hObject != NULL);
		_ASSERT(!m_bAcquired);

		m_bAcquired = m_pObject->Lock(dwTimeOut);
		return m_bAcquired;
	}

	BOOL Unlock()
	{
		_ASSERT(m_pObject != NULL);
		if (m_bAcquired)
			m_bAcquired = !m_pObject->Unlock();

		// successfully unlocking means it isn't acquired
		return !m_bAcquired;
	}

	BOOL Unlock(LONG lCount, LPLONG lPrevCount = NULL)
	{
		_ASSERT(m_pObject != NULL);
		if (m_bAcquired)
			m_bAcquired = !m_pObject->Unlock(lCount, lPrevCount);

		// successfully unlocking means it isn't acquired
		return !m_bAcquired;
	}


// Implementation
public:
	~CNGSingleLock() {};

protected:
	CNGSyncObject* m_pObject;
	HANDLE  m_hObject;
	BOOL    m_bAcquired;
};

#endif // !defined(__NGMTSYNC_H__)
