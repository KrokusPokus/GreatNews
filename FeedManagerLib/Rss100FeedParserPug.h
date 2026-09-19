#pragma once

#include "NewsFeedParserImplPugXml.h"

class CRss100FeedParserPug : public CNewsFeedParserImplPugXml
{
public:
	CRss100FeedParserPug(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser);
	~CRss100FeedParserPug(void);

public:
	virtual size_t ExtractNews(NewsItemVector&);

protected:
	virtual void Parse();
};
