#include "StdAfx.h"
#include "SearchTool.h"
#include "ExceptionBase.h"
#include "FeedManagerErrorCode.h"

/////////////////////////////////////////////////////
//
//
SearchToolPtr CSearchTool::CreateTool(const CString& searchString)
{
	if(searchString.Find(_T("re:")) == 0)
		return new CRegExpSearchTool(searchString.Mid(3));
	else
		return new CTextSearchTool(searchString);
}


////////////////////////////////////////////////////////////////////
//
//
CTextSearchTool::CTextSearchTool(const CString& searchString)
{
	m_searchString = searchString;
	m_matchedString = searchString;

	m_searchString.MakeLower();
}

bool FindNoCase(const CString& str, const CString& searchStr)
{
	CString temp = str;
	temp.MakeLower();
	return (temp.Find(searchStr)>=0);
}

bool CTextSearchTool::Match(NewsItemPtr& item)
{
	if(!item)
		return false;

	return (FindNoCase(item->m_title, m_searchString)
			|| FindNoCase(item->m_description, m_searchString)
			|| FindNoCase(item->m_author, m_searchString));
}


////////////////////////////////////////////////////////////////////
//
//
CRegExpSearchTool::CRegExpSearchTool(const CString& searchString)
{
	REParseError status = m_re.Parse(searchString, FALSE);
	if(status != REPARSE_ERROR_OK)
		throw new CExceptionBase(ERR_FM_GENERICERR, _T("Invalid regular expression: "+searchString));
}

bool RegMatch(LPCTSTR str, CAtlRegExp<>& re, CString& matched)
{
	CAtlREMatchContext<> mc;
	if(re.Match(str, &mc) == TRUE)
	{
		ptrdiff_t nLength = mc.m_Match.szEnd - mc.m_Match.szStart;
		matched.Format(_T("%.*s"), nLength, mc.m_Match.szStart);

		return true;
	}
	else
	{
		return false;
	}
}

bool CRegExpSearchTool::Match(NewsItemPtr& item)
{
	if(!item)
		return false;

	return (RegMatch(item->m_title, m_re, m_matchedString)
			|| RegMatch(item->m_description, m_re, m_matchedString)
			|| RegMatch(item->m_author, m_re, m_matchedString));
}

