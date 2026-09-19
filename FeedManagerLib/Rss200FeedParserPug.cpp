#include "StdAfx.h"
#include "Rss200FeedParserPug.h"
#include "GNUtil.h"

CRss200FeedParserPug::CRss200FeedParserPug(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser)
	: CRss09xFeedParserPug(pContent, pxmlparser)
{
	m_newsFeed.m_format = GetFormatName(RSS200);
}

CRss200FeedParserPug::~CRss200FeedParserPug(void)
{
}

void CRss200FeedParserPug::Parse()
{
	CRss09xFeedParserPug::Parse();
}

bool CRss200FeedParserPug::PopulateNewsItem(NewsItemPtr& pNews, pug::xml_node& item)
{
	CRss09xFeedParserPug::PopulateNewsItem(pNews, item);

	ParseNode(pNews->m_author, item, _T("./author"));
	ParseNode(pNews->m_commentsURL, item, _T("./comments"));

	// get pod casting enclosure URL
	pug::xml_node pod_casting = item.first_element_by_attribute(_T("enclosure"), _T("type"), _T("audio/mpeg"));
	if(!pod_casting.empty() && pod_casting.has_attribute(_T("url")))
		if(CGNUtil::IsValidPoscastingUrl(pod_casting.attribute(_T("url")).value()))
			pNews->m_podCastingURL = pod_casting.attribute(_T("url")).value();

	return true;
}
