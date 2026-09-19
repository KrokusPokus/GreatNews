#pragma once

#include "NewsFeedParserImplPugXml.h"

class CRss09xFeedParserPug :
	public CNewsFeedParserImplPugXml
{
public:
	CRss09xFeedParserPug(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser);
	~CRss09xFeedParserPug(void);

public:
	virtual size_t ExtractNews(NewsItemVector&);

protected:
	virtual void Parse();
	virtual bool PopulateNewsItem(NewsItemPtr& pNews, pug::xml_node& item);
};
