#pragma once

#include <vector>
#include "loki\SmartPtr.h"
#include "ExceptionBase.h"
#include "FeedManagerErrorCode.h"
#include "NewsItem.h"
#include "NewsFeed.h"

class CNewsFeed;
class CNewsFeedParser;

typedef Loki::SmartPtr<CNewsFeedParser, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> NewsFeedParserPtr;
class CNewsFeedParser
{
public:
	enum ParserImplType
	{
		Auto = 0,
		ImplMSXML = 1,
		ImplPug = 2
	};
	enum RSSFormat
	{
		Unknown,
		RSS200,
		RSS09x,
		RSS100,
		ATOM030,
		ATOM100};

	CNewsFeedParser(void);
	virtual ~CNewsFeedParser(void);

public:
	static NewsFeedParserPtr CreateParser(LPCTSTR url, std::vector<BYTE>& stream, ParserImplType implType=Auto);
	static NewsFeedParserPtr CreateParserFromURL(LPCTSTR url, LPCTSTR user=NULL, LPCTSTR password=NULL, ParserImplType implType=Auto);
	static CString GetFormatName(int format);
	static RSSFormat GetFormat(const CString& rootTagName, CString version);

	// extract news items from this parser
	virtual size_t ExtractNews(NewsItemVector&)=0;

public:
	CNewsFeed m_newsFeed;
	static bool m_bKeepOriginalLink; // save item's original link instead of feedburner's
};

