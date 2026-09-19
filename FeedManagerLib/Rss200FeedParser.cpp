#include "StdAfx.h"
#include "Rss200FeedParser.h"
#include "GNUtil.h"

CRss200FeedParser::CRss200FeedParser(void)
{
	m_newsFeed.m_format = GetFormatName(RSS200);
}

CRss200FeedParser::~CRss200FeedParser(void)
{
}

void CRss200FeedParser::Parse()
{
	CRss09xFeedParser::Parse();
}

bool CRss200FeedParser::PopulateNewsItem(NewsItemPtr& pNews, MSXML2::IXMLDOMElementPtr& item)
{
	CRss09xFeedParser::PopulateNewsItem(pNews, item);

	ParseNode(pNews->m_author, item, _T("author"));
	ParseNode(pNews->m_commentsURL, item, _T("comments"));

	// get pod casting enclosure URL
	//MSXML2::IXMLDOMElementPtr spPodCastingEnclosure = item->selectSingleNode(_T("enclosure[@type='audio/mpeg' and @url]"));
	MSXML2::IXMLDOMElementPtr spPodCastingEnclosure = item->selectSingleNode(_T("enclosure[@url]"));
	if(spPodCastingEnclosure)
	{
		_bstr_t value = spPodCastingEnclosure->getAttribute(_T("url"));
		if(CGNUtil::IsValidPoscastingUrl((LPCTSTR)value))
			pNews->m_podCastingURL = (LPCTSTR)value;
	}

	return true;
}
