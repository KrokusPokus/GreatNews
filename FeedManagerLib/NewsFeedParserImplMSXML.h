#pragma once

#include "loki\SmartPtr.h"
#include "ExceptionBase.h"
#include "FeedManagerErrorCode.h"
#include "NewsItem.h"
#include "NewsFeed.h"
#include "NewsFeedParser.h"

#import "msxml3.dll"

class CNewsFeedParserImplMSXML : public CNewsFeedParser
{
public:
	CNewsFeedParserImplMSXML(void);
	virtual ~CNewsFeedParserImplMSXML(void);

	static NewsFeedParserPtr CreateParser(std::vector<BYTE>& stream);

	virtual size_t ExtractNews(NewsItemVector&)=0;

protected:
	static int ParseFormat(MSXML2::IXMLDOMDocument2Ptr& newsfeedDoc);
	static CNewsFeedParserImplMSXML* CreateParser(int format);
	static CNewsFeedParserImplMSXML* CreateParser(MSXML2::IXMLDOMDocument2Ptr& spXMLDoc);

	virtual void Parse()=0;

protected:
	virtual bool PopulateNewsItem(NewsItemPtr& pNews, MSXML2::IXMLDOMElementPtr& item);

	bool ParseNode(CString& value, MSXML2::IXMLDOMElementPtr& spNode, LPCTSTR xpath);
	bool ParseChannelDC(MSXML2::IXMLDOMElementPtr& spChannel); // parse channel's dc elements
	bool ExtractContent(CString& value, MSXML2::IXMLDOMElementPtr& spNode, LPCTSTR xpath);
	_bstr_t ExtractContent(MSXML2::IXMLDOMElementPtr& spNode);

protected:
	MSXML2::IXMLDOMDocument2Ptr m_newsfeedDoc;
};

