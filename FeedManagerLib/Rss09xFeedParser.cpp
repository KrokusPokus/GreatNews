#include "StdAfx.h"
#include "Rss09xFeedParser.h"
#include "GMTimeLib.h"

CRss09xFeedParser::CRss09xFeedParser(void)
{
	m_newsFeed.m_format = GetFormatName(RSS09x);
}

CRss09xFeedParser::~CRss09xFeedParser(void)
{
}

void CRss09xFeedParser::Parse()
{
	m_newsfeedDoc->setProperty(_T("SelectionNamespaces"), 
		_T("xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:content=\"http://purl.org/rss/1.0/modules/content/\" xmlns:wfw=\"http://wellformedweb.org/CommentAPI/\" xmlns:feedburner=\"http://rssnamespace.org/feedburner/ext/1.0\""));

	MSXML2::IXMLDOMElementPtr spRoot = m_newsfeedDoc->documentElement;
	MSXML2::IXMLDOMElementPtr spChannel = spRoot->selectSingleNode("channel");
	if(spChannel == NULL)
		return;

	ParseChannelDC(spChannel);

	ParseNode(m_newsFeed.m_title, spChannel, _T("title"));
	ParseNode(m_newsFeed.m_description, spChannel, _T("description"));
	ParseNode(m_newsFeed.m_website, spChannel, _T("link"));
	ParseNode(m_newsFeed.m_language, spChannel, _T("language"));
	ParseNode(m_newsFeed.m_image, spChannel, _T("image/url"));

	MSXML2::IXMLDOMElementPtr spBuildDate = spChannel->selectSingleNode("lastBuildDate");
	if(spBuildDate != NULL)
	{
		m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(spBuildDate->text);
	}
	else
	{
		MSXML2::IXMLDOMElementPtr spPubDate = spChannel->selectSingleNode("pubDate");
		if(spPubDate != NULL)
			m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(spPubDate->text);
	}

	MSXML2::IXMLDOMElementPtr spTtl = spChannel->selectSingleNode("ttl");
	if(spTtl != NULL)
	{
		_bstr_t ttl = spTtl->text;
		if(ttl.length())
			m_newsFeed.m_ttl = _ttol((LPCTSTR)ttl);
	}
}

size_t CRss09xFeedParser::ExtractNews(NewsItemVector& newsItems)
{
	if(m_newsfeedDoc == NULL)
		throw CExceptionBase(ERR_FM_GENERICERR, _T("NewsFeed Parser hasn't been initialized."));

	newsItems.clear();

	MSXML2::IXMLDOMNodeListPtr itemList = m_newsfeedDoc->selectNodes(_T("/rss/channel/item"));
	MSXML2::IXMLDOMElementPtr item;
	while(NULL != (item=itemList->nextNode()))
	{
		NewsItemPtr newsItem(new CNewsItem());

		if(PopulateNewsItem(newsItem, item))
			newsItems.push_back(newsItem);
	}

	return newsItems.size();
}

bool CRss09xFeedParser::PopulateNewsItem(NewsItemPtr& pNews, MSXML2::IXMLDOMElementPtr& item)
{
	if(!CNewsFeedParserImplMSXML::PopulateNewsItem(pNews, item))
		return false;

	MSXML2::IXMLDOMElementPtr temp;
	ParseNode(pNews->m_title, item, _T("title"));
	if(pNews->m_url.IsEmpty() || !CNewsFeedParser::m_bKeepOriginalLink)
		ParseNode(pNews->m_url, item, _T("link"));
	if(pNews->m_description.GetLength()==0)
		ExtractContent(pNews->m_description, item, _T("description"));
	
	temp = item->selectSingleNode(_T("guid"));
	pNews->m_guid = (temp != NULL && temp->text.length()>0) ? (LPCTSTR)temp->text : pNews->m_url;
	if(pNews->m_guid.GetLength() == 0) // if no guid and no link, we use news title as guid
		pNews->m_guid = pNews->m_title;

	temp = item->selectSingleNode(_T("pubDate"));
	if(temp)
		pNews->m_date = CGMTimeHelper::ParseDate((LPCTSTR)temp->text);

	return true;
}
