#pragma once

#include <process.h>    /* _beginthread, _endthread */

class CGNThread  
{
public:
	CGNThread(bool bSelfDelete=true) 
		: m_hThread(0), m_nThreadID(0), m_bSelfDelete(bSelfDelete), m_bFinished(false)
	{
	}

	virtual ~CGNThread()
	{
		ClearThread();
	}
	
	void ClearThread()
	{
		if(m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
			m_nThreadID = 0;
		}
		m_bFinished = false;
	}

	DWORD Start()
	{
		if(m_bFinished) // if this thread finished last run, let's destroy it
		{
			ClearThread();
		}

		// create thread in suspend mode
		if(!m_hThread)
		{
			m_hThread = (HANDLE)_beginthreadex( NULL, 0, ThreadProc, (LPVOID)this, CREATE_SUSPENDED,&m_nThreadID);
			_ASSERT(m_hThread);
		}
		
		return ResumeThread(m_hThread);
	}

	DWORD Pause()
	{
		_ASSERT(m_hThread);
		return SuspendThread(m_hThread);
	}

	BOOL Stop()
	{
		// Dangerous!!!! Should avoid using this function!!!
		_ASSERT(m_hThread);
		
		return TerminateThread(m_hThread,-1);
	}

	UINT GetThreadID() {return m_nThreadID;}
	HANDLE GetThreadHandle() {return m_hThread;}
	void SetFinishedFlag() {m_bFinished = true;}

	virtual DWORD Run() = 0;

protected:
	static unsigned __stdcall ThreadProc(LPVOID lpvThreadParam)
	{
		CGNThread* thisThread = (CGNThread*)lpvThreadParam;
		bool bDelete = thisThread->m_bSelfDelete;
		DWORD dwRet = thisThread->Run();
		thisThread->SetFinishedFlag();

		// kill thread object when thread finished;
		if(bDelete)
			delete thisThread;


		return dwRet;
	}

protected:
	bool m_bSelfDelete;
	bool m_bFinished;
	HANDLE m_hThread;
	UINT  m_nThreadID;
	
};

