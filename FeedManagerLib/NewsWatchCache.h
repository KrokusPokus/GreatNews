#pragma once

#include <map>
#include "FeedManagerLib.h"

class CNewsWatchCache
{
public:
	~CNewsWatchCache(void);

public:
	static void Init();
	static bool IsInited() {return m_bInited;}

	static void Empty();
	
	static NewsWatchPtr GetNewsWatch(ULONG_PTR id);
	static NewsWatchPtr GetHighestNewsWatch(std::vector<ULONG_PTR>& watchIDs);
	
private:
	CNewsWatchCache(void);
	static void LoadAllWatches();


	typedef std::map<ULONG_PTR, NewsWatchPtr> WatchCacheType;
	static bool m_bInited;
	static WatchCacheType m_mapWatches;

};
