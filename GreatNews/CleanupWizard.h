#pragma once

#include "MyDDX.h"
#include "CheckedFeedTree.h"
#include "GNPropertySheet.h"

class CCleanupWizard: public CGNPropertySheet<CCleanupWizard>
{

	struct CData
	{
		CData()
			: m_dwCbodays(1),m_chkDeleteMarked(false),m_chkDeleteUnread(false)
			,m_chkCompress(false), m_chkKeepItems(true), m_numOfItemsToKeep(0)
		{
		}

		////// Data variables from dialog IDD_CLEANUP_DAYS ( ).
		INT_PTR m_dwCbodays;
		bool m_chkDeleteMarked;
		bool m_chkDeleteUnread;
		bool m_chkCompress;
		bool m_chkKeepItems;
		bool m_chkDeleteTemp;
		int m_numOfItemsToKeep;
		////// Data variables from dialog IDD_CLEANUP_CHANNELS ( ).
		NewsFeedVector channelIDs;
		////// Data variables from dialog IDD_CLEANUP_PROGRESS ( ).
	
	} m_Data;


	template <class CData> // Common data for all pages in property sheet/wizard
	class CCleanupDays: public CPropertyPageImpl<CCleanupDays<CData> >
	                            , public CMyWinDataExchange<CCleanupDays<CData> > //  DDX implementation, call DoDataExchange() where relevant.
	{

		//  CComboBox                   idcCbodays;
		//  CButton                     idcChkdeletemarked;
		//  CButton                     idcChkdeleteunread;
		
		public:
			CCleanupDays() : CPropertyPageImpl<CCleanupDays>(_T("Cleanup Wizard")) {};

			enum {IDD = IDD_CLEANUP_DAYS};

			CData *m_pData;

			BEGIN_MSG_MAP(CCleanupDays)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				CHAIN_MSG_MAP(CPropertyPageImpl<CCleanupDays>)
			END_MSG_MAP()

			BEGIN_DDX_MAP(CCleanupDays)
				DDX_COMBO_INDEX(IDC_CBODAYS, m_pData->m_dwCbodays)
				DDX_CHECK(IDC_CHKDELETEUNREAD, m_pData->m_chkDeleteUnread)
				DDX_CHECK(IDC_CHKDELETEMARKED, m_pData->m_chkDeleteMarked)
				DDX_CHECK(IDC_CHKCOMPRESS, m_pData->m_chkCompress)
				DDX_CHECK(IDC_CHKCLEANUPITEM, m_pData->m_chkKeepItems)
				DDX_CHECK(IDC_CHKDELETTEMP, m_pData->m_chkDeleteTemp)
				DDX_INT(IDC_EDTCLEANUPITEM, m_pData->m_numOfItemsToKeep)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		
		int OnSetActive(void);
		int OnKillActive(void);
		
	}; // class CCleanupDays
	CCleanupDays<CData> m_pageDays;


	template <class CData> // Common data for all pages in property sheet/wizard
	class CCleanupChannels: public CPropertyPageImpl<CCleanupChannels<CData> >
	                            , public CWinDataExchange<CCleanupChannels<CData> > //  DDX implementation, call DoDataExchange() where relevant.
	{

		//  CButton                     idcDeselectall;
		//  CListViewCtrl               idcLstchannels;
		//  CButton                     idcRdoall;
		//  CButton                     idcRdoselective;
		
		public:

			enum {IDD = IDD_CLEANUP_CHANNELS};

			CData *m_pData;

			BEGIN_MSG_MAP(CCleanupChannels)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				COMMAND_ID_HANDLER(IDC_BTNSELECTALL, OnSelectAll)
				COMMAND_ID_HANDLER(IDC_BTNUNSELECTALL, OnUnselectAll)
				CHAIN_MSG_MAP(CPropertyPageImpl<CCleanupChannels>)
				REFLECT_NOTIFICATIONS()
			END_MSG_MAP()

		
			BEGIN_DDX_MAP(CCleanupChannels)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
			LRESULT OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
		CCheckedFeedTreeCtrl m_tree;

		int OnSetActive(void);
		int OnKillActive(void);
	}; // class CCleanupChannels
	CCleanupChannels<CData> m_pageChannels;

	template <class CData> // Common data for all pages in property sheet/wizard
	class CCleanupProgress: public CPropertyPageImpl<CCleanupProgress<CData> >
	                            , public CWinDataExchange<CCleanupProgress<CData> > //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:

			enum {IDD = IDD_CLEANUP_PROGRESS};

			CData *m_pData;

			BEGIN_MSG_MAP(CCleanupProgress)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				CHAIN_MSG_MAP(CPropertyPageImpl<CCleanupProgress>)
			END_MSG_MAP()

		
			BEGIN_DDX_MAP(CCleanupProgress)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		
		int OnSetActive(void);
		int OnWizardNext();
	}; // class CCleanupProgress
	CCleanupProgress<CData> m_pageProgress;

	BEGIN_MSG_MAP(CCleanupWizard)
		CHAIN_MSG_MAP(CGNPropertySheet<CCleanupWizard>)
	END_MSG_MAP()


	CCleanupWizard(_U_STRINGorID title = (LPCTSTR) NULL, UINT uStartPage = 0, HWND hWndParent = NULL )
	              : CGNPropertySheet<CCleanupWizard>(title, uStartPage, hWndParent)
	{
		m_psh.dwFlags |= (PSH_NOAPPLYNOW | PSH_WIZARD );

		// Add property pages, give them access to common data.
		AddPage ( m_pageDays ); m_pageDays.m_pData = &m_Data;
		AddPage ( m_pageChannels ); m_pageChannels.m_pData = &m_Data;
		AddPage ( m_pageProgress ); m_pageProgress.m_pData = &m_Data;
	} // constructor
}; // class CCleanupWizard


