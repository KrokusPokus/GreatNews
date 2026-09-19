#include "StdAfx.h"
#include "resource.h"
#include "GNStatusBar.h"

CGNStatusBar::CGNStatusBar(void) : 
	m_bRssIndicatorEnabled(false), m_nRssFlashCount(0),
	m_bPopupIndicatorEnabled(false), m_nPopupFlashCount(0)
{
}

CGNStatusBar::~CGNStatusBar(void)
{
}

HWND CGNStatusBar::Create(HWND hWndParent, UINT nTextID, DWORD dwStyle, UINT nID)
{
	if(NULL == CMultiPaneStatusBarCtrlImpl<CGNStatusBar>::Create(hWndParent, nTextID, dwStyle, nID))
		return NULL;

	int panels[] = {ID_DEFAULT_PANE, ID_PROGRESS, ID_RSS_INDICATOR, ID_POPUP_INDICATOR};
	SetPanes(panels, sizeof(panels)/sizeof(int), false);

	return m_hWnd;
}

LRESULT CGNStatusBar::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT l = baseClass::OnSize(uMsg, wParam, lParam, bHandled);

	if(m_progressBar)
	{
		CRect rect;
		if(GetPaneRect(ID_PROGRESS,&rect))
		{
			rect.right-=4;

			m_progressBar.SetWindowPos(NULL, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE |SWP_NOSIZE);
		}
	}

	if(m_rssIndicator)
	{
		CRect rect;
		if(GetPaneRect(ID_RSS_INDICATOR,&rect))
		{
			m_rssIndicator.SetWindowPos(NULL, rect.left+3, rect.top+2,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE |SWP_NOSIZE);
		}
	}

	if(m_popupIndicator)
	{
		CRect rect;
		if(GetPaneRect(ID_POPUP_INDICATOR,&rect))
		{
			m_popupIndicator.SetWindowPos(NULL, rect.left+3, rect.top+2,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE |SWP_NOSIZE);
		}
	}

	return l;
}

void CGNStatusBar::ShowProgressBar()
{
	if(!::IsWindow(m_progressBar))
	{
		CRect rc;
		GetPaneRect(ID_PROGRESS,&rc);
		rc.right-=4;
		m_progressBar.Create(m_hWnd, rc, NULL, WS_CHILD|WS_VISIBLE );
	}
	m_progressBar.SetRange( 0, 100 );
}

void CGNStatusBar::SetProgressPos(INT_PTR percent)
{
	if(::IsWindow(m_progressBar))
		m_progressBar.SetPos(percent);
}

void CGNStatusBar::EndProgressBar()
{
	if(::IsWindow(m_progressBar))
		m_progressBar.DestroyWindow();
}

void CGNStatusBar::EnableRssIndicator(bool bEnable)
{
	if(bEnable && !m_bRssIndicatorEnabled)
	{
		// turn it on
		CRect rect;
		GetPaneRect(ID_RSS_INDICATOR,&rect);
		rect.left+=3;
		rect.top+=2;
		rect.bottom+=2;

		m_rssIndicator.Create(m_hWnd, rect,  NULL,
			WS_CHILDWINDOW | WS_VISIBLE | SS_BITMAP | SS_LEFT |SS_NOTIFY , 0, IDC_RSS_ICON);
		m_rssIndicator.SetBitmap(AtlLoadBitmapImage(IDB_RSS,LR_DEFAULTCOLOR));
		
		m_nRssFlashCount=0;
		// SetTimer(IDC_TIMER_RSS, 125, NULL); // don't flash, it's annoying
		m_bRssIndicatorEnabled = true;
	}
	else if(!bEnable && m_bRssIndicatorEnabled)
	{
		KillTimer(IDC_TIMER_RSS);
		m_rssIndicator.DestroyWindow();

		// turn it off
		m_bRssIndicatorEnabled = false;
	}
}

void CGNStatusBar::EnablePopupIndicator(bool bEnable)
{
	if(bEnable)
	{
		KillTimer(IDC_TIMER_POPUP);

		// turn it on
		CRect rect;
		GetPaneRect(ID_POPUP_INDICATOR,&rect);
		rect.left+=3;
		rect.top+=2;
		rect.bottom+=2;

		if(!::IsWindow(m_popupIndicator))
		{
			m_popupIndicator.Create(m_hWnd, rect,  NULL,
				WS_CHILDWINDOW | WS_VISIBLE | SS_BITMAP | SS_LEFT |SS_NOTIFY , 0, IDC_POPUP_ICON);
			m_popupIndicator.SetBitmap(AtlLoadBitmapImage(IDB_BLOCKPOPUP,LR_DEFAULTCOLOR));
		}
		else
			m_popupIndicator.ShowWindow(SW_SHOW);

		m_nPopupFlashCount = 0;
		SetTimer(IDC_TIMER_POPUP, 125, NULL);
		m_bPopupIndicatorEnabled = true;
	}
	else if(!bEnable && m_bPopupIndicatorEnabled)
	{
		KillTimer(IDC_TIMER_POPUP);
		m_popupIndicator.DestroyWindow();

		// turn it off
		m_bPopupIndicatorEnabled = false;
	}
}

void CGNStatusBar::OnTimer(UINT_PTR idEvent)
{
	if(idEvent == IDC_TIMER_RSS && ::IsWindow(m_rssIndicator.m_hWnd))
	{
		m_rssIndicator.ShowWindow(m_rssIndicator.IsWindowVisible() ? SW_HIDE : SW_SHOW);

		// we don't want the RSS icon keep flashing
		m_nRssFlashCount++;
		if(m_nRssFlashCount>=10)
			KillTimer(IDC_TIMER_RSS);
	}
	else if(idEvent == IDC_TIMER_POPUP && ::IsWindow(m_popupIndicator.m_hWnd))
	{
		m_popupIndicator.ShowWindow(m_popupIndicator.IsWindowVisible() ? SW_HIDE : SW_SHOW);

		// we don't want the RSS icon keep flashing
		m_nPopupFlashCount++;
		if(m_nPopupFlashCount>=10)
			KillTimer(IDC_TIMER_POPUP);
	}
}

void CGNStatusBar::OnDestroy()
{
	if(::IsWindow(m_rssIndicator.m_hWnd))
		m_rssIndicator.DestroyWindow();

	if(::IsWindow(m_progressBar.m_hWnd))
		m_progressBar.DestroyWindow();

	SetMsgHandled(false);
}

LRESULT CGNStatusBar::OnRssIconClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	if(wID == IDC_RSS_ICON)
		GetParent().PostMessage(WM_COMMAND, ID_IE_RSSICON_CLICKED);
	else if(wID == IDC_POPUP_ICON)
		GetParent().PostMessage(WM_COMMAND, ID_IE_POPUPICON_CLICKED);
	return 0;
}

CPoint CGNStatusBar::GetMenuPos(UINT panelID)
{
	CRect rect;
	GetPaneRect(panelID,&rect);
	CPoint pt(rect.right, rect.bottom);
	ClientToScreen(&pt);
	return pt;
}
