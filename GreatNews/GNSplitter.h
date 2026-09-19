#pragma once

#include "atlsplit2.h"

#define CLICK_SIZE 75

class CGNSplitter :
    public CDynSplitterWindowImpl<CGNSplitter>,
	public CThemeImpl<CGNSplitter>
{
public:
	typedef CDynSplitterWindowImpl<CGNSplitter> baseClass;
    DECLARE_WND_CLASS_EX(_T("GN_SplitterWindow"), CS_DBLCLKS, COLOR_WINDOW)
 
	CGNSplitter() : m_bMouseCaptured(false), m_nPositionBeforeCollapse(-1),m_bCollapsed(false)
	{
	}

    BEGIN_MSG_MAP(CGNSplitter)
		CHAIN_MSG_MAP (CThemeImpl<CGNSplitter>)	// should be here, not at bottom
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(IsCollapsed())
			DefWindowProc(); // we want to erase background if it's collaspsed;
	
		return 1;
	}

	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		OpenThemeData (L"Button");
		SetMsgHandled(FALSE);
		return 0;
	}

    // Overrideables
    void DrawSplitterBar(CDCHandle dc)
    {
		baseClass::DrawSplitterBar(dc);
		DrawClickRect(dc);
    }
 
	void SetBarSize(int newBarSize, int minPaneSize=1)
	{
		m_cxySplitBar = newBarSize;
		m_cxyMin[SPLIT_PANE_LEFT] = m_cxyMin[SPLIT_PANE_RIGHT] = minPaneSize;
        UpdateSplitterLayout();
	}

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		if(IsOverClickRect(point))
		{
			m_bMouseCaptured = false;
			::ReleaseCapture();
			CClientDC clientDC(m_hWnd);
			DrawClickRect(clientDC.m_hDC, false);

			ToggleCollapse();
		}
		else
		{
			SetMsgHandled(false);
		}
	}

	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if(IsOverClickRect(point))
		{
			if(!m_bMouseCaptured)
			{
				m_bMouseCaptured = true;
				::SetCapture(m_hWnd);
	
				CClientDC clientDC(m_hWnd);
				DrawClickRect(clientDC.m_hDC, true);
			}
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		}
		else
		{
			if(m_bMouseCaptured)
			{
				m_bMouseCaptured = false;
				::ReleaseCapture();

				CClientDC clientDC(m_hWnd);
				DrawClickRect(clientDC.m_hDC, false);
			}
			else
			{
				SetMsgHandled(false);
			}
		}
		
	}

	bool IsCollapsed() {return m_bCollapsed;}

	int GetCollapsePos()
	{
		return IsCollapsed() ? m_nPositionBeforeCollapse : GetSplitterPos();
	}
	
	void SetCollapsePos(int n, bool bUpdate = true)
	{
		m_nPositionBeforeCollapse = n;
		if(IsCollapsed())
			ToggleCollapse();
		else
			SetSplitterPos(n, bUpdate);
	}

	void Collapse(bool bCollapse = true)
	{
		if(m_bCollapsed != bCollapse)
			ToggleCollapse();
	}

	void ToggleCollapse()
	{
		if(m_bCollapsed)
		{
			SetSplitterPos(m_nPositionBeforeCollapse);
		}
		else
		{
			m_nPositionBeforeCollapse = GetSplitterPos();
			SetSplitterPos(1);
		}

		m_bCollapsed = !m_bCollapsed;
	}
	
	bool SetSplitterPos(int xyPos = -1, bool bUpdate = true)
	{
		bool bRet = baseClass::SetSplitterPos(xyPos, bUpdate);

		// this is called during mouse dragging?
		if (bRet && ::GetKeyState(VK_LBUTTON) < 0 && ::GetCapture() == m_hWnd)
		{
			// if so collapsed flag need to be updated
			m_bCollapsed = (GetSplitterPos() <= 1);
		}

		return bRet;
	}

protected:
	int m_nPositionBeforeCollapse;
	bool m_bCollapsed;

	bool m_bMouseCaptured;

protected:
	void DrawClickRect(CDCHandle dc, bool bHighlight = false)
	{
		CRect rect;
		GetClickRect(rect);

		if (m_hTheme != NULL)
		{
			// draw themed!
			UINT uFrameState = PBS_DEFAULTED;
			if(bHighlight)
				uFrameState |= PBS_HOT;
			DrawThemeBackground (dc, BP_PUSHBUTTON, uFrameState, &rect, NULL);

		}
		else
		{
			HBRUSH oldBrush;
			HPEN oldPen = dc.SelectStockPen(WHITE_PEN);

			if(bHighlight)
			{
				oldBrush = dc.SelectStockBrush(DKGRAY_BRUSH);
				//dc.FillSolidRect ( &rect, GetSysColor(COLOR_ACTIVECAPTION) );
			}
			else
			{
				oldBrush = dc.SelectStockBrush(LTGRAY_BRUSH);
				//dc.FillSolidRect ( &rect, GetSysColor(COLOR_INACTIVECAPTION) );
			}
			rect.InflateRect(1,1);
			dc.RoundRect(&rect, CPoint(5,5) );
			dc.SelectBrush(oldBrush);
			dc.SelectPen(oldPen);
		}
	}

	BOOL IsOverClickRect(CPoint& pt)
	{
		CRect rc;
		GetClickRect(rc);

		rc.InflateRect(1,1); // make click a little easier

		return rc.PtInRect(pt);
	}

	void GetClickRect(CRect& rect)
	{
		GetSplitterBarRect ( &rect );

		if(IsSplitVertical())
		{
			rect.top = rect.Height()/2 - CLICK_SIZE/2;
			rect.bottom = rect.top+CLICK_SIZE;
		}
		else
		{
			//rect.OffsetRect(0,1);
			//rect.bottom--;
			rect.left = rect.Width()/2 - CLICK_SIZE/2;
			rect.right = rect.left+CLICK_SIZE;
		}
	}
};
 
