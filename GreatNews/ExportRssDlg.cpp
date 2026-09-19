#include "stdafx.h"
#include "resource.h"
#include "ExportRssDlg.h"
#include "FeedManagerLib.h"
#include "GNFileDialog.h"
#include "GNResourceManager.h"

#import "msxml3.dll"

LRESULT CExportRssDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLTRACE("CExportRssDlg::OnInitDialog()\n");
	DoDataExchange(false);

	CenterWindow(GetParent());

	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->ApplyLanguageToWindow(m_hWnd, _T("ExportRSSDialog"));

	return 0;
} // OnInitDialog

LRESULT CExportRssDlg::OnIdcancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
} // OnIdcancel

LRESULT CExportRssDlg::OnBtnbrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CGNFileDialog dlg(true, _T("xml"), _T(""), OFN_HIDEREADONLY, 
		_T("RSS Files (rss, xml)\0*.rss;*.xml\0All Files (*.*)\0*.*\0"),
		NULL, false);

	if(dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_FILENAME, dlg.m_szFileName);
	}

	return 0;
} // OnBtnbrowse

LRESULT CExportRssDlg::OnIdok(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(!DoDataExchange(true))
	{
		return 0;
	}

	CWaitCursor wc;
	bool bSucceeded = false;

	try
	{
		MSXML2::IXMLDOMDocumentPtr spDoc(_T("MSXML2.DOMDocument.3.0")); 
		MSXML2::IXMLDOMElementPtr rss = spDoc->createElement("rss");
		rss->setAttribute("rss","2.0");
		spDoc->appendChild(rss);

		MSXML2::IXMLDOMElementPtr channel = spDoc->createElement("channel");
		m_batchContent->ExportRss(channel);
		rss->appendChild(channel);

		std::vector<ULONG_PTR>& IDs = m_batchContent->m_vectNewsIDs;
		size_t size = IDs.size();
		std::vector<ULONG_PTR>::iterator itBeginId = IDs.begin();
		if(size != 0)
		{
			NewsItemVector items;
			CNewsItem::GetNewsItemsByID(items, m_batchContent->m_vectNewsIDs);

			for(NewsItemVector::iterator it=items.begin(); it != items.end(); ++it)
			{
				NewsItemPtr& item = *it;
				MSXML2::IXMLDOMElementPtr rssItem = spDoc->createElement(_T("item"));
				item->ExportRss(rssItem);
				channel->appendChild(rssItem);
			}
		}

		spDoc->save((LPCTSTR)m_fileName);

		CString msg;
		msg.Format(ResManagerPtr->GetString(IDS_EXPORTRSSSUMMARY),
			IDs.size(), (LPCTSTR)m_fileName);
		MessageBox(msg, _T("GreatNews"),MB_OK|MB_ICONINFORMATION);
		bSucceeded = true;
	}
	CATCH_ALL_ERROR()

	if(bSucceeded)
		EndDialog(wID);

	return 0;
} // OnIdok

/////////////////////////////////////////////////////////////////////////////////////////
