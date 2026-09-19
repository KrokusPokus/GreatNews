#include "StdAfx.h"
#include "ImportThread.h"
#include "NewsItemCache.h"
#include "NewsFeedTreeItem.h"
#include "FeedGroupTreeItem.h"
#include <strsafe.h>

CImportThread::CImportThread(void)
	: CGNThread(false), m_bStop(false), m_hwndToNotify(0), 
	m_hwndTree(0), m_bUpdateAfterImport(false), m_nDefaultGroup(0)
{
	Clear();
}

void CImportThread::Clear()
{
	m_cntGroupImported = 0;
	m_cntGroupSkipped = 0;
	m_cntGroupFailed = 0;
	m_cntFeedImported = 0;
	m_cntFeedSkipped = 0;
	m_cntFeedFailed = 0;

	m_bStop = false;
	m_stopEvent.ResetEvent();
}

CImportThread::~CImportThread(void)
{
}

DWORD CImportThread::Run()
{
	ATLASSERT(m_hwndTree);

	Clear();

	::CoInitialize(NULL);

	try
	{
		FeedGroupPtr rootGroup = CFeedGroup::GetRootFeedGroup();
		HTREEITEM rootItem = TreeView_GetRoot(m_hwndTree);
		HTREEITEM childItem = TreeView_GetChild(m_hwndTree, rootItem);

		CNewsFeedTreeItem* pNewsFeedItem;
		CFeedGroupTreeItem* pFeedGroupItem;
		while(!m_bStop && childItem)
		{
			if(GetCheckState(childItem))
			{
				if(NULL != (pNewsFeedItem=dynamic_cast<CNewsFeedTreeItem*>(GetFeedTreeItem(childItem))))
				{
					// this is a channel at root level
					NewsFeedPtr feed = pNewsFeedItem->m_newsFeed;
					feed->m_groupID = m_nDefaultGroup ? m_nDefaultGroup : rootGroup->m_id;
					ImportNewsFeed(feed);
				}
				else if(NULL != (pFeedGroupItem=dynamic_cast<CFeedGroupTreeItem*>(GetFeedTreeItem(childItem))))
				{
					ImportFeedGroup(pFeedGroupItem->m_feedGroup);

					if(pFeedGroupItem->m_feedGroup->m_id)
					{
						HTREEITEM feedItem = TreeView_GetChild(m_hwndTree, childItem);

						while(!m_bStop && feedItem)
						{
							if(GetCheckState(feedItem))
							{
								pNewsFeedItem=dynamic_cast<CNewsFeedTreeItem*>(GetFeedTreeItem(feedItem));
								ATLASSERT(pNewsFeedItem);
								NewsFeedPtr feed = pNewsFeedItem->m_newsFeed;
								feed->m_groupID = pFeedGroupItem->m_feedGroup->m_id;

								ImportNewsFeed(feed);
							}
		
							feedItem = TreeView_GetNextSibling(m_hwndTree, feedItem);
						}
					}
				}
			}
			
			childItem = TreeView_GetNextSibling(m_hwndTree, childItem);
		}

		//
		// import watch && label
		//
		if(!m_bStop && m_importWatchLabel && m_spDoc != NULL)
		{
			MSXML2::IXMLDOMNodeListPtr spWatchList = m_spDoc->selectNodes(_T("/opml/watchList/watch"));
			MSXML2::IXMLDOMNodePtr spWatch;
			while(NULL !=(spWatch=spWatchList->nextNode()))
			{
				CNewsWatch watch;
				watch.LoadFromXml(spWatch);

				try
				{
					watch.Save();
				}
				catch(...)
				{
					// ignore all errors
				}
			}

			MSXML2::IXMLDOMNodeListPtr spLabelList = m_spDoc->selectNodes(_T("/opml/labelList/label"));
			MSXML2::IXMLDOMNodePtr spLabel;
			while(NULL != (spLabel=spLabelList->nextNode()))
			{
				CTag tag;
				tag.LoadFromXml(spLabel);

				try
				{
					tag.Save();
				}
				catch(...)
				{
					// ignore all errors
				}
			}

		}
	}
	catch(...)
	{
	}

	m_stopEvent.SetEvent();

	if(::IsWindow(m_hwndToNotify))
	{
		::PostMessage(m_hwndToNotify, MM_IMPORT_FINISHED, 0 ,0);
	}

	::CoUninitialize();
	return 0;
}

void CImportThread::ImportFeedGroup(FeedGroupPtr group)
{
	CString temp;
	try
	{
		ULONG_PTR groupID = CFeedGroup::GetIdFromName(group->m_name);
		if(groupID == 0)
		{
			group->Save();
			temp.Format(ResManagerPtr->GetString(IDS_IMPORTGROUPOK),group->m_name);
			m_cntGroupImported++;
		}
		else
		{
			group->m_id = groupID;
			temp.Format(ResManagerPtr->GetString(IDS_IMPORTSKIPGRP),group->m_name);
			m_cntGroupSkipped++;
		}
	}
	catch(...)
	{
		temp.Format(ResManagerPtr->GetString(IDS_IMPORTGROUPFAILED),group->m_name);
		m_cntGroupFailed++;
	}

	PostStatusMessage(temp);
}

void CImportThread::ImportNewsFeed(NewsFeedPtr feed)
{
	CString temp;
	try
	{
		feed->m_url.Trim();
		if(feed->m_url.GetLength()==0)
		{
			throw CExceptionBase(ERR_FM_GENERICERR, _T("due to invalid URL"));
		}

		ULONG_PTR feedID = CNewsFeed::GetIdFromXmlUrl(feed->m_url);
		if(feedID !=0)
		{
			temp.Format(ResManagerPtr->GetString(IDS_IMPORTSKIPFEED),feed->m_title);
			m_cntFeedSkipped++;
		}
		else
		{
			temp.Format(ResManagerPtr->GetString(IDS_IMPORTINGFEED),feed->m_title);
			PostStatusMessage(temp);

			NewsFeedParserPtr spParser;
			if(m_bUpdateAfterImport)
			{
				spParser = CNewsFeedParser::CreateParserFromURL(feed->m_url);
				CString title = feed->m_title; // we want to save specified title
				feed->CopyProperties(spParser->m_newsFeed, CNewsFeed::Web);
				if(title.GetLength())
					feed->m_title = title;
			}

			int nRetry = 0;
			while(true)
			{
				try
				{
					feed->Save();
					break;
				}
				catch(const CUniqueNameException&)
				{
					nRetry++;
					if(nRetry>=30)
						throw; // insane

					// non unique name, try to solve it by appending -1
					feed->m_title = feed->m_title+_T("-1");
				}
			}

			if(nRetry!=0)
				temp.Format(ResManagerPtr->GetString(IDS_IMPORTRENAME),feed->m_title);
			else
				temp.Format(ResManagerPtr->GetString(IDS_IMPORTFEEDOK),feed->m_title);
			m_cntFeedImported++;
			
			if(m_bUpdateAfterImport)
			{
				try
				{
					// save news items
					NewsItemVector newsItems;
					spParser->ExtractNews(newsItems);
					feed->RetrieveNewsToDB(newsItems, spParser->m_newsFeed);
				}
				catch(...)
				{
					// ignore this error for now if we cannot reterive news
				}
			}
		}
	}
	catch(const CExceptionBase& e)
	{
		temp.Format(ResManagerPtr->GetString(IDS_IMPORTFEEDFAILED),feed->m_title, e.GetErrorMsg());
		m_cntFeedFailed++;
	}
	catch(...)
	{
		temp.Format(ResManagerPtr->GetString(IDS_IMPORTFEEDFAILED0),feed->m_title);
		m_cntFeedFailed++;
	}

	PostStatusMessage(temp);
}

void CImportThread::PostStatusMessage(const CString& text)
{
	if(::IsWindow(m_hwndToNotify))
	{
		LPTSTR p = new TCHAR[text.GetLength()+1];
		StringCchCopy(p, text.GetLength()+1, (LPCTSTR)text);
		::PostMessage(m_hwndToNotify, MM_IMPORT_STATUS, 0,(LPARAM)p);
	}
}

BOOL CImportThread::GetCheckState(HTREEITEM hItem) const
{
	UINT uRet = TreeView_GetItemState(m_hwndTree, hItem, TVIS_STATEIMAGEMASK);
	return (uRet >> 12) - 1;
}

CFeedTreeItem* CImportThread::GetFeedTreeItem(HTREEITEM hItem)
{
	TVITEM item = { 0 };
	item.hItem = hItem;
	item.mask = TVIF_PARAM;
	BOOL bRet = TreeView_GetItem(m_hwndTree, &item);
	return (CFeedTreeItem*)(bRet ? item.lParam : NULL);

}
