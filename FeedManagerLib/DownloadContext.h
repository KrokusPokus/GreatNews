#pragma once

#include <vector>
#include <atlutil.h>
#include <Wininet.h>
#include "loki\SmartPtr.h"

class CDownloadContext
{
public:
	enum DownloadOptions
	{
		NoOption = 0,
		BypassCache = 1
	};

	CDownloadContext(void);
	~CDownloadContext(void);

	static int m_maxFeedSizeInK;

public:
	void SetNotifyHandle(HANDLE hReady);
	void SetDownloadParam(HANDLE hEvent, LPCTSTR url, LPCTSTR user=NULL, LPCTSTR password=NULL, DownloadOptions options=NoOption);
	bool StartDownload(HANDLE hInternet);

	bool Step();
	void AsyncCallback(DWORD dwContext,
						DWORD dwInternetStatus,
						LPVOID lpStatusInfo,
						DWORD dwStatusInfoLen);

	void Cleanup();
	void Close();
	void Notify(bool bSuccess);
	void Abort();
	bool DeflateContent();

public:
	DownloadOptions m_options;
	HANDLE m_hsemReady;
	bool m_bReady;
	bool m_bFailed;
	ULONG_PTR m_contextID;
	std::vector<BYTE> m_data;
	UINT m_dataCRC;
	DWORD m_httpStatusCode;
	CUrl m_urlCrack;
	CString m_username;
	CString m_password;

public:
	enum DownloadSteps
	{
		StepInternetConnect,
		StepHttpOpenRequest,
		StepHttpSendRequest,
		StepHttpQueryInfo
		,StepReadFile,
		StepCopyData
	};
	DownloadSteps m_currentStep;

	HANDLE m_hInternet;
	HANDLE m_hConnect;
	HANDLE m_hRequest;

	CString m_errorMsg;
	int m_errorCode;
	CString m_httpLastModified;
	CString m_httpETag;
	CString m_httpContentEncoding;
	CString m_httpContentType;
	CString m_httpLocation;

private:
	BYTE m_lpReadBuff[10240];
	INTERNET_BUFFERSA m_InetBuff;
	
private:
	CComAutoCriticalSection m_cs;
	int m_wininetAsyncCallbackStepCount; // this is the step count that how many times we were called by wininet
};

typedef Loki::SmartPtr<CDownloadContext, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> DownloadContextPtr;
