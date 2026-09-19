#include "StdAfx.h"
#include "Rss100FeedParser.h"
#include "GMTimeLib.h"

CRss100FeedParser::CRss100FeedParser(void)
{
	m_newsFeed.m_format = GetFormatName(RSS100);
}

CRss100FeedParser::~CRss100FeedParser(void)
{
}

void CRss100FeedParser::Parse()
{
	m_newsfeedDoc->setProperty(_T("SelectionNamespaces"),
		_T("xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" xmlns:a=\"http://purl.org/rss/1.0/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:content=\"http://purl.org/rss/1.0/modules/content/\" xmlns:wfw=\"http://wellformedweb.org/CommentAPI/\" xmlns:feedburner=\"http://rssnamespace.org/feedburner/ext/1.0\" "));

	MSXML2::IXMLDOMElementPtr spRoot = m_newsfeedDoc->documentElement;
	MSXML2::IXMLDOMElementPtr spChannel = spRoot->selectSingleNode("a:channel");
	if(spChannel == NULL)
	{
		// try another default namespace
		m_newsfeedDoc->setProperty(_T("SelectionNamespaces"),
			_T("xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" xmlns:a=\"http://my.netscape.com/rdf/simple/0.9/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:content=\"http://purl.org/rss/1.0/modules/content/\" xmlns:wfw=\"http://wellformedweb.org/CommentAPI/\" xmlns:feedburner=\"http://rssnamespace.org/feedburner/ext/1.0\""));

		spChannel = spRoot->selectSingleNode("a:channel");
		if(spChannel == NULL)
			return;
	}

	ParseChannelDC(spChannel);

	ParseNode(m_newsFeed.m_title, spChannel, _T("a:title"));
	ParseNode(m_newsFeed.m_description, spChannel, _T("a:description"));
	ParseNode(m_newsFeed.m_website, spChannel, _T("a:link"));

	MSXML2::IXMLDOMElementPtr spTemp = spChannel->selectSingleNode("a:image[@rdf:resource]");
	if(spTemp != NULL)
		m_newsFeed.m_image = (LPCTSTR)(_bstr_t)spTemp->getAttribute("rdf:resource");
}

size_t CRss100FeedParser::ExtractNews(NewsItemVector& newsItems)
{
	if(m_newsfeedDoc == NULL)
		throw CExceptionBase(ERR_FM_GENERICERR, _T("NewsFeed Parser hasn't been initialized."));

	newsItems.clear();

	MSXML2::IXMLDOMNodeListPtr itemList = m_newsfeedDoc->selectNodes(_T("/rdf:RDF/a:item"));
	MSXML2::IXMLDOMElementPtr item,temp;
	while(NULL != (item=itemList->nextNode()))
	{
		NewsItemPtr newsItem(new CNewsItem());

		CNewsFeedParserImplMSXML::PopulateNewsItem(newsItem, item);

		ParseNode(newsItem->m_title, item, _T("a:title"));
		if(newsItem->m_url.IsEmpty() || !CNewsFeedParser::m_bKeepOriginalLink)
			ParseNode(newsItem->m_url, item, _T("a:link"));
		if(newsItem->m_description.GetLength()==0)
			ParseNode(newsItem->m_description, item, _T("a:description"));
		
		temp = item->selectSingleNode(_T("guid"));
		newsItem->m_guid = temp ? (LPCTSTR)temp->text : newsItem->m_url;
		if(newsItem->m_guid.GetLength() == 0) // if no guid and no link, we use news title as guid
			newsItem->m_guid = newsItem->m_title;

		newsItems.push_back(newsItem);
	}

	return newsItems.size();
}
