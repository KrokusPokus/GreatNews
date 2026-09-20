#include "StdAfx.h"
#include "resource.h"
#include "FeedTreeCtrl.h"
#include "FeedManagerLib.h"
#include "WtlExt.h"
#include <strsafe.h>

#ifndef GCL_HCURSOR
/* x64 WinDDK build doesn't have this (been superceded
by GCLP_HCURSOR); manually define for now. FIXME! */
#define GCL_HCURSOR -12
#endif

CFeedTreeCtrl::CFeedTreeCtrl(void) : 
	m_bDragging(false),
  m_pDragItem(NULL),
  m_hDragItem(NULL),
  m_hPreviousNode(NULL),
	m_bAllowLabelEditing(false)
{
	m_UniformResourceLocatorFormat = (CLIPFORMAT)RegisterClipboardFormat(_T("UniformResourceLocator"));
	AcceptFormat(m_UniformResourceLocatorFormat);
}

CFeedTreeCtrl::~CFeedTreeCtrl(void)
{
}

LRESULT CFeedTreeCtrl::OnClick(LPNMHDR pnmh)
{
	SetFocus();

	DWORD dwPos = GetMessagePos();
	CPoint pt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );
	CPoint spt = pt;
	ScreenToClient( &pt );
	UINT uFlags;
	CTreeItem htItem = HitTest(pt, &uFlags);
	if (htItem && (uFlags & TVHT_ONITEM))
		GetParent().PostMessage(WM_COMMAND, CMD_ID_REFRESH, 0L);

	SetMsgHandled(FALSE);

	return 0;
}

LRESULT CFeedTreeCtrl::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;

	int cx = LOWORD(lParam);
	int cy = HIWORD(lParam);

	if(cx<=2)
	{
		if(IsWindowVisible())
			ShowWindow(SW_HIDE);

		return 1;
	}

	if(cx>2 && !IsWindowVisible())
		ShowWindow(SW_SHOW);

	return 1;
}

LRESULT CFeedTreeCtrl::OnBeginLabelEdit(LPNMHDR pnmh)
{
	// check this label editing is from menu command(allow) or mouse action(deny)
	if(!m_bAllowLabelEditing)
		return TRUE; // cancel it

	m_bAllowLabelEditing = false;

	LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)pnmh; 
	CFeedTreeItem* pFeedTreeItem = (CFeedTreeItem*)ptvdi->item.lParam;
	ATLASSERT(pFeedTreeItem);

	return pFeedTreeItem->CanRename() ? FALSE : TRUE;
}

LRESULT CFeedTreeCtrl::OnEndLabelEdit(LPNMHDR pnmh)
{
	try
	{
		LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)pnmh; 
		if(ptvdi->item.pszText == NULL) // label editing canceled?
			return FALSE;

		CFeedTreeItem* pFeedTreeItem = (CFeedTreeItem*)ptvdi->item.lParam;
		ATLASSERT(pFeedTreeItem);

		if(pFeedTreeItem->GetName() != ptvdi->item.pszText)
		{
			pFeedTreeItem->Rename(ptvdi->item.pszText);
			StringCchCopy(ptvdi->item.pszText, ptvdi->item.cchTextMax, 
				(LPCTSTR)pFeedTreeItem->GetTreeNodeText());
			return TRUE;
		}

	}
	CATCH_ALL_ERROR()

	return FALSE;
}



LRESULT CFeedTreeCtrl::OnBeginDrag(LPNMHDR pnmh)
{
	LPNMTREEVIEW pNMTreeView = (LPNMTREEVIEW) pnmh;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	CFeedTreeItem* pFeedTreeItem = (CFeedTreeItem*)GetItemData(hItem);
	if(!pFeedTreeItem || !pFeedTreeItem->CanDrag())
	{
		SetMsgHandled(false);
		return 0;
	}

	m_dragImage = CreateDragImage(hItem);
	ATLASSERT(m_dragImage);

	//
	// Compute the coordinates of the "hot spot"--the location of the
	// cursor relative to the upper left corner of the item rectangle.
	//
	CRect rect;
	GetItemRect (hItem, rect, TRUE);
	CPoint point (pNMTreeView->ptDrag.x, pNMTreeView->ptDrag.y);
	CPoint hotSpot = point;
	hotSpot.x -= rect.left;
	hotSpot.y -= rect.top;

	//
	// Convert the client coordinates in "point" to coordinates relative
	// to the upper left corner of the control window.
	//
	CPoint client (0, 0);
	ClientToScreen (&client);
	GetWindowRect (rect);
	point.x += client.x - rect.left;
	point.y += client.y - rect.top;

	SetCapture ();
	m_dragImage.BeginDrag(0, hotSpot);
	m_dragImage.DragEnter(m_hWnd, point);
	m_hDragItem = hItem;
	m_pDragItem = (CFeedTreeItem*)GetItemData(m_hDragItem);
	m_bDragging = true;

	return 0;
}

/*
void CFeedTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	static HCURSOR hcurNo = LoadCursor ( NULL, IDC_NO );

	if(m_bDragging)
	{
		// Erase the old drag image and draw a new one.
		if(m_pDragItem->GetType() == CFeedTreeItem::Watch)
			RemoveInsertMark();
		m_dragImage.DragMove (point);

		// Highlight the drop target if the cursor is over an item.
		HTREEITEM hItem = HighlightDropTarget (point);
		if(hItem)
		{
			CFeedTreeItem* pFeedTreeItem = (CFeedTreeItem*)GetItemData(hItem);
			if(hItem == m_hDragItem || !pFeedTreeItem || !pFeedTreeItem->CanDrop(m_pDragItem))
				hItem = NULL;
		}

		if(m_pDragItem->GetType() == CFeedTreeItem::Watch && hItem)
			SetInsertMark(hItem, true);

		::SetCursor (hItem == NULL ? hcurNo : (HCURSOR)GetClassLong(m_hWnd, GCL_HCURSOR));
	}
	else
		SetMsgHandled(false);

	return;
}
*/
void CFeedTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
// OLE-System-Cursor einmalig aus ole32.dll laden
    static HMODULE hOle32   = ::GetModuleHandle(_T("ole32.dll"));
    static HCURSOR hcurNo   = hOle32 ? ::LoadCursor(hOle32, MAKEINTRESOURCE(1)) : ::LoadCursor(NULL, IDC_NO);
    static HCURSOR hcurMove = hOle32 ? ::LoadCursor(hOle32, MAKEINTRESOURCE(2)) : ::LoadCursor(NULL, IDC_ARROW);
    static HCURSOR hcurArrow  = ::LoadCursor(NULL, IDC_ARROW);   // Standard-Pfeil

    if (!m_bDragging)
    {
        SetMsgHandled(false);
        return;
    }

    // 1. Drag-Bild vor der Positionsänderung aktualisieren
    if (m_pDragItem && m_pDragItem->GetType() == CFeedTreeItem::Watch)
        RemoveInsertMark();

    m_dragImage.DragMove(point);

    // 2. Drop-Ziel ermitteln und validieren
    HTREEITEM hItem = HighlightDropTarget(point);
    CFeedTreeItem* pTargetItem = hItem ? (CFeedTreeItem*)GetItemData(hItem) : NULL;

    bool bValidDrop = (hItem != NULL) && 
                      (hItem != m_hDragItem) && 
                      (pTargetItem != NULL) && 
                      pTargetItem->CanDrop(m_pDragItem);

    if (!bValidDrop)
    {
        hItem = NULL;
        pTargetItem = NULL;
    }

    // 3. Passenden Cursor basierend auf Aktion und Ziel wählen
    HCURSOR hSelectedCursor = hcurNo; // Standard: Nicht erlaubt

    if (bValidDrop)
    {
        // Fall A: Reordering / Einsortieren (z. B. bei Watches oder Channels)
        if (m_pDragItem->GetType() == CFeedTreeItem::Watch)
        {
            SetInsertMark(hItem, true);
            hSelectedCursor = hcurMove; // Signalisiert vertikales Einsortieren zwischen Elementen
        }
        // Fall B: Verschieben IN eine Gruppe / einen Ordner
        else if (pTargetItem->GetType() == CFeedTreeItem::Group)
        {
            hSelectedCursor = hcurMove; // Signalisiert Droppen IN ein Objekt
        }
        // Fall C: Regulärer Move auf einen anderen Feed
        else
        {
            hSelectedCursor = hcurMove;
        }

        // Optional: Modifier-Key-Prüfung (z. B. STRG für Kopieren statt Verschieben)
        if (nFlags & MK_CONTROL)
        {
            // Falls Kopieren unterstützt wird, könnte hier ein Kopier-Cursor gesetzt werden
            // hSelectedCursor = hcurCopy;
        }
    }

    // 4. Cursor setzen
    ::SetCursor(hSelectedCursor);
}

void CFeedTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(!m_bDragging)
  {
		SetMsgHandled(false);
	  return;
  }

	// Terminate the dragging operation and release the mouse.
	m_dragImage.DragLeave(m_hWnd);
	m_dragImage.EndDrag();
	::ReleaseCapture();
	m_bDragging = false;
	SelectDropTarget(NULL);
	m_dragImage.Destroy();

	if(m_pDragItem->GetType() == CFeedTreeItem::Watch)
		RemoveInsertMark();

	// Get the HTREEITEM of the drop target and exit now if it's NULL.
	HTREEITEM hItem = HitTest (point, &nFlags);
	if (hItem == NULL || hItem==m_hDragItem)
		return;

	CFeedTreeItem* pFeedTreeItem = (CFeedTreeItem*)GetItemData(hItem);
	if(!pFeedTreeItem || !pFeedTreeItem->CanDrop(m_pDragItem))
		return;

	// Move the dragged item to the drop point.
	if(pFeedTreeItem->GetType() == CFeedTreeItem::Channel || pFeedTreeItem->GetType() == CFeedTreeItem::Group)
		MoveFeed(hItem, m_hDragItem);
	else if(pFeedTreeItem->GetType() == CFeedTreeItem::Watch)
		MoveWatch(hItem, m_hDragItem);

	m_hDragItem = NULL;
	m_pDragItem = NULL;
}

void CFeedTreeCtrl::MoveWatch(HTREEITEM hDest, HTREEITEM hSrc)
{
	try
	{
		CNewsWatchTreeItem* pFromTreeItem = dynamic_cast<CNewsWatchTreeItem*>((CFeedTreeItem*)GetItemData(hSrc));
		ATLASSERT(pFromTreeItem);

		SetItemData(hSrc, 0);
		DeleteItem(hSrc);

		CTreeItem watchRoot = GetWatchRootItem();
		CTreeItem newItem = InsertItem(watchRoot, pFromTreeItem, hDest);
		if(newItem == NULL)
			return; // something's wrong

		std::vector<ULONG_PTR> watchIDs;
		watchIDs.reserve(20);
		CTreeItem child = watchRoot.GetChild();
		while(child!=NULL)
		{
			CNewsWatchTreeItem* pItem = (CNewsWatchTreeItem*)child.GetData();
			watchIDs.push_back(pItem->m_NewsWatch->m_id);
			child = child.GetNextSibling();
		}
		
		CNewsWatch::SetOrder(watchIDs);
		CNewsItemCache::Empty();

		newItem.Select();

		GetParent().PostMessage(WM_COMMAND, CMD_ID_REFRESH, 0L);
	}
	CATCH_ALL_ERROR()
}

ULONG_PTR CFeedTreeCtrl::GetFeedGroupId(HTREEITEM hItem)
{
	ULONG_PTR groupID = 0;

	CNewsFeedTreeItem* pFeedToTreeItem = 
		dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)GetItemData(hItem));
	if(pFeedToTreeItem != NULL)
		groupID = pFeedToTreeItem->m_newsFeed->m_groupID;
	else
	{
		CFeedGroupTreeItem* pGroupItem = dynamic_cast<CFeedGroupTreeItem*>((CFeedTreeItem*)GetItemData(hItem));
		if(pGroupItem != NULL)
			groupID = pGroupItem->m_feedGroup->m_id;
	}

	return groupID;
}

void CFeedTreeCtrl::MoveFeed(HTREEITEM hDest, HTREEITEM hSrc)
{
	try
	{
		CNewsFeedTreeItem* pFeedTreeItem = dynamic_cast<CNewsFeedTreeItem*>((CFeedTreeItem*)GetItemData(hSrc));
		ATLASSERT(pFeedTreeItem);

		//
		// change feed group
		//
		NewsFeedPtr newsFeed = pFeedTreeItem->m_newsFeed;
		ULONG_PTR groupID = GetFeedGroupId(hDest);
		if(groupID == 0)
			return; // what's wrong?
		if(newsFeed->m_groupID == groupID)
			return; // same group, nothing need to do
		hDest = FindNodeByIdType(groupID, CFeedTreeItem::Group); // adjust to the target group node

		newsFeed->m_groupID = groupID;
		newsFeed->Save();

		// move the tree item
		MoveFeedNode(hDest, hSrc);
	}
	CATCH_ALL_ERROR()
}

void CFeedTreeCtrl::MoveFeedNode(HTREEITEM hDest, HTREEITEM hSrc)
{
	CFeedTreeItem* pFeedTreeItem = (CFeedTreeItem*)GetItemData(hSrc);
	if(pFeedTreeItem->GetUnreadCount())
	{
		// adjust unread count
		HTREEITEM hParent = GetParentItem(hSrc);
		AdjustUnreadCounts(CTreeItem(hParent, this), -pFeedTreeItem->GetUnreadCount());
		AdjustUnreadCounts(CTreeItem(hDest, this), pFeedTreeItem->GetUnreadCount());
	}
	SetItemData(hSrc, 0);
	DeleteItem(hSrc);
	CTreeItem newItem = InsertItem(hDest, pFeedTreeItem);
	newItem.Select();
}

LRESULT CFeedTreeCtrl::OnGetInfoTip(LPNMHDR pnmh)
{
	LPNMTVGETINFOTIP pGetInfoTip = (LPNMTVGETINFOTIP)pnmh;
	
	CFeedTreeItem* pItem = (CFeedTreeItem*)pGetInfoTip->lParam;
  if(!pItem || pItem->GetType() != CFeedTreeItem::Channel || pItem->m_bUpdateFailed) {
    SetMsgHandled(false);
    return 0;
  }

	CNewsFeedTreeItem* pFeedItem = (CNewsFeedTreeItem*)pItem;
	if(pFeedItem->m_newsFeed->m_updateErrMsg.GetLength())
	{
		StringCchCopy (pGetInfoTip->pszText, pGetInfoTip->cchTextMax, 
			(LPCTSTR)(pFeedItem->m_newsFeed->m_updateErrMsg));
	}
	else
	{
		StringCchCopy (pGetInfoTip->pszText, pGetInfoTip->cchTextMax, _T("Update failed"));
	}

	return 0;
}

LRESULT CFeedTreeCtrl::OnClickContextMenu(LPNMHDR pnmh)
{
	DWORD dwPos = GetMessagePos();
	CPoint pt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );
	return HandleContextMenu(pt);
}

LRESULT CFeedTreeCtrl::OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt( GET_X_LPARAM( lParam ), GET_Y_LPARAM ( lParam ) );
	if(pt.x == -1 && pt.y==-1)
	{
		// this message is generated by SHIFT+F10 or Context Meny key
		CTreeItem treeItem = GetSelectedItem();
		if(!treeItem)
			treeItem = GetRootItem();
		RECT rc;
		treeItem.GetRect(&rc, TRUE);
		pt.x = rc.left+5;
		pt.y = rc.top+5;
		ClientToScreen(&pt);
	}

	return HandleContextMenu(pt);
}

LRESULT CFeedTreeCtrl::HandleContextMenu(CPoint pt)
{
	CPoint spt = pt;
	/* convert to screen co-ords for the hittesting to work */
	ScreenToClient( &pt );
	UINT uFlags;
	CTreeItem htItem = HitTest(pt, &uFlags);
	if (htItem && (uFlags & TVHT_ONITEM))
	{
		htItem.Select();

		CFeedTreeItem* pItem = (CFeedTreeItem*)htItem.GetData();

        CMenu menu;
		UINT menuID = pItem->GetContextMenuID();
		if(menuID > 0)
			menu.LoadMenu(menuID);

		CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
		pResMngr->ApplyLanguageToMenu(menu.GetSubMenu(0));

		menu.GetSubMenu(0).TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, spt.x, spt.y, GetParent());
	}
	else
		SetMsgHandled(false);

	return 0;
}



DWORD CFeedTreeCtrl::OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD)
{
	LPNMTVCUSTOMDRAW lpTVCD = (LPNMTVCUSTOMDRAW) lpNMCD;

	CFeedTreeItem* pItem = (CFeedTreeItem*)lpTVCD->nmcd.lItemlParam;
	if(pItem)
	{
		// foreground color
		COLORREF txtColor = CFeedTreeItem::DefaultColor;
		HTREEITEM hItem = (HTREEITEM)lpNMCD->dwItemSpec;
		HTREEITEM hParent = GetParentItem(hItem);
		if(hParent)
		{
			CFeedTreeItem* pParentItem = (CFeedTreeItem*)GetItemData(hParent);
			if(pParentItem->Disabled()) // propogate parent's disabled color to children
				txtColor = pParentItem->GetColor();
		}

		if(txtColor == CFeedTreeItem::DefaultColor) // if parent hasn't been disabled
			txtColor = pItem->GetColor();

		if(txtColor!= CFeedTreeItem::DefaultColor)
			lpTVCD->clrText = txtColor;

		// background color
		COLORREF bkColor = pItem->GetBkColor();
		if(bkColor != CFeedTreeItem::DefaultColor)
			lpTVCD->clrTextBk = bkColor;
	}

	return CDRF_DODEFAULT;
}


HTREEITEM CFeedTreeCtrl::HighlightDropTarget(CPoint point)
{
//*DEBUG*/ char buf[256];
//*DEBUG*/ snprintf(buf, sizeof(buf), "[DEBUG] GreatNews HighlightDropTarget() at %d/%d", point.x, point.y);
//*DEBUG*/ OutputDebugStringA(buf);

	// Find out which item (if any) the cursor is over.
	UINT nFlags;
	HTREEITEM hItem = HitTest (point, &nFlags);

	// Highlight the item, or unhighlight all items if the cursor isn't over an item.
	m_dragImage.DragShowNolock (FALSE);
	SelectDropTarget (hItem);
	m_dragImage.DragShowNolock (TRUE);

	// Return the handle of the highlighted item.
	return hItem;
}


LRESULT CFeedTreeCtrl::OnReceiveCheckUnreadResult(UINT /*uMsg*/, WPARAM , LPARAM pUpdateResult, BOOL& /*bHandled*/)
{
	std::auto_ptr<CCheckUnreadResult> spUpdateResult((CCheckUnreadResult*)pUpdateResult);

	CTreeItem node = FindNodeByIdType(spUpdateResult->m_id, spUpdateResult->m_type);
	if(!node)
		return 0;

	AdjustUnreadCounts(node, spUpdateResult->m_unread);

	return 0;
}

void CFeedTreeCtrl::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_DELETE)
		GetParent().PostMessage(WM_COMMAND, ID_CHANNEL_DELETE, 0);
	else if(nChar == VK_RETURN)
	{
		GetParent().PostMessage(WM_COMMAND, CMD_ID_REFRESH, 0);
		SetMsgHandled(FALSE);
	}
	else
		SetMsgHandled(FALSE);

	return;
}

LRESULT CFeedTreeCtrl::OnItemSelectionChanged(LPNMHDR pnmh)
{
	LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;
	m_hPreviousNode = pnmtv->itemOld.hItem;
	return 0;
}

LRESULT CFeedTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	LRESULT lRes = DefWindowProc();
	if(lRes == -1)
		return -1;

	RegisterDropWindow(m_hWnd);

	return 0;
}

void CFeedTreeCtrl::OnDestroy()
{
	UnregisterDropWindow();
}

DWORD CFeedTreeCtrl::OnDragOver(UINT unFmt, void *pDrop, CPoint pt)
{
//*DEBUG*/ char buf[256];
//*DEBUG*/ snprintf(buf, sizeof(buf), "[DEBUG] GreatNews OnDragOver() at %d/%d", pt.x, pt.y);
//*DEBUG*/ OutputDebugStringA(buf);

  UINT nFlags;
  HTREEITEM hDropOn = HitTest(pt, &nFlags);
  if(hDropOn!=NULL)
		SelectItem(hDropOn);

	return IsValidDrop(unFmt, pDrop, pt);
}

void CFeedTreeCtrl::OnDragDrop(UINT unFmt, void *pDrop, CPoint pt)
{
//*DEBUG*/ char buf[256];
//*DEBUG*/ snprintf(buf, sizeof(buf), "[DEBUG] GreatNews OnDragDrop(A) at %d/%d", pt.x, pt.y);
//*DEBUG*/ OutputDebugStringA(buf);

	UINT nFlags;
	HTREEITEM hDropOn = HitTest(pt, &nFlags);
	if(hDropOn == NULL  || !(nFlags & (TVHT_ONITEMLABEL | TVHT_ONITEMICON | TVHT_ONITEM)))	// Test
		return;
//*DEBUG*/ snprintf(buf, sizeof(buf), "[DEBUG] GreatNews-Wine OnDragDrop(B) at %d/%d", pt.x, pt.y);
//*DEBUG*/ OutputDebugStringA(buf);

	CDragDropData* pData = new CDragDropData();
	pData->m_url = (LPCSTR)pDrop;
    pData->m_defaultGroupId = GetFeedGroupId(hDropOn);
	if(pData->m_defaultGroupId <= 0)
		return; // something is wrong

	// Check for valid URL
	GetParent().PostMessage(WM_COMMAND, CMD_ID_SUBCHANNEL, (LPARAM)pData);
}

DWORD CFeedTreeCtrl::IsValidDrop(UINT unFmt, void *pDrop, CPoint pt)
{
//*DEBUG*/ char buf[256];
//*DEBUG*/ snprintf(buf, sizeof(buf), "[DEBUG] GreatNews IsValidDrop() at %d/%d", pt.x, pt.y);
//*DEBUG*/ OutputDebugStringA(buf);

  UINT nFlags;
  HTREEITEM hDropOn = HitTest(pt, &nFlags);
  if(hDropOn==NULL || GetFeedGroupId(hDropOn) <= 0)
		return DROPEFFECT_NONE;

	if(m_UniformResourceLocatorFormat == unFmt && pDrop != NULL)
	{
		CString url = (LPCSTR)pDrop;
		url.MakeLower();
		if(  url.Find(_T("http")) == 0 
			|| url.Find(_T("feed")) == 0 
			|| url.Find(_T("www.")) == 0 )
			return DROPEFFECT_LINK;
	}

	return DROPEFFECT_NONE;
}

void CFeedTreeCtrl::OnMButtonUp(UINT nFlags, CPoint point)
{
  UINT uFlags;
	CTreeItem htItem = HitTest(point, &uFlags);
	if (htItem && (uFlags & TVHT_ONITEM))
	{
		htItem.Select();
		GetParent().PostMessage(WM_COMMAND, CMD_ID_SHOWCHANNELWEBSITE, 0);
	}

	return;
}
