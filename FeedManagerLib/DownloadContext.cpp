#include "StdAfx.h"
#include "DownloadContext.h"
#include "zutil.h"
#include "GNUtil.h"

#define MAX_ASYNC_STEP_ALLOWED	2000		// put a limit here to prevent dead loop

int CDownloadContext::m_maxFeedSizeInK = 1000;

void DumpStatus(DWORD status)
{
	switch(status)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		AtlTrace(_T("INTERNET_STATUS_RESOLVING_NAME			10\n"));
		break;
	case INTERNET_STATUS_NAME_RESOLVED:
		AtlTrace(_T("INTERNET_STATUS_NAME_RESOLVED          11\n"));
		break;
	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		AtlTrace(_T("INTERNET_STATUS_CONNECTING_TO_SERVER       20\n"));
		break;
	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		AtlTrace(_T("INTERNET_STATUS_CONNECTED_TO_SERVER       21\n"));
		break;
	case INTERNET_STATUS_SENDING_REQUEST:
		AtlTrace(_T("INTERNET_STATUS_SENDING_REQUEST       30\n"));
		break;
	case INTERNET_STATUS_REQUEST_SENT:
		AtlTrace(_T("INTERNET_STATUS_REQUEST_SENT       31\n"));
		break;
	case INTERNET_STATUS_RECEIVING_RESPONSE:
		AtlTrace(_T("INTERNET_STATUS_RECEIVING_RESPONSE       40\n"));
		break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		AtlTrace(_T("INTERNET_STATUS_RESPONSE_RECEIVED       41\n"));
		break;
	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
		AtlTrace(_T("INTERNET_STATUS_CTL_RESPONSE_RECEIVED       42\n"));
		break;
	case INTERNET_STATUS_PREFETCH:
		AtlTrace(_T("INTERNET_STATUS_PREFETCH       43\n"));
		break;
	case INTERNET_STATUS_CLOSING_CONNECTION:
		AtlTrace(_T("INTERNET_STATUS_CLOSING_CONNECTION       50\n"));
		break;
	case INTERNET_STATUS_CONNECTION_CLOSED:
		AtlTrace(_T("INTERNET_STATUS_CONNECTION_CLOSED       51\n"));
		break;
	case INTERNET_STATUS_HANDLE_CREATED:
		AtlTrace(_T("INTERNET_STATUS_HANDLE_CREATED       60\n"));
		break;
	case INTERNET_STATUS_HANDLE_CLOSING:
		AtlTrace(_T("INTERNET_STATUS_HANDLE_CLOSING       70\n"));
		break;
	case INTERNET_STATUS_DETECTING_PROXY:
		AtlTrace(_T("INTERNET_STATUS_DETECTING_PROXY       80\n"));
		break;
	case INTERNET_STATUS_REQUEST_COMPLETE:
		AtlTrace(_T("INTERNET_STATUS_REQUEST_COMPLETE       100\n"));
		break;
	case INTERNET_STATUS_REDIRECT:
		AtlTrace(_T("INTERNET_STATUS_REDIRECT       110\n"));
		break;
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
		AtlTrace(_T("INTERNET_STATUS_INTERMEDIATE_RESPONSE       120\n"));
		break;
	case INTERNET_STATUS_USER_INPUT_REQUIRED:
		AtlTrace(_T("INTERNET_STATUS_USER_INPUT_REQUIRED       140\n"));
		break;
	case INTERNET_STATUS_STATE_CHANGE:
		AtlTrace(_T("INTERNET_STATUS_STATE_CHANGE       200\n"));
		break;
	case INTERNET_STATUS_COOKIE_SENT:
		AtlTrace(_T("INTERNET_STATUS_COOKIE_SENT       320\n"));
		break;
	case INTERNET_STATUS_COOKIE_RECEIVED:
		AtlTrace(_T("INTERNET_STATUS_COOKIE_RECEIVED       321\n"));
		break;
	case INTERNET_STATUS_PRIVACY_IMPACTED:
		AtlTrace(_T("INTERNET_STATUS_PRIVACY_IMPACTED       324\n"));
		break;
	case INTERNET_STATUS_P3P_HEADER:
		AtlTrace(_T("INTERNET_STATUS_P3P_HEADER       325\n"));
		break;
	case INTERNET_STATUS_P3P_POLICYREF:
		AtlTrace(_T("INTERNET_STATUS_P3P_POLICYREF       326\n"));
		break;
	case INTERNET_STATUS_COOKIE_HISTORY:
		AtlTrace(_T("INTERNET_STATUS_COOKIE_HISTORY       327\n"));
		break;
	default:
		AtlTrace(_T("Unknown stauts code[%d]\n"), status);
	}
}


//
//
//
CDownloadContext::CDownloadContext(void) :
	m_hRequest(NULL), m_hConnect(NULL),m_dataCRC(0),m_wininetAsyncCallbackStepCount(0)
{
	Cleanup();
}

CDownloadContext::~CDownloadContext(void)
{
}

void CDownloadContext::AsyncCallback(DWORD dwContext,
									DWORD dwInternetStatus,
									LPVOID lpStatusInfo,
									DWORD dwStatusInfoLen)
{
	if((ULONG_PTR)dwContext != m_contextID)
	{
		// what's going on???
		AtlTrace(_T("----- Invalid call back handler ---- "));
		return;
	}

	// AtlTrace(_T("------ Callback Thread:%d Step=%d\n"), GetCurrentThreadId(), (int)m_currentStep);
	// DumpStatus(dwInternetStatus);

	if(m_currentStep == StepInternetConnect)
	{
		switch(dwInternetStatus)
		{
		case INTERNET_STATUS_HANDLE_CREATED:
			m_hConnect = (HINTERNET)(((INTERNET_ASYNC_RESULT *)lpStatusInfo)->dwResult);
			break;
		case INTERNET_STATUS_REQUEST_COMPLETE:
			m_currentStep = StepHttpOpenRequest;
			Step();
			break;
		}
	}
	else if(m_currentStep == StepHttpOpenRequest)
	{
		switch(dwInternetStatus)
		{
		case INTERNET_STATUS_HANDLE_CREATED:
			m_hRequest = (HINTERNET)(((INTERNET_ASYNC_RESULT *)lpStatusInfo)->dwResult);
			break;
		case INTERNET_STATUS_REQUEST_COMPLETE:
			m_currentStep = StepHttpSendRequest;
			Step();
			break;
		}
	}
	else if(m_currentStep == StepHttpSendRequest)
	{
		switch(dwInternetStatus)
		{
		case INTERNET_STATUS_REQUEST_COMPLETE:
			m_currentStep = StepHttpQueryInfo;
			Step();
			break;
		}
	}
	else if(m_currentStep == StepReadFile)
	{
		switch(dwInternetStatus)
		{
		case INTERNET_STATUS_REQUEST_COMPLETE:
			Step();
			break;
		}
	}
}


void CDownloadContext::SetDownloadParam(HANDLE hReady, LPCTSTR url, LPCTSTR user, LPCTSTR password, DownloadOptions options)
{
	m_hsemReady = hReady;
	m_options = options;
	m_username = user;
	m_password = password;

	m_urlCrack.CrackUrl(url);
	//if(user != NULL)
	//	m_urlCrack.SetUserName(user);
	//if(password != NULL)
	//	m_urlCrack.SetPassword(password);
}

bool CDownloadContext::StartDownload(HANDLE hInternet)
{
	if(m_urlCrack.GetScheme() == ATL_URL_SCHEME_FILE)
	{
		if(!CGNUtil::ReadBinFile(m_urlCrack.GetUrlPath(), m_data))
		{
			m_errorCode = GetLastError();
			m_errorMsg.Format(CString(_T("Failed to load file ")) + m_urlCrack.GetUrlPath());

			Notify(false);
			return false;
		}
		else
		{
			if(m_data.size()==0)
			{
				m_errorMsg = _T("Response returned with zero size.");
				Notify(false);
				return false;
			}
			else
			{
				m_httpStatusCode = HTTP_STATUS_OK;

				// calculate data crc
				uLong crc = crc32(0L, Z_NULL, 0);
				crc = crc32(crc, &m_data[0], (uInt)m_data.size());
				m_dataCRC = crc;

				Notify(true);
				return true;
			}
		}

	}
	else
	{
		m_wininetAsyncCallbackStepCount  = 0;

		m_hInternet = hInternet;
		
		m_currentStep = StepInternetConnect;

		return Step();
	}
}

bool CDownloadContext::Step()
{
	// AtlTrace(_T("Step Thread:%d \n"), GetCurrentThreadId());

	m_wininetAsyncCallbackStepCount++;
	if(m_wininetAsyncCallbackStepCount > MAX_ASYNC_STEP_ALLOWED)
	{
		m_errorCode = GetLastError();
		m_errorMsg.Format(_T("Internet download failed due to too many steps."));
		Notify(false);
		return false;
	}

	ATLASSERT(m_hInternet !=NULL);
	if(m_currentStep == StepInternetConnect)
	{
		// First call that will actually complete asynchronously even
		// though there is no network traffic
		// AtlTrace(_T("InternetConnect\n"));
		m_hConnect = InternetConnect(m_hInternet, 
								m_urlCrack.GetHostName(), 
								m_urlCrack.GetPortNumber(),
								m_urlCrack.GetUserName(),
								m_urlCrack.GetPassword(),
								INTERNET_SERVICE_HTTP,
								0,
								(DWORD)m_contextID); // Connection handle's Context
		if (m_hConnect == NULL)
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				m_errorCode = GetLastError();
				m_errorMsg.Format(_T("InternetConnect failed, error[%d]"),m_errorCode);
				Notify(false);
				return false;
			}

			return true; // wait for async completion
		}
		else
		{
			m_currentStep = StepHttpOpenRequest;
		}
	}

	if(m_hConnect == NULL)
		return false; // don't know what happened

	if(m_currentStep == StepHttpOpenRequest)
	{
		// Open the request
		// AtlTrace(_T("HttpOpenRequest\n"));
		DWORD flags = INTERNET_FLAG_KEEP_CONNECTION
					| INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP
					| INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS
					| INTERNET_FLAG_IGNORE_CERT_CN_INVALID
					| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID
					| INTERNET_FLAG_NO_UI;
		if((m_options & BypassCache) != 0)
			flags |= INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_RELOAD | INTERNET_FLAG_RESYNCHRONIZE;
		if(m_urlCrack.GetScheme() == ATL_URL_SCHEME_HTTPS)
			flags |= INTERNET_FLAG_SECURE;

		CString object = m_urlCrack.GetUrlPath();
		object += m_urlCrack.GetExtraInfo();
		m_hRequest = HttpOpenRequest(m_hConnect, 
								_T("GET"), 
								object,
								_T("HTTP/1.0"),
								NULL,
								NULL,
								flags,
								(DWORD)m_contextID);  // Request handle's context 

		if (m_hRequest == NULL)
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				m_errorCode = GetLastError();
				m_errorMsg = _T("Failed to open internet request");
				Notify(false);
				return false;
			}
			// Wait until we get the request handle
			return true;
		}
		else
		{
			m_currentStep = StepHttpSendRequest;
		}
	}

	ATLASSERT(m_hRequest != NULL);
	if(m_currentStep == StepHttpSendRequest)
	{	
		// pass in user name and password
		if(m_username.GetLength())
		{
			InternetSetOption(m_hRequest,  INTERNET_OPTION_USERNAME , (LPVOID)(LPCTSTR)m_username, m_username.GetLength());
			if(m_password.GetLength())
			{
				InternetSetOption(m_hRequest,  INTERNET_OPTION_PASSWORD , (LPVOID)(LPCTSTR)m_password, m_password.GetLength());
			}
		}

		// CString str = _T("Accept-Encoding: identity");
		CString str = _T("Accept-Encoding: gzip");
		if(m_httpLastModified.GetLength())
			str.AppendFormat(_T("\r\nIf-Modified-Since: %s"), (LPCTSTR)m_httpLastModified);
		if(m_httpETag.GetLength())
			str.AppendFormat(_T("\r\nIf-None-Match: %s"), (LPCTSTR)m_httpETag);
		
		USES_CONVERSION;
		LPCSTR header = T2A((LPCTSTR)str);
		// AtlTrace(_T("HttpSendRequest\n"));
		if (!HttpSendRequestA(m_hRequest, 
							header, 
							-1, 
							NULL,
							0))
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				m_errorCode = GetLastError();
				m_errorMsg = _T("Failed to send internet request");
				Notify(false);
				return false;
			}
			return true;
		}
		else
		{
			m_currentStep = StepHttpQueryInfo;
		}
	}


	if(m_currentStep == StepHttpQueryInfo)
	{
		DWORD index=0;
		DWORD size = sizeof(m_httpStatusCode);
		if(!HttpQueryInfo(m_hRequest,
					HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,
					&m_httpStatusCode,
					&size,
					&index))
		{
			m_errorCode = GetLastError();
			m_errorMsg = _T("Failed to query internet info");
			Notify(false);
			return false;
		}

		// check if we need proxy authentication
		if ( m_httpStatusCode == HTTP_STATUS_DENIED || m_httpStatusCode == HTTP_STATUS_PROXY_AUTH_REQ)
		{
			// We have to read all outstanding data on the Internet handle
			// before we can resubmit request. Just discard the data.
			char szData[51];
			DWORD dwSize = sizeof (DWORD) ;
			do
			{
				InternetReadFile (m_hRequest, (LPVOID)szData, 50, &dwSize);
			}
			while (dwSize != 0);

			// if no option works so far, show login dialog
			if ( InternetErrorDlg (GetDesktopWindow(),
								m_hRequest,
								ERROR_INTERNET_INCORRECT_PASSWORD,
								FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
								FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
								FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL) == ERROR_INTERNET_FORCE_RETRY )
			{
				m_currentStep = StepHttpSendRequest;
				Step(); // retry SendRequest step
				return true;
			}
			else
			{
				m_errorCode = GetLastError();
				m_errorMsg = _T("Authentication failed.");
				Notify(false);
				return false;
			}
		}


		// check status code before proceeding
		if(m_httpStatusCode != HTTP_STATUS_OK) // 200
		{
			Notify(true); // we are done, let client deal with status code
			return true;
		}

		//
		// now retrieve Etag and Last-Modified header
		//
		TCHAR buffer[255];
		DWORD buffSize = sizeof(buffer)*sizeof(TCHAR);
		index=0;
		if(HttpQueryInfo(m_hRequest,
					HTTP_QUERY_LAST_MODIFIED,
					buffer,
					&buffSize,
					&index))
		{
			m_httpLastModified = buffer;
		}

		index=0;
		if(HttpQueryInfo(m_hRequest,
					HTTP_QUERY_ETAG,
					buffer,
					&buffSize,
					&index))
		{
			m_httpETag = buffer;
		}

		index=0;
		if(HttpQueryInfo(m_hRequest,
					HTTP_QUERY_CONTENT_ENCODING,
					buffer,
					&buffSize,
					&index))
		{
			m_httpContentEncoding = buffer;
		}

		index=0;
		if(HttpQueryInfo(m_hRequest,
					HTTP_QUERY_CONTENT_TYPE,
					buffer,
					&buffSize,
					&index))
		{
			m_httpContentType = buffer;
		}

		index=0;
		if(HttpQueryInfo(m_hRequest,
					HTTP_QUERY_LOCATION,
					buffer,
					&buffSize,
					&index))
		{
			m_httpLocation = buffer;
		}

		// if somehow failed to retrieve content encoding, we want to try it again.
		// this will fix www.theserverside.com's problem
		if(m_httpContentEncoding.GetLength()==0) 
		{
			// get all http headers
			TCHAR debugBuffer[1024]={0};
			DWORD debugSize=sizeof(debugBuffer)/sizeof(TCHAR);
			if(HttpQueryInfo(m_hRequest, HTTP_QUERY_RAW_HEADERS,debugBuffer, &debugSize, NULL))
			{
			// try to find the content-encoding header
#define CONTENT_ENCODING _T("Content-Encoding:")
				LPCTSTR lpHeader = debugBuffer;
				while(*lpHeader)
				{
					CString str = lpHeader;
					if(str.Find(CONTENT_ENCODING)==0)
					{
						m_httpContentEncoding = str.Mid((int)_tcslen(CONTENT_ENCODING));
						m_httpContentEncoding.Trim();
						break;
					}

					lpHeader += _tcslen(lpHeader)+1;
				}
			}
		}


		//
		// Move to next step
		//
		m_currentStep = StepReadFile;
	}

	while(m_currentStep == StepReadFile)
	{
		FillMemory(&m_InetBuff, sizeof(m_InetBuff), 0);
		m_InetBuff.dwStructSize = sizeof(m_InetBuff);
		m_InetBuff.lpvBuffer = m_lpReadBuff;
		m_InetBuff.dwBufferLength = sizeof(m_lpReadBuff) - 1;

		// AtlTrace(_T("InternetReadFileEx\n"));
		if (!InternetReadFileExA(m_hRequest,
							&m_InetBuff,
							IRF_NO_WAIT,
							(DWORD)m_contextID))
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				m_errorCode = GetLastError();
				m_errorMsg.Format(_T("ReadFile failed, error[%d]"),m_errorCode);
				Notify(false);
				return false;
			}
			else
				return true;
		}
		
		if(m_InetBuff.dwBufferLength == 0)
		{
			// try to deflate content if it's gzipped
			if(!DeflateContent())
			{
				m_errorMsg = _T("Failed to deflate feed response");
				Notify(false);
				return false;
			}
			else if(m_data.size()==0)
			{
				m_errorMsg = _T("Response returned with zero size.");
				Notify(false);
				return false;
			}
			else
			{
				// calculate data crc
				uLong crc = crc32(0L, Z_NULL, 0);
				crc = crc32(crc, &m_data[0], (uInt)m_data.size());
				m_dataCRC = crc;

				Notify(true);
				return true;
			}
		}

		m_data.insert(m_data.end(), m_lpReadBuff, m_lpReadBuff + m_InetBuff.dwBufferLength);
		if(m_data.size()>(size_t)m_maxFeedSizeInK*1024) // hard code a limit to prevent some huge feed flood user machine
		{
			m_errorMsg.Format(_T("Feed content exceeds %dK."), m_maxFeedSizeInK);
			Notify(false);
			return false;
		}
	}

	return true;
}

void CDownloadContext::Close()
{
	if(m_hRequest != NULL)
	{
		InternetCloseHandle(m_hRequest);
		m_hRequest = NULL;
	}

	if(m_hConnect != NULL)
	{
		InternetCloseHandle(m_hConnect);
		m_hConnect = NULL;
	}


}

void CDownloadContext::Cleanup()
{
	Close();

	m_contextID = 0;
	m_bReady = false;
	m_bFailed = false;
	m_httpStatusCode = 0;
	m_data.clear();
	m_hsemReady = NULL;
}

void CDownloadContext::Notify(bool bSuccess)
{
	Close();

	CComCritSecLock<CComAutoCriticalSection> olock(m_cs);

	m_bReady = true;
	m_bFailed = !bSuccess;
	if(m_hsemReady && !::ReleaseSemaphore(m_hsemReady, 1, NULL))
	{
		ATLASSERT(FALSE);
	}
	m_hsemReady = NULL;
}

void CDownloadContext::Abort()
{
	m_errorMsg = _T("Download was aborted due to internet connection timeout.");
	Notify(false);
}


void CDownloadContext::SetNotifyHandle(HANDLE hReady)
{
	m_hsemReady = hReady;
}


bool CDownloadContext::DeflateContent()
{

#define HEAD_SIZE 10

	if(m_httpContentEncoding != _T("gzip") || m_data.size() <= HEAD_SIZE)
		return true; // nothing to do

	AtlTrace(_T("Decompressing gzip content[%s]\n"), m_urlCrack.GetHostName());

#define BUFFER_SIZE 256000
	BYTE fixedBuffer[BUFFER_SIZE];
	BYTE* pUncompressBuffer = fixedBuffer;
	ULONG_PTR uncompressLen = BUFFER_SIZE;
	int err;
	z_stream d_stream; /* decompression stream */

	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;

	//
	// gzip header format: http://www.gzip.org/zlib/rfc-gzip.html#header-trailer
	//
	BYTE* pStart = &m_data[0] + HEAD_SIZE; // skip the header
	BYTE* pEnd = &m_data[0]+m_data.size();

	BYTE flag = m_data[3];
	if(flag & 0x4) // has FEXTRA?
	{
		unsigned short len = *pStart;
		pStart += len+2;
		if(pStart>=pEnd)
			return false;
	}
	if(flag & 0x8) // has FNAME?
	{
		while(pStart<pEnd && *pStart++);

		if(pStart>=pEnd)
			return false;
	}
	if(flag & 0x10) // has FCOMMENT?
	{
		while(pStart<pEnd && *pStart++);

		if(pStart>=pEnd)
			return false;
	}
	if(flag & 0x1) // has FHCRC?
	{
		pStart += 2;
	}

	d_stream.next_in  = pStart;
	d_stream.avail_in = pEnd-pStart;
	d_stream.next_out = fixedBuffer;
	d_stream.avail_out = uncompressLen;

	err = inflateInit2(&d_stream, -MAX_WBITS);
	if(err != Z_OK)
		return false;

	err = inflate(&d_stream, Z_NO_FLUSH);
	if(err != Z_OK && err != Z_STREAM_END)
		return false;

	err = inflateEnd(&d_stream);
	if(err != Z_OK)
		return false;

	m_data.clear();
	m_data.assign(fixedBuffer, fixedBuffer+d_stream.total_out);

	m_httpContentEncoding.Empty(); // clear content encoding because we have deflated already

	return true;
}
