#pragma once

#include "NewsFilter.h"
#include "FeedTreeCtrlBase.h"
#include "CheckUnreadRequest.h"
#include "DragDropObj.h"

class CDragDropData
{
public:
	CString m_url;
	ULONG_PTR m_defaultGroupId;
};

class CFeedTreeCtrl : public CFeedTreeCtrlImpl<CFeedTreeCtrl>,
					  public CCustomDraw<CFeedTreeCtrl>,
					  public CDropTargetEx
{
public:
	CFeedTreeCtrl(void);
	~CFeedTreeCtrl(void);

	BEGIN_MSG_MAP(CFeedTreeCtrl)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_RCLICK, OnClickContextMenu)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnItemSelectionChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_GETINFOTIP, OnGetInfoTip)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_BEGINDRAG, OnBeginDrag)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)

		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MSG_WM_MBUTTONUP(OnMButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(MM_SETUNREAD, OnReceiveCheckUnreadResult)
		CHAIN_MSG_MAP_ALT(CCustomDraw<CFeedTreeCtrl>, 1)
		CHAIN_MSG_MAP(CFeedTreeCtrlImpl<CFeedTreeCtrl>)
		//FORWARD_COMMANDS()
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	LRESULT OnReceiveCheckUnreadResult(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    LRESULT OnBeginLabelEdit(LPNMHDR pnmh);
    LRESULT OnEndLabelEdit(LPNMHDR pnmh);
    LRESULT OnBeginDrag(LPNMHDR pnmh);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);
	void OnDestroy();
	LRESULT HandleContextMenu(CPoint pt);
    LRESULT OnGetInfoTip(LPNMHDR pnmh);
    LRESULT OnClickContextMenu(LPNMHDR pnmh);
    LRESULT OnClick(LPNMHDR pnmh);
    LRESULT OnItemSelectionChanged(LPNMHDR pnmh);
	void OnMButtonUp(UINT nFlags, CPoint point);

	// custom draw
	DWORD OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD);
    DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {        
        return  CDRF_NOTIFYITEMDRAW;
    }
	//void SelectItem(HTREEITEM hItem)
	//{
	//	TreeView_SelectItem(m_hWnd, hItem);
	//}

	DWORD OnDragOver(UINT unFmt, void *pDrop, CPoint pt);
	void OnDragDrop(UINT unFmt, void *pDrop, CPoint pt);
	DWORD IsValidDrop(UINT unFmt, void *pDrop, CPoint pt);

public:
	void SetNewsFilter(CNewsFilter* p) {m_pNewsFilter = p;}
	void MoveFeedNode(HTREEITEM hDest, HTREEITEM hSrc);
	ULONG_PTR GetFeedGroupId(HTREEITEM hItem);

	HTREEITEM HighlightDropTarget(CPoint point);
	HTREEITEM m_hPreviousNode;

	bool m_bAllowLabelEditing;
private:
	CNewsFilter* m_pNewsFilter;
	CLIPFORMAT m_UniformResourceLocatorFormat;

	bool m_bDragging;
	CImageList m_dragImage;
	HTREEITEM m_hDragItem;
	CFeedTreeItem* m_pDragItem;
	void MoveFeed(HTREEITEM hDest, HTREEITEM hSrc);
	void MoveWatch(HTREEITEM hDest, HTREEITEM hSrc);
};
