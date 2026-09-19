#pragma once

#include <map>
#include "FeedTreeItem.h"
#include "WatchRootTreeItem.h"
#include "RootGroupFeedTreeItem.h"
#include "NewsFeedTreeItem.h"
#include "FeedGroupTreeItem.h"
#include "RootGroupFeedTreeItem.h"
#include "NewsWatchTreeItem.h"
#include "TagRootTreeItem.h"
#include "TagTreeItem.h"
#include "NewsFeedCache.h"
#include "atlwfile.h"
#include "GreatNewsConfig.h"
#include "FaviconRequest.h"
#include "GNResourceManager.h"

enum LoadTreeType
{
    LoadChannel = 1,
    LoadGroup = 2,
    LoadWatch = 4,
    LoadLabel = 8,
    LoadSearch = 16,
    LoadAll = LoadChannel + LoadGroup + LoadWatch + LoadLabel
};

struct NodeMapKey
{
    NodeMapKey(CFeedTreeItem::Type t, ULONG_PTR id) : m_type((int)t), m_id(id) {}

    int m_type;
    ULONG_PTR m_id;

};
inline bool operator < (const NodeMapKey& key, const NodeMapKey& key1)
{
    return (key.m_id<key1.m_id || (key.m_id == key1.m_id && key.m_type < key1.m_type));
}

template <class T, class TBase = CTreeViewCtrlEx, class TWinTraits = CControlWinTraits>
class CFeedTreeCtrlImpl : public CWindowImpl<T, TBase, TWinTraits>
{
    typedef CWindowImpl<T, TBase, TWinTraits> Base;
    typedef std::map<NodeMapKey, CTreeItem> FeedNodeMap;

public:
    DECLARE_WND_SUPERCLASS(_T("GN_FeedTree"), TBase::GetWndClassName())

    BEGIN_MSG_MAP(CFeedTreeCtrlImpl)
        MESSAGE_HANDLER(MM_FAVICONREADY, OnFaviconReady)
        COMMAND_ID_HANDLER_EX(ID_TREEMENU_EXPANDALL, OnExpandAll)
        COMMAND_ID_HANDLER_EX(ID_TREEMENU_COLLAPSEALL, OnCollapseAll)
        COMMAND_ID_HANDLER_EX(ID_TREEMENU_COLLAPSETOGROUP, OnCollapseToGroup)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_DELETEITEM, OnTreeItemDelete)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_RCLICK, OnContextMenu)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

public:
    CFeedTreeCtrlImpl() : m_pFaviconRequest(NULL)
    {
    }

public:
    LRESULT OnFaviconReady(UINT /*uMsg*/, WPARAM channelID, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CTreeItem node = FindNodeByIdType(channelID, CFeedTreeItem::Channel);
        if(node == NULL)
        {
            AtlTrace(_T("Channel %d cannot be found\n"), channelID);
            return 0;
        }

        CNewsFeedTreeItem* pChannelItem = dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)node.GetData());

        if(LoadFavicon(pChannelItem))
            node.SetImage(pChannelItem->m_faviconIndex, pChannelItem->m_faviconIndex);

        return 0;
    }

    LRESULT OnContextMenu(LPNMHDR pnmh)
    {
        DWORD dwPos = GetMessagePos();
        CPoint spt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );

        CMenu menu;
        menu.LoadMenu(IDR_TREEMENU_DEFAULT);

        CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
        pResMngr->ApplyLanguageToMenu(menu.GetSubMenu(0));

        menu.GetSubMenu(0).TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, spt.x, spt.y, m_hWnd);
        return 0;
    }

    LRESULT OnTreeItemDelete(LPNMHDR pnmh)
    {
        LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;

        HTREEITEM delItem = pnmtv->itemOld.hItem;
        for(FeedNodeMap::iterator it = m_mapFeedSearch.begin();
            it!=m_mapFeedSearch.end(); ++it)
        {
            if(it->second == delItem)
            {
                m_mapFeedSearch.erase(it);
                break;
            }
        }

        if(pnmtv->itemOld.lParam != 0)
        {
            CFeedTreeItem* pItem = (CFeedTreeItem*)pnmtv->itemOld.lParam;
            delete pItem;
        }
        return 0;
    }

    LRESULT OnExpandAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
    {
        ExpandAll();
        GetRootItem().EnsureVisible();
        return 0;
    }

    LRESULT OnCollapseToGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
    {
        CollapseToGroup();
        return 0;
    }

    LRESULT OnCollapseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
    {
        ExpandAll(false);
        return 0;
    }

    void CollapseToGroup(HTREEITEM groupNode = NULL)
    {
        GetRootItem().Expand(TVE_EXPAND);

        CTreeItem childNode = GetRootItem().GetChild();
        while(childNode)
        {
            childNode.Expand((childNode == groupNode)? TVE_EXPAND:TVE_COLLAPSE);
            childNode = childNode.GetNextSibling();
        }

        GetRootItem().EnsureVisible();

        if(m_tagRoot)
            m_tagRoot.Expand(TVE_COLLAPSE);
        if(m_watchRoot)
            m_watchRoot.Expand(TVE_COLLAPSE);
    }

    void SetFaviconRequest(CFaviconRequest* pFaviconRequest)
    {
        m_pFaviconRequest = pFaviconRequest;
    }

    HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
            DWORD dwStyle = 0, DWORD dwExStyle = 0,
            ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
    {
        dwStyle |= TVS_LINESATROOT|TVS_HASBUTTONS |TVS_SHOWSELALWAYS |TVS_TRACKSELECT |TVS_NOHSCROLL;
        // dwStyle |= TVS_LINESATROOT|TVS_HASBUTTONS |TVS_SHOWSELALWAYS | TVS_NOHSCROLL;
        HWND hWnd = Base::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
        if(!::IsWindow(hWnd))
            return hWnd;
        if(m_pFaviconRequest)
            m_pFaviconRequest->SetNotifyWnd(hWnd);

        SetTreeImageList();
        SetFont(AtlGetDefaultGuiFont());
        SetItemHeight(18); // favicons are 16*16
        return hWnd;
    }

    void SetTreeImageList()
    {
        CImageList imageList;
        // we cannot use CImageList::CreateFromFile() because that one won't handle true color bmp. Sucks!
        imageList.Create(16, 16,  ILC_COLOR32 | ILC_MASK, 0, 1);
        HBITMAP hImage = (HBITMAP)::LoadImage(ATL::_AtlBaseModule.GetResourceInstance(),MAKEINTRESOURCE(IDB_TREE),IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR);
        if(hImage)
        {
            imageList.Add(hImage, RGB(203,203,203));
            imageList.SetOverlayImage(UpdatingOverlay,1);
            imageList.SetOverlayImage(WaitingOverlay,2);
            SetImageList(imageList,TVSIL_NORMAL);
        }
    }

    void ShowEverything()
    {
        CheckAll();
        ExpandAll();
        CTreeItem itemRoot = GetRootItem();
        itemRoot.SetState(INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);
        itemRoot.EnsureVisible();
    }

    bool ShowChannelStatus(ULONG_PTR channelID, ChannelStatus status)
    {
        CTreeItem item = FindNodeByIdType(channelID, CFeedTreeItem::Channel);
        if (item == NULL)
            return false;

        item.SetState(INDEXTOOVERLAYMASK(status), TVIS_OVERLAYMASK);
        return true;
    }

    void ClearAllChannelStatus()
    {
        std::for_each(m_mapFeedSearch.begin(), m_mapFeedSearch.end(), ClearChannelStatus);
    }


    CTreeItem InsertItem(HTREEITEM itemParent, CFeedTreeItem* pItem, HTREEITEM hInsertAfter=TVI_LAST, BOOL bSort=TRUE)
    {
        CTreeItem treeItem = CTreeViewCtrlEx::InsertItem(
            TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE,
            pItem->GetTreeNodeText(),
            pItem->GetIcon(), pItem->GetIcon(),
            0, 0,
            (DWORD_PTR)pItem,
            itemParent,
            hInsertAfter);

        if(pItem->GetUnreadCount())
            treeItem.SetState(TVIS_BOLD, TVIS_BOLD);

        // add into map for faster search
        if(treeItem != NULL && pItem->GetId() != 0)
            m_mapFeedSearch.insert(FeedNodeMap::value_type(NodeMapKey(pItem->GetType(), pItem->GetId()), treeItem));

        if(bSort && pItem->GetType() != CFeedTreeItem::Watch)
            SortNode(itemParent);
        return treeItem;
    }

    CTreeItem InsertItem(HTREEITEM itemParent, TagPtr pTag, BOOL bSort)
    {
        CTagTreeItem* pItem = new CTagTreeItem(pTag);
        return InsertItem(itemParent, pItem, TVI_LAST,  bSort);
    }

    CTreeItem InsertItem(HTREEITEM itemParent, NewsWatchPtr pNewsWatch)
    {
        CNewsWatchTreeItem* pItem = new CNewsWatchTreeItem(pNewsWatch);
        return InsertItem(itemParent, pItem, TVI_LAST,  FALSE);
    }

    CTreeItem InsertItem(HTREEITEM itemParent, NewsFeedPtr pNewsFeed, BOOL bSort)
    {
        CNewsFeedTreeItem* pItem = new CNewsFeedTreeItem(pNewsFeed);

        if(pNewsFeed->m_id)
        {
            //Add to cache, just in case this is a new feed
            CNewsFeedCache::AddFeed(pNewsFeed);

            //Try to figure out the favicon if so configured
            if(!g_GreatNewsConfig.m_bNoFavicons)
            {
                if(!pNewsFeed->IsSearchChannel() && !LoadFavicon(pItem))
                {
                    // try to download favicon
                    if(m_pFaviconRequest)
                        m_pFaviconRequest->AddFeed(pNewsFeed);
                }
            }
        }
        return InsertItem(itemParent, pItem,  TVI_LAST,  bSort);
    }

    CTreeItem InsertItem(HTREEITEM itemParent, FeedGroupPtr pFeedGroup, BOOL bSort)
    {
        CFeedGroupTreeItem* pItem = new CFeedGroupTreeItem(pFeedGroup);
        return InsertItem(itemParent, pItem,  TVI_LAST,  bSort);
    }

    void RefreshAllChildNodes(CTreeItem& node)
    {
        RefreshNode(node);
        CTreeItem childNode = node.GetChild();
        while(childNode)
        {
            RefreshAllChildNodes(childNode);
            childNode = childNode.GetNextSibling();
        }
    }

    void RefreshNode(CTreeItem& treeItem)
    {
        CFeedTreeItem* pItem = (CFeedTreeItem*)treeItem.GetData();

        // update the text
        CString text;
        treeItem.GetText(text);
        CString newText = pItem->GetTreeNodeText();
        if(text != newText)
        {
            treeItem.SetText(newText);
            treeItem.SetState(pItem->ShowInBold() ? TVIS_BOLD : 0, TVIS_BOLD);
        }

        // update the icon
        if(treeItem.GetImageIndex() != pItem->GetIcon())
            treeItem.SetImage(pItem->GetIcon(), pItem->GetIcon());
    }

    void RefreshSelectedNode()
    {
        CTreeItem treeItem = GetSelectedItem();
        if(!treeItem)
            return;

        RefreshNode(treeItem);
    }

    void MarkAllRead()
    {
        for(FeedNodeMap::iterator it = m_mapFeedSearch.begin();
            it!=m_mapFeedSearch.end(); ++it)
        {
            CTreeItem node = it->second;
            CFeedTreeItem* pTreeItem = (CFeedTreeItem*)node.GetData();
            ATLASSERT(pTreeItem);
            pTreeItem->SetUnreadCount(0);
            RefreshNode(node);
        }

        if(m_tagRoot)
        {
            ((CFeedTreeItem*)m_tagRoot.GetData())->SetUnreadCount(0);
            RefreshNode(m_tagRoot);
        }

        if(m_watchRoot)
        {
            ((CFeedTreeItem*)m_watchRoot.GetData())->SetUnreadCount(0);
            RefreshNode(m_watchRoot);
        }
    }

    void MarkRead(HTREEITEM node)
    {
        CTreeItem item(node, this);
        MarkRead(item);
    }

    void MarkRead(CTreeItem node)
    {
        // reset this node's unread count
        CFeedTreeItem* pTreeItem = (CFeedTreeItem*)node.GetData();
        INT_PTR unreadCount = pTreeItem->GetUnreadCount();
        internalMarkRead(node);

        // calculate ancestors' unread counts
        AdjustUnreadCounts(node.GetParent(), 0 - unreadCount);
    }


    //
    // adjust unread count for specified node and its ancestors
    //
    void AdjustUnreadCounts(CTreeItem& treeItem, INT_PTR delta)
    {
        if(treeItem == NULL || delta == 0)
            return;

        CFeedTreeItem* pTreeItem = (CFeedTreeItem*)treeItem.GetData();
        INT_PTR currentUnreadCount = pTreeItem->GetUnreadCount();
        if(delta + currentUnreadCount < 0) // don't give negative counts!
            delta = -currentUnreadCount;

        ATLASSERT(pTreeItem);
        pTreeItem->AdjustUnreadCount(delta);
        RefreshNode(treeItem);

        CTreeItem parentItem = treeItem.GetParent();
        while(parentItem != NULL)
        {
            CFeedTreeItem* pParentTreeItem = (CFeedTreeItem*)parentItem.GetData();
            pParentTreeItem->AdjustUnreadCount(delta);
            RefreshNode(parentItem);
            parentItem = parentItem.GetParent();
        }
    }

    void AdjustUnreadCounts(NewsItemPtr& pNewsItem, int delta)
    {
        CTreeItem treeItem = FindNodeByIdType(pNewsItem->m_feedID, CFeedTreeItem::Channel);
        if(treeItem == NULL)
            return;
        return AdjustUnreadCounts(treeItem, delta);
    }

    CTreeItem GetUnreadNode(CFeedTreeItem::Type nodeType)
    {
        // flatten the tree into node list
        std::vector<HTREEITEM> allItems;
        size_t markPosition = GetAllNodes(allItems, GetSelectedItem());

        size_t size = allItems.size();
        if (size == 0)
            return NULL;

        // scan the list only once
        size_t n = markPosition;
        HTREEITEM rootItem = GetRootItem();
        while (markPosition != (n = n + 1 < size ? n + 1 : 0))
        {
            CFeedTreeItem* pItem = (CFeedTreeItem*)GetItemData(allItems[n]);
            ATLASSERT(pItem);
            if(allItems[n] != rootItem // do not return root node
                && pItem->GetType() == nodeType
                && pItem->GetUnreadCount() > 0)
                return CTreeItem(allItems[n], this);
        }

        return NULL;
    }

    CTreeItem GetWatchRootItem()
    {
        return m_watchRoot;
    }
    CTreeItem GetTagRootItem()
    {
        return m_tagRoot;
    }

    CFeedTreeItem* GetSelectedFeedItem()
    {
        CTreeItem item = GetSelectedItem();
        if(item)
            return (CFeedTreeItem*)item.GetData();

        return NULL;
    }

    CTreeItem FindNodeByIdType(ULONG_PTR id, CFeedTreeItem::Type t)
    {
        FeedNodeMap::iterator it = m_mapFeedSearch.find(NodeMapKey(t, id));
        if(it != m_mapFeedSearch.end())
            return it->second;

        return CTreeItem();
    }

    void ExpandAll(bool bExpand=true)
    {
        GetRootItem().Expand(bExpand? TVE_EXPAND:TVE_COLLAPSE);

        CTreeItem childNode = GetRootItem().GetChild();
        while(childNode)
        {
            childNode.Expand(bExpand? TVE_EXPAND:TVE_COLLAPSE);
            childNode = childNode.GetNextSibling();
        }
    }

    void CheckAll(bool bCheck = true, CTreeItem startItem=NULL)
    {
        CTreeItem rootItem = GetRootItem();

        if (startItem == NULL)
            startItem = rootItem;

        if (startItem != rootItem)
            SetCheckState(startItem, bCheck);

        CTreeItem childNode = startItem.GetChild();
        while (childNode)
        {
            CheckAll(bCheck, childNode);
            childNode = childNode.GetNextSibling();
        }
    }

    void NormalizeSelection(CTreeItem item=NULL)
    {
        // if an item is checked, make sure its parent is also checked
        if(item == NULL)
            item = GetRootItem();

        if(GetCheckState(item))
        {
            CTreeItem parentItem = item.GetParent();
            if(parentItem && !GetCheckState(parentItem))
                SetCheckState(parentItem, true);
        }
        CTreeItem childNode = item.GetChild();
        while(childNode)
        {
            NormalizeSelection(childNode);
            childNode = childNode.GetNextSibling();
        }

    }

    void UpdateUIUnreadCount()
    {
        for(FeedNodeMap::iterator it = m_mapFeedSearch.begin(); it != m_mapFeedSearch.end(); ++it)
        {
            CTreeItem item = it->second;
            CFeedTreeItem* pItem = (CFeedTreeItem*)item.GetData();
            pItem->UpdateUIUnreadCount();
            RefreshNode(item);
        }
    }

    size_t RetrieveUIUnreadCount(std::vector<std::pair<ULONG_PTR, INT_PTR> >& counts, int type)
    {
        counts.clear();

        for(FeedNodeMap::iterator it = m_mapFeedSearch.begin(); it != m_mapFeedSearch.end(); ++it)
        {
            NodeMapKey key = it->first;
            if(key.m_type == type)
            {
                CTreeItem item = it->second;
                CFeedTreeItem* pItem = (CFeedTreeItem*)item.GetData();
                if(pItem->IsUnreadCountChanged())
                    counts.push_back(std::pair<ULONG_PTR, INT_PTR>(key.m_id, pItem->GetUnreadCount()));
            }
        }

        return counts.size();
    }

public:
    void LoadTree(LoadTreeType loadType = LoadAll)
    {
        DeleteAllItems();

        // Load news feeds and groups
        if(loadType & LoadChannel || loadType & LoadGroup ) // group is loaded if channel need to be loaded
        {
            // insert root group
            FeedGroupPtr spRootFeedGroup = CFeedGroup::GetRootFeedGroup();
            CRootGroupFeedTreeItem* pItem = new CRootGroupFeedTreeItem(spRootFeedGroup);
            CTreeItem itemRoot = InsertItem(TVI_ROOT, pItem, TVI_LAST, FALSE);

            // insert child groups
            FeedGroupVector childGroups;
            spRootFeedGroup->GetChildGroups(childGroups, CFeedGroup::SortByName);
            // std::sort(childGroups.begin(), childGroups.end(), LessGroup);
            for(FeedGroupVector::iterator it = childGroups.begin(); it != childGroups.end(); ++it)
            {
                FeedGroupPtr& spGroup = *it;
                InsertItem(itemRoot, spGroup, FALSE);
            }
            GetRootItem().Expand(TVE_EXPAND);
        }

        if(loadType & LoadChannel)
        {
            CNewsFeedCache::Load();

            // load all newsfeed
            NewsFeedVector newsfeeds;
            CNewsFeedCache::GetAllNewsFeeds(newsfeeds);
            std::sort(newsfeeds.begin(), newsfeeds.end(), LessFeed);

            for(NewsFeedVector::iterator nit = newsfeeds.begin(); nit != newsfeeds.end(); ++nit)
            {
                NewsFeedPtr& spNewsFeed = *nit;
                if(spNewsFeed->IsCommentChannel())
                    continue; // we don't have to show comment channel on tree

                if((LoadSearch & loadType)==0 && spNewsFeed->IsSearchChannel())
                    continue; // skip search channels if they are not explicitly asked for

                CTreeItem groupItem = FindNodeByIdType(spNewsFeed->m_groupID, CFeedTreeItem::Group);
                if (groupItem != NULL)
                    InsertItem(groupItem, spNewsFeed, FALSE);
            }

            GetRootItem().Expand(TVE_EXPAND);
        }

        // load the watch tree
        if(loadType & LoadWatch)
        {
            CWatchRootTreeItem* pWR = new CWatchRootTreeItem();
            m_watchRoot = InsertItem(TVI_ROOT, pWR, TVI_LAST, FALSE);

            NewsWatchVector newsWatches;
            if(CNewsWatch::GetAllNewsWatches(newsWatches, CNewsWatch::SortByDisplayOrder))
                for(NewsWatchVector::iterator it = newsWatches.begin(); it != newsWatches.end();++it)
                    InsertItem(m_watchRoot, *it);
        }

        // load labels
        if(loadType & LoadLabel)
        {
            CTagRootTreeItem* pTR = new CTagRootTreeItem();
            m_tagRoot = InsertItem(TVI_ROOT, pTR, TVI_LAST, FALSE);
            TagVector allTags;
            if(CTag::GetAllTags(allTags))
            {
                for(TagVector::iterator it = allTags.begin(); it != allTags.end(); ++it)
                {
                    TagPtr tag = *it;
                    pTR->AdjustUnreadCount(tag->m_unreadCount);
                    InsertItem(m_tagRoot, tag, FALSE);
                }
            }
            RefreshNode(m_tagRoot);
            SortNode(m_tagRoot);
        }
    }


    void SetSelectedChannels(std::vector<ULONG_PTR>& channels)
    {
        CheckAll(false);

        for(std::vector<ULONG_PTR>::iterator it = channels.begin(); it!=channels.end(); ++it)
        {
            CTreeItem treeNode = FindNodeByIdType(*it, CFeedTreeItem::Channel);
            if(treeNode != NULL)
                SetCheckState(treeNode, true);
        }

        NormalizeSelection();
    }

    bool IsChannelUpdating(ULONG_PTR channelID)
    {
        CTreeItem item = FindNodeByIdType(channelID, CFeedTreeItem::Channel);
        if(item == NULL)
            return false;
        int overlay = GetItemState(item, TVIS_OVERLAYMASK) & TVIS_OVERLAYMASK;
        return ( overlay != 0);
    }

    int GetChannelCount()
    {
        return (int)std::count_if(m_mapFeedSearch.begin(), m_mapFeedSearch.end(), isChannel);
    }


    FeedGroupPtr GetDefaultFeedGroup()
    {
        CTreeItem item = GetSelectedItem();
        if(item == NULL)
            item = GetRootItem();

        CFeedTreeItem* pItem = (CFeedTreeItem*)item.GetData();
        return pItem->GetFeedGroup();
    }

    size_t GetChildChannels(ULONG_PTR nGroupID,NewsFeedVector& channels, bool bIncludeDisabled)
    {
        CTreeItem groupNode = FindNodeByIdType(nGroupID, CFeedTreeItem::Group);
        if(groupNode == NULL)
            return 0;

        CTreeItem childNode = groupNode.GetChild();
        while(childNode)
        {
            CNewsFeedTreeItem* pChannelItem = dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)childNode.GetData());
            if(pChannelItem && (!IsFeedDisabled(pChannelItem) || bIncludeDisabled))
                channels.push_back(pChannelItem->m_newsFeed);

            childNode = childNode.GetNextSibling();
        }

        return channels.size();
    }

    size_t GetAllChannels(NewsFeedVector& channels, bool bSelectedOnly = false)
    {
        channels.clear();
        channels.reserve(GetCount());

        CTreeItem rootNode = GetRootItem();
        CTreeItem childNode = rootNode.GetChild();
        while(childNode)
        {
            CTreeItem currentNode = childNode;
            childNode = childNode.GetNextSibling();

            CFeedGroupTreeItem* pGroup = dynamic_cast<CFeedGroupTreeItem*>((CFeedTreeItem*)currentNode.GetData());
            if(pGroup)
            {
                CTreeItem feedNode = currentNode.GetChild();
                while(feedNode)
                {
                    if(!bSelectedOnly || GetCheckState(feedNode))
                    {
                        CNewsFeedTreeItem* pFeed = dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)feedNode.GetData());
                        if(pFeed)
                        {
                            channels.push_back(pFeed->m_newsFeed);
                        }
                    }

                    feedNode = feedNode.GetNextSibling();
                }
                continue;
            }

            //  feed at root level
            CNewsFeedTreeItem* pFeed = dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)currentNode.GetData());
            if(pFeed && (!bSelectedOnly || GetCheckState(currentNode)))
            {
                channels.push_back(pFeed->m_newsFeed);
            }
        }

        return channels.size();

    }

    void SortNode(HTREEITEM node)
    {
        TVSORTCB sortCB = {0};
        sortCB.hParent = node;
        sortCB.lpfnCompare=CompareFunc;

        SortChildrenCB(&sortCB, 0);
    }

    bool IsFeedDisabled(CNewsFeedTreeItem* pFeedItem)
    {
        if(pFeedItem->Disabled())
            return true;

        CTreeItem node = FindNodeByIdType(pFeedItem->m_newsFeed->m_groupID, CFeedTreeItem::Group);
        if(node != NULL)
        {
            CFeedGroupTreeItem* pGroupItem = (CFeedGroupTreeItem*)node.GetData();
            if(pGroupItem && pGroupItem->Disabled())
                return true;
        }

        return false;
    }

    bool IsFeedDisabled(NewsFeedPtr feed)
    {
        if(feed->m_bDisabled)
            return true;

        CTreeItem node = FindNodeByIdType(feed->m_groupID, CFeedTreeItem::Group);
        if(node != NULL)
        {
            CFeedGroupTreeItem* pGroupItem = (CFeedGroupTreeItem*)node.GetData();
            if(pGroupItem && pGroupItem->Disabled())
                return true;
        }

        return false;
    }

    bool HasBloglinesSyncChannel()
    {
        for(FeedNodeMap::iterator it = m_mapFeedSearch.begin(); it != m_mapFeedSearch.end(); ++it)
        {
            CTreeItem item = it->second;
            CFeedTreeItem* pItem = (CFeedTreeItem*)item.GetData();
            if(pItem->GetType() == CFeedTreeItem::Channel)
            {
                CNewsFeedTreeItem* pFeedItem = (CNewsFeedTreeItem*)pItem;
                if(pFeedItem->m_newsFeed->IsBloglinesSyncChannel())
                    return true;
            }
        }

        return false;
    }
protected:
    size_t GetAllNodes(std::vector<HTREEITEM>& allItems, HTREEITEM markItem = NULL)
    {
        allItems.clear();
        allItems.reserve(GetCount());
        size_t markPosition = 0;

        internalGetAllNodes(allItems, markPosition, GetRootItem(), markItem);
        return markPosition;
    }


protected:
    FeedNodeMap m_mapFeedSearch;

    CTreeItem m_watchRoot;
    CTreeItem m_tagRoot;

    CFaviconRequest* m_pFaviconRequest;

private:
    bool LoadFavicon(CNewsFeedTreeItem* pItem)
    {
        NewsFeedPtr& pNewsFeed = pItem->m_newsFeed;

        CString faviconFile = g_GreatNewsConfig.GetFaviconDir();
        faviconFile.AppendFormat(_T("\\F%d.ico"), pNewsFeed->m_id);

        // if the file is too old, let's delete it
        CFile file;
        FILETIME create, access, modify;
        if(file.Open(faviconFile) && file.GetFileTime(&create, &access, &modify))
        {
            file.Close();
            CTime createTime(create);
            CTimeSpan span = CTime::GetCurrentTime() - createTime;
            if(span.GetDays() > 7) // created a week ago?
                CFile::Delete(faviconFile);
        }

        if(CFile::FileExists(faviconFile))
        {
            // load favicon to image list
            HICON hFavi = (HICON)::LoadImage(NULL,faviconFile,IMAGE_ICON,16,16,LR_LOADFROMFILE);
            if(hFavi != NULL)
            {
                CImageList imageList = GetImageList(TVSIL_NORMAL);
                if(imageList != NULL)
                {
                    pItem->m_faviconIndex = imageList.AddIcon(hFavi);
                    ::DestroyIcon(hFavi);
                    return true;
                }
            }
        }

        return false;
    }

    void internalGetAllNodes(std::vector<HTREEITEM>& allItems, size_t& markPosition, HTREEITEM startItem, HTREEITEM markItem)
    {
        if (startItem == markItem)
            markPosition = allItems.size();

        allItems.push_back(startItem);
        HTREEITEM childItem = GetChildItem(startItem);
        while(NULL != childItem)
        {
            internalGetAllNodes(allItems, markPosition, childItem, markItem);
            childItem = GetNextSiblingItem(childItem);
        }
    }

    // mark specified node and all its children as read
    void internalMarkRead(CTreeItem& node)
    {
        // reset this node's unread count
        CFeedTreeItem* pTreeItem = (CFeedTreeItem*)node.GetData();
        ATLASSERT(pTreeItem);
        pTreeItem->SetUnreadCount(0);
        RefreshNode(node);

        // reset children's unread count
        CTreeItem childNode = node.GetChild();
        while(childNode)
        {
            internalMarkRead(childNode);
            RefreshNode(childNode);
            childNode = childNode.GetNextSibling();
        }
    }

private:

    static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
    {
        CFeedTreeItem* pItem1 = (CFeedTreeItem*)lParam1;
        CFeedTreeItem* pItem2 = (CFeedTreeItem*)lParam2;

        if(pItem1->GetType()<pItem2->GetType())
            return -1;

        // we only compare two node types, so it has to be the same type to come here
        return pItem1->GetName().CompareNoCase(pItem2->GetName());
    }

private: // STL help functions
    static bool isChannel(const std::pair<NodeMapKey, CTreeItem>& pr)
    {
        return (pr.first.m_type == CFeedTreeItem::Channel);
    }
    static void ClearChannelStatus(const std::pair<NodeMapKey, CTreeItem>& t)
    {
        if(isChannel(t))
        {
            CTreeItem item = t.second;
            item.SetState(INDEXTOOVERLAYMASK(0), TVIS_OVERLAYMASK);
        }
    }
};

class CFeedTreeCtrlBase : public CFeedTreeCtrlImpl<CFeedTreeCtrlBase>
{
public:
    DECLARE_WND_SUPERCLASS(_T("GN_FEEDTREE"), GetWndClassName())
};
