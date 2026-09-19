#if !defined(__GUARD_H__)
#define __GUARD_H__

template<class LOCK>
class CGuard
{
public:
	CGuard< LOCK >(LOCK& lock) : m_lock(lock)
	{
		lock.Lock();
	}
	virtual ~CGuard< LOCK >()
	{
		m_lock.Unlock();
	}

protected:
	LOCK& m_lock;
};

#endif // !defined(__GUARD_H__)
