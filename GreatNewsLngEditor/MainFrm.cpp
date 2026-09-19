#include "stdafx.h"
#include "resource.h"
#include "AboutDlg.h"
#include "MainFrm.h"
#include "ExceptionBase.h"
#include "GNFileDialog.h"
#include "WtlExt.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIEnable(ID_FILE_SAVE, m_view.GetModified());
	UIEnable(ID_EDIT_COPYONELINE, m_view.GetSelectedRow()>=0);

	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();

	// m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
        WS_EX_CLIENTEDGE);
    m_view.SetExtendedGridStyle(GS_EX_CONTEXTMENU);
	m_view.AddColumn(_T("Left Language Pack"),300,CGridCtrl::EDIT_NONE);
	m_view.AddColumn(_T("Right Language Pack"),300,CGridCtrl::EDIT_TEXT);
	m_view.SetListener(this);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_rightFileName.GetLength() == 0)
		return 0;

	try
	{
		FILE* file = _tfopen(m_rightFileName, _T("wb"));
		if(file == NULL)
			MessageBox(_T("Cannot open file to write"));

		for(int i=0;i<m_view.GetRowCount();++i)
		{
			_bstr_t text = m_view.GetItem(i,1);
			_fputts((LPCTSTR)text, file);
			_fputts(_T("\r\n"), file);
		}
		fclose(file);

		m_view.ClearModified();

		MessageBox(_T("Language pack was saved successfully"));
	}
	CATCH_ALL_ERROR()


	return 0;
}

LRESULT CMainFrame::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
LRESULT CMainFrame::OnFileOpenLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	try
	{
		CGNFileDialog dlg(true, _T("gn_*.ini"), _T(""), 0, 
			_T("GreatNews Language Packs (.ini)\0*.ini\0All Files (*.*)\0*.*\0"),
			NULL, false);

		if(dlg.DoModal() == IDOK)
		{
			m_leftFileName = dlg.m_szFileName;
			SyncGrid();
			m_view.ClearModified();
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}

LRESULT CMainFrame::OnFileOpenRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	try
	{
		CGNFileDialog dlg(true, _T("gn_*.ini"), _T(""), 0, 
			_T("GreatNews Language Packs (.ini)\0*.ini\0All Files (*.*)\0*.*\0"),
			NULL, false);

		if(dlg.DoModal() == IDOK)
		{
			m_rightFileName = dlg.m_szFileName;
			SyncGrid();
			m_view.ClearModified();
		}
	}
	CATCH_ALL_ERROR()

	return 0;
}


LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

void CMainFrame::ReadAllLines(std::vector<CString>& allLines, const CString& fileName)
{
	allLines.clear();
	FILE* file = _tfopen(fileName, _T("rb"));
	if (file == NULL)
	{
		throw new CExceptionBase(100, _T("Failed to open file:") + fileName);
		return;
	}

	TCHAR buffer[1024];
	while(!feof(file))
	{
		CString str = _fgetts(buffer, 1024, file);
		str.Trim(_T(" \r\n"));
		allLines.push_back(str);
	}
	fclose(file);
}

CLineItem::Status ignoreCommentLine(const CString& line, CLineItem::Status defaultStatus)
{
	CString str = line;
	str.Trim();
	if(!str.GetLength() || str[0]==_T(';'))
		return CLineItem::Same;
	else
		return defaultStatus;
}

CString GetLineTag(const CString& line)
{
	if(!line.GetLength() || line[0]==_T(';'))
		return _T("");

	int pos = line.Find(_T("="));
	if (pos < 0)
		return _T("");

	return line.Left(pos);
}

bool ScanNext(std::vector<CString>& lines, int index, const CString& text)
{
	for(UINT i=index; i<lines.size(); ++i)
	{
		CString str = lines[i];
		str.Trim();
		if(str == text)
			return true;
		if(str.GetLength()) // non empty line that is not equal to specified text
			return false; 
	}

	return false;
}

void CMainFrame::SyncGrid()
{
	m_content.clear();

	std::vector<CString> leftLines, rightLines;
	if(m_leftFileName.GetLength())
		ReadAllLines(leftLines, m_leftFileName);
	if(m_rightFileName.GetLength())
		ReadAllLines(rightLines, m_rightFileName);

	UINT leftIndex = 0;
	UINT rightIndex = 0;
	for(UINT i = 0; i<leftLines.size()+rightLines.size(); ++i)
	{
		if(leftIndex>=leftLines.size() && rightIndex>=rightLines.size())
			break;

		if(leftIndex >= leftLines.size())
		{
			CLineItem::Status status = 
				ignoreCommentLine(rightLines[rightIndex], CLineItem::Deleted);

			m_content.push_back(new CLineItem(NULL, rightLines[rightIndex++], status));
			continue;
		}
		else if(rightIndex >= rightLines.size())
		{
			CLineItem::Status status = 
				ignoreCommentLine(leftLines[leftIndex], CLineItem::Missing);
			
			m_content.push_back(new CLineItem(leftLines[leftIndex++], NULL, status));
		}
		else
		{
			CString leftText = leftLines[leftIndex].Trim();
			CString rightText = rightLines[rightIndex].Trim();

			if(leftText == rightText
				|| ((!leftText.GetLength() || leftText[0]==_T(';')) && (!rightText.GetLength() || rightText[0]==_T(';')))
				|| (GetLineTag(leftText)==GetLineTag(rightText) && GetLineTag(leftText)!=_T("")))
			{
				m_content.push_back(new CLineItem(leftText, rightText));
				leftIndex++;
				rightIndex++;
				continue;
			}

			if(!leftText.GetLength() && rightText.GetLength())
			{
				// assuming left side always has more items
				m_content.push_back(new CLineItem(NULL, NULL));
				leftIndex++;
				continue;
			}
		
			if(leftText.GetLength() && !rightText.GetLength())
			{
				if(ScanNext(rightLines, rightIndex, leftText)) 
				{
					// the next non empty item is the same as this left string
					m_content.push_back(new CLineItem(NULL, NULL));
					rightIndex++;
				}
				else
				{
					m_content.push_back(new CLineItem(leftText, NULL, CLineItem::Missing));
					leftIndex++;
				}
				continue;
			}

			if(leftText[0]==_T(';') && rightText[0]!=_T(';'))
			{
				m_content.push_back(new CLineItem(leftText, NULL));
				leftIndex++;
				continue;
			}

			if(leftText[0]!=_T(';') && rightText[0]==_T(';'))
			{
				m_content.push_back(new CLineItem(NULL, rightText));
				rightIndex++;
				continue;
			}

			m_content.push_back(new CLineItem(leftText, NULL, CLineItem::Missing));
			leftIndex++;
		}
	}

	m_view.SetRedraw(FALSE);
	m_view.DeleteAllItems();
	for(UINT i = 0; i < m_content.size(); ++i)
	{
		LineItemPtr& item = m_content[i];
		int row = m_view.AddRow();
		m_view.SetItem(row,0,(LPCTSTR)item->m_left);
		m_view.SetItem(row,1,(LPCTSTR)item->m_right);
	}
	m_view.SetRedraw(TRUE);
}


COLORREF CMainFrame::GetCellColor(UINT uID,long nRow,LPCTSTR pszColumn)
{
	if(nRow >= (long)m_content.size())
		return (COLORREF)-1;

	LineItemPtr& item = m_content[nRow];
	if(item->m_status == CLineItem::Deleted)
        // Blue-ish for Deleted
        return RGB(192,192,255);
	else if(item->m_status == CLineItem::Missing)
        // And red-ish for Missing
        return RGB(255,192,192);

	// Return (COLORREF)-1 to use default colors
  return (COLORREF)-1;
}

LRESULT CMainFrame::OnEditCopyToRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int n=0;
	for(UINT i=0; i<m_content.size(); ++i)
	{
		LineItemPtr& item = m_content[i];

		if(item->m_status == CLineItem::Missing)
		{
			++n;
			item->m_right = item->m_left;
			m_view.SetItem(i,1,(LPCTSTR)item->m_right);
		}
	}
	CString msg;
	msg.Format(_T("%d entries were copied to right side"), n);
	MessageBox(msg);

	return 0;
}

LRESULT CMainFrame::OnEditCopyOneLine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int n = m_view.GetSelectedRow();
	if(n >= 0)
	{
		_bstr_t str = m_view.GetItem(n, 0);
		m_view.SetItem(n, 1, str);
	}

	return 0;
}

