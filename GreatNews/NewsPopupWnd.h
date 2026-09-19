#pragma once

#include "PinIcons.h"

#define IDT_APPEARING		101
#define IDT_WAITING			102
#define IDT_DISAPPEARING	103

#define TASKBAR_ON_TOP		1
#define TASKBAR_ON_LEFT		2
#define TASKBAR_ON_RIGHT	3
#define TASKBAR_ON_BOTTOM	4

#define TITLE_HEIGHT		20
#define BUTTON_SIZE			17

#define MM_SHOWNEWSITEM			(WM_USER+300)

class CNewsPopupWnd : public CWindowImpl<CNewsPopupWnd>
{
public:
	DECLARE_WND_CLASS_EX(_T("GN_NEWSPOPUP"), CS_DBLCLKS, COLOR_WINDOW)

	BEGIN_MSG_MAP(CNewsPopupWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SIZE(OnSize)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
	    NOTIFY_CODE_HANDLER (LVN_ITEMACTIVATE , OnItemActivate)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

public:
	CNewsPopupWnd() : m_wndNotify(NULL)
	{
		Init();
	}

	HWND Create(HWND notifyWnd)
	{
		m_wndNotify = notifyWnd;

		return CWindowImpl<CNewsPopupWnd>::Create(
			FindWindow(_T("Shell_TrayWnd"), _T("")),0,0,WS_POPUP|WS_BORDER, WS_EX_CLIENTEDGE);
	}

	void Show(NewsItemVector* pItems, DWORD dwTimeToLiveSeconds=60, DWORD dwTimeToShow=300,DWORD dwTimeToHide=100,int nIncrement=3)
	{
		KillTimer(IDT_WAITING);
		KillTimer(IDT_APPEARING);
		KillTimer(IDT_DISAPPEARING);

		if(!IsWindowVisible())
		{
			m_newsList.DeleteAllItems();
		}

		// check if we have anything to show
		if(!pItems)
			return;
		if(pItems->empty())
		{
			delete pItems;
			return;
		}

		// save the new items
		if(m_newsList)
		{
			for(NewsItemVector::iterator it = pItems->begin(); it != pItems->end(); ++it)
			{
				NewsItemPtr item = *it;
				m_newsList.InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE, 0, item->GetTitle(), 0, 0, 0, item->m_id);
			}
		}
		delete pItems;

		m_dwTimeToShow=dwTimeToShow;
		m_dwTimeToLiveSeconds=dwTimeToLiveSeconds;
		m_nDispearingSeconds=dwTimeToLiveSeconds;
		m_dwTimeToHide=dwTimeToHide;
		m_nIncrement = nIncrement;

		CRect rcDesktop;
		::SystemParametersInfo(SPI_GETWORKAREA,0,&rcDesktop,0);
		int nDesktopWidth=rcDesktop.right-rcDesktop.left;
		int nDesktopHeight=rcDesktop.bottom-rcDesktop.top;
		int nScreenWidth=::GetSystemMetrics(SM_CXSCREEN);
		int nScreenHeight=::GetSystemMetrics(SM_CYSCREEN);

		// calculate desirable popup size
		m_nSkinWidth = 300;
		m_nSkinHeight = 200;
		CRect rc(0,TITLE_HEIGHT,m_nSkinWidth-5,m_nSkinHeight-5);
		m_newsList.SetWindowPos(NULL, rc, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);

		BOOL bTaskbarOnRight=nDesktopWidth<nScreenWidth && rcDesktop.left==0;
		BOOL bTaskbarOnLeft=nDesktopWidth<nScreenWidth && rcDesktop.left!=0;
		BOOL bTaskBarOnTop=nDesktopHeight<nScreenHeight && rcDesktop.top!=0;
		BOOL bTaskbarOnBottom=nDesktopHeight<nScreenHeight && rcDesktop.top==0;

		if (bTaskbarOnRight)
		{
			m_dwDelayBetweenShowEvents=m_dwTimeToShow/(m_nSkinWidth/m_nIncrement);
			m_dwDelayBetweenHideEvents=m_dwTimeToHide/(m_nSkinWidth/m_nIncrement);

			m_nStartPosX=rcDesktop.right;
			m_nStartPosY=rcDesktop.bottom-m_nSkinHeight;
			m_nTaskbarPlacement=TASKBAR_ON_RIGHT;
		}
		else if (bTaskbarOnLeft)
		{
			m_dwDelayBetweenShowEvents=m_dwTimeToShow/(m_nSkinWidth/m_nIncrement);
			m_dwDelayBetweenHideEvents=m_dwTimeToHide/(m_nSkinWidth/m_nIncrement);

			m_nStartPosY=rcDesktop.bottom-m_nSkinHeight;
			m_nStartPosX=rcDesktop.left;
			m_nTaskbarPlacement=TASKBAR_ON_LEFT;
		}
		else if (bTaskBarOnTop)
		{
			m_dwDelayBetweenShowEvents=m_dwTimeToShow/(m_nSkinHeight/m_nIncrement);
			m_dwDelayBetweenHideEvents=m_dwTimeToHide/(m_nSkinHeight/m_nIncrement);

			m_nStartPosX=rcDesktop.right-m_nSkinWidth;
			m_nStartPosY=rcDesktop.top;
			m_nTaskbarPlacement=TASKBAR_ON_TOP;
		}
		else //if (bTaskbarOnBottom)
		{
			m_dwDelayBetweenShowEvents=m_dwTimeToShow/(m_nSkinHeight/m_nIncrement);
			m_dwDelayBetweenHideEvents=m_dwTimeToHide/(m_nSkinHeight/m_nIncrement);

			m_nStartPosX=rcDesktop.right-m_nSkinWidth;
			m_nStartPosY=rcDesktop.bottom;
			m_nTaskbarPlacement=TASKBAR_ON_BOTTOM;
		}

		m_nWinSizeY = 0;
		m_nWinSizeX = 0;
		m_nCurrentPosY = m_nStartPosY;
		m_nCurrentPosX = m_nStartPosX;

		SetTimer(IDT_APPEARING,m_dwDelayBetweenShowEvents,NULL);
	}

	void Hide()
	{
		Init();

		KillTimer(IDT_DISAPPEARING);
		ShowWindow(SW_HIDE);
	}

public:
	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		m_newsList.Create(m_hWnd, 0, 0, WS_CHILD|WS_VISIBLE|LVS_NOCOLUMNHEADER | LVS_SINGLESEL| WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT  );
		m_newsList.SetExtendedListViewStyle(LVS_EX_LABELTIP |LVS_EX_FULLROWSELECT|LVS_EX_UNDERLINEHOT |LVS_EX_ONECLICKACTIVATE,
											LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT|LVS_EX_UNDERLINEHOT |LVS_EX_ONECLICKACTIVATE );

		m_newsList.InsertColumn(0,_T("Title"), LVCFMT_LEFT, 275, 0 );
		CImageList imageList;
		imageList.CreateFromImage( IDB_LIST, 16, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION );
		m_newsList.SetImageList(imageList,LVSIL_SMALL);
		imageList.Detach();

		m_fnTitle.CreateFont(
			14,                        // nHeight
			0,                         // nWidth
			0,                         // nEscapement
			0,                         // nOrientation
			FW_SEMIBOLD,               // nWeight
			FALSE,                     // bItalic
			FALSE,                     // bUnderline
			0,                         // cStrikeOut
			ANSI_CHARSET,              // nCharSet
			OUT_DEFAULT_PRECIS,        // nOutPrecision
			CLIP_DEFAULT_PRECIS,       // nClipPrecision
			DEFAULT_QUALITY,           // nQuality
			DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
			_T("MS Shell Dlg"));     // lpszFacename

		return 0;
	}


	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if(m_bClosing)
			return;

		if(!m_bMouseTracking)
		{
			TRACKMOUSEEVENT t_MouseEvent;
			t_MouseEvent.cbSize      = sizeof(TRACKMOUSEEVENT);
			t_MouseEvent.dwFlags     = TME_LEAVE | TME_HOVER;
			t_MouseEvent.hwndTrack   = m_hWnd;
			t_MouseEvent.dwHoverTime = 1;

			::_TrackMouseEvent(&t_MouseEvent);

			m_bMouseTracking = true;

			KillTimer(IDT_DISAPPEARING);
			SetTimer(IDT_APPEARING,m_dwDelayBetweenShowEvents,NULL);

		}

		if(m_rcClose.PtInRect(point))
		{
			if(!m_inButton)
			{
				m_inButton = true;
				Invalidate();
				UpdateWindow();
	
				CWindowDC dc(m_hWnd);
				dc.DrawEdge(m_rcClose,BDR_RAISEDINNER/*|BF_ADJUST*/ ,BF_RECT); //look like button raise 
			}
		}
		else if(m_rcPin.PtInRect(point))
		{
			if(!m_inButton)
			{
				m_inButton = true;
				Invalidate();
				UpdateWindow();

				CWindowDC dc(m_hWnd);
				dc.DrawEdge(m_rcPin,BDR_RAISEDINNER/*|BF_ADJUST*/ ,BF_RECT); //look like button raise 
			}
		}
		else if(m_inButton)
		{
			m_inButton = false;
			Invalidate();
			UpdateWindow();
		}

		SetMsgHandled(false);

		return;
	}

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		if(m_rcClose.PtInRect(point))
		{
			Invalidate();
			UpdateWindow();

			CWindowDC dc(m_hWnd);
			dc.DrawEdge(m_rcClose,BDR_SUNKENINNER/*|BF_ADJUST*/ ,BF_RECT); //look like button raise 
		}
		else if(m_rcPin.PtInRect(point))
		{
			Invalidate();
			UpdateWindow();

			CWindowDC dc(m_hWnd);
			dc.DrawEdge(m_rcPin,BDR_SUNKENINNER/*|BF_ADJUST*/ ,BF_RECT); //look like button raise 
		}
		return;
	}

	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		if(m_rcClose.PtInRect(point))
		{
			Invalidate();
			UpdateWindow();

			KillTimer(IDT_WAITING);
			KillTimer(IDT_APPEARING);
			KillTimer(IDT_DISAPPEARING);
			SetTimer(IDT_DISAPPEARING,m_dwDelayBetweenHideEvents,NULL);

			m_bClosing = true;
		}
		else if(m_rcPin.PtInRect(point))
		{
			m_bPinned = !m_bPinned;
			m_nDispearingSeconds = m_dwTimeToLiveSeconds;
			if(!m_bPinned)
			{
				SetTimer(IDT_WAITING,1000,NULL);
			}
			Invalidate();
			UpdateWindow();
		}
		return;
	}

	void OnMouseLeave()
	{
		m_bMouseTracking = false;
		if(m_inButton)
		{
			m_inButton = false;
			Invalidate();
			UpdateWindow();
		}
	}

	void OnTimer(UINT_PTR idEvent)
	{
		bool bShowCompleted = false;
		switch (idEvent)
		{
		case IDT_APPEARING:
			m_nAnimStatus=IDT_APPEARING;
			switch(m_nTaskbarPlacement)
			{
			case TASKBAR_ON_BOTTOM:
				if (m_nWinSizeY<m_nSkinHeight)
				{
					if ((m_nWinSizeY+m_nIncrement)>m_nSkinHeight)
						m_nCurrentPosY-=m_nSkinHeight-m_nWinSizeY;
					else
						m_nCurrentPosY-=m_nIncrement;

					m_nWinSizeX = m_nSkinWidth;
					m_nWinSizeY = (m_nStartPosY-m_nCurrentPosY);
				}
				else
				{
					bShowCompleted = true;
				}
				SetWindowPos(NULL,m_nCurrentPosX,m_nCurrentPosY,m_nWinSizeX,m_nWinSizeY,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
				break;
			case TASKBAR_ON_TOP:
				if (m_nWinSizeY<m_nSkinHeight)
				{
					if ((m_nWinSizeY+m_nIncrement)>m_nSkinHeight)
						m_nWinSizeY += m_nSkinHeight-m_nWinSizeY;
					else
						m_nWinSizeY += m_nIncrement;

					m_nWinSizeX = m_nSkinWidth;
					Invalidate();
				}
				else
				{
					bShowCompleted = true;
				}
				SetWindowPos(NULL,m_nCurrentPosX,m_nCurrentPosY,m_nWinSizeX,m_nWinSizeY,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
				break;
			case TASKBAR_ON_LEFT:
				if (m_nWinSizeX<m_nSkinWidth)
				{
					m_nWinSizeY = m_nSkinHeight;

					if ((m_nWinSizeX+m_nIncrement)>m_nSkinWidth)
						m_nWinSizeX += m_nSkinWidth-m_nWinSizeX;
					else
						m_nWinSizeX += m_nIncrement;

					Invalidate();
				}
				else
				{
					bShowCompleted = true;
				}
				SetWindowPos(NULL,m_nCurrentPosX,m_nCurrentPosY,m_nWinSizeX,m_nWinSizeY,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
				break;
			case TASKBAR_ON_RIGHT:
				if (m_nWinSizeX<m_nSkinWidth)
				{
					if ((m_nWinSizeX+m_nIncrement)>m_nSkinWidth)
						m_nCurrentPosX-=m_nSkinWidth-m_nWinSizeX;
					else
						m_nCurrentPosX-=m_nIncrement;

					m_nWinSizeY = m_nSkinHeight;
					m_nWinSizeX = (m_nStartPosX-m_nCurrentPosX);
				}
				else
				{
					bShowCompleted = true;
				}
				SetWindowPos(NULL,m_nCurrentPosX,m_nCurrentPosY,m_nWinSizeX,m_nWinSizeY,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
				break;
			}

			if(!IsWindowVisible())
			{
				ShowWindow(SW_SHOW);
			}

			if(bShowCompleted)
			{
				KillTimer(IDT_APPEARING);
				m_nDispearingSeconds = m_dwTimeToLiveSeconds;
				SetTimer(IDT_WAITING,1000,NULL);
				m_nAnimStatus=IDT_WAITING;
			}
			
			break;

		case IDT_WAITING:
			if(!m_bMouseTracking)
			{
				m_nDispearingSeconds--;
				Invalidate();
				UpdateWindow();
				if(m_nDispearingSeconds<=0)
				{
					KillTimer(IDT_WAITING);
					SetTimer(IDT_DISAPPEARING,m_dwDelayBetweenHideEvents,NULL);
				}
			}
			break;

		case IDT_DISAPPEARING:
			m_nAnimStatus=IDT_DISAPPEARING;
			switch(m_nTaskbarPlacement)
			{
			case TASKBAR_ON_BOTTOM:
				if (m_nWinSizeY>0)
				{
					m_nCurrentPosY+=m_nIncrement;
					m_nWinSizeX = m_nSkinWidth;
					m_nWinSizeY = (m_nStartPosY-m_nCurrentPosY);
				}
				else
				{
					Hide();
				}
				SetWindowPos(NULL,m_nCurrentPosX,m_nCurrentPosY,m_nWinSizeX,m_nWinSizeY,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
				break;
			case TASKBAR_ON_TOP:
				if (m_nWinSizeY>0)
				{
					m_nWinSizeX = m_nSkinWidth;
					m_nWinSizeY -= m_nIncrement;
					Invalidate();
					SetWindowPos(NULL,m_nCurrentPosX,m_nCurrentPosY,m_nWinSizeX,m_nWinSizeY,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
				}
				else
				{
					Hide();
				}
				break;
			case TASKBAR_ON_LEFT:
				if (m_nWinSizeX>0)
				{
					m_nWinSizeX-=m_nIncrement;
					m_nWinSizeY = m_nSkinHeight;
					Invalidate();
					SetWindowPos(NULL,m_nCurrentPosX,m_nCurrentPosY,m_nWinSizeX,m_nWinSizeY,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
				}
				else
				{
					Hide();
				}
				break;
			case TASKBAR_ON_RIGHT:
				if (m_nWinSizeX>0)
				{
					m_nCurrentPosX+=m_nIncrement;
					m_nWinSizeY = m_nSkinHeight;
					m_nWinSizeX = (m_nStartPosX-m_nCurrentPosX);
					Invalidate();
					SetWindowPos(NULL,m_nCurrentPosX,m_nCurrentPosY,m_nWinSizeX,m_nWinSizeY,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
				}
				else
				{
					Hide();
				}
				break;
			}

		default:
			SetMsgHandled(FALSE);
			break;
		}
	}

	void OnSize(UINT uFlag, CSize size)
	{
		//if(m_newsList)
		//{
		//	CRect rc;
		//	GetClientRect(rc);
		//	rc.top+=20;
		//	if(rc.top>0)
		//		m_newsList.SetWindowPos(NULL, rc, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		//}
	}

	void OnPaint(HDC hDC)
	{
		if(!m_bPinned)
		{
			m_titleMsg.Format(_T("%d new messages. (%d seconds to close)"),
							m_newsList.GetItemCount(),
							m_nDispearingSeconds);
		}
		else
		{
			m_titleMsg.Format(_T("%d new messages."),
							m_newsList.GetItemCount());
		}

		CPaintDC dc(m_hWnd);
		dc.FillRect(m_rcTitle,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
		dc.SetBkMode(TRANSPARENT);
		HFONT oldFont = dc.SelectFont(m_fnTitle);
		dc.TextOut(4,4,m_titleMsg);
		dc.SelectFont(oldFont);

		dc.DrawIconEx(m_rcPin.TopLeft(),
					m_icons.GetIcon(m_bPinned ? CPinIcons::sPinned : CPinIcons::sUnPinned),
					CSize(11,11));
		DrawCloseButton(dc, m_rcClose);

		return;
	}

	LRESULT OnItemActivate(int idCtrl, LPNMHDR pnmh, BOOL &bHandled)
	{
		LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)pnmh;
		DWORD_PTR n = m_newsList.GetItemData(lpnmia->iItem);
		if(m_wndNotify)
			::PostMessage(m_wndNotify, MM_SHOWNEWSITEM, 0, n);

		return 0;
	}

private:
    void DrawCloseButton(CPaintDC& dc, CRect rcClose)
    {
		//dc.FillRect(this,(HBRUSH)LongToPtr(COLOR_3DFACE + 1));
        CPen pen;
        pen.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_BTNTEXT));

        HPEN hPenOld = dc.SelectPen(pen);
		const int sp=2;
		const int size=7;
        dc.MoveTo(rcClose.left+sp, rcClose.top+sp);
        dc.LineTo(rcClose.left+sp+size, rcClose.top+sp+size);
        dc.MoveTo(rcClose.left + sp+1, rcClose.top+sp);
        dc.LineTo(rcClose.left+sp+size + 1, rcClose.top+sp+size);

        dc.MoveTo(rcClose.left+sp, rcClose.top+sp+size-1);
        dc.LineTo(rcClose.left+sp+size, rcClose.top+sp -1 );
        dc.MoveTo(rcClose.left + sp +1, rcClose.top+sp+size -1);
        dc.LineTo(rcClose.left+sp+size +1, rcClose.top + sp -1);

        dc.SelectPen(hPenOld);
	}

	void Init()
	{
		m_nWinSizeX = 0;
		m_nWinSizeY = 0;
		m_bMouseTracking = false;
		m_rcClose = CRect(300-4-BUTTON_SIZE,5,300-4,5+BUTTON_SIZE);
		m_rcPin = m_rcClose;
		m_rcPin.OffsetRect(-1-BUTTON_SIZE,0);
		m_rcTitle = CRect(0,0,300,TITLE_HEIGHT);
		m_inButton = false;
		m_bPinned = false;
		m_nDispearingSeconds = 0;
		m_bClosing = false;
	}

private:
	HWND m_wndNotify;
	CListViewCtrl m_newsList;

	int m_nStartPosX;
	int m_nStartPosY;
	int m_nCurrentPosX;
	int m_nCurrentPosY;
	int m_nTaskbarPlacement;
	int m_nIncrement;

	int m_nSkinWidth;
	int m_nSkinHeight;
	int	m_nWinSizeX;
	int	m_nWinSizeY;

	DWORD m_dwTimeToShow;
	DWORD m_dwTimeToLiveSeconds;
	DWORD m_dwTimeToHide;
	DWORD m_dwDelayBetweenShowEvents;
	DWORD m_dwDelayBetweenHideEvents;

	int m_nAnimStatus;

	bool m_bMouseTracking;

	CString m_titleMsg;

	CPinIcons m_icons;

	CRect m_rcTitle;
	CRect m_rcPin;
	CRect m_rcClose;
	CRect m_rcSeconds;

	bool m_inButton;
	bool m_bPinned;

	int m_nDispearingSeconds;

	CFont m_fnTitle;

	bool m_bClosing;
};
