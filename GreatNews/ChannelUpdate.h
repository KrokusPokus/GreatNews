#pragma once

#include <list>
#include "NGMTSync.h"
#include "FeedManagerLib.h"

#define MM_UPDATEDATAREADY	(WM_APP + 200)
#define MM_UPDATEPROGRESS	(WM_APP + 202)	
#define MM_UPDATEDONE		(WM_APP + 203)	
#define MM_UPDATEERROR		(WM_APP + 204)

class CChannelUpdate
{
public:
	CChannelUpdate(int maxConcurrentUpdate);
	~CChannelUpdate(void);

public:
	// these methods should be called from main thread
	void Start(HWND wnd);
	void Stop();
    void EnqueueUpdate(NewsFeedPtr feed, bool bHighPriority);

	// this method should be called from download thread
	void DownloadUpdate();

	// this method should be called from store thread
	void StoreUpdate();

private:
	HWND m_hwndToNotify;

	std::list<NewsFeedPtr> m_toBeUpdated;
	std::list<NewsFeedPtr> m_toBeStored;

	CComAutoCriticalSection m_csToBeUpdated;
	CComAutoCriticalSection m_csToBeStored;
	
	CNGSemaphore* m_semHasRequest;
	CNGSemaphore* m_semCanDownload;

private:
	bool WaitForDownloadProcessObject(HANDLE* pHandle, bool& bQuit);
	HANDLE m_hDownloadThread;
	DWORD m_downloadThreadID;

	bool WaitForStoreProcessObject(bool& bQuit);
	union
	{
		struct 
		{
			HANDLE m_hStoreTimer;
			HANDLE m_hReadyToStore;
		};
		HANDLE m_hStoreWaitHandles[2];
	};
	HANDLE m_hStoreThread;
	DWORD m_storeThreadID;

	int m_maxConcurrentUpdate;

private:
	void CheckTimeOut();

private: // STL helpers
	static void FeedAbortUpdate(NewsFeedPtr& feed);
	static void FeedCheckTimeout(NewsFeedPtr& feed);
	static CTimeSpan m_timeoutSpan;

};
