#pragma once

#include <map>
#include "DownloadContext.h"
#include "Singleton.h"

#import "msxml3.dll"

class CAsyncDownloadManager
{
public:
	~CAsyncDownloadManager(void);

public:
	bool Init(bool bUseProxy, CString proxyURL, CString proxyBypass, CString proxyUserName, CString proxyPassword);
	void Uninit();

	bool StartDownload(DownloadContextPtr context);
	void FinishDownload(ULONG_PTR contextID);

	bool Download(DownloadContextPtr context);
	bool DownloadToFile(const CString& rawUrl, const CString& fileName);
	MSXML2::IXMLDOMDocument2Ptr GetDomFromUrl(const CString& rawUrl);

private:
	typedef std::map<ULONG_PTR, DownloadContextPtr> CONTEXT_MAP;
	HANDLE m_hInternet;
	ULONG_PTR m_nextContextID;

	CString m_errorMsg;
	int m_errorCode;

	static CComAutoCriticalSection m_cs;
	static CONTEXT_MAP m_contexts;
	static void __stdcall Callback(HINTERNET hInternet,
							DWORD dwContext,
							DWORD dwInternetStatus,
							LPVOID lpStatusInfo,
							DWORD dwStatusInfoLen);

private:
	CAsyncDownloadManager(void);
	friend class CGNSingleton<CAsyncDownloadManager>;
};
