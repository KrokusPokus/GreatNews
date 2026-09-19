#include "StdAfx.h"
#include "AtomFeedParser.h"
#include "GMTimeLib.h"

CAtomFeedParser::CAtomFeedParser(void)
{
	m_newsFeed.m_format = GetFormatName(ATOM030);
}

CAtomFeedParser::~CAtomFeedParser(void)
{
}

void CAtomFeedParser::Parse()
{
	m_newsfeedDoc->setProperty(_T("SelectionNamespaces"),
		_T("xmlns:atom=\"http://purl.org/atom/ns#\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:wfw=\"http://wellformedweb.org/CommentAPI/\" xmlns:feedburner=\"http://rssnamespace.org/feedburner/ext/1.0\""));

	MSXML2::IXMLDOMElementPtr spRoot = m_newsfeedDoc->documentElement;
	ParseNode(m_newsFeed.m_title, spRoot, _T("atom:title"));
	ParseNode(m_newsFeed.m_description, spRoot, _T("atom:tagline"));
	if(m_newsFeed.m_description.GetLength()==0)
	{
		ParseNode(m_newsFeed.m_description, spRoot, _T("atom:info"));
	}
	MSXML2::IXMLDOMElementPtr spModified = spRoot->selectSingleNode("atom:modified");
	if(spModified != NULL)
	{
		m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(spModified->text);
	}

	// try to get the website
	MSXML2::IXMLDOMElementPtr spTemp = spRoot->selectSingleNode(_T("atom:link[@href and @rel=\"alternate\"]"));
	if(spTemp != NULL)
		m_newsFeed.m_website = spTemp->getAttribute(_T("href"));

}

size_t CAtomFeedParser::ExtractNews(NewsItemVector& newsItems)
{
	if(m_newsfeedDoc == NULL)
	{
		throw CExceptionBase(ERR_FM_GENERICERR, _T("NewsFeed Parser hasn't been initialized."));
	}

	newsItems.clear();

	MSXML2::IXMLDOMNodeListPtr itemList = 
		m_newsfeedDoc->selectNodes(_T("/atom:feed/atom:entry"));
	MSXML2::IXMLDOMElementPtr item;
	while(NULL != (item=itemList->nextNode()))
	{
		NewsItemPtr newsItem(new CNewsItem());
	
		if(PopulateNewsItem(newsItem, item))
		{
			newsItems.push_back(newsItem);
		}
	}

	return newsItems.size();

}

bool CAtomFeedParser::PopulateNewsItem(NewsItemPtr pNews, MSXML2::IXMLDOMElementPtr& spItem)
{
	if(!CNewsFeedParserImplMSXML::PopulateNewsItem(pNews, spItem))
		return false;

	ParseNode(pNews->m_title, spItem, _T("atom:title"));
	ParseNode(pNews->m_guid, spItem, _T("atom:id"));
	ExtractContent(pNews->m_description, spItem, _T("atom:content"));
	if(pNews->m_description.GetLength() == 0)
		ExtractContent(pNews->m_description, spItem, _T("atom:summary"));

	if(CNewsFeedParser::m_bKeepOriginalLink)
	{
		ParseNode(pNews->m_url, spItem, _T("feedburner:origLink"));
	}
	if(pNews->m_url.IsEmpty() || !CNewsFeedParser::m_bKeepOriginalLink)
	{
		MSXML2::IXMLDOMElementPtr spTemp = spItem->selectSingleNode(_T("atom:link[@href and @rel='alternate']"));
		if(spTemp == NULL)
		{
			spTemp = spItem->selectSingleNode(_T("atom:link[@href]"));
		}
		if(spTemp != NULL)
			pNews->m_url = spTemp->getAttribute(_T("href"));
	}
	
	MSXML2::IXMLDOMElementPtr spTemp = spItem->selectSingleNode("atom:issued");
	if(spTemp)
		pNews->m_date = CGMTimeHelper::ParseDate((LPCTSTR)spTemp->text);

	return true;
}