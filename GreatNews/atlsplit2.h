// Extention to Windows Template Library
// by Miguel Hasse de Oliveira (miguel.hasse@netcabo.pt)

#ifndef __ATLSPLIT2_H__
#define __ATLSPLIT2_H__

#pragma once

#ifndef __cplusplus
	#error WTL requires C++ compilation (use a .cpp suffix)
#endif

#ifdef _WIN32_WCE
	#error atlsplit2.h is not supported on Windows CE
#endif

#ifndef __ATLAPP_H__
	#error atlsplit2.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error atlsplit2.h requires atlwin.h to be included first
#endif

#ifndef __ATLSPLIT_H__
	#error atlsplit2.h requires atlsplit.h to be included first
#endif


namespace WTL
{

///////////////////////////////////////////////////////////////////////////////
// CDynSplitterImpl - Provides splitter support to any window

// Splitter extended styles
#define SPLIT_VERTICAL			0x00000010

// Note: SPLIT_PROPORTIONAL and SPLIT_RIGHTALIGNED/SPLIT_BOTTOMALIGNED are 
// mutually exclusive. If both are set, splitter defaults to SPLIT_PROPORTIONAL

template <class T>
class CDynSplitterImpl
{
public:
	enum { m_nPanesCount = 2, m_nPropMax = 10000 };

	HWND m_hWndPane[m_nPanesCount];
	RECT m_rcSplitter;
	int m_xySplitterPos;
	int m_nDefActivePane;
	int m_cxySplitBar;              // splitter bar width/height
	int m_cxyMin[m_nPanesCount];    // minimum pane sizes
	int m_cxyBarEdge;              	// splitter bar edge
	bool m_bFullDrag;
	int m_cxyDragOffset;
	int m_cxySnapSize;
	int m_nProportionalPos;
	bool m_bUpdateProportionalPos;
	DWORD m_dwExtendedStyle;       // splitter specific extended styles
	int m_nSinglePane;             // single pane mode

	static HCURSOR m_hCursorV;
	static HCURSOR m_hCursorH;

// Constructor
	CDynSplitterImpl() :
			m_xySplitterPos(-1), m_nDefActivePane(SPLIT_PANE_NONE), 
			m_cxySplitBar(0), m_cxyBarEdge(0), m_bFullDrag(true), 
			m_cxyDragOffset(0), m_nProportionalPos(0), m_bUpdateProportionalPos(true),
			m_dwExtendedStyle(SPLIT_PROPORTIONAL), m_cxySnapSize(1),
			m_nSinglePane(SPLIT_PANE_NONE)
	{
		m_hWndPane[SPLIT_PANE_LEFT] = NULL;
		m_hWndPane[SPLIT_PANE_RIGHT] = NULL;

		m_cxyMin[SPLIT_PANE_LEFT] = 0;
		m_cxyMin[SPLIT_PANE_RIGHT] = 0;

		::SetRectEmpty(&m_rcSplitter);

		if (m_hCursorV == NULL || m_hCursorH == NULL)
		{
			CStaticDataInitCriticalSectionLock lock;
			if (FAILED(lock.Lock()))
			{
				ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CSplitterImpl::CSplitterImpl.\n"));
				ATLASSERT(FALSE);
				return;
			}

			if (m_hCursorV == NULL)
				m_hCursorV = ::LoadCursor(NULL, IDC_SIZEWE);

			if (m_hCursorH == NULL)
				m_hCursorH = ::LoadCursor(NULL, IDC_SIZENS);

			lock.Unlock();
		}
	}

// Attributes
public:
	void SetSplitterRect(LPRECT lpRect = NULL, bool bUpdate = true)
	{
		if (lpRect == NULL)
		{
			T* pT = static_cast<T*>(this);
			pT->GetClientRect(&m_rcSplitter);
		}
		else
			m_rcSplitter = *lpRect;

		if (IsProportional())
			UpdateProportionalPos();
		else if (IsRightAligned())
			UpdateRightAlignPos();

		if (bUpdate)
			UpdateSplitterLayout();
	}

	void GetSplitterRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);
		*lpRect = m_rcSplitter;
	}

	bool SetSplitterPos(int xyPos = -1, bool bUpdate = true)
	{
		if (xyPos == -1)   // -1 == middle
		{
			xyPos = ((IsSplitVertical() ? m_rcSplitter.right - m_rcSplitter.left :
				m_rcSplitter.bottom - m_rcSplitter.top) - m_cxySplitBar - m_cxyBarEdge) / 2;
		}

		// Adjust if out of valid range
		int cxyMin, cxyMax = (IsSplitVertical() ? m_rcSplitter.right - m_rcSplitter.left : 
			m_rcSplitter.bottom - m_rcSplitter.top) - m_cxySplitBar - m_cxyBarEdge;

		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			cxyMin = m_cxyMin[SPLIT_PANE_LEFT];
			cxyMax -= m_cxyMin[SPLIT_PANE_RIGHT];
		}
		else
		{
			cxyMin = m_cxyMin[m_nSinglePane];
			cxyMax -= m_cxyMin[m_nSinglePane];
		}

		if (m_cxySnapSize > 1 && m_xySplitterPos != -1)
		{
			int modSnapDiv = xyPos % m_cxySnapSize;

			if (modSnapDiv != 0)
			{
				if (modSnapDiv > (m_cxySnapSize > m_cxyBarEdge ?
					m_cxySnapSize - m_cxyBarEdge : m_cxySnapSize / 2))
					xyPos += m_cxySnapSize;

				xyPos -= modSnapDiv;
			}
		}

		if (xyPos < cxyMin + m_cxyBarEdge)
			xyPos = cxyMin;
		else if (xyPos > cxyMax)
			xyPos = cxyMax;

		// Set new position and update if requested
		bool bRet = (m_xySplitterPos != xyPos);
		m_xySplitterPos = xyPos;

		if (m_bUpdateProportionalPos)
		{
			if (IsProportional())
				StoreProportionalPos();
			else if (IsRightAligned())
				StoreRightAlignPos();
		}
		else m_bUpdateProportionalPos = true;

		if (bUpdate && bRet)
			UpdateSplitterLayout();

		return bRet;
	}

	int GetSplitterPos() const
	{
		return m_xySplitterPos;
	}

	bool SetSinglePaneMode(int nPane = SPLIT_PANE_NONE)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT || nPane == SPLIT_PANE_NONE);
		if (!(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT || nPane == SPLIT_PANE_NONE))
			return false;

		if (nPane != SPLIT_PANE_NONE)
		{
			if (!::IsWindowVisible(m_hWndPane[nPane]))
				::ShowWindow(m_hWndPane[nPane], SW_SHOW);

			int nOtherPane = (nPane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
			::ShowWindow(m_hWndPane[nOtherPane], SW_HIDE);

			if (m_nDefActivePane != nPane)
				m_nDefActivePane = nPane;
		}
		else if (m_nSinglePane != SPLIT_PANE_NONE)
		{
			int nOtherPane = (m_nSinglePane == SPLIT_PANE_LEFT) ? SPLIT_PANE_RIGHT : SPLIT_PANE_LEFT;
			::ShowWindow(m_hWndPane[nOtherPane], SW_SHOW);
		}

		m_nSinglePane = nPane;
		UpdateSplitterLayout();
		return true;
	}

	int GetSinglePaneMode() const
	{
		return m_nSinglePane;
	}

	DWORD GetSplitterExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetSplitterExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		m_dwExtendedStyle = (dwMask == 0) ? dwExtendedStyle :
			(m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);

		if ((dwExtendedStyle & SPLIT_VERTICAL) != (dwPrevStyle & SPLIT_VERTICAL))
		{
			GetSystemSettings(false);

			T* pT = static_cast<T*>(this);
			pT->SetSplitterPos();
		}
#ifdef _DEBUG
		if (IsProportional() && IsRightAligned())
			ATLTRACE2(atlTraceUI, 0, "CSplitterImpl::SetSplitterExtendedStyle - SPLIT_PROPORTIONAL and SPLIT_RIGHTALIGNED are mutually exclusive, defaulting to SPLIT_PROPORTIONAL.\n");
#endif //_DEBUG
		return dwPrevStyle;
	}

// Splitter operations
	void SetSplitterPanes(HWND hWndLeftTop, HWND hWndRightBottom, bool bUpdate = true)
	{
		m_hWndPane[SPLIT_PANE_LEFT] = hWndLeftTop;
		m_hWndPane[SPLIT_PANE_RIGHT] = hWndRightBottom;
		ATLASSERT(m_hWndPane[SPLIT_PANE_LEFT] == NULL || m_hWndPane[SPLIT_PANE_RIGHT] == NULL || m_hWndPane[SPLIT_PANE_LEFT] != m_hWndPane[SPLIT_PANE_RIGHT]);
		if (bUpdate)
			UpdateSplitterLayout();
	}

	bool SetSplitterPane(int nPane, HWND hWnd, bool bUpdate = true)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if (nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;
		m_hWndPane[nPane] = hWnd;
		ATLASSERT(m_hWndPane[SPLIT_PANE_LEFT] == NULL || m_hWndPane[SPLIT_PANE_RIGHT] == NULL || m_hWndPane[SPLIT_PANE_LEFT] != m_hWndPane[SPLIT_PANE_RIGHT]);
		if (bUpdate)
			UpdateSplitterLayout();
		return true;
	}

	HWND GetSplitterPane(int nPane) const
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);
		if (nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;

		return m_hWndPane[nPane];
	}

	bool SetActivePane(int nPane)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);

		if (nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;

		if (m_nSinglePane != SPLIT_PANE_NONE && nPane != m_nSinglePane)
			return false;

		::SetFocus(m_hWndPane[nPane]);
		m_nDefActivePane = nPane;
		return true;
	}

	int GetActivePane() const
	{
		int nRet = SPLIT_PANE_NONE;
		HWND hWndFocus = ::GetFocus();
		if (hWndFocus != NULL)
		{
			for (int nPane = 0; nPane < m_nPanesCount; ++nPane)
			{
				if (hWndFocus == m_hWndPane[nPane] || ::IsChild(m_hWndPane[nPane], hWndFocus))
				{
					nRet = nPane;
					break;
				}
			}
		}
		return nRet;
	}

	bool ActivateNextPane(bool bNext = true)
	{
		int nPane = m_nSinglePane;
		if (nPane == SPLIT_PANE_NONE)
		{
			switch (GetActivePane())
			{
			case SPLIT_PANE_LEFT:
				nPane = SPLIT_PANE_RIGHT;
				break;
			case SPLIT_PANE_RIGHT:
				nPane = SPLIT_PANE_LEFT;
				break;
			default:
				nPane = bNext ? SPLIT_PANE_LEFT : SPLIT_PANE_RIGHT;
				break;
			}
		}
		return SetActivePane(nPane);
	}

	bool SetDefaultActivePane(int nPane)
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);
		if (nPane != SPLIT_PANE_LEFT && nPane != SPLIT_PANE_RIGHT)
			return false;

		m_nDefActivePane = nPane;
		return true;
	}

	bool SetDefaultActivePane(HWND hWnd)
	{
		for (int nPane = 0; nPane < m_nPanesCount; ++nPane)
		{
			if (hWnd == m_hWndPane[nPane])
			{
				m_nDefActivePane = nPane;
				return true;
			}
		}
		return false;   // not found
	}

	int GetDefaultActivePane() const
	{
		return m_nDefActivePane;
	}

	void DrawSplitter(CDCHandle dc)
	{
		ATLASSERT(dc.m_hDC != NULL);
		if (m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			return;

		T* pT = static_cast<T*>(this);
		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			pT->DrawSplitterBar(dc);
			for (int nPane = 0; nPane < m_nPanesCount; ++nPane)
			{
				if (m_hWndPane[nPane] == NULL)
					pT->DrawSplitterPane(dc, nPane);
			}
		}
		else if (m_hWndPane[m_nSinglePane] == NULL)
			pT->DrawSplitterPane(dc, m_nSinglePane);
	}

	void ToggleSplitOrientation()
	{
		SetSplitterExtendedStyle(~(m_dwExtendedStyle & SPLIT_VERTICAL), SPLIT_VERTICAL);
	}

// Overrideables
public:
	void DrawSplitterBar(CDCHandle dc)
	{
		RECT rect;
		if (GetSplitterBarRect(&rect))
		{
			dc.FillRect(&rect, COLOR_3DFACE);
			// draw 3D edge if needed
			T* pT = static_cast<T*>(this);
			if ((pT->GetExStyle() & WS_EX_CLIENTEDGE) != 0)
				dc.DrawEdge(&rect, EDGE_RAISED, 
					IsSplitVertical() ? (BF_LEFT | BF_RIGHT) : (BF_TOP | BF_BOTTOM));
		}
	}

	// called only if pane is empty
	void DrawSplitterPane(CDCHandle dc, int nPane)
	{
		RECT rect;
		if (GetSplitterPaneRect(nPane, &rect))
		{
			T* pT = static_cast<T*>(this);
			if ((pT->GetExStyle() & WS_EX_CLIENTEDGE) == 0)
				dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
			dc.FillRect(&rect, COLOR_APPWORKSPACE);
		}
	}

// Message map and handlers
public:
	BEGIN_MSG_MAP(CDynSplitterImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
#endif //!_WIN32_WCE
		if (IsInteractive())
		{
			MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
			MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
		}
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
#ifndef _WIN32_WCE
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
#endif //!_WIN32_WCE
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		GetSystemSettings(false);
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		// try setting position if not set
		if (m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			pT->SetSplitterPos();

		// do painting...
		if (wParam == NULL)
		{
			if (pT->GetUpdateRect(NULL) == 0)
				return 0;

			CPaintDC dc(pT->m_hWnd);
			pT->DrawSplitter(dc.m_hDC);
		}
		else 
			pT->DrawSplitter((HDC)wParam);

		return 0;
	}

	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		if ((HWND)wParam == pT->m_hWnd && LOWORD(lParam) == HTCLIENT)
		{
			DWORD dwPos = ::GetMessagePos();
			POINT ptPos = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			pT->ScreenToClient(&ptPos);
			if (IsOverSplitterBar(ptPos.x, ptPos.y))
				return 1;
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if ((wParam & MK_LBUTTON) && ::GetCapture() == pT->m_hWnd)
		{
			int xyNewSplitPos = IsSplitVertical() ?
				xPos - m_rcSplitter.left - m_cxyDragOffset :
				yPos - m_rcSplitter.top - m_cxyDragOffset;

			if (xyNewSplitPos == -1)   // avoid -1, that means middle
				xyNewSplitPos = -2;

			if (m_xySplitterPos != xyNewSplitPos)
			{
				if (m_bFullDrag)
				{
					if (pT->SetSplitterPos(xyNewSplitPos, true))
						pT->UpdateWindow();
				}
				else
				{
					DrawGhostBar();
					pT->SetSplitterPos(xyNewSplitPos, false);
					DrawGhostBar();
				}
			}
		}
		else		// not dragging, just set cursor
		{
			if (IsOverSplitterBar(xPos, yPos))
				::SetCursor(IsSplitVertical() ? m_hCursorV : m_hCursorH);

			bHandled = FALSE;
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (IsOverSplitterBar(xPos, yPos))
		{
			T* pT = static_cast<T*>(this);
			pT->SetCapture();
			::SetCursor(IsSplitVertical() ? m_hCursorV : m_hCursorH);

			if (!m_bFullDrag)
				DrawGhostBar();

			m_cxyDragOffset = IsSplitVertical() ? 
				xPos - m_rcSplitter.left - m_xySplitterPos :
				yPos - m_rcSplitter.top - m_xySplitterPos;
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		::ReleaseCapture();
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnLButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->SetSplitterPos();   // middle
		return 0;
	}

	LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (!m_bFullDrag)
		{
			DrawGhostBar();
			UpdateSplitterLayout();
			T* pT = static_cast<T*>(this);
			pT->UpdateWindow();
		}
		return 0;
	}

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM, BOOL& bHandled)
	{
		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			if (m_nDefActivePane == SPLIT_PANE_LEFT || m_nDefActivePane == SPLIT_PANE_RIGHT)
				::SetFocus(m_hWndPane[m_nDefActivePane]);
		}
		else
		{
			::SetFocus(m_hWndPane[m_nSinglePane]);
		}
		bHandled = FALSE;
		return 1;
	}

#ifndef _WIN32_WCE
	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		LRESULT lRet = pT->DefWindowProc(uMsg, wParam, lParam);
		if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
		{
			DWORD dwPos = ::GetMessagePos();
			POINT pt = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
			pT->ScreenToClient(&pt);
			RECT rcPane;
			for (int nPane = 0; nPane < m_nPanesCount; ++nPane)
			{
				if (GetSplitterPaneRect(nPane, &rcPane) && ::PtInRect(&rcPane, pt))
				{
					m_nDefActivePane = nPane;
					break;
				}
			}
		}
		return lRet;
	}
#endif //!_WIN32_WCE

	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		GetSystemSettings(true);
		return 0;
	}

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = 0;
		int nPane = GetActivePane();

		if (nPane != SPLIT_PANE_NONE) 
			lRet = ::SendMessage(m_hWndPane[nPane], WM_FORWARDMSG, 0, lParam);

		return lRet;
	}

// Implementation - internal helpers
public:
	void UpdateSplitterLayout()
	{
		if (m_nSinglePane == SPLIT_PANE_NONE && m_xySplitterPos == -1)
			return;

		T* pT = static_cast<T*>(this);
		RECT rect = { 0, 0, 0, 0 };

		if (m_nSinglePane == SPLIT_PANE_NONE)
		{
			if (GetSplitterBarRect(&rect))
				pT->InvalidateRect(&rect);

			HDWP hdwpPanes;
			if ((hdwpPanes = ::BeginDeferWindowPos(m_nPanesCount)) == NULL)
				return;

			for (int nPane = 0; nPane < m_nPanesCount; ++nPane)
			{
				if (GetSplitterPaneRect(nPane, &rect))
				{
					if (m_hWndPane[nPane] != NULL)
					{
						if ((hdwpPanes = ::DeferWindowPos(hdwpPanes, m_hWndPane[nPane], NULL, 
							rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
							SWP_NOACTIVATE | SWP_NOZORDER)) == NULL)
							return;
					}
					else
						pT->InvalidateRect(&rect);
				}
			}
			::EndDeferWindowPos(hdwpPanes);
		}
		else
		{
			if (GetSplitterPaneRect(m_nSinglePane, &rect))
			{
				if (m_hWndPane[m_nSinglePane] != NULL)
				{
					::SetWindowPos(m_hWndPane[m_nSinglePane], NULL, 
						rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 
						SWP_NOACTIVATE | SWP_NOZORDER);
				}
				else
					pT->InvalidateRect(&rect);
			}
		}
	}

	bool GetSplitterBarRect(LPRECT lpRect) const
	{
		ATLASSERT(lpRect != NULL);
		if (m_nSinglePane != SPLIT_PANE_NONE || m_xySplitterPos == -1)
			return false;

		if (IsSplitVertical())
		{
			lpRect->left = m_rcSplitter.left + m_xySplitterPos;
			lpRect->top = m_rcSplitter.top;
			lpRect->right = m_rcSplitter.left + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
			lpRect->bottom = m_rcSplitter.bottom;
		}
		else
		{
			lpRect->left = m_rcSplitter.left;
			lpRect->top = m_rcSplitter.top + m_xySplitterPos;
			lpRect->right = m_rcSplitter.right;
			lpRect->bottom = m_rcSplitter.top + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
		}
		return true;
	}

	bool GetSplitterPaneRect(int nPane, LPRECT lpRect) const
	{
		ATLASSERT(nPane == SPLIT_PANE_LEFT || nPane == SPLIT_PANE_RIGHT);
		ATLASSERT(lpRect != NULL);
		bool bRet = true;
		if (m_nSinglePane != SPLIT_PANE_NONE)
		{
			if (nPane == m_nSinglePane)
				*lpRect = m_rcSplitter;
			else
				bRet = false;
		}
		else if (nPane == SPLIT_PANE_LEFT)
		{
			if (IsSplitVertical())
			{
				lpRect->left = m_rcSplitter.left;
				lpRect->top = m_rcSplitter.top;
				lpRect->right = m_rcSplitter.left + m_xySplitterPos;
				lpRect->bottom = m_rcSplitter.bottom;
			}
			else
			{
				lpRect->left = m_rcSplitter.left;
				lpRect->top = m_rcSplitter.top;
				lpRect->right = m_rcSplitter.right;
				lpRect->bottom = m_rcSplitter.top + m_xySplitterPos;
			}
		}
		else if (nPane == SPLIT_PANE_RIGHT)
		{
			if (IsSplitVertical())
			{
				lpRect->left = m_rcSplitter.left + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
				lpRect->top = m_rcSplitter.top;
				lpRect->right = m_rcSplitter.right;
				lpRect->bottom = m_rcSplitter.bottom;
			}
			else
			{
				lpRect->left = m_rcSplitter.left;
				lpRect->top = m_rcSplitter.top + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge;
				lpRect->right = m_rcSplitter.right;
				lpRect->bottom = m_rcSplitter.bottom;
			}
		}
		else
		{
			bRet = false;
		}
		return bRet;
	}

	bool IsOverSplitterRect(int x, int y) const
	{
		// -1 == don't check
		return ((x == -1 || (x >= m_rcSplitter.left && x <= m_rcSplitter.right)) &&
			(y == -1 || (y >= m_rcSplitter.top && y <= m_rcSplitter.bottom)));
	}

	bool IsOverSplitterBar(int x, int y) const
	{
		if (m_nSinglePane != SPLIT_PANE_NONE || 
			m_xySplitterPos == -1 || !IsOverSplitterRect(x, y))
			return false;

		int xy = IsSplitVertical() ? x : y;
		int xyOff = IsSplitVertical() ? m_rcSplitter.left : m_rcSplitter.top;

		return ((xy >= (xyOff + m_xySplitterPos)) && 
			(xy < xyOff + m_xySplitterPos + m_cxySplitBar + m_cxyBarEdge));
	}

	void DrawGhostBar()
	{
		RECT rect = { 0, 0, 0, 0 };
		if (GetSplitterBarRect(&rect))
		{
			// invert the brush pattern (looks just like frame window sizing)
			T* pT = static_cast<T*>(this);
			CWindowDC dc(pT->m_hWnd);
			CBrush brush = CDCHandle::GetHalftoneBrush();

			if (brush.m_hBrush != NULL)
			{
				CBrushHandle brushOld = dc.SelectBrush(brush);
				dc.PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
				dc.SelectBrush(brushOld);
			}
		}
	}

	void GetSystemSettings(bool bUpdate)
	{
		bool bVertical = IsSplitVertical();
#ifndef _WIN32_WCE
		m_cxySplitBar = ::GetSystemMetrics(bVertical ? SM_CXSIZEFRAME : SM_CYSIZEFRAME);
#else // CE specific
		m_cxySplitBar = 2 * ::GetSystemMetrics(bVertical ? SM_CXEDGE : SM_CYEDGE);
#endif //_WIN32_WCE

		T* pT = static_cast<T*>(this);
		if ((pT->GetExStyle() & WS_EX_CLIENTEDGE))
		{
			m_cxyBarEdge = 2 * ::GetSystemMetrics(bVertical ? SM_CXEDGE : SM_CYEDGE);
		}
		else if (m_cxyBarEdge > 0)
		{
			m_cxyBarEdge = 0;
			m_cxyMin[0] = m_cxyMin[1] += 2 * ::GetSystemMetrics(bVertical ? SM_CXEDGE : SM_CYEDGE);
		}

#ifndef _WIN32_WCE
		::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &m_bFullDrag, 0);
#endif //!_WIN32_WCE

		if (bUpdate)
			UpdateSplitterLayout();
	}

	bool IsProportional() const
	{
		return ((m_dwExtendedStyle & SPLIT_PROPORTIONAL) != 0);
	}

	void StoreProportionalPos()
	{
		int cxyTotal = IsSplitVertical() ? 
			(m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge) :
			(m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge);

		m_nProportionalPos = (cxyTotal > 0) ? ::MulDiv(m_xySplitterPos, m_nPropMax, cxyTotal) : 0;
		ATLTRACE2(atlTraceUI, 0, "CSplitterImpl::StoreProportionalPos - %i\n", m_nProportionalPos);
	}

	void UpdateProportionalPos()
	{
		int cxyTotal = IsSplitVertical() ? 
			(m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge) : 
			(m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge);

		if (cxyTotal > 0)
		{
			int xyNewPos = ::MulDiv(m_nProportionalPos, cxyTotal, m_nPropMax);
			m_bUpdateProportionalPos = false;
			T* pT = static_cast<T*>(this);
			pT->SetSplitterPos(xyNewPos, false);
		}
	}

	bool IsRightAligned() const
	{
		return ((m_dwExtendedStyle & SPLIT_RIGHTALIGNED) != 0);
	}

	void StoreRightAlignPos()
	{
		int cxyTotal = IsSplitVertical() ?
			(m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge) :
			(m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge);

		m_nProportionalPos = (cxyTotal > 0) ? cxyTotal - m_xySplitterPos : 0;
		ATLTRACE2(atlTraceUI, 0, "CSplitterImpl::StoreRightAlignPos - %i\n", m_nProportionalPos);
	}

	void UpdateRightAlignPos()
	{
		int cxyTotal = IsSplitVertical() ?
			(m_rcSplitter.right - m_rcSplitter.left - m_cxySplitBar - m_cxyBarEdge) :
			(m_rcSplitter.bottom - m_rcSplitter.top - m_cxySplitBar - m_cxyBarEdge);

		if (cxyTotal > 0)
		{
			m_bUpdateProportionalPos = false;
			T* pT = static_cast<T*>(this);
			pT->SetSplitterPos(cxyTotal - m_nProportionalPos, false);
		}
	}

	bool IsInteractive() const
	{
		return ((m_dwExtendedStyle & SPLIT_NONINTERACTIVE) == 0) && (m_cxySnapSize > 0);
	}

	bool IsSplitVertical() const
	{
		return ((m_dwExtendedStyle & SPLIT_VERTICAL) != 0);
	}

	void SetSnapSize(int cxySnapSize) 
	{
		m_cxySnapSize = abs(cxySnapSize);
	}

	int GetSnapSize() const
	{
		return m_cxySnapSize;
	}
};

template <class T> HCURSOR CDynSplitterImpl<T>::m_hCursorV = NULL;
template <class T> HCURSOR CDynSplitterImpl<T>::m_hCursorH = NULL;


///////////////////////////////////////////////////////////////////////////////
// CSplitterWindowImpl - Implements a splitter window

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CDynSplitterWindowImpl :
	public ATL::CWindowImpl< T, TBase, TWinTraits >,
	public CDynSplitterImpl< T >
{
public:
	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS, COLOR_WINDOW)

	typedef CDynSplitterImpl< T >   _baseClass;

	BEGIN_MSG_MAP(CDynSplitterWindowImpl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(_baseClass)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// handled, no background painting needed
		return 1;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SetSplitterRect();

		bHandled = FALSE;
		return 1;
	}
};


}; //namespace WTL

#endif // __ATLSPLIT2_H__
