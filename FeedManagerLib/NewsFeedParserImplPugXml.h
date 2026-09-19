#pragma once

#include <vector>
#include "NewsFeedParser.h"

#define PUGAPI_VARIANT 0x58475550	//The Pug XML library variant we are using in this implementation.
#define PUGAPI_VERSION_MAJOR 1		//The Pug XML library major version we are using in this implementation.
#define PUGAPI_VERSION_MINOR 2		//The Pug XML library minor version we are using in this implementation.
#include "pugxml.h"

class CNewsFeedParserImplPugXml : public CNewsFeedParser
{
public:
	CNewsFeedParserImplPugXml(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser);
	virtual ~CNewsFeedParserImplPugXml(void);

public:
	static NewsFeedParserPtr CreateParser(std::vector<BYTE>& stream);

	virtual size_t ExtractNews(NewsItemVector&)=0;

protected:
	static int ParseFormat(pug::xml_node& root);
	static CNewsFeedParserImplPugXml* CreateParser(int format, std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser);

	virtual void Parse()=0;

protected:
	virtual bool PopulateNewsItem(NewsItemPtr& pNews, pug::xml_node& item);

	bool ParseChannelDC(pug::xml_node& channel); // parse channel's dc elements
	bool ParseNode(CString& value, pug::xml_node& node, LPCTSTR xpath, bool bUnescape=false);
	bool ExtractContent(CString& value, pug::xml_node& node, LPCTSTR xpath);
	CString ExtractContent(pug::xml_node& node);

protected:
	std::vector<wchar_t>* m_pContent;
	pug::xml_parser* m_pXmlParser;
};
