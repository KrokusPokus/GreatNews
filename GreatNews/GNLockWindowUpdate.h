#pragma once

class CGNLockWindowUpdate
{
public:
	CGNLockWindowUpdate(HWND hWnd) : m_bReleased(false)
	{
		ATLASSERT(::IsWindow(hWnd));
		::LockWindowUpdate(hWnd);
	}

	~CGNLockWindowUpdate(void)
	{
		Release();
	}

	void Release()
	{
		if(!m_bReleased)
		{
			::LockWindowUpdate(NULL);
			m_bReleased = true;
		}
	}

private:
	bool m_bReleased;
};
