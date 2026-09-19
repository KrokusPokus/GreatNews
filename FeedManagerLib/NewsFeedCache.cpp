#include "StdAfx.h"
#include "NewsFeedCache.h"

NewsFeedCache CNewsFeedCache::m_newsFeedCache;

void CNewsFeedCache::Empty()
{
	m_newsFeedCache.clear();
}

NewsFeedPtr CNewsFeedCache::GetNewsFeed(ULONG_PTR id)
{
	NewsFeedCache::iterator it = m_newsFeedCache.find(id);
	if(it!=m_newsFeedCache.end())
	{
		return it->second;
	}

	// not found in cache, try to load it
	NewsFeedPtr feed = new CNewsFeed();
	try
	{
		feed->Init(id);
		m_newsFeedCache.insert(NewsFeedCache::value_type(feed->m_id, feed));
		return feed;
	}
	catch(...)
	{
	}

	return NULL;
}

void CNewsFeedCache::AddFeed(NewsFeedPtr feed)
{
	ATLASSERT(feed->m_id > 0);
	m_newsFeedCache[feed->m_id]=feed;
}

void CNewsFeedCache::Load()
{
	Empty();

	NewsFeedVector newsfeeds;
	CNewsFeed::GetAllNewsFeeds(newsfeeds, true);

	for(NewsFeedVector::iterator nit = newsfeeds.begin(); nit!=newsfeeds.end(); ++nit)
	{
		NewsFeedPtr feed = *nit;
		m_newsFeedCache.insert(NewsFeedCache::value_type(feed->m_id, feed));
	}
}

NewsFeedPtr CNewsFeedCache::GetCommentFeed(ULONG_PTR itemID)
{
	for(NewsFeedCache::iterator it=m_newsFeedCache.begin(); it!=m_newsFeedCache.end(); ++it)
	{
		NewsFeedPtr& feed = it->second;
		if(feed->m_parent_item_id == itemID)
			return feed;
	}

	return NULL;
}

void CNewsFeedCache::RemoveFeed(ULONG_PTR feedId)
{
	for(NewsFeedCache::iterator it=m_newsFeedCache.begin(); it!=m_newsFeedCache.end(); ++it)
	{
		NewsFeedPtr& feed = it->second;
		if(feed->m_id == feedId)
		{
			m_newsFeedCache.erase(it);
			return;
		}
	}
}

size_t CNewsFeedCache::GetCommentFeeds(NewsFeedVector& commentFeeds, ULONG_PTR channelID)
{
	for(NewsFeedCache::iterator it=m_newsFeedCache.begin(); it!=m_newsFeedCache.end(); ++it)
	{
		NewsFeedPtr& feed = it->second;
		if(feed->m_parent_feed_id == channelID)
		{
			commentFeeds.push_back(feed);
		}
	}

	return commentFeeds.size();
}

void CNewsFeedCache::GetAllNewsFeeds(NewsFeedVector& feeds)
{
	for(NewsFeedCache::iterator it=m_newsFeedCache.begin(); it!=m_newsFeedCache.end(); ++it)
	{
		NewsFeedPtr feed = it->second;
		feeds.push_back(feed);
	}
}


