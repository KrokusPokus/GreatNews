#include "StdAfx.h"
#include "AtomFeedParserPug.h"
#include "GMTimeLib.h"

CAtomFeedParserPug::CAtomFeedParserPug(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser)
	: CNewsFeedParserImplPugXml(pContent, pxmlparser)
{
	m_newsFeed.m_format = GetFormatName(ATOM030);
}

CAtomFeedParserPug::~CAtomFeedParserPug(void)
{
}

void CAtomFeedParserPug::Parse()
{
	pug::xml_node root = m_pXmlParser->root_element();
	ParseNode(m_newsFeed.m_title, root, _T("./title"));
	ParseNode(m_newsFeed.m_description, root, _T("./tagline"));
	if(m_newsFeed.m_description.GetLength()==0)
	{
		ParseNode(m_newsFeed.m_description, root, _T("./info"));
	}
	pug::xml_node modified = root.first_element_by_path(_T("./modified"));
	if(!modified.empty())
	{
		m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(modified.child_value());
	}

	// try to get the website
	pug::xml_node alter_link = root.first_element_by_attribute(_T("link"), _T("rel"), _T("alternate"));
	if(!alter_link.empty() && alter_link.has_attribute(_T("href")))
	{
		m_newsFeed.m_website = alter_link.attribute(_T("href")).value();
	}
}

size_t CAtomFeedParserPug::ExtractNews(NewsItemVector& newsItems)
{
	if(m_pXmlParser == NULL)
	{
		throw CExceptionBase(ERR_FM_GENERICERR, _T("NewsFeed Parser hasn't been initialized."));
	}

	newsItems.clear();

	pug::xml_node_list entries;
	m_pXmlParser->root_element().all_elements_by_name(_T("entry"),entries);
	for(unsigned int i=0; i < entries.size(); ++i)
	{
		pug::xml_node node = entries[i];
		NewsItemPtr newsItem(new CNewsItem());

		if(PopulateNewsItem(newsItem, node))
		{
			newsItems.push_back(newsItem);
		}

	}

	return newsItems.size();
}

bool CAtomFeedParserPug::PopulateNewsItem(NewsItemPtr pNews, pug::xml_node& item)
{
	ParseNode(pNews->m_title, item, _T("./title"));
	ParseNode(pNews->m_guid, item, _T("./id"));
	ExtractContent(pNews->m_description, item, _T("./content"));
	if(pNews->m_description.GetLength() == 0)
		ExtractContent(pNews->m_description, item, _T("./summary"));

	pug::xml_node alter_link = item.first_element_by_attribute(_T("link"), _T("rel"), _T("alternate"));
	if(!alter_link.empty() && alter_link.has_attribute(_T("href")))
	{
		pNews->m_url = alter_link.attribute(_T("href")).value();
	}
	else
	{
		alter_link = item.first_element_by_path(_T("./link"));
		if(!alter_link.empty() && alter_link.has_attribute(_T("href")))
			pNews->m_url = alter_link.attribute(_T("href")).value();
	}

	pug::xml_node temp = item.first_element_by_path(_T("./issued"));
	if(!temp.empty())
		pNews->m_date = CGMTimeHelper::ParseDate(temp.child_value());

	return true;
}