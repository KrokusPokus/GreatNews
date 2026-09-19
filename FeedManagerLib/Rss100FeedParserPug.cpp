#include "StdAfx.h"
#include "Rss100FeedParserPug.h"
#include "GMTimeLib.h"

CRss100FeedParserPug::CRss100FeedParserPug(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser)
	: CNewsFeedParserImplPugXml(pContent, pxmlparser)
{
	m_newsFeed.m_format = GetFormatName(RSS100);
}

CRss100FeedParserPug::~CRss100FeedParserPug(void)
{
}

void CRss100FeedParserPug::Parse()
{
	pug::xml_node root = m_pXmlParser->root_element();
	pug::xml_node channel = root.first_element_by_path(_T("./channel"));
	if(channel.empty())
		return;

	ParseChannelDC(channel);

	ParseNode(m_newsFeed.m_title, channel, _T("./title"));
	ParseNode(m_newsFeed.m_description, channel, _T("./description"));
	ParseNode(m_newsFeed.m_website, channel, _T("./link"),true);

	pug::xml_node temp = channel.first_element_by_path(_T("./image"));
	if(!temp.empty() && temp.has_attribute(_T("rdf:resource")))
		m_newsFeed.m_image = temp.attribute(_T("rdf:resource")).value();
}

size_t CRss100FeedParserPug::ExtractNews(NewsItemVector& newsItems)
{
	if(m_pXmlParser == NULL)
		throw CExceptionBase(ERR_FM_GENERICERR, _T("NewsFeed Parser hasn't been initialized."));

	newsItems.clear();

	pug::xml_node_list items;
	m_pXmlParser->root_element().all_elements_by_name(_T("item"),items);
	for(unsigned int i=0; i < items.size(); ++i)
	{
		pug::xml_node item = items[i];
		NewsItemPtr newsItem(new CNewsItem());

		CNewsFeedParserImplPugXml::PopulateNewsItem(newsItem, item);

		ParseNode(newsItem->m_title, item, _T("./title"));
		if(newsItem->m_url.IsEmpty() || !CNewsFeedParser::m_bKeepOriginalLink)
			ParseNode(newsItem->m_url, item, _T("./link"),true);
		if(newsItem->m_description.GetLength()==0)
			ParseNode(newsItem->m_description, item, _T("./description"));
		
		pug::xml_node temp = item.first_element_by_path(_T("./guid"));
		if(!temp.empty())
			newsItem->m_guid = temp.child_value();
		if(newsItem->m_guid.GetLength()==0) // if no guid, use link as guid
			newsItem->m_guid = newsItem->m_url;
		if(newsItem->m_guid.GetLength()==0) // if no guid and no link, we use news title as guid
			newsItem->m_guid = newsItem->m_title;

		newsItems.push_back(newsItem);
	}

	return newsItems.size();
}
