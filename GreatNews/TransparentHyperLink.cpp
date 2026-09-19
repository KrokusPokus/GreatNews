#include "StdAfx.h"
#include "TransparentHyperLink.h"

CTransparentHyperLink::CTransparentHyperLink(void)
{
}

CTransparentHyperLink::~CTransparentHyperLink(void)
{
}

void CTransparentHyperLink::DoEraseBackground(CDCHandle dc)
{
	HBRUSH hBrush = (HBRUSH)::SendMessage(GetParent(), WM_CTLCOLORLISTBOX , (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if(hBrush != NULL)
	{
		RECT rect = { 0 };
		GetClientRect(&rect);
		dc.FillRect(&rect, hBrush);
	}
}