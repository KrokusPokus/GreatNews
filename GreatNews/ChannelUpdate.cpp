#include "StdAfx.h"
#include "ChannelUpdate.h"
#include "GreatNewsConfig.h"
#include "UpdateDownloadThread.h"
#include "UpdateStoreThread.h"
#include "GNResourceManager.h"
#include "resource.h"

CTimeSpan CChannelUpdate::m_timeoutSpan;

CChannelUpdate::CChannelUpdate(int maxConcurrentUpdate) :
	m_hwndToNotify(NULL),
	m_hDownloadThread(NULL),
	m_hStoreThread(NULL),
	m_downloadThreadID(0),
	m_storeThreadID(0),
	m_hReadyToStore(NULL),
	m_hStoreTimer(NULL),
	m_maxConcurrentUpdate(maxConcurrentUpdate)
{
}

CChannelUpdate::~CChannelUpdate(void)
{
	Stop();
}

void CChannelUpdate::Start(HWND wnd)
{
	m_semHasRequest = new CNGSemaphore(0, 65535);
	m_semCanDownload = new CNGSemaphore(m_maxConcurrentUpdate, m_maxConcurrentUpdate);

	m_hReadyToStore = ::CreateSemaphore(NULL, 0, m_maxConcurrentUpdate, NULL);
	m_hStoreTimer = ::CreateWaitableTimer(NULL, FALSE, _T("GN_WaitableTimer"));
    if (!m_hStoreTimer)
    {
		ATLTRACE(_T("CreateWaitableTimer failed (%d)\n"), GetLastError());
    }

	m_timeoutSpan = CTimeSpan(0,0,0,g_GreatNewsConfig.GetChannelUpdateTimeOut());
	m_hwndToNotify = wnd;

	CUpdateDownloadThread* pDownloadThread = new CUpdateDownloadThread(*this);
	pDownloadThread->Start();
	m_downloadThreadID = pDownloadThread->GetThreadID();
	m_hDownloadThread = pDownloadThread->GetThreadHandle();

	CUpdateStoreThread* pStoreThread = new CUpdateStoreThread(*this);
	pStoreThread->Start();
	m_storeThreadID = pStoreThread->GetThreadID();
	m_hStoreThread = pStoreThread->GetThreadHandle();
}

void CChannelUpdate::FeedAbortUpdate(NewsFeedPtr& feed)
{
	feed->AbortUpdate();	
}

void CChannelUpdate::Stop()
{
	if(m_downloadThreadID)
	{
		::PostThreadMessage(m_storeThreadID, WM_QUIT, 0,0);
		::PostThreadMessage(m_downloadThreadID, WM_QUIT, 0,0);

		::WaitForSingleObject(m_hStoreThread, 5000);
		::WaitForSingleObject(m_hDownloadThread, 5000);

		m_hDownloadThread = NULL;
		m_hStoreThread = NULL;

		m_storeThreadID = 0;
		m_downloadThreadID = 0;

		CComCritSecLock<CComAutoCriticalSection> lockToBeUpdated(m_csToBeUpdated);
		m_toBeUpdated.clear();
		lockToBeUpdated.Unlock();

		CComCritSecLock<CComAutoCriticalSection> lockToBeStored(m_csToBeStored);
		for_each(m_toBeStored.begin(), m_toBeStored.end(), FeedAbortUpdate);
		m_toBeStored.clear();
		lockToBeStored.Unlock();
	}

	if(m_semHasRequest)
	{
		delete m_semHasRequest;
		m_semHasRequest = NULL;
	}
	if(m_semCanDownload)
	{
		delete m_semCanDownload;
		m_semCanDownload = NULL;
	}

	if(m_hReadyToStore)
	{
		::CloseHandle(m_hReadyToStore);
		m_hReadyToStore = NULL;
	}
	if(m_hStoreTimer)
	{
		::CloseHandle(m_hStoreTimer);
		m_hStoreTimer = NULL;
	}
}

void CChannelUpdate::EnqueueUpdate(NewsFeedPtr feed, bool bHighPriority)
{
	ATLTRACE(_T("EnqueueUpdate Thread=[%d]\n"), ::GetCurrentThreadId());

	CComCritSecLock<CComAutoCriticalSection> lockToBeUpdated(m_csToBeUpdated);
	if(feed->m_downloadContext == NULL) // if not NULL, this feed is already downloading
	{
		if(bHighPriority)
			m_toBeUpdated.push_front(feed);
		else
			m_toBeUpdated.push_back(feed);
	    
		if(!m_semHasRequest->Unlock()) // Notify consumers that the queue has an object
		{
			ATLASSERT(FALSE);
		}
		lockToBeUpdated.Unlock();
	}
    return;
}

void CChannelUpdate::StoreUpdate()
{
	LARGE_INTEGER lElapse;
	lElapse.QuadPart = -100000000; // 10 sec delay
	if (!::SetWaitableTimer(m_hStoreTimer, &lElapse, 10*1000, NULL, NULL, 0))
    {
        ATLTRACE(_T("SetWaitableTimer failed (%d)\n"), GetLastError());
    }

	bool bQuit = false;
	while ( !bQuit )
	{
		ATLTRACE(_T("StoreUpdate Thread=[%d]\n"), ::GetCurrentThreadId());

		//
		// pump the thread msg Q and check if there's download that need to be saved
		//
		if(!WaitForStoreProcessObject(bQuit))
			continue;

		// find the ready feed
		NewsFeedPtr feedToBeStored;
		CComCritSecLock<CComAutoCriticalSection> lockToBeStored(m_csToBeStored);
		for(std::list<NewsFeedPtr>::iterator it = m_toBeStored.begin(); it != m_toBeStored.end(); ++it)
		{
			NewsFeedPtr feed = *it;
			if(feed->IsDownloadReady())
			{
				m_toBeStored.erase(it);
				feedToBeStored = feed;
				break;
			}
		}
		lockToBeStored.Unlock();

		if(feedToBeStored != NULL)
		{
			try
			{
				int numOfNewItems = feedToBeStored->StoreUpdate();
				if(::IsWindow(m_hwndToNotify))
				{
					::PostMessage(m_hwndToNotify, MM_UPDATEDATAREADY,
									feedToBeStored->m_id ,
									numOfNewItems);
				}
			}
			catch(...)
			{
				if(::IsWindow(m_hwndToNotify))
				{
					::PostMessage(m_hwndToNotify, MM_UPDATEERROR, feedToBeStored->m_id ,0);
				}
			}

			feedToBeStored = NULL;
		}

		if(m_semCanDownload && !m_semCanDownload->Unlock())
		{
			ATLASSERT(FALSE);
		}
	}
	::CancelWaitableTimer(m_hStoreTimer);
}

void CChannelUpdate::DownloadUpdate()
{
	bool bQuit = false;

	while ( !bQuit )
	{
		ATLTRACE(_T("DownloadUpdate Thread=[%d]\n"), ::GetCurrentThreadId());

		//
		// pump the thread msg Q and wait for events
		//
		if(!WaitForDownloadProcessObject(&m_semHasRequest->m_hObject, bQuit))
			continue;

		if(!WaitForDownloadProcessObject(&m_semCanDownload->m_hObject, bQuit))
			continue;

		//
		// process download request
		//
	
		// get the request
		CComCritSecLock<CComAutoCriticalSection> lockToBeUpdated(m_csToBeUpdated);
		NewsFeedPtr feed = m_toBeUpdated.front();
		m_toBeUpdated.pop_front();
		lockToBeUpdated.Unlock();

		//
		if(::IsWindow(m_hwndToNotify))
		{
			CString text;
			text.Format(ResManagerPtr->GetString(IDS_UPDATING), (LPCTSTR)feed->m_title);
			LPTSTR p = new TCHAR[text.GetLength()+1];
			StringCchCopy(p, text.GetLength()+1, (LPCTSTR)text);
			::PostMessage(m_hwndToNotify, MM_UPDATEPROGRESS, feed->m_id,(LPARAM)p);
		}

		// 
		CComCritSecLock<CComAutoCriticalSection> lockToBeStored(m_csToBeStored);
		m_toBeStored.push_back(feed);
		lockToBeStored.Unlock();

		try
		{
			feed->StartDownloading(m_hReadyToStore);
		}
		catch(...)
		{
			ATLTRACE(_T("Error while start feed downloading...\n"));
		}
	}
}

bool CChannelUpdate::WaitForDownloadProcessObject(HANDLE* pHandle, bool& bQuit)
{
	while(true)
	{
		switch (::MsgWaitForMultipleObjects(1,pHandle,FALSE,INFINITE,QS_ALLEVENTS))
		{
		case WAIT_OBJECT_0:  // process
			return true;

		case WAIT_OBJECT_0+1:
			{
				// There is a windows message, handle it...
				MSG msg;
				while (::PeekMessage(&msg,0,0,0,PM_REMOVE))
				{
					// Abort on a WM_QUIT message
					if (msg.message == WM_QUIT)
					{
						bQuit = true;
						return false;// continue;
					}

					// Translate and dispatch the message
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
		}
	}

	// should not be here
	return false;
}

bool CChannelUpdate::WaitForStoreProcessObject(bool& bQuit)
{
	while(true)
	{
		switch (::MsgWaitForMultipleObjects(2,m_hStoreWaitHandles,FALSE,INFINITE,QS_ALLEVENTS))
		{
		case WAIT_OBJECT_0:  // timer
			CheckTimeOut();
			break;

		case WAIT_OBJECT_0+1: // process
			return true;

		case WAIT_OBJECT_0+2:
			{
				// There is a windows message, handle it...
				MSG msg;
				while (::PeekMessage(&msg,0,0,0,PM_REMOVE))
				{
					// Abort on a WM_QUIT message
					if (msg.message == WM_QUIT)
					{
						bQuit = true;
						return false;// continue;
					}

					// Translate and dispatch the message
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
		}
	}

	// should not be here
	return false;
}

void CChannelUpdate::FeedCheckTimeout(NewsFeedPtr& feed)
{
	feed->CheckTimeout(m_timeoutSpan);	
}

void CChannelUpdate::CheckTimeOut()
{
	try
	{
		CComCritSecLock<CComAutoCriticalSection> lockToBeStored(m_csToBeStored);
		for_each(m_toBeStored.begin(), m_toBeStored.end(), FeedCheckTimeout);
		lockToBeStored.Unlock();
	}
	catch(...)
	{
	}
}