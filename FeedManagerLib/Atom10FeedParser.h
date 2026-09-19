#pragma once

#include "NewsFeedParserImplMSXML.h"

class CAtom10FeedParser :
	public CNewsFeedParserImplMSXML
{
public:
	CAtom10FeedParser(void);
	virtual ~CAtom10FeedParser(void);

	virtual size_t ExtractNews(NewsItemVector&);

protected:
	virtual void Parse();

	bool PopulateNewsItem(NewsItemPtr pNews, MSXML2::IXMLDOMElementPtr& item);

};
