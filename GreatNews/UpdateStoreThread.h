#pragma once

#include "GNThread.h"
#include "ChannelUpdate.h"

class CUpdateStoreThread :
	public CGNThread
{
public:
	CUpdateStoreThread(CChannelUpdate& channelUpdate): m_channelUpdate(channelUpdate) {};

	virtual DWORD Run()
	{
#ifdef MULTITHREADED
		HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
		HRESULT hRes = ::CoInitialize(NULL);
#endif
		ATLASSERT(SUCCEEDED(hRes));

		m_channelUpdate.StoreUpdate();

		::CoUninitialize();
		
		return 0;
	}

public:
	CChannelUpdate& m_channelUpdate;
};
