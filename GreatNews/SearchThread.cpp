#include "StdAfx.h"
#include "SearchThread.h"
#include "NewsItemCache.h"
#include "SearchTool.h"

#define PREFETCH_SIZE	20

CSearchThread::CSearchThread(void)
	: CGNThread(false), m_bStop(false), m_hwndToNotify(0)
{
}

CSearchThread::~CSearchThread(void)
{
}

DWORD CSearchThread::Run()
{
	UINT msg = MM_SEARCH_NOTFOUND;

	try
	{
		SearchToolPtr searchTool = CSearchTool::CreateTool(m_searchText);

		std::vector<ULONG_PTR>& vectNewsIDs = m_content->m_vectNewsIDs;
		while(!m_bStop && m_nSearching < (int)vectNewsIDs.size())
		{
			if(m_nSearching % PREFETCH_SIZE == 0)
			{
				std::vector<ULONG_PTR> IDsToLoad(vectNewsIDs.begin()+m_nSearching,
					m_nSearching+PREFETCH_SIZE >= (int)vectNewsIDs.size() ? vectNewsIDs.end() : vectNewsIDs.begin()+m_nSearching+PREFETCH_SIZE); 

				CNewsItemCache::FillCache(IDsToLoad, true);
			}

			m_searchText.MakeLower();
			NewsItemPtr item = CNewsItemCache::GetNewsItem(vectNewsIDs[m_nSearching]);

			if(searchTool->Match(item))
			{
				m_matchedString = searchTool->m_matchedString;
				msg = MM_SEARCH_FOUND;
				break;
			}

			CNewsItemCache::CheckCacheSize();

			m_nSearching++;
		}
	}
	catch(...)
	{
	}

	if(::IsWindow(m_hwndToNotify))
	{
		::PostMessage(m_hwndToNotify, msg, m_nSearching ,0);
	}
	m_stopEvent.SetEvent();

	return 0;
}

