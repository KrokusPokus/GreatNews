#if !defined(__SINGLETON_H__)
#define __SINGLETON_H__

#include "NGMTSync.h"
#include "Guard.h"

using namespace std;


template < class SINGLETON_TYPE, class MUTEX = CNGCriticalSection >
class CGNSingleton
{
public :
	~CGNSingleton<SINGLETON_TYPE,MUTEX>()
	{
		Destroy();
	}

	static void Destroy()
	{
		CGuard<MUTEX> lock(m_oSyncLock);
		if (m_pTypeInstance) 
		{
			delete m_pTypeInstance;
			m_pTypeInstance = NULL;
		}
	}
	
	static SINGLETON_TYPE * Instance()
	{
		// Using double-check pattern to maintain a single instance
		// of m_pTypeInstance
		if ( !m_pTypeInstance ){
			CGuard<MUTEX> lock(m_oSyncLock);

			if (!m_pTypeInstance)
				m_pTypeInstance = new SINGLETON_TYPE ;
		}
		return m_pTypeInstance ;
	}

	static bool Exists()
	{
		return (m_pTypeInstance!=NULL);
	}


private :
	static SINGLETON_TYPE * m_pTypeInstance ;
	static MUTEX m_oSyncLock;
};


template <class SINGLETON_TYPE, class MUTEX>
SINGLETON_TYPE * CGNSingleton<SINGLETON_TYPE, MUTEX> :: m_pTypeInstance ;

template <class SINGLETON_TYPE, class MUTEX>
MUTEX CGNSingleton<SINGLETON_TYPE, MUTEX> :: m_oSyncLock ;


/////////////////////////////////////////////////////////////////////////
#endif // !defined(__SINGLETON_H__)
