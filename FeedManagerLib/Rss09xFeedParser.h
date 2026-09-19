#pragma once

#include "NewsFeedParserImplMSXML.h"

class CRss09xFeedParser :
	public CNewsFeedParserImplMSXML
{
public:
	CRss09xFeedParser(void);
	~CRss09xFeedParser(void);

public:
	virtual size_t ExtractNews(NewsItemVector&);

protected:
	virtual void Parse();
	virtual bool PopulateNewsItem(NewsItemPtr& pNews, MSXML2::IXMLDOMElementPtr& item);
};
