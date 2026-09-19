// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "AboutDlg.h"
#include "version.h"
#include "FeedManagerLib.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	CStatic sName;
	sName.Attach(GetDlgItem(IDC_BUILDNUM));
	//fnTitle.CreatePointFont(12, _T("MS Shell Dlg 2"));
	fnTitle.CreateFont(
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
	sName.SetFont(fnTitle);

	CString execVer;
	USES_CONVERSION; 
	execVer.Format(_T("GreatNews version %s"), A2T(PRODUCT_VERSION_STR));
	sName.SetWindowText(execVer);


	CString dbVer;
	dbVer.Format(_T("Data file version %s"), (LPCTSTR)FeedManagerLib::GetDbVersion());
	GetDlgItem(IDC_DBVER).SetWindowText(dbVer);

	m_link.SubclassWindow(GetDlgItem(IDC_URL));
	m_link.SetHyperLink(_T("http://www.curiostudio.com"));
	m_graphiclink.SubclassWindow(GetDlgItem(IDC_GRAPHIC));
	m_graphiclink.SetHyperLink(_T("http://www.thinkjam.org/mercury"));
	m_contactLink.SubclassWindow(GetDlgItem(IDC_CONTACTS));
	m_contactLink.SetHyperLink(_T("mailto:greatnews@curiostudio.com"));

	CString strCredit;
	strCredit.LoadString(IDS_CREDIT);
	GetDlgItem(IDC_CREDIT).SetWindowText(strCredit);

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
