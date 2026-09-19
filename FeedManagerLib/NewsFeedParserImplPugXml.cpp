#include "StdAfx.h"
#include <sstream>
#include "NewsFeedParserImplPugXml.h"
#include "GMTimeLib.h"
#include "GNUtil.h"
#include "Rss09xFeedParserPug.h"
#include "Rss200FeedParserPug.h"
#include "Rss100FeedParserPug.h"
#include "AtomFeedParserPug.h"

CNewsFeedParserImplPugXml::CNewsFeedParserImplPugXml(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser) : 
	m_pContent(pContent), m_pXmlParser(pxmlparser)
{
	ATLASSERT(pContent);
	ATLASSERT(m_pXmlParser);
}

CNewsFeedParserImplPugXml::~CNewsFeedParserImplPugXml(void)
{
	delete m_pXmlParser;
	delete m_pContent;
}


NewsFeedParserPtr CNewsFeedParserImplPugXml::CreateParser(std::vector<BYTE>& stream)
{
	AtlTrace("Using pug parser...\n");

	if(stream.size()<5)
	{
		throw CExceptionBase(ERR_FM_GENERICERR, _T("Invalid rss feed content."));
	}

	std::vector<wchar_t>* pDecoded = new std::vector<wchar_t>();
	std::vector<wchar_t>& decoded = *pDecoded;

	if(!CGNUtil::StreamToUnicode(stream, decoded))
	{
		throw CExceptionBase(ERR_FM_GENERICERR, _T("Unknown rss feed encoding"));
	}
	
	pug::xml_parser* pxmlparser = new pug::xml_parser;
	LPWSTR pText = &decoded[0];
	while(*pText && *pText != '<') 	++pText; // find the beginning of xml

	pxmlparser->parse(pText,16); //16 = parse_cdata;
	pug::xml_node root = pxmlparser->root_element();
	if(root.empty())
	{
		CString msg;
		msg.Format(_T("Invalid channel content"));

		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, msg);
	}

	int format = ParseFormat(root);

	CNewsFeedParserImplPugXml* pParser = CreateParser(format, pDecoded, pxmlparser);
	if(pParser != NULL)
	{
		pParser->Parse();
	}
	else
	{
		delete pDecoded;
		delete pxmlparser;

		CString msg;
		msg.Format(_T("Invalid channel content"));

		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, msg);

	}

	return pParser;
}


int CNewsFeedParserImplPugXml::ParseFormat(pug::xml_node& rootNode)
{
	int format = Unknown;
	try
	{
		CString rootName = rootNode.name();
		CString ver = _T("2.0"); // default to 2.0 if the channel doesn't specify one
		if(rootNode.has_attribute(_T("version")))
		{
			ver = rootNode.attribute(_T("version")).value();
		}

		format = GetFormat(rootName, ver);
	}
	catch(...)
	{
		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, _T("Unknown channel format"));
	}

	return format;
}

CNewsFeedParserImplPugXml* CNewsFeedParserImplPugXml::CreateParser(int format, std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser)
{
	switch(format)
	{
	case RSS09x:
		return new CRss09xFeedParserPug(pContent, pxmlparser);
	case RSS200:
		return new CRss200FeedParserPug(pContent, pxmlparser);
	case RSS100:
		return new CRss100FeedParserPug(pContent, pxmlparser);
	case ATOM030:
		return new CAtomFeedParserPug(pContent, pxmlparser);
	default:
		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, _T("Unknown channel format"));
	}
	

	return NULL;
}

bool CNewsFeedParserImplPugXml::PopulateNewsItem(NewsItemPtr& pNews, pug::xml_node& item)
{
	// dc elements

	// dc:date is not the modification timestamp? seems http://www.exblog.jp/ got it wrong?
	if(m_newsFeed.m_url.Find(_T(".exblog.jp"))==-1)
	{
		pug::xml_node temp = item.first_element_by_path(_T("./dc:date"));
		if(!temp.empty())
			pNews->m_date = CGMTimeHelper::ParseDate(temp.child_value());
	}

	ParseNode(pNews->m_author, item, _T("./dc:creator"));

	// content elements
	ExtractContent(pNews->m_description, item, _T("./content:encoded"));

	// wfw elements
	ParseNode(pNews->m_commentFeedURL, item, _T("./wfw:commentRSS"));

	// original link
	if(CNewsFeedParser::m_bKeepOriginalLink)
	{
		ParseNode(pNews->m_url, item, _T("./feedburner:origLink"),true);
	}

	return true;
}


//
//	common utility functions used by derived feed parsers
//
bool CNewsFeedParserImplPugXml::ParseNode(CString& value, pug::xml_node& node, LPCTSTR xpath, bool bUnescape)
{
	pug::xml_node temp = node.first_element_by_path(xpath);
	if(!temp.empty())
	{
		value = temp.child_value();
		if(bUnescape)
		{
			value.Replace(_T("&amp;"), _T("&"));
			//value.Replace(_T("&lt;"), _T("<"));
			//value.Replace(_T("&gt;"), _T(">"));
		}
		return true;
	}
	else
		return false;
}

bool CNewsFeedParserImplPugXml::ExtractContent(CString& value, pug::xml_node& node, LPCTSTR xpath)
{
	pug::xml_node temp = node.first_element_by_path(xpath);
	if(!temp.empty())
	{
		value = ExtractContent(temp);
		return true;
	}
	else
		return false;
}

CString CNewsFeedParserImplPugXml::ExtractContent(pug::xml_node& node)
{
	if(!node.has_child_nodes())
		return _T("");

	pug::xml_node::iterator it = node.begin();
	if(it->type() == pug::node_cdata)
		return it->value();
	else if(it->type() == pug::node_pcdata)
	{
		// need to unescape html tags
		CString str = it->value();
		str.Replace(_T("&amp;"), _T("&"));
		str.Replace(_T("&lt;"), _T("<"));
		str.Replace(_T("&gt;"), _T(">"));
		str.Replace(_T("&quot;"), _T("\""));
		return str;
	}

	// if we arrive here, then we have html elements directly inside the tag
	//		eg:  <content><p>blalblb</p></content
	// we have to keep the html tag, to return <p>blblablb</p>
	std::wostringstream ostream;
	node.outer_xml(ostream, _T('\0'), false);
	CString str = ostream.str().c_str();

	int nStart = str.Find(_T('>'));
	int nEnd = str.ReverseFind(_T('<'));
	if(nStart>0 && nStart < nEnd)
		str = str.Mid(nStart+1, nEnd-nStart-1);
	else
		str.Empty();

	return (LPCTSTR)str;
}

bool CNewsFeedParserImplPugXml::ParseChannelDC(pug::xml_node& channel) // parse channel's dc elements
{
	ParseNode(m_newsFeed.m_language, channel, _T("./dc:language"));
	pug::xml_node node = channel.first_element_by_path(_T("./dc:date"));
	if(!node.empty())
	{
		m_newsFeed.m_lastModified = CGMTimeHelper::ParseDate(node.child_value());
	}
	ParseNode(m_newsFeed.m_author, channel, _T("./dc:creator"));

	return true;
}