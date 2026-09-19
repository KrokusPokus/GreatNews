#pragma once

#include "NewsFilter.h"
#include "FeedTreeCtrlBase.h"

#define MM_CHECKSTATECHANGE (WM_USER + 100)

class CCheckedFeedTreeCtrl : public CFeedTreeCtrlImpl<CCheckedFeedTreeCtrl>
{
public:
	BEGIN_MSG_MAP(CCheckedFeedTreeCtrl)
		MESSAGE_HANDLER(MM_CHECKSTATECHANGE, OnCheckStatusChange)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_CLICK, OnClick)
		CHAIN_MSG_MAP(CFeedTreeCtrlImpl<CCheckedFeedTreeCtrl>)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnCheckStatusChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		HTREEITEM hItemChanged = (HTREEITEM)lParam;
		CTreeItem item(hItemChanged, this);
		bool bChecked = GetCheckState(hItemChanged)==TRUE;
		CheckAll(bChecked, item);
		return 0;
	}

    LRESULT OnClick(LPNMHDR lpnmh)
	{
		DWORD dwPos = GetMessagePos();
		CPoint pt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );
		CPoint spt = pt;
		ScreenToClient( &pt );
		UINT uFlags;
		CTreeItem htItem = HitTest(pt, &uFlags);
		if (htItem && (uFlags & TVHT_ONITEMSTATEICON))
		{
			PostMessage(MM_CHECKSTATECHANGE, 0, (LPARAM)htItem.m_hTreeItem);
		}

		SetMsgHandled(FALSE);

		return 0;
	}
};
