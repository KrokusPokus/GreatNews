#pragma once
#pragma comment( lib, "FeedManagerLib.lib" )

#include "loki\SmartPtr.h" // Loki's smartptr
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"
#include "FeedGroup.h"
#include "NewsFeed.h"
#include "NewsFeedParser.h"
#include "NewsWatch.h"
#include "Tag.h"
#include "NewsItemCache.h"
#include "NewsWatchCache.h"
#include "NewsFeedCache.h"
#include "SearchChannelDef.h"

namespace FeedManagerLib
{
	enum TimeMark
	{
		StartupTime,
		ShutdownTime
	};

	void CompactDatabase();
	CString GetDbVersion();
	
	void MarkTime(TimeMark mark);
	bool CheckLastShutdown();
#ifdef _DEBUG
	CString DebugSql(const CString& sql);
#endif
}
