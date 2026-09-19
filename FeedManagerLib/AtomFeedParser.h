#pragma once

#include "NewsFeedParserImplMSXML.h"

class CAtomFeedParser :
	public CNewsFeedParserImplMSXML
{
public:
	CAtomFeedParser(void);
	~CAtomFeedParser(void);

	virtual size_t ExtractNews(NewsItemVector&);

protected:
	virtual void Parse();

	bool PopulateNewsItem(NewsItemPtr pNews, MSXML2::IXMLDOMElementPtr& item);
};
