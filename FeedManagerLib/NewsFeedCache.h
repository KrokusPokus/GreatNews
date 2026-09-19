#pragma once

#include <map>
#include "NewsFeed.h"

typedef std::map<ULONG_PTR, NewsFeedPtr> NewsFeedCache;

class CNewsFeedCache
{
public:
	~CNewsFeedCache() {};

	static void Load();
	static void Empty();
	static NewsFeedPtr GetNewsFeed(ULONG_PTR id);
	static NewsFeedPtr GetCommentFeed(ULONG_PTR itemID);
	static void AddFeed(NewsFeedPtr feed);
	static void RemoveFeed(ULONG_PTR feedId);
	static void GetAllNewsFeeds(NewsFeedVector& feeds);
	static size_t GetCommentFeeds(NewsFeedVector& commentFeeds, ULONG_PTR channelID);

private:
	CNewsFeedCache(){};

private:
	static NewsFeedCache m_newsFeedCache;
};
