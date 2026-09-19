#include "StdAfx.h"
#include "SearchChannelDef.h"
#include "GNUtil.h"

SearchChannelDefVector CSearchChannelDef::m_searchChannelDefs;

bool CSearchChannelDef::IsSearchChannelUrl(const CString& url)
{
	return (url.Find(SEARCH_SCHEME) == 0);
}
bool CSearchChannelDef::CrackSearchUrl(const CString& url, CString& searchChannel, CString& keyword)
{
	// parse search channels
	// search URL looks like: Search://ChannelName/Key word
	int nLen = (int)_tcslen(SEARCH_SCHEME);
	int nPos = url.Find(_T('/'),nLen);
	if(nPos>nLen)
	{
		searchChannel = url.Mid(nLen, nPos-nLen);
		keyword = url.Right(url.GetLength() - nPos -1);
		//keyword.Replace(_T('+'), _T(' '));
		keyword = CGNUtil::EscapeUrl(keyword);

		return true;
	}
	return false;
}

void CSearchChannelDef::AssembleSearchUrl(CString& url, const CString& searchChannel, const CString& keyword)
{
	// make the url string:
	CString str = keyword;
	str.Replace(_T(' '), _T('+'));
	url = SEARCH_SCHEME + searchChannel + _T("/")+str;
}

bool CSearchChannelDef::ExpandUrl(CString& url)
{
	CString channelName, keyword;
	if(!CrackSearchUrl(url, channelName, keyword))
		return false;

	url = GetUrlPattern(channelName);
	if(url.GetLength()==0)
		return false;

	url.Replace(_T("%KEYWORD%"), keyword);

	return true;
}

CString CSearchChannelDef::GetUrlPattern(const CString& channelName)
{
	for(std::vector<CSearchChannelDef>::iterator it=m_searchChannelDefs.begin();
		it!=m_searchChannelDefs.end(); ++it)
	{
		CSearchChannelDef& def = *it;
		if(def.m_name == channelName)
			return def.m_urlPattern;
	}
	return _T("");
}
