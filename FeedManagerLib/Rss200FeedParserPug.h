#pragma once

#include "Rss09xFeedParserPug.h"

class CRss200FeedParserPug :
	public CRss09xFeedParserPug
{
public:
	CRss200FeedParserPug(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser);
	virtual ~CRss200FeedParserPug(void);

protected:
	virtual void Parse();
	virtual bool PopulateNewsItem(NewsItemPtr& pNews, pug::xml_node& item);

};
