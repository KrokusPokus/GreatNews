#pragma once

#include <vector>
#include "loki\SmartPtr.h"

#define SEARCH_SCHEME _T("Search://")

class CSearchChannelDef;
typedef std::vector<CSearchChannelDef> SearchChannelDefVector;

class CSearchChannelDef
{
public:
	CSearchChannelDef(LPCTSTR name, LPCTSTR url) : m_name(name), m_urlPattern(url) {}

	CString m_name;
	CString m_urlPattern;

public:
	static bool IsSearchChannelUrl(const CString& url);
	static bool CrackSearchUrl(const CString& url, CString& searchChannel, CString& keyword);
	static void AssembleSearchUrl(CString& url, const CString& searchChannel, const CString& keyword);
	static bool ExpandUrl(CString& url);
	static CString GetUrlPattern(const CString& channelName);

public:
	static SearchChannelDefVector m_searchChannelDefs;
};


