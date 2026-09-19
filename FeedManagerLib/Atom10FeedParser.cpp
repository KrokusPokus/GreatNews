#include "StdAfx.h"
#include "Atom10FeedParser.h"
#include "GMTimeLib.h"
#include "GNUtil.h"

CAtom10FeedParser::CAtom10FeedParser(void)
{
	m_newsFeed.m_format = GetFormatName(ATOM100);
}

CAtom10FeedParser::~CAtom10FeedParser(void)
{
}

void CAtom10FeedParser::Parse()
{
	m_newsfeedDoc->setProperty(_T("SelectionNamespaces"),
		_T("xmlns:atom10=\"http://www.w3.org/2005/Atom\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:wfw=\"http://wellformedweb.org/CommentAPI/\" xmlns:feedburner=\"http://rssnamespace.org/feedburner/ext/1.0\""));

	MSXML2::IXMLDOMElementPtr spRoot = m_newsfeedDoc->documentElement;
	ParseNode(m_newsFeed.m_title, spRoot, _T("atom10:title"));
	ParseNode(m_newsFeed.m_description, spRoot, _T("atom10:subtitle"));
	ParseNode(m_newsFeed.m_image, spRoot, _T("atom10:logo"));

	MSXML2::IXMLDOMElementPtr spModified = spRoot->selectSingleNode("atom10:updated");
	if(spModified != NULL)
	{
		m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(spModified->text);
	}

	// try to get the website
	MSXML2::IXMLDOMElementPtr spTemp = spRoot->selectSingleNode(_T("atom10:link[@href and @rel=\"alternate\"]"));
	if(spTemp == NULL) // if no alternate link, use the one w/o @rel
	{
		spTemp = spRoot->selectSingleNode(_T("atom10:link[@href and not(@rel)]"));
	}
	if(spTemp != NULL)
		m_newsFeed.m_website = spTemp->getAttribute(_T("href"));

	// try to get the id
	CString id;
	ParseNode(id, spRoot, _T("atom10:id"));
	if(m_newsFeed.m_website.GetLength() == 0  // we don't have a <link..> element
		&& id.Find(_T("http://")) == 0)	// but the id starts with http://
	{
		m_newsFeed.m_website = id; // then use the id as home url
	}

	// try to get author
	ParseNode(m_newsFeed.m_author, spRoot, _T("atom10:author/autom10:name"));

}

size_t CAtom10FeedParser::ExtractNews(NewsItemVector& newsItems)
{
	if(m_newsfeedDoc == NULL)
	{
		throw CExceptionBase(ERR_FM_GENERICERR, _T("NewsFeed Parser hasn't been initialized."));
	}

	newsItems.clear();

	MSXML2::IXMLDOMNodeListPtr itemList = 
		m_newsfeedDoc->selectNodes(_T("/atom10:feed/atom10:entry"));
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

bool CAtom10FeedParser::PopulateNewsItem(NewsItemPtr pNews, MSXML2::IXMLDOMElementPtr& spItem)
{
	if(!CNewsFeedParserImplMSXML::PopulateNewsItem(pNews, spItem))
		return false;

	ParseNode(pNews->m_title, spItem, _T("atom10:title"));
	ParseNode(pNews->m_guid, spItem, _T("atom10:id"));
	ParseNode(pNews->m_author, spItem, _T("atom10:author/atom10:name"));
	ExtractContent(pNews->m_description, spItem, _T("atom10:content"));
	if(pNews->m_description.GetLength() == 0)
		ExtractContent(pNews->m_description, spItem, _T("atom10:summary"));

	//
	//
	//
	if(CNewsFeedParser::m_bKeepOriginalLink)
	{
		ParseNode(pNews->m_url, spItem, _T("feedburner:origLink"));
	}
	if(pNews->m_url.IsEmpty() || !CNewsFeedParser::m_bKeepOriginalLink)
	{
		MSXML2::IXMLDOMElementPtr spTemp = spItem->selectSingleNode(_T("atom10:link[@href and @rel='alternate']"));
		if(spTemp == NULL)
		{
			spTemp = spItem->selectSingleNode(_T("atom10:link[@href]"));
		}
		if(spTemp != NULL)
			pNews->m_url = spTemp->getAttribute(_T("href"));
	}

	// _bstr_t xml = spItem->xml;
	_variant_t newsBaseUrl = spItem->getAttribute(_T("xml:base"));
	if(newsBaseUrl.vt != VT_NULL)
	{
		TCHAR szUrl[2048];
		DWORD dwSize = 2048;
		if(InternetCombineUrl (m_newsFeed.m_website,(LPCTSTR)(_bstr_t)newsBaseUrl,szUrl,&dwSize,ICU_BROWSER_MODE))
		{
			dwSize = 2048;
			if(InternetCombineUrl (szUrl,pNews->m_url,szUrl,&dwSize,ICU_BROWSER_MODE))
			{
				pNews->m_url = szUrl;
			}
		}
	}
	
	//
	//
	//
	MSXML2::IXMLDOMElementPtr spTemp = spItem->selectSingleNode("atom10:updated");
	if(spTemp == NULL)
	{
		spTemp = spItem->selectSingleNode("atom10:published");
	}
	if(spTemp)
		pNews->m_date = CGMTimeHelper::ParseDate((LPCTSTR)spTemp->text);

	//
	//
	//
	//spTemp = spItem->selectSingleNode(_T("atom10:link[@href and @rel='alternate']"));
	//if(spTemp == NULL)
	//{
	//	spTemp = spItem->selectSingleNode(_T("atom10:link[@href and not(@rel)]"));
	//}
	//if(spTemp != NULL)
	//	pNews->m_url = spTemp->getAttribute(_T("href"));

	//
	//
	//
	MSXML2::IXMLDOMElementPtr spPodCastingEnclosure = spItem->selectSingleNode(_T("atom10:link[@href and @rel='enclosure']"));
	if(spPodCastingEnclosure)
	{
		_bstr_t value = spPodCastingEnclosure->getAttribute(_T("href"));
		if(CGNUtil::IsValidPoscastingUrl((LPCTSTR)value))
			pNews->m_podCastingURL = (LPCTSTR)value;
	}

	return true;
}