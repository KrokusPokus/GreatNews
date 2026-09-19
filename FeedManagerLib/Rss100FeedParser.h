#pragma once

#include "NewsFeedParserImplMSXML.h"

class CRss100FeedParser :
	public CNewsFeedParserImplMSXML
{
public:
	CRss100FeedParser(void);
	~CRss100FeedParser(void);

public:
	virtual size_t ExtractNews(NewsItemVector&);

protected:
	virtual void Parse();
};
