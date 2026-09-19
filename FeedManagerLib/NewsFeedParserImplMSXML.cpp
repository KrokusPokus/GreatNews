#include "StdAfx.h"
#include "NewsFeedParserImplMSXML.h"
#include "SafeArrayVariant.h"
#include "Rss09xFeedParser.h"
#include "Rss200FeedParser.h"
#include "Rss100FeedParser.h"
#include "AtomFeedParser.h"
#include "Atom10FeedParser.h"
#include "GMTimeLib.h"
#include "GNUtil.h"

CNewsFeedParserImplMSXML::CNewsFeedParserImplMSXML(void)
{
}

CNewsFeedParserImplMSXML::~CNewsFeedParserImplMSXML(void)
{
}

NewsFeedParserPtr CNewsFeedParserImplMSXML::CreateParser(std::vector<BYTE>& stream)
{
	if(stream.size() == 0)
		return NULL;

	CSafeArrayVariant v(&stream[0], (int)stream.size());

	MSXML2::IXMLDOMDocument2Ptr spDoc(_T("MSXML2.DOMDocument.3.0"));
	spDoc->validateOnParse = false; // do not validate DTD/Schema
	spDoc->resolveExternals = false;
	spDoc->load(v);

	CGNUtil::CheckDomError(spDoc);

	return CreateParser(spDoc);	
}

CNewsFeedParserImplMSXML* CNewsFeedParserImplMSXML::CreateParser(MSXML2::IXMLDOMDocument2Ptr& spXMLDoc)
{
	int format = ParseFormat(spXMLDoc);
	CNewsFeedParserImplMSXML* pParser = CreateParser(format);
	if(pParser != NULL)
	{
		pParser->m_newsfeedDoc = spXMLDoc;
		pParser->Parse();
	}

	return pParser;
}

int CNewsFeedParserImplMSXML::ParseFormat(MSXML2::IXMLDOMDocument2Ptr& newsfeedDoc)
{
	int format = Unknown;
	try
	{
#ifdef DEBUG
		_bstr_t xml = newsfeedDoc->xml;
#endif

		MSXML2::IXMLDOMElementPtr spRoot = newsfeedDoc->documentElement;
		_bstr_t rootName = spRoot->tagName;
		_bstr_t ver;
		MSXML2::IXMLDOMAttributePtr verAttr = spRoot->getAttributeNode(_T("version"));
		if(verAttr != NULL)
		{
			ver = verAttr->text;
		}

		format = GetFormat((LPCTSTR)rootName, (LPCTSTR)ver);
	}
	catch(...)
	{
		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, _T("Unknown channel format"));
	}

	return format;
}

CNewsFeedParserImplMSXML* CNewsFeedParserImplMSXML::CreateParser(int format)
{
	switch(format)
	{
	case RSS09x:
		return new CRss09xFeedParser();
	case RSS200:
		return new CRss200FeedParser();
	case RSS100:
		return new CRss100FeedParser();
	case ATOM030:
		return new CAtomFeedParser();
	case ATOM100:
		return new CAtom10FeedParser();
	default:
		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, _T("Unknown channel format"));
	}
	

	return NULL;
}



//
//	common utility functions used by derived feed parsers
//
bool CNewsFeedParserImplMSXML::ParseNode(CString& value, MSXML2::IXMLDOMElementPtr& spNode, LPCTSTR xpath)
{
	MSXML2::IXMLDOMElementPtr spTemp = spNode->selectSingleNode(xpath);
	if(spTemp != NULL)
	{
		CString temp = (LPCTSTR)spTemp->text;
		if(temp.GetLength() != 0)
			value = temp;
		return true;
	}
	else
		return false;

}

bool CNewsFeedParserImplMSXML::ParseChannelDC(MSXML2::IXMLDOMElementPtr& spChannel) // parse channel's dc elements
{
	// assumes namespace has set already
	ParseNode(m_newsFeed.m_language, spChannel, _T("dc:language"));

	// dc:date is not the modification timestamp? seems http://www.exblog.jp/ got it wrong?
	if(m_newsFeed.m_url.Find(_T(".exblog.jp"))==-1)
	{
		MSXML2::IXMLDOMElementPtr spModified = spChannel->selectSingleNode("dc:date");
		if(spModified != NULL)
		{
			m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(spModified->text);
		}
	}

	ParseNode(m_newsFeed.m_author, spChannel, _T("dc:creator"));

	return true;
}

bool CNewsFeedParserImplMSXML::PopulateNewsItem(NewsItemPtr& pNews, MSXML2::IXMLDOMElementPtr& item)
{
#ifdef DEBUG
	_bstr_t xml = item->xml;
#endif

	// dc elements
	MSXML2::IXMLDOMElementPtr spTemp = item->selectSingleNode(_T("dc:date"));
	if(spTemp)
		pNews->m_date = CGMTimeHelper::ParseDate((LPCTSTR)spTemp->text);

	ParseNode(pNews->m_author, item, _T("dc:creator"));

	// content elements
	ExtractContent(pNews->m_description, item, _T("content:encoded"));

	// wfw elements
	ParseNode(pNews->m_commentFeedURL, item, _T("wfw:commentRSS"));
	ParseNode(pNews->m_commentFeedURL, item, _T("wfw:commentRss"));

	// orginal url
	if(CNewsFeedParser::m_bKeepOriginalLink)
	{
		ParseNode(pNews->m_url, item, _T("feedburner:origLink"));
	}

	return true;
}

bool CNewsFeedParserImplMSXML::ExtractContent(CString& value, MSXML2::IXMLDOMElementPtr& spNode, LPCTSTR xpath)
{
	MSXML2::IXMLDOMElementPtr spTemp = spNode->selectSingleNode(xpath);
	if(spTemp != NULL)
	{
		_bstr_t temp = ExtractContent(spTemp);
		if(temp.length() != 0)
			value = (LPCTSTR)temp;
		return true;
	}
	else
		return false;
}

_bstr_t CNewsFeedParserImplMSXML::ExtractContent(MSXML2::IXMLDOMElementPtr& spNode)
{
#ifdef DEBUG
	_bstr_t peek = spNode->xml;
#endif

	bool hasElement = false;
	MSXML2::IXMLDOMNodePtr child = spNode->firstChild;
	while(child != NULL)
	{
		if(child->nodeType == NODE_ELEMENT)
		{
			hasElement = true;
			break;
		}
		child = child->nextSibling;
	}

	if(!hasElement)
	{
		return spNode->text;
	}
	else
	{
		// if html is directly inside the node, we have to keep the html tags.....
		CString str = (LPCTSTR)spNode->xml;
		int nStart = str.Find(_T('>'));
		int nEnd = str.ReverseFind(_T('<'));
		if(nStart>0 && nStart < nEnd)
			str = str.Mid(nStart+1, nEnd-nStart-1);
		else
			str.Empty();

		return (LPCTSTR)str;
	}
}
