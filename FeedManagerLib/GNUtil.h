#pragma once

#include <vector>
#include "atlwfile.h"

#import "msxml3.dll"


#define GN_ARRAYSIZE(X)  (sizeof(X)/sizeof(X[0]))

class CGNUtil
{
public:
	enum GNUrlType
	{
		HTML,
		RssForSure,
		RssMaybe,
		OpmlForSure,
		Other,
		LocalFile
	};

	static CString NormalizeURL(LPCTSTR szUrl);
	static int CheckUrlType(LPCTSTR sUrl);
	static bool IsFeedUrl(CString& sUrl);
	static bool IsOpmlUrl(LPCTSTR sUrl);
	static bool IsInternetUrl(const CString& sUrl);
	static bool WriteToFile(const CString& fileName, const CString& content);
	static CString GetTempPathName();
	static void CleanupGNTempFiles();
	static bool ReadStringFromFile(const CString& fileName, CString& content);
	static bool ReadBinFile(LPCTSTR fileName, std::vector<BYTE>& content);
	static void CheckDomError(MSXML2::IXMLDOMDocumentPtr spDoc);
	static bool StreamToUnicode(std::vector<BYTE>& in, std::vector<wchar_t>& out);
	static ULONG CalcCRC32(LPBYTE pBuffer, size_t len);
	static void OpenUrlExternally(LPCTSTR url, bool bBackGround = false, bool bNewWindow = false);
	static CString EscapeUrl(LPCTSTR url);
	static CString FixUrl(LPCTSTR url);
	static bool IsValidPoscastingUrl(LPCTSTR podCastingURL);
	static void UnescapeHtml(CString& html);
	static CString GetLastErrorMsg(LPCTSTR lpszFunction);

public:
	static CString m_browserPath;
	static LPCTSTR GetBrowser();

	static CString m_podCastingExtensions;

};

