#include "StdAfx.h"
#include "NewsItemCache.h"

NewsItemCache CNewsItemCache::m_newsItemCache;
int CNewsItemCache::m_maxNumOfItems = 800;

void CNewsItemCache::Init()
{
}

void CNewsItemCache::Uninit()
{
	Empty();
}

void CNewsItemCache::Empty()
{
	m_newsItemCache.clear();
}

NewsItemPtr CNewsItemCache::GetNewsItem(ULONG_PTR id)
{
	NewsItemCache::iterator it = m_newsItemCache.find(id);
	if(it!=m_newsItemCache.end())
	{
		return it->second;
	}

	// not found in cache, try to load it
	NewsItemPtr item(new CNewsItem());
	try
	{
		item->Init(id);
		CheckCacheSize();
		m_newsItemCache.insert(NewsItemCache::value_type(item->m_id, item));
		return item;
	}
	catch(...)
	{
	}

	return NULL;
}

void CNewsItemCache::FillCache(std::vector<ULONG_PTR>& IDs, bool bNoSizeCheck)
{
	std::vector<ULONG_PTR> IDsToLoad;
	IDsToLoad.reserve(IDs.size());

	for(std::vector<ULONG_PTR>::iterator it=IDs.begin(); it!=IDs.end(); ++it)
	{
		NewsItemCache::iterator itItem = m_newsItemCache.find(*it);
		if(itItem==m_newsItemCache.end())
		{
			IDsToLoad.push_back(*it);
		}
	}

	if(!IDsToLoad.empty())
	{
		ATLTRACE(_T("ItemCache loading [%d] items\n"), IDsToLoad.size());
	
		if(!bNoSizeCheck)
			CheckCacheSize();

		NewsItemVector newsItemToCache;
		CNewsItem::GetNewsItemsByID(newsItemToCache, IDsToLoad);
		for(NewsItemVector::iterator it=newsItemToCache.begin(); it!=newsItemToCache.end();++it)
		{
			NewsItemPtr item = *it;
			m_newsItemCache.insert(NewsItemCache::value_type(item->m_id, item));
		}
	}
}

void CNewsItemCache::ClearAllWatches()
{
	Empty();
}

void CNewsItemCache::CheckCacheSize()
{
	if((int)m_newsItemCache.size() > m_maxNumOfItems)
	{
		// remove first 10%
		int nItemToRemove = (int)m_newsItemCache.size()- int(0.9*m_maxNumOfItems);
		ATLTRACE(_T("ItemCache removing [%d] items from cache\n"), nItemToRemove);

		// the item was ordered by news id, older news should have smaller ids...
		NewsItemCache::iterator itBegin = m_newsItemCache.begin();
		NewsItemCache::iterator itEnd = itBegin;
		std::advance(itEnd, nItemToRemove);
		m_newsItemCache.erase(itBegin, itEnd);
	}
}

void SetReadTime(const std::pair<ULONG_PTR, NewsItemPtr>& iter)
{
	iter.second->SetReadFlag();
}

class CSetItemReadTime
{
public:
	CSetItemReadTime(NewsItemCache& cache):m_newsItemCache(cache) {}

	void operator()(ULONG_PTR id)
	{
		NewsItemCache::iterator it = m_newsItemCache.find(id);
		if(it!=m_newsItemCache.end())
			SetReadTime(*it);
	}

private:
	NewsItemCache& m_newsItemCache;
};

void CNewsItemCache::MarkAllRead(std::vector<ULONG_PTR>& IDs)
{
	CSetItemReadTime settime(m_newsItemCache);
	for_each(IDs.begin(), IDs.end(), settime);
}

void CNewsItemCache::MarkAllRead()
{
	for_each(m_newsItemCache.begin(), m_newsItemCache.end(), SetReadTime);
}
