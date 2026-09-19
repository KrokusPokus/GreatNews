#pragma once

#include "GNPropertySheet.h"
#include "CheckedFeedTree.h"
#include "CLogListBox.h"
#include "ImportThread.h"

class CImportWizard: public CGNPropertySheet<CImportWizard>
{

	struct CData
	{
		CData(): 
			m_nImportType(0),m_nImportSelected(0),m_bReloadTree(true),
			m_updateChannel(false), m_importWatchLabel(false), m_bShowGroupPage(false), m_nDefaultGroup(0){}

		int m_nImportType;
		int m_nImportSelected;
		CString m_fileName;
		CString m_url;
		
		int m_updateChannel;
		int m_importWatchLabel;

		HWND m_hwndTree;
		bool m_bReloadTree;
		MSXML2::IXMLDOMDocumentPtr m_spDoc;

		bool m_bShowGroupPage;
		ULONG_PTR m_nDefaultGroup;
	} m_Data;

	class CPageFileName: public CPropertyPageImpl<CPageFileName>
	                           , public CWinDataExchange<CPageFileName> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:

			enum {IDD = IDD_IMPORT_FILE};

			CData *m_pData;

			BEGIN_MSG_MAP(CPageFileName)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				COMMAND_ID_HANDLER(IDC_BTNBROWSE, OnBtnbrowse)
				COMMAND_ID_HANDLER(IDC_RDOIMPORTWELLKNOWN, OnRdoimportwellknown)
				COMMAND_ID_HANDLER(IDC_RDOIMPORTSELECTFILE, OnRdoImportSelectFile)
				COMMAND_ID_HANDLER(IDC_RDOIMPORTINTERNET, OnRdoImportInternet)
				CHAIN_MSG_MAP(CPropertyPageImpl<CPageFileName>)
			END_MSG_MAP()

			BEGIN_DDX_MAP(CPageFileName)
				DDX_TEXT(IDC_TXTFILENAME, m_pData->m_fileName)
				DDX_TEXT(IDC_TXTURL, m_pData->m_url)
				DDX_RADIO(IDC_RDOIMPORTWELLKNOWN, m_pData->m_nImportType)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		
			LRESULT OnBtnbrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
			LRESULT OnRdoimportwellknown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
			LRESULT OnRdoImportSelectFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
			LRESULT OnRdoImportInternet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
			CPageFileName(void);

		int OnSetActive(void);
		int OnKillActive(void);

	}; // class CPageFileName
	CPageFileName m_pageFileName;

	class CPageSelect: public CPropertyPageImpl<CPageSelect>
	                           , public CWinDataExchange<CPageSelect> //  DDX implementation, call DoDataExchange() where relevant.
	{
	public:

		enum {IDD = IDD_IMPORT_SELECT};

		CData *m_pData;

		BEGIN_MSG_MAP(CPageSelect)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDC_BTNSELECTALL, OnSelectAll)
			COMMAND_ID_HANDLER(IDC_BTNUNSELECTALL, OnUnselectAll)
			CHAIN_MSG_MAP(CPropertyPageImpl<CPageSelect>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		BEGIN_DDX_MAP(CPageSelect)
			DDX_CHECK(IDC_UPDATE, m_pData->m_updateChannel)
			DDX_CHECK(IDC_CHKIMPORTWATCHLABEL, m_pData->m_importWatchLabel)
		END_DDX_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
		CPageSelect(void);

		int OnSetActive(void);
		int OnKillActive(void);

	private:
		CCheckedFeedTreeCtrl m_treeSelect;
	}; // class CPageSelect
	CPageSelect m_pageSelect;

	class CPageGroup: public CPropertyPageImpl<CPageGroup>
	                           , public CWinDataExchange<CPageGroup> //  DDX implementation, call DoDataExchange() where relevant.
	{
	public:

		enum {IDD = IDD_IMPORT_GROUP};

		CData *m_pData;

		BEGIN_MSG_MAP(CPageGroup)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDC_NEWGROUP, OnNewGroup)
			CHAIN_MSG_MAP(CPropertyPageImpl<CPageGroup>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		BEGIN_DDX_MAP(CPageGroup)
		END_DDX_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnNewGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
		CPageGroup(void);

		int OnSetActive(void);
		int OnKillActive(void);

	private:
		CFeedTreeCtrlBase m_treeGroup;
	}; // class CPageGroup
	CPageGroup m_pageGroup;


	class CPageProgress: public CPropertyPageImpl<CPageProgress>
	                           , public CWinDataExchange<CPageProgress> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:

			enum {IDD = IDD_IMPORT_PROGRESS};

			CData *m_pData;

			BEGIN_MSG_MAP(CPageProgress)
				MESSAGE_HANDLER(MM_IMPORT_STATUS, OnImportStatus)
				MESSAGE_HANDLER(MM_IMPORT_FINISHED, OnImportFinished)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				COMMAND_ID_HANDLER(IDC_STOP, OnStop)
				CHAIN_MSG_MAP(CPropertyPageImpl<CPageProgress>)
			END_MSG_MAP()
		
			BEGIN_DDX_MAP(CPageProgress)
			END_DDX_MAP()

			LRESULT OnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
			LRESULT OnImportStatus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			LRESULT OnImportFinished(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			CPageProgress(void);

			int OnSetActive(void);
			BOOL OnQueryCancel();
			INT_PTR OnWizardFinish(void);
			int OnWizardNext();

			BOOL IsImporting();

		private:
			CStatic m_txtImporting;
			CLogListBox m_lstProgress;
			CImportThread m_importThread;
			bool m_bImporting;

	}; // class CPageProgress
	CPageProgress m_pageProgress;

	BEGIN_MSG_MAP(CImportWizard)
		CHAIN_MSG_MAP(CGNPropertySheet<CImportWizard>)
	END_MSG_MAP()


	CImportWizard(_U_STRINGorID title = (LPCTSTR) NULL, UINT uStartPage = 0, HWND hWndParent = NULL )
	             : CGNPropertySheet<CImportWizard>(title, uStartPage, hWndParent)
	{
		m_psh.dwFlags |= (PSH_NOAPPLYNOW | PSH_WIZARD);

		// Add property pages, give them access to common data.
		AddPage ( m_pageFileName ); m_pageFileName.m_pData = &m_Data;
		AddPage ( m_pageSelect ); m_pageSelect.m_pData = &m_Data;
		AddPage ( m_pageGroup ); m_pageGroup.m_pData = &m_Data;
		AddPage ( m_pageProgress ); m_pageProgress.m_pData = &m_Data;
	} // constructor
}; // class CImportWizard

