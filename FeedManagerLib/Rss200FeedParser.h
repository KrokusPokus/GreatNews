#pragma once

#include "Rss09xFeedParser.h"

class CRss200FeedParser :
	public CRss09xFeedParser
{
public:
	CRss200FeedParser(void);
	virtual ~CRss200FeedParser(void);

protected:
	virtual void Parse();
	virtual bool PopulateNewsItem(NewsItemPtr& pNews, MSXML2::IXMLDOMElementPtr& item);

};
