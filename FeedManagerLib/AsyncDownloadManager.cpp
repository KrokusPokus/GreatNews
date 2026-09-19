#include "StdAfx.h"
#include "AsyncDownloadManager.h"
#include "FeedManagerErrorCode.h"
#include "GNUtil.h"
#include "ExceptionBase.h"
#include "atlwfile.h"
#include "SafeArrayVariant.h"

CAsyncDownloadManager::CONTEXT_MAP CAsyncDownloadManager::m_contexts;
CComAutoCriticalSection CAsyncDownloadManager::m_cs;


CAsyncDownloadManager::CAsyncDownloadManager(void)
	: m_hInternet(NULL),m_errorCode(0),m_nextContextID(1)
{
}

CAsyncDownloadManager::~CAsyncDownloadManager(void)
{
}


void __stdcall CAsyncDownloadManager::Callback(HINTERNET hInternet,
				DWORD dwContext,
				DWORD dwInternetStatus,
				LPVOID lpStatusInfo,
				DWORD dwStatusInfoLen)
{

	DownloadContextPtr pContext;

	CComCritSecLock<CComAutoCriticalSection> olock(m_cs);
	CONTEXT_MAP::iterator it = m_contexts.find((ULONG_PTR)dwContext);
	if(it!=m_contexts.end())
	{
		pContext = it->second;
	}
	olock.Unlock();

	if(pContext != NULL)
	{
		pContext->AsyncCallback(dwContext, dwInternetStatus, lpStatusInfo, dwStatusInfoLen);
	}
}

bool CAsyncDownloadManager::Init(bool bUseProxy, CString proxyURL, CString proxyBypass, CString proxyUserName, CString proxyPassword)
{
	Uninit();

	if(bUseProxy && proxyURL.GetLength())
	{
		m_hInternet = InternetOpen(_T("GreatNews/1.0"),
							INTERNET_OPEN_TYPE_PROXY,
							(LPCTSTR)proxyURL,
							proxyBypass.GetLength()?(LPCTSTR)proxyBypass : NULL,
							INTERNET_FLAG_ASYNC); // ASYNC Flag
	}
	else
	{
		m_hInternet = InternetOpen(_T("GreatNews/1.0"),
							INTERNET_OPEN_TYPE_PRECONFIG,
							NULL,
							NULL,
							INTERNET_FLAG_ASYNC); // ASYNC Flag
	}

	if (m_hInternet == NULL)
	{
		m_errorCode = GetLastError();
		m_errorMsg.Format(_T("InternetOpen failed, error[%d]"),m_errorCode);
		return false;
	}

	// set proxy password if we have any
	if(bUseProxy && proxyUserName.GetLength())
	{
		InternetSetOption(m_hInternet, INTERNET_OPTION_PROXY_USERNAME , (LPVOID)(LPCTSTR)proxyUserName, proxyUserName.GetLength());
		InternetSetOption(m_hInternet, INTERNET_OPTION_PROXY_PASSWORD , (LPVOID)(LPCTSTR)proxyPassword, proxyPassword.GetLength());
	}

	if (InternetSetStatusCallback(m_hInternet, (INTERNET_STATUS_CALLBACK)Callback) == INTERNET_INVALID_STATUS_CALLBACK)
	{
		m_errorCode = GetLastError();
		m_errorMsg.Format(_T("InternetSetStatusCallback failed, error[%d]"),m_errorCode);
		return false;
	}

	return true;
}

void CAsyncDownloadManager::Uninit()
{
	if(m_hInternet)
	{
		InternetCloseHandle(m_hInternet);
		m_hInternet = NULL;
	}

	return;
}

bool CAsyncDownloadManager::StartDownload(DownloadContextPtr pContext)
{
	pContext->m_contextID = ::InterlockedIncrement((LONG*)&m_nextContextID);

	CComCritSecLock<CComAutoCriticalSection> olock(m_cs);
	m_contexts.insert(CONTEXT_MAP::value_type(pContext->m_contextID, pContext));
	olock.Unlock();

	return pContext->StartDownload(m_hInternet);
}

void CAsyncDownloadManager::FinishDownload(ULONG_PTR contextID)
{
	DownloadContextPtr pContext;
	CComCritSecLock<CComAutoCriticalSection> olock(m_cs);
	CONTEXT_MAP::iterator it = m_contexts.find(contextID);
	if(it!=m_contexts.end())
	{
		pContext = it->second;
		m_contexts.erase(it);
	}
	olock.Unlock();

	if(pContext != NULL)
		pContext->Close();
}

bool CAsyncDownloadManager::Download(DownloadContextPtr context)
{
	CNGSemaphore sem(0,1);
	context->SetNotifyHandle(sem);
	StartDownload(context);
	sem.Lock();
	FinishDownload(context->m_contextID);

	return !context->m_bFailed;
}


bool CAsyncDownloadManager::DownloadToFile(const CString& rawUrl, const CString& fileName)
{
	try
	{
		// download the raw data
		DownloadContextPtr context(new CDownloadContext());
		context->SetDownloadParam(NULL, (LPCTSTR)rawUrl);
		if(!Download(context))
		{
			CString msg;
			msg.Format(_T("Channel update was aborted due to %s"), (LPCTSTR)context->m_errorMsg);
			AtlTrace(_T("%s\n"), (LPCTSTR)msg);
			return false;
		}

		long statusCode = context->m_httpStatusCode;
		if(statusCode != 200)
		{
			CString msg;
			msg.Format(_T("Error reading from channel website. Return status is [%d]"), statusCode);
			AtlTrace(_T("%s\n"), (LPCTSTR)msg);
			return false;
		}

		if(context->m_data.empty())
			return false;

		CFile file;
		if(!file.Open(fileName, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL))
			return false;

		if(!file.Write(&context->m_data[0], (DWORD)context->m_data.size()))
			return false;
	}
	catch(...)
	{
		return false;
	}

	return true;
}

MSXML2::IXMLDOMDocument2Ptr CAsyncDownloadManager::GetDomFromUrl(const CString& rawUrl)
{
	if(rawUrl.GetLength() == 0)
		return NULL;

	CString normalizedUrl = CGNUtil::NormalizeURL(rawUrl);

	MSXML2::IXMLDOMDocument2Ptr spDoc = NULL;

	try
	{
		// download the raw data
		DownloadContextPtr context(new CDownloadContext());
		context->SetDownloadParam(NULL, (LPCTSTR)normalizedUrl);
		if(!Download(context))
		{
			CString msg;
			msg.Format(_T("Channel update was aborted due to timeout"));
			AtlTrace(_T("%s\n"), (LPCTSTR)msg);
			throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, msg);
		}

		long statusCode = context->m_httpStatusCode;

		if(statusCode != 200)
		{
			CString msg;
			msg.Format(_T("Error reading from channel website. Return status is [%d]"), statusCode);
			AtlTrace(_T("%s\n"), (LPCTSTR)msg);
			throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, msg);
		}
		AtlTrace(_T("size=%d\n"), context->m_data.size());
		CSafeArrayVariant v(&context->m_data[0], (int)context->m_data.size());
		context->Cleanup();

		spDoc.CreateInstance("MSXML2.DOMDocument.3.0");
		spDoc->validateOnParse = false; // do not validate DTD/Schema
		spDoc->resolveExternals = false;
		spDoc->load(v);
		//CGNUtil::CheckDomError(spDoc);

		try
		{
			CGNUtil::CheckDomError(spDoc);
		}
		catch(const CInvalidCharacterException&)
		{
			// try to solve invalid character problem
			throw;
		}
	}
	catch(_com_error& err)
	{
		_bstr_t desc = err.Description();
		throw CExceptionBase(ERR_FM_GENERICERR, (LPCTSTR)desc);
	}

	return spDoc;
}
