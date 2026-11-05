#include "StdAfx.h"
#include "BlogToolType.h"
#include "FeedManager.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "FeedManagerLibHelper.h"
#include "GNUtil.h"

CBlogToolType::CBlogToolType(void)
{
	m_type = Unknown;
	m_hPlugin = NULL;
	m_pGetNameProc = NULL;
	m_pBlogThisProc = NULL;
}

CBlogToolType::CBlogToolType(LPCTSTR name, LPCTSTR url, Type type)
{
	m_name = name;
	m_defaultURL = url;
	m_type = type;

	m_hPlugin = NULL;
	m_pGetNameProc = NULL;
	m_pBlogThisProc = NULL;
}

CBlogToolType::CBlogToolType(LPCTSTR pluginPath)
{
	m_type = Unknown;
	m_hPlugin = NULL;
	m_pGetNameProc = NULL;
	m_pBlogThisProc = NULL;

	try
	{
		m_hPlugin = ::LoadLibrary(pluginPath);
		if(m_hPlugin == NULL)
			return;

		m_pGetNameProc = (GET_NAME_PROC)::GetProcAddress(m_hPlugin, "GetGreatNewsPluginName");
		m_pBlogThisProc = (BLOG_THIS_PROC)::GetProcAddress(m_hPlugin, "BlogThis");
		
		m_name = m_pGetNameProc();

		m_type = Plugin; // everything is all set, let's set the type
	}
	catch(...)
	{
	}
}

CBlogToolType::~CBlogToolType(void)
{
	if(m_hPlugin)
		::FreeLibrary(m_hPlugin);
}

size_t CBlogToolType::GetAllBlogToolTyes(BlogToolTypeVector& allTypes, const CString& pluginPath)
{
	allTypes.clear();
	allTypes.reserve(20);

	//
	// load all builtin blog tools
	//
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql = _T("SELECT tool_type_name, default_url FROM blog_tool_type;");
		CppSQLite3Query q = db.execQuery(sql);
		while(!q.eof())
		{
			allTypes.push_back(new CBlogToolType(q.getStringField(0),q.getStringField(1)));
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	// Add this special one
	allTypes.push_back(new CBlogToolGenericExternalType());


	//
	// Load all plugins
	//
	CFindFile finder;
	BOOL bFound = finder.FindFile(pluginPath + _T("\\*.dll"));
	while(bFound)
	{
		if(IsGreatNewsPlugin(finder.GetFilePath()))
			allTypes.push_back(new CBlogToolType(finder.GetFilePath()));
		bFound = finder.FindNextFile();
	}

	return allTypes.size();
}

bool CBlogToolType::IsGreatNewsPlugin(LPCTSTR filePath)
{
	try
	{
		HMODULE hInst = ::LoadLibrary(filePath);
		if(hInst == NULL)
			return false;

		FARPROC procName = ::GetProcAddress(hInst, "GetGreatNewsPluginName");
		FARPROC procBlogThis = ::GetProcAddress(hInst, "BlogThis");

		bool bValid = (procName != NULL && procBlogThis != NULL);

		::FreeLibrary(hInst);

		return bValid;

	}
	catch(...)
	{
		return false;
	}

	return false;
}

void CBlogToolType::BlogThis(NewsItemPtr& item, LPCTSTR pluginOptions)
{
	ATLASSERT(m_pBlogThisProc);

	MSXML2::IXMLDOMDocumentPtr spDoc = item->GenerateFeedXml();
	
	CString xml = (LPCTSTR)spDoc->xml;

	m_pBlogThisProc(xml, pluginOptions);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//
CBlogToolGenericExternalType::CBlogToolGenericExternalType()
: CBlogToolType(_T("Generic external blog tool"),
				_T("<Executable path to the tool and command line parameters>"),
				GenericExternal)
{
}

// Makes a value coming from a feed safe for use inside a (quoted) command line argument:
// it must not be able to close the quote and start additional arguments.
static CString QuoteCmdLineValue(const CString& value, int maxLength)
{
	CString result;
	int count = min(value.GetLength(), maxLength);
	for(int i = 0; i < count; ++i)
	{
		TCHAR c = value[i];
		if(c == _T('"'))
			result += _T("\\\"");
		else if(c == _T('\r') || c == _T('\n'))
			result += _T(' ');
		else
			result.AppendChar(c);
	}

	// a trailing backslash would escape the closing quote of the template
	int trailing = 0;
	for(int i = result.GetLength() - 1; i >= 0 && result[i] == _T('\\'); --i)
		++trailing;
	if(trailing % 2 != 0)
		result += _T('\\');

	return result;
}

void CBlogToolGenericExternalType::BlogThis(NewsItemPtr& item, LPCTSTR pluginOptions)
{
	CString cmdLine = pluginOptions;
	const bool bNeedHtmlFile = (cmdLine.Find(_T("%HTMLFILE%")) >= 0);

	cmdLine.Replace(_T("%TITLE%"), QuoteCmdLineValue(item->GetTitle(), 1000));
	cmdLine.Replace(_T("%AUTHOR%"), QuoteCmdLineValue(item->m_author, 1000));
	cmdLine.Replace(_T("%LINK%"), QuoteCmdLineValue(item->GetLink(), 2000));
	cmdLine.Replace(_T("%TEXT%"), QuoteCmdLineValue(item->m_description, 8000));

	if(bNeedHtmlFile)
	{
		CString htmlContent;
		htmlContent.Format(_T("<blockquote><a href=\"%s\">%s</a><br>%s</blockquote>"), (LPCTSTR)item->GetLink(), (LPCTSTR)item->GetTitle(),
		(LPCTSTR)item->m_description);

		TCHAR szTempPath[MAX_PATH];
		TCHAR szTempFileName[MAX_PATH];
		DWORD dwLen = ::GetTempPath(MAX_PATH, szTempPath);
		if(dwLen == 0 || dwLen >= MAX_PATH || 0 == ::GetTempFileName(szTempPath, _T("w"), 0, szTempFileName))
			return;
		CString fileName = szTempFileName;
		CGNUtil::WriteToFile(fileName, htmlContent);
		cmdLine.Replace(_T("%HTMLFILE%"), fileName);
	}

	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	// CreateProcess may modify the command line, so it needs a writable buffer of its own
	LPTSTR pszCmdLine = cmdLine.GetBuffer(cmdLine.GetLength() + 1);
	BOOL bCreated = ::CreateProcess(NULL, pszCmdLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
	cmdLine.ReleaseBuffer();

	if(bCreated)
	{
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
	}
}
