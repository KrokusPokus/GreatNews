#include "StdAfx.h"
#include "Rss09xFeedParserPug.h"
#include "GMTimeLib.h"

CRss09xFeedParserPug::CRss09xFeedParserPug(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser)
	: CNewsFeedParserImplPugXml(pContent, pxmlparser)
{
	m_newsFeed.m_format = GetFormatName(RSS09x);
}

CRss09xFeedParserPug::~CRss09xFeedParserPug(void)
{
}

void CRss09xFeedParserPug::Parse()
{
	pug::xml_node channel = m_pXmlParser->document().first_element_by_path(_T("/rss/channel"));
	if(channel.empty())
		return;

	ParseChannelDC(channel);

	ParseNode(m_newsFeed.m_title, channel, _T("./title"));
	ParseNode(m_newsFeed.m_description, channel, _T("./description"));
	ParseNode(m_newsFeed.m_website, channel, _T("./link"),true);
	ParseNode(m_newsFeed.m_language, channel, _T("./language"));
	ParseNode(m_newsFeed.m_image, channel, _T("./image/url"));

	pug::xml_node buildDate = channel.first_element_by_path(_T("./lastBuildDate"));
	if(!buildDate.empty())
	{
		m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(buildDate.child_value());
	}
	else
	{
		pug::xml_node pubDate = channel.first_element_by_path(_T("./pubDate"));
		if(!pubDate.empty())
			m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(pubDate.child_value());
	}

	pug::xml_node ttl = channel.first_element_by_path(_T("./ttl"));
	if(!ttl.empty())
	{
		LPCTSTR value = ttl.child_value();
		if(value)
			m_newsFeed.m_ttl = _ttol(value);
	}
}

size_t CRss09xFeedParserPug::ExtractNews(NewsItemVector& newsItems)
{
	if(m_pXmlParser == NULL)
		throw CExceptionBase(ERR_FM_GENERICERR, _T("NewsFeed Parser hasn't been initialized."));

	newsItems.clear();

	pug::xml_node channel = m_pXmlParser->document().first_element_by_path(_T("/rss/channel"));
	if(channel.empty())
		return 0;
	pug::xml_node_list nodeList;
	channel.all_elements_by_name(_T("item"),nodeList);
	for(unsigned int i=0; i<nodeList.size(); ++i)
	{
		pug::xml_node node = nodeList[i];
		NewsItemPtr newsItem(new CNewsItem());
		if(PopulateNewsItem(newsItem, node))
			newsItems.push_back(newsItem);
	}

	return newsItems.size();
}

bool CRss09xFeedParserPug::PopulateNewsItem(NewsItemPtr& pNews, pug::xml_node& item)
{
	if(!CNewsFeedParserImplPugXml::PopulateNewsItem(pNews, item))
		return false;

	ParseNode(pNews->m_title, item, _T("./title"));
	if(pNews->m_url.IsEmpty() || !CNewsFeedParser::m_bKeepOriginalLink)
		ParseNode(pNews->m_url, item, _T("./link"),true);
	if(pNews->m_description.GetLength()==0)
		ExtractContent(pNews->m_description, item, _T("./description"));
	
	ParseNode(pNews->m_guid, item, _T("./guid"));
	if(pNews->m_guid.GetLength() == 0) // if no guid, we use link to identify the news
		pNews->m_guid = pNews->m_url;
	if(pNews->m_guid.GetLength() == 0) // if no guid and no link, we use news title as guid
		pNews->m_guid = pNews->m_title;

	pug::xml_node node = item.first_element_by_path(_T("./pubDate"));
	if(!node.empty())
		pNews->m_date = CGMTimeHelper::ParseDate(node.child_value());

	return true;
}
