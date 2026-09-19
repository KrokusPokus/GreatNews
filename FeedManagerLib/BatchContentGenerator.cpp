#include "StdAfx.h"
#include <math.h>
#include "BatchContentGenerator.h"
#include "NewsItemCache.h"

int CBatchContentGenerator::m_nPageSize;
int CBatchContentGenerator::m_nCurrentPage;
int CBatchContentGenerator::m_nTotalPage;
std::vector<ULONG_PTR> CBatchContentGenerator::m_vectNewsIDs;

bool CBatchContentGenerator::m_b10x;

CBatchContentGenerator::CBatchContentGenerator(void)
{
}

CBatchContentGenerator::~CBatchContentGenerator(void)
{
}


int CBatchContentGenerator::CalcTotalPage()
{
	int size = (int)m_vectNewsIDs.size();
	int totalPage = 1;
	if(m_nPageSize!=0 && size > m_nPageSize)
	{
		// totalPage = size/m_nPageSize + (size%m_nPageSize ? 1 : 0);
		totalPage = (int)ceil(double(size)/m_nPageSize);
	}
	return totalPage;
}

void CBatchContentGenerator::GetCurrentPageItems(NewsItemVector& newsItems, int nPage)
{
	m_unreadItemIDs.clear();
	m_channelUnreadCounts.clear();

	// calculate pages
	switch((Page)nPage)
	{
	case CurrentPage:
		break;
	case PrevPage:
		m_nCurrentPage = max(m_nCurrentPage-1,0);
		break;
	case NextPage:
		m_nCurrentPage = min(m_nTotalPage-1, m_nCurrentPage+1);
		break;
	case LastPage:
		m_nCurrentPage = m_nTotalPage-1;
		break;
	case AllPage:
		if(!m_b10x)
			SetPageSize(m_nPageSize*10);
		else
			SetPageSize(m_nPageSize/10);
		m_b10x = !m_b10x;
		break;
	default:
		m_nCurrentPage = min(m_nTotalPage, max(nPage,0));
	}

	std::vector<ULONG_PTR>::iterator itBegin = m_vectNewsIDs.begin()+m_nPageSize*m_nCurrentPage;
	std::vector<ULONG_PTR>::iterator itEnd = (m_nPageSize*(m_nCurrentPage+1)>=m_vectNewsIDs.size() ?
				m_vectNewsIDs.end() : m_vectNewsIDs.begin()+m_nPageSize*(m_nCurrentPage+1));
	std::vector<ULONG_PTR> IDs(itBegin,itEnd);
	CNewsItemCache::FillCache(IDs, true);

	newsItems.clear();
	newsItems.reserve(itEnd-itBegin);

	while(itBegin!=itEnd)
	{
		NewsItemPtr& spItem = CNewsItemCache::GetNewsItem(*itBegin);
		newsItems.push_back(spItem);

		if(spItem->IsUnread())
		{
			m_unreadItemIDs.push_back(spItem->m_id);

			std::map<ULONG_PTR, int>::iterator itMap = m_channelUnreadCounts.find(spItem->m_feedID);
			if( itMap != m_channelUnreadCounts.end())
				itMap->second = itMap->second + 1;
			else
				m_channelUnreadCounts.insert(std::map<ULONG_PTR, int>::value_type(spItem->m_feedID, 1));
		}
		itBegin++;
	}

	CNewsItem::BatchLoadWatchIDs(newsItems);
}

void CBatchContentGenerator::InitGenerator(int nPageSize)
{
	m_nPageSize = nPageSize;
	m_nCurrentPage = 0;
	m_vectNewsIDs.clear();
	GetNewsItemIDs(m_vectNewsIDs,m_pNewsFilter);
	m_nTotalPage = CalcTotalPage();
	m_b10x = false;
}

void CBatchContentGenerator::SetPageSize(int nPageSize)
{
	m_nPageSize = nPageSize;
	m_nCurrentPage = 0;
	m_nTotalPage = CalcTotalPage();
}

size_t CBatchContentGenerator::GetNumOfItems()
{
	return m_vectNewsIDs.size();
}

CString CBatchContentGenerator::GeneratePageContentHTML(int nPage)
{
	NewsItemVector newsItems;
	GetCurrentPageItems(newsItems, nPage);

	CString html;
	bool bSingleItem = (newsItems.size()<=1);
	for(NewsItemVector::iterator it=newsItems.begin(); it != newsItems.end(); ++it)
	{
		NewsItemPtr spItem = *it;
		html.Append(spItem->internalGenerateHTML(bSingleItem));
	}

	return html;
}

CString CBatchContentGenerator::GeneratePager()
{
	// if(m_nTotalPage <= 1)
	//if(m_nTotalPage <= 1) // if we only have one page
	//{
	//	if(!(m_vectNewsIDs.size()>1 && HasUnreadItem())) // don't have more than one items and some are unread
	//		return _T("");
	//}

	CString html;
	html.Format(_T("<div class=\"gn_pager\"><span class=\"gn_pager\">"));
	if(m_nCurrentPage == 0)
	{
		html.Append(_T("<span>|&lt;</span>&nbsp;<span>&lt;Previous</span>&nbsp;"));
	}
	else
	{
		html.Append(_T("<a href=\"http://localhost/#GreatNewsTag_GotoPage_0\" onMouseOver=\"window.status='Goto first page'; return true;\" class=\"gn_pagerlink\">|&lt;</a>&nbsp;"));
		html.Append(_T("<a href=\"http://localhost/#GreatNewsTag_GotoPage_-2\" onMouseOver=\"window.status='Goto previous page'; return true;\" class=\"gn_pagerlink\">&lt;Previous</a>&nbsp;"));
	}
	html.AppendFormat(_T("<span class=\"gn_pagenumber\">Page %d/%d</span>&nbsp;"),
		m_nCurrentPage+1, m_nTotalPage);
	html.Append(_T("<a href=\"http://localhost/#GreatNewsTag_GotoPage_-3\" onMouseOver=\"window.status='Goto next page'; return true;\" class=\"gn_pagerlink\">Next&gt;</a>&nbsp;"));
	if(m_nCurrentPage >= m_nTotalPage-1)
	{
		html.Append(_T("<span>&gt;|</span>&nbsp;"));
	}
	else
	{
		html.Append(_T("<a href=\"http://localhost/#GreatNewsTag_GotoPage_-4\" onMouseOver=\"window.status='Goto last page'; return true;\" class=\"gn_pagerlink\">&gt;|</a>&nbsp;"));
	}

	if(m_b10x)
		html.Append(_T("<a href=\"http://localhost/#GreatNewsTag_GotoPage_-5\" onMouseOver=\"window.status='Show less items'; return true;\" class=\"gn_pagerlink\">/10</a></span></div>\n"));
	else if(m_vectNewsIDs.size()<=(size_t)m_nPageSize)
		html.Append(_T("<span>x10</span></span></div>\n"));
	else
		html.Append(_T("<a href=\"http://localhost/#GreatNewsTag_GotoPage_-5\" onMouseOver=\"window.status='Show more items'; return true;\" class=\"gn_pagerlink\">x10</a></span></div>\n"));

	return html;
}


void CBatchContentGenerator::ExportRss(MSXML2::IXMLDOMElementPtr& spChannel)
{
	MSXML2::IXMLDOMDocumentPtr spDoc = spChannel->ownerDocument;

	AddElement(spDoc,spChannel,_T("title"), GetNewsSourceName());
	AddElement(spDoc,spChannel,_T("link"), _T("http://home.comcast.net/~ceciliachen/ROR/"));
	AddElement(spDoc,spChannel,_T("description"),_T(""));
	AddElement(spDoc,spChannel,_T("generator"),_T("GreatNews 1.0"));
	AddElement(spDoc,spChannel,_T("docs"),_T("http://blogs.law.harvard.edu/tech/rss"));
}
