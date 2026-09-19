#pragma once

#include <map>
#include "NewsItem.h"

typedef std::map<ULONG_PTR, NewsItemPtr> NewsItemCache;

class CNewsItemCache
{
public:
	~CNewsItemCache() {};

	static void Init();
	static void Uninit();
	static void Empty();
	static NewsItemPtr GetNewsItem(ULONG_PTR id);
	static void FillCache(std::vector<ULONG_PTR>& IDs, bool bNoSizeCheck);
	static void ClearAllWatches();
	static void CheckCacheSize();

	static void MarkAllRead();
	static void MarkAllRead(std::vector<ULONG_PTR>& IDs);

private:
	CNewsItemCache(){};

private:
	static int m_maxNumOfItems;
	static NewsItemCache m_newsItemCache;
};
