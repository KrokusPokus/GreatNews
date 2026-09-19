#pragma once

#include "NewsFeedParserImplPugXml.h"

class CAtomFeedParserPug : public CNewsFeedParserImplPugXml
{
public:
	CAtomFeedParserPug(std::vector<wchar_t>* pContent, pug::xml_parser* pxmlparser);
	~CAtomFeedParserPug(void);

	virtual size_t ExtractNews(NewsItemVector&);

protected:
	virtual void Parse();

	bool PopulateNewsItem(NewsItemPtr pNews, pug::xml_node& item);
};
