#pragma once

#include "GNPropertySheet.h"
#include "CheckedFeedTree.h"
#include "CLogListBox.h"

class CExportWizard: public CGNPropertySheet<CExportWizard>
{

	struct CData
	{
		CData(): m_nExportType(0),m_nExportSelected(0),m_exportWatchLabel(0) {}

		int m_nExportType;
		int m_nExportSelected;
		int m_exportWatchLabel;
		CString m_fileName;
	
		CCheckedFeedTreeCtrl m_treeSelect;


	} m_Data;

	class CPageSelect: public CPropertyPageImpl<CPageSelect>
	                           , public CWinDataExchange<CPageSelect> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:

			enum {IDD = IDD_EXPORT_SELECT};

			CData *m_pData;

			BEGIN_MSG_MAP(CPageSelect)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				COMMAND_ID_HANDLER(IDC_BTNSELECTALL, OnSelectAll)
				COMMAND_ID_HANDLER(IDC_BTNUNSELECTALL, OnUnselectAll)
				CHAIN_MSG_MAP(CPropertyPageImpl<CPageSelect>)
				REFLECT_NOTIFICATIONS()
			END_MSG_MAP()

			BEGIN_DDX_MAP(CPageSelect)
				DDX_CHECK(IDC_CHKEXPORTWATCHLABEL, m_pData->m_exportWatchLabel)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
			LRESULT OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
			CPageSelect(void);

		int OnSetActive(void);
		int OnKillActive(void);
		
	}; // class CPageSelect
	CPageSelect m_pageSelect;

	class CPageFileName: public CPropertyPageImpl<CPageFileName>
	                           , public CWinDataExchange<CPageFileName> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:

			enum {IDD = IDD_EXPORT_FILE};

			CData *m_pData;

			BEGIN_MSG_MAP(CPageFileName)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				COMMAND_ID_HANDLER(IDC_BTNBROWSE, OnBtnbrowse)
				CHAIN_MSG_MAP(CPropertyPageImpl<CPageFileName>)
			END_MSG_MAP()

			BEGIN_DDX_MAP(CPageFileName)
				DDX_TEXT(IDC_TXTFILENAME, m_pData->m_fileName)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		
			LRESULT OnBtnbrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
			CPageFileName(void);

		int OnSetActive(void);
		int OnKillActive(void);
		int OnWizardNext();
		INT_PTR OnWizardFinish(void);

	}; // class CPageFileName
	CPageFileName m_pageFileName;

	BEGIN_MSG_MAP(CExportWizard)
		CHAIN_MSG_MAP(CGNPropertySheet<CExportWizard>)
	END_MSG_MAP()


	CExportWizard(_U_STRINGorID title = (LPCTSTR) NULL, UINT uStartPage = 0, HWND hWndParent = NULL )
	             : CGNPropertySheet<CExportWizard>(title, uStartPage, hWndParent)
	{
		m_psh.dwFlags |= (PSH_NOAPPLYNOW | PSH_WIZARD);

		// Add property pages, give them access to common data.
		AddPage ( m_pageSelect ); m_pageSelect.m_pData = &m_Data;
		AddPage ( m_pageFileName ); m_pageFileName.m_pData = &m_Data;
	} // constructor
}; // class CExportWizard

