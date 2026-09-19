#include "StdAfx.h"
#include "GNUtil.h"
#include <shellapi.h>
#include <atlrx.h>
#include <atlfile.h>
#include <atlutil.h>
#include "ExceptionBase.h"
#include "FeedManagerErrorCode.h"
#include <strsafe.h>
#include "zlib.h"

#include <iostream>

CString CGNUtil::m_podCastingExtensions;
CString CGNUtil::m_browserPath;

//
// Internal help function to convert charset to codepage
//		HKEY_CLASSES_ROOT\MIME\Database\Charset 
int CharsetToCodepage(LPCTSTR charset)
{
	if(_tcsicmp(charset, _T("GB18030"))==0)
	{
		if(::IsValidCodePage(54936))
			return 54936;
		else
			throw CExceptionBase(ERR_FM_GENERICERR, _T("Your computer doesn't appear to support GB18030.\nPlease download \"GB18030 Support Package\" from Microsoft."));
	}

	CString key;
	key.Format(_T("MIME\\Database\\Charset\\%s"), charset);

	CRegKey reg;
	TCHAR alias[255];
	ULONG size = 254;
	while(ERROR_SUCCESS == reg.Open(HKEY_CLASSES_ROOT, key, KEY_READ))
	{
		DWORD codepage=0;
		if(ERROR_SUCCESS == reg.QueryDWORDValue(_T("InternetEncoding"), codepage))
		{
			reg.Close();
			return codepage;
		}
		
		if(ERROR_SUCCESS != reg.QueryStringValue(_T("AliasForCharset"), alias, &size))
		{
			reg.Close();
			return 0;
		}

		key.Format(_T("MIME\\Database\\Charset\\%s"), alias);
	}

	//CString str;
	//str.Format(_T("Unrecognized encoding: %s."), charset);
	//throw CExceptionBase(ERR_FM_GENERICERR, str);

	return 0;
}

//
// Internal help function to find encoding string in xml PI
//		<?xml xxxxxxxxxxx  encoding="UTF-8" xxxxxx ?>
int FindCodePageInXml(std::vector<BYTE>& in)
{
	char checkBuffer[101]={0};	// the PI has to be in 8-bit char encoding
	size_t checkSize = min(100, in.size()); // only check the first 100 bytes
	strncpy(checkBuffer, (char*)&in[0], checkSize);

	// find the encoding name
	char* pEncoding = strstr(checkBuffer, "encoding");
	if(pEncoding == NULL)
		return 0;

	char* pOpenQuote=NULL;
	bool bClosed = false;
	while(*pEncoding)
	{
		if(*pEncoding == '\'' || *pEncoding == '"')
		{
			if(pOpenQuote == NULL)
				pOpenQuote = pEncoding;
			else
			{
				bClosed = true;
				*pEncoding=0;
				break;
			}
		}
		pEncoding++;
	}

	if(!bClosed)
		return 0;
	
	CString charset = pOpenQuote+1;
	charset = charset.Trim();
	
	return CharsetToCodepage(charset);	
}


CString CGNUtil::NormalizeURL(LPCTSTR szUrl)
{
	CString url(szUrl);
	url.Trim();
	int pos = url.Find(_T("://"));
	if(pos == -1)
	{
		return _T("http://")+url;
	}

	CString scheme = url.Left(pos);
	if(scheme.CompareNoCase(_T("feed"))==0)
	{
		return _T("http")+url.Mid(pos);
	}

	return url;
}

// internal help routine
bool CheckFeedUrlPrefix(CString& sUrl, LPCTSTR prefix, LPCTSTR replace=NULL)
{
	if(sUrl.Find(prefix) == 0)
	{
		sUrl = replace + sUrl.Mid((int)_tcslen(prefix));
		return true;
	}
	else
		return false;
}

bool CGNUtil::IsFeedUrl(CString& sUrl)
{
	// check well known feed format
	if(::CheckFeedUrlPrefix(sUrl, _T("feed:http"), _T("http")))
		return true;
	else if(::CheckFeedUrlPrefix(sUrl, _T("http://add.my.yahoo.com/content?url="))) // http://add.my.yahoo.com/content?url=http://www.userscape.com/blog/feed/rss2/
		return true;
	else if(::CheckFeedUrlPrefix(sUrl, _T("http://www.newsgator.com/ngs/subscriber/subext.aspx?url="))) // http://www.newsgator.com/ngs/subscriber/subext.aspx?url=http://www.userscape.com/blog/feed/rss2/
		return true;
	else if(::CheckFeedUrlPrefix(sUrl, _T("http://www.bloglines.com/sub/"))) // http://www.bloglines.com/sub/http://www.userscape.com/blog/feed/rss2/
		return true;
	else if(sUrl.Find(_T("http://sourceforge.net/export/rss2_projnews.php?"))==0)
		return true;
	else if(sUrl.Find(_T("http://sourceforge.net/export/rss2_projfiles.php?"))==0)
		return true;
	else if(sUrl.Find(_T("http://sourceforge.net/export/rss2_projdocs.php?"))==0)
		return true;
	else if(sUrl.Find(_T("http://sourceforge.net/export/rss2_projsummary.php?"))==0)
		return true;
	else if(sUrl.Find(_T("http://feeds.feedburner.com/")) == 0 && sUrl.Find(_T("?")) == -1 && sUrl.Find(_T("http://feeds.feedburner.com/~")) == -1 )
	{
		// http://feeds.feedburner.com/ArsalanZaidisBlog is a news feed
		// http://feeds.feedburner.com/ArsalanZaidisBlog?asdfasd is a page
		return true;
	}
		
	int urlType = CheckUrlType(sUrl);
	return (urlType==RssForSure || urlType==RssMaybe);
}


bool CGNUtil::IsOpmlUrl(LPCTSTR sUrl)
{
	int urlType = CheckUrlType(sUrl);
	return (urlType==OpmlForSure);
}

int CGNUtil::CheckUrlType(LPCTSTR sUrl)
{
	CString szURL(sUrl);
	szURL.Trim();

	CUrl url;
	url.CrackUrl(szURL);
	CString schemeName = url.GetSchemeName();
	if(schemeName.CompareNoCase(_T("feed"))==0)
		return RssForSure;

	ATL_URL_SCHEME scheme = url.GetScheme();
	if(scheme == ATL_URL_SCHEME_FILE)
		return LocalFile;

	CString fileName = url.GetUrlPath();
	CPath path = fileName;
	CString ext = path.GetExtension();
	if(ext.CompareNoCase(_T(".rdf"))==0 || 
		ext.CompareNoCase(_T(".rss"))==0 ||
		ext.CompareNoCase(_T(".rss2"))==0 ||
		ext.CompareNoCase(_T(".atom"))==0)
	{
		return RssForSure;
	}
	else if(ext.CompareNoCase(_T(".xml"))==0)
	{
		return RssMaybe;
	}
	else if(ext.CompareNoCase(_T(".opml"))==0)
	{
		return OpmlForSure;
	}
	else if(ext.CompareNoCase(_T(".html"))==0 || ext.CompareNoCase(_T(".htm"))==0)
	{
		return HTML;
	}

	return Other;
}


bool CGNUtil::WriteToFile(const CString& fileName, const CString& content)
{
	CAtlFile file;
	HRESULT hr=file.Create(fileName,GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN );
	if(hr!=S_OK)
	{
		CString msg;
		msg.Format(_T("Failed to create temp file [%s]\n"),(LPCTSTR)fileName);
		AtlTrace(msg);
		return false;
	}

#ifdef UNICODE
	int bufferSize = WideCharToMultiByte(CP_UTF8,
								0,
								content,
								-1,
								0,
								0,
								0,
								0);
	if(bufferSize==0)
	{
		CString str;
		str.Format(_T("Unicode -> UTF8 conversion failed.[%d]\n"), GetLastError());
		AtlTrace(str);
		return false;
	}
	char* pBuffer = new char[bufferSize];
	int size = WideCharToMultiByte(CP_UTF8,
								0,
								content,
								-1,
								pBuffer,
								bufferSize,
								0,
								0);

	if(size > 0)
	{
		file.Write(pBuffer, size-1); // -1 to remove the trailing 0
	}
	else
	{
		CString str;
		str.Format(_T("Unicode -> UTF8 conversion failed.[%d]\n"), GetLastError());
		AtlTrace(str);
	}
	delete [] pBuffer;
#else
	file.Write((LPCTSTR)content, content.GetLength());
#endif


	file.Close();

	return true;
}

bool CGNUtil::ReadBinFile(LPCTSTR fileName, std::vector<BYTE>& content)
{
	CFile licFile;
	if(!licFile.Open(fileName))
		return false;
	int length = licFile.GetSize();
	content.clear();
	content.assign(length, 0);
	DWORD count=0;
	licFile.Seek(0);
	if(!licFile.Read((LPVOID)&content[0], length, &count) || count != length)
	{
		content.clear();
		return false;
	}

	return true;
}

bool CGNUtil::ReadStringFromFile(const CString& fileName, CString& content)
{
	CAtlFile file;
	if(S_OK == file.Create(fileName,GENERIC_READ, FILE_SHARE_READ,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0))
	{
		ULONGLONG size;
		file.GetSize(size);
		DWORD s = (DWORD)size;
		char* pBuf = new char[s+1];
		DWORD readSize;
		file.Read((LPVOID)pBuf, s+1,readSize);
		ATLASSERT(readSize<=s);
		pBuf[readSize]=0;
		content = pBuf;
		delete[] pBuf;
		file.Close();
		return true;
	}
	
	return false;
}

void CGNUtil::CheckDomError(MSXML2::IXMLDOMDocumentPtr spDoc)
{
#ifndef XML_E_BADCHARDATA
#define XML_E_BADCHARDATA 0xC00CE508
#endif

	MSXML2::IXMLDOMParseErrorPtr spError = spDoc->parseError;
	if(spError == NULL)
		return;

	long errorCode = spError->errorCode;
	if(errorCode != 0) /// && errorCode != XML_E_BADCHARDATA) // ignore bad char
	{
		CString msg;
		_bstr_t reason = spError->reason;
		msg.Format(_T("Invalid channel content.\n\n \
Reason: %s\n \
Line Number: %d \n \
Line Position: %d \n"),
			(LPCTSTR)reason,
			spError->line,
			spError->linepos);

		if(errorCode == XML_E_BADCHARDATA)
			throw CInvalidCharacterException(ERR_NFP_INVALIDNEWSFEED, msg);
		else
			throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, msg);
	}
}


bool CGNUtil::IsInternetUrl(const CString& sUrl)
{
	return (sUrl.Find(_T("http://"))==0 || sUrl.Find(_T("https://"))==0);

}

CString CGNUtil::GetTempPathName()
{
	TCHAR tempPath[MAX_PATH-20];
	DWORD dwRet = ::GetTempPath(sizeof(tempPath)/sizeof(TCHAR), tempPath);
	if(dwRet == 0 || dwRet > sizeof(tempPath)/sizeof(TCHAR))
	{
		// something's wrong
		throw CExceptionBase(ERR_FM_GENERICERR, _T("Cannot find temp directory"));
	}
	if(tempPath[dwRet-1] != _T('\\') && tempPath[dwRet-1] != _T('/'))
	{
		StringCchCat(tempPath, sizeof(tempPath)/sizeof(TCHAR), _T("\\"));
	}

	return tempPath;
}

void CGNUtil::CleanupGNTempFiles()
{
	CString tempPath = CGNUtil::GetTempPathName();

	CFindFile finder;
	BOOL bFound = finder.FindFile(tempPath + _T("~*.htm"));
	while(bFound)
	{
		::DeleteFile(finder.GetFilePath());
		bFound = finder.FindNextFile();
	}
}
bool CGNUtil::StreamToUnicode(std::vector<BYTE>& in, std::vector<wchar_t>& out)
{
	out.clear();
	if(in.size() < 5)
	{
		return false;
	}

	//
	// determine codepage used by in stream
	//
	int codepage = CP_UTF8;
	LPCSTR pIn = (LPCSTR)&in[0];
	int inSize = (int)in.size();

	// check BOM
	//Bytes			Encoding Form 
	//00 00 FE FF	UTF-32, big-endian 
	//FF FE 00 00	UTF-32, little-endian 
	//FE FF			UTF-16, big-endian 
	//FF FE			UTF-16, little-endian 
	//EF BB BF		UTF-8 

	if(in[0]==0 && in[1]==0 && in[2]==0xFE && in[3]==0xFF)
	{
		codepage = 12001; // Unicode UCS-4 Big-Endian
		pIn = (LPCSTR)&in[4];
		inSize -= 4;
	}
	else if(in[0]==0xFF && in[1]==0xFE && in[2]==0 && in[3]==0)
	{
		codepage = 12000;	// Unicode UCS-4 Little-Endian  
		pIn = (LPCSTR)&in[4];
		inSize -= 4;
	}
	else if(in[0]==0xFE && in[1]==0xFF)
	{
		codepage = 1201; // Unicode UCS-2 Big-Endian    
		pIn = (LPCSTR)&in[2];
		inSize -= 2;
	}
	else if(in[0]==0xFF && in[1]==0xFE)
	{
		codepage = 1200; //  Unicode UCS-2 Little-Endian (BMP of ISO 10646)     
		pIn = (LPCSTR)&in[2];
		inSize -= 2;
	}
	else if(in[0]==0xFF && in[1]==0xFE)
	{
		codepage = 1200; //  Unicode UCS-2 Little-Endian (BMP of ISO 10646)     
		pIn = (LPCSTR)&in[2];
		inSize -= 2;
	}
	else if(in[0]==0xEF && in[1]==0xBB && in[2]==0xBF)
	{
		codepage = 65001; // Unicode UTF-8      
		pIn = (LPCSTR)&in[3];
		inSize -= 3;
	}
	else
	{
		// no BOM, let's check if there's xml PI to get encoding
		codepage = FindCodePageInXml(in);
		if(codepage == 0)
			codepage = CP_UTF8;
	}
	


	//
	//
	//
	int bufferSize = MultiByteToWideChar(codepage,
								0,
								pIn,
								inSize,
								0,
								0);
	if(bufferSize==0)
	{
		CString str;
		str.Format(_T("stream to unicode conversion failed.[%d]\n"), GetLastError());
		throw CExceptionBase(ERR_FM_GENERICERR, str);
	}

	out.resize(bufferSize*2);
	int size = MultiByteToWideChar(codepage,
								0,
								pIn,
								inSize,
								&out[0],
								bufferSize*2);

	// ATLASSERT(size == bufferSize);
	if(size == bufferSize) // just in case release build got some problem
	{
		out[bufferSize] = 0; // zero terminate the string
		return true;
	}
	else
	{
		CString str;
		str.Format(_T("stream to unicode conversion failed.[%d]\n"), GetLastError());
		throw CExceptionBase(ERR_FM_GENERICERR, str);
	}
	
	return false;
}


LPCTSTR CGNUtil::GetBrowser()
{
	if(m_browserPath.IsEmpty())
	{
		CString openKeyName = _T("http\\shell\\");
		CRegKey reg;
		if(ERROR_SUCCESS == reg.Open(HKEY_CLASSES_ROOT, _T("http\\shell"), KEY_READ))
		{
			TCHAR entry[251];
			ULONG size = 250;
			if(ERROR_SUCCESS == reg.QueryStringValue(NULL, entry, &size))
			{
				openKeyName += entry;
				openKeyName += _T("\\command");
			}
			else
			{
				openKeyName += _T("open\\command");
			}
		}

		if(ERROR_SUCCESS == reg.Open(HKEY_CLASSES_ROOT, openKeyName, KEY_READ))
		{
			TCHAR path[MAX_PATH+1]={0};
			ULONG size = MAX_PATH;
			if(ERROR_SUCCESS == reg.QueryStringValue(NULL, path, &size))
			{
				m_browserPath = path;
				if(m_browserPath.Find(_T("Opera")) == -1)
				{
					PathRemoveArgs(path);	
					m_browserPath = path;
				}
			}
		}
		reg.Close();
	}

	return m_browserPath;
}

void CGNUtil::OpenUrlExternally(LPCTSTR url, bool bBackGround, bool bNewWindow)
{
	int openFlag = bBackGround ? SW_SHOWNA : SW_SHOWNORMAL;
	CString browserPath = GetBrowser();
	CString encodedUrl = url;
	encodedUrl.Replace(_T(" "), _T("%20"));
	
//* Debugging */	CString msg;
//* Debugging */	msg.Format(_T("URL: %s\nBrowser: %s"), (LPCTSTR)encodedUrl, (LPCTSTR)browserPath);
//* Debugging */	::MessageBox(NULL, msg, _T("Debug: OpenUrlExternally"), MB_OK | MB_ICONINFORMATION);

	if (bNewWindow && !browserPath.IsEmpty())
	{
		::ShellExecute(NULL, NULL, browserPath, encodedUrl, NULL, openFlag);
	}
	else
	{
		::ShellExecute(NULL, NULL, encodedUrl, NULL, NULL, openFlag);
	}
}

CString CGNUtil::EscapeUrl(LPCTSTR url)
{
	CString escapedUrl = url;

	DWORD len=0;
	AtlEscapeUrl(url, escapedUrl.GetBuffer(1024), &len, 1024,ATL_URL_DECODE);
	escapedUrl.ReleaseBuffer();

	return escapedUrl;
}

CString CGNUtil::FixUrl(LPCTSTR url)
{
	CString fixUrl = url;

	int pos = fixUrl.Find(_T("?"));
	if(pos<0)
	{
		fixUrl.Replace(_T("&"), _T("%26"));
	}
	else
	{
		CString website = fixUrl.Left(pos);
		website.Replace(_T("&"), _T("%26"));
		fixUrl = website + fixUrl.Mid(pos);
	}
	return fixUrl;
}

bool CGNUtil::IsValidPoscastingUrl(LPCTSTR podCastingURL)
{
	CUrl url;
	if(!url.CrackUrl(podCastingURL))
		return false;

	CString path = url.GetUrlPath();
	if(path.GetLength())
	{
		CString ext = PathFindExtension(path);
		ext = ext.MakeLower();
		if(m_podCastingExtensions.Find(ext) >=0)
			return true;
	}

	return false;
}


ULONG CGNUtil::CalcCRC32(LPBYTE pBuffer, size_t len)
{
	uLong crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, pBuffer, (uInt)len);

	return crc;
}

void CGNUtil::UnescapeHtml(CString& html)
{
#define ARRAY_SIZE	42
	static TCHAR* escapes[ARRAY_SIZE][2] = 
     {{  _T("&lt;")     , _T("<") } ,
      {  _T("&gt;")     , _T(">") } ,
      {  _T("&quot;")   , _T("\"")} ,
      {  _T("&nbsp;")   , _T(" ") } ,
      {  _T("&amp;")    , _T("&") } ,
      {  _T("&apos;")   , _T("'") } , 
      {  _T("&agrave;") , _T("à") } ,
      {  _T("&Agrave;") , _T("À") } ,
      {  _T("&acirc;")  , _T("â") } ,
      {  _T("&auml;")   , _T("ä") } ,
      {  _T("&Auml;")   , _T("Ä") } ,
      {  _T("&Acirc;")  , _T("Â") } ,
      {  _T("&aring;")  , _T("å") } ,
      {  _T("&Aring;")  , _T("Å") } , 
      {  _T("&aelig;")  , _T("æ") } , 
      {  _T("&AElig;")  , _T("Æ") } ,
      {  _T("&ccedil;") , _T("ç") } ,
      {  _T("&Ccedil;") , _T("Ç") } ,
      {  _T("&eacute;") , _T("é") } ,
      {  _T("&Eacute;") , _T("É") } ,
      {  _T("&egrave;") , _T("è") } ,
      {  _T("&Egrave;") , _T("È") } ,
      {  _T("&ecirc;")  , _T("ê") } ,
      {  _T("&Ecirc;")  , _T("Ê") } ,
      {  _T("&euml;")   , _T("ë") } ,
      {  _T("&Euml;")   , _T("Ë") } ,
      {  _T("&iuml;")   , _T("ï") } , 
      {  _T("&Iuml;")   , _T("Ï") } ,
      {  _T("&ocirc;")  , _T("ô") } ,
      {  _T("&Ocirc;")  , _T("Ô") } ,
      {  _T("&ouml;")   , _T("ö") } ,
      {  _T("&Ouml;")   , _T("Ö") } ,
      {  _T("&oslash;") , _T("ø") } ,
      {  _T("&Oslash;") , _T("Ø") } ,
      {  _T("&szlig;")  , _T("ß") } ,
      {  _T("&ugrave;") , _T("ù") } ,
      {  _T("&Ugrave;") , _T("Ù") } ,
      {  _T("&ucirc;")  , _T("û") } ,
      {  _T("&Ucirc;")  , _T("Û") } , 
      {  _T("&uuml;")   , _T("ü") } ,
      {  _T("&Uuml;")   , _T("Ü") } 
	 };

	if(html.Find(_T("&")) >= 0)
	{
		for(int i = 0; i < ARRAY_SIZE; ++i)
		{
			html.Replace(escapes[i][0], escapes[i][1]);
		}
	}
}

CString CGNUtil::GetLastErrorMsg(LPCTSTR lpszFunction)
{
	TCHAR szBuf[256]; 
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	StringCchPrintf(szBuf, 256,
		_T("%s. Reason[%d: %s]"), 
		lpszFunction, dw, lpMsgBuf); 

	return szBuf;
}
