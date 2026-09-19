#include "StdAfx.h"
#include "NewsWatchCache.h"

CNewsWatchCache::WatchCacheType CNewsWatchCache::m_mapWatches;
bool  CNewsWatchCache::m_bInited=false;

CNewsWatchCache::CNewsWatchCache(void)
{
}

CNewsWatchCache::~CNewsWatchCache(void)
{
}

void CNewsWatchCache::Init()
{
	LoadAllWatches();
	m_bInited = true;
}

void CNewsWatchCache::Empty()
{
	m_mapWatches.clear();
	m_bInited = false;
}

void CNewsWatchCache::LoadAllWatches()
{
	m_mapWatches.clear();

	NewsWatchVector watches;
	CNewsWatch::GetAllNewsWatches(watches);
	for(NewsWatchVector::iterator it=watches.begin(); it!= watches.end(); ++it)
	{
		NewsWatchPtr watch = *it;
		m_mapWatches.insert(WatchCacheType::value_type(watch->m_id, watch));
	}
}

NewsWatchPtr CNewsWatchCache::GetNewsWatch(ULONG_PTR id)
{
	std::map<ULONG_PTR, NewsWatchPtr>::iterator it = m_mapWatches.find(id);
	if(it!= m_mapWatches.end())
		return it->second;
	else
		return NULL;
}

NewsWatchPtr CNewsWatchCache::GetHighestNewsWatch(std::vector<ULONG_PTR>& watchIDs)
{

	if(!m_bInited)
	{
		Init();
	}

	NewsWatchPtr highestWatch;

	for(std::vector<ULONG_PTR>::iterator it=watchIDs.begin();it!=watchIDs.end();++it)
	{
		NewsWatchPtr watch = GetNewsWatch(*it);
		if(watch && (highestWatch==NULL || watch->m_order<highestWatch->m_order))
			highestWatch=watch;
	}
	return highestWatch;
}

