#pragma once

#include "GNThread.h"
#include "ChannelUpdate.h"

class CUpdateDownloadThread :
	public CGNThread
{
public:
	CUpdateDownloadThread(CChannelUpdate& channelUpdate): m_channelUpdate(channelUpdate) {};

	virtual DWORD Run() 
	{
#ifdef MULTITHREADED
		HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
		HRESULT hRes = ::CoInitialize(NULL);
#endif
		ATLASSERT(SUCCEEDED(hRes));

		m_channelUpdate.DownloadUpdate();
		
		::CoUninitialize();

		return 0;
	}

public:
	CChannelUpdate& m_channelUpdate;
};
