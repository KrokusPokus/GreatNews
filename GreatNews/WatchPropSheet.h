#pragma once

#include "NewsWatch.h"
#include "CheckedFeedTree.h"
#include "GNPropertySheet.h"
#include "ColorButton.h"

class CWatchPropSheet: public CGNPropertySheet<CWatchPropSheet>
{
	class CPageData
	{
	public:
		CNewsWatch* m_pNewsWatch;
		void SetNewsWatch(CNewsWatch* p){m_pNewsWatch = p;}
	};

	class CWatchPropPageName: public CPropertyPageImpl<CWatchPropPageName>
	                             , public CWinDataExchange<CWatchPropPageName> //  DDX implementation, call DoDataExchange() where relevant.
								 , public CPageData
	{
	public:
		CWatchPropPageName() : CPropertyPageImpl<CWatchPropPageName>(_T("News Watch Properties")) {};

		enum {IDD = IDD_WATCH_NAME};

		BEGIN_MSG_MAP(CWatchPropPageName)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDC_WATCH_WORDS, EN_CHANGE, OnKeywordsChange)
			CHAIN_MSG_MAP(CPropertyPageImpl<CWatchPropPageName>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		BEGIN_DDX_MAP(CWatchPropPageName)
			DDX_TEXT(IDC_WATCH_WORDS, m_pNewsWatch->m_matchCriteria)
			DDX_TEXT(IDC_WATCH_NAME, m_pNewsWatch->m_title)
			DDX_RADIO(IDC_RDOANY, m_pNewsWatch->m_allWords)
			DDX_RADIO(IDC_HIGHLIGHT, (int)m_pNewsWatch->m_nAction)
			DDX_CHECK(IDC_CHKCASE, m_pNewsWatch->m_matchCase)
			DDX_CHECK(IDC_CHKWHOLE, m_pNewsWatch->m_wholeWord)
			DDX_CHECK(IDC_CHKDISABLEWATCH, m_pNewsWatch->m_bDisabled)
		END_DDX_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnKeywordsChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

		int OnSetActive(void);
		int OnKillActive(void);
		CString GenerateTitle(CString keyword);

		CButton m_chkTitle;
		CButton m_chkContent;
		CButton m_chkAuthor;
		CButton m_chkURL;
		CColorButton m_btnFrontColor;
		CColorButton m_btnBkColor;

		bool m_bAutoGenerateName;

	}; // class CWatchPropPageName
	CWatchPropPageName m_WatchPropPageName;



	class CWatchPropPageChannels: public CPropertyPageImpl<CWatchPropPageChannels>
	                             , public CWinDataExchange<CWatchPropPageChannels> //  DDX implementation, call DoDataExchange() where relevant.
								 , public CPageData
	{
	public:
		CWatchPropPageChannels() : CPropertyPageImpl<CWatchPropPageChannels>(_T("News Watch Properties")) {};

		enum {IDD = IDD_WATCH_CHANNELS};

		BEGIN_MSG_MAP(CWatchPropPageChannels)
			COMMAND_ID_HANDLER(IDC_BTNSELECTALL, OnSelectAll)
			COMMAND_ID_HANDLER(IDC_BTNUNSELECTALL, OnUnselectAll)
			COMMAND_ID_HANDLER(IDC_RDOALL, OnRdoall)
			COMMAND_ID_HANDLER(IDC_RDOSELECTIVE, OnRdoselective)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			CHAIN_MSG_MAP(CPropertyPageImpl<CWatchPropPageChannels>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		
		BEGIN_DDX_MAP(CWatchPropPageChannels)
			DDX_RADIO(IDC_RDOALL, m_pNewsWatch->m_bWatchSelectiveChannels)
		END_DDX_MAP()

		LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUnselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnDeselectall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnRdoall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnRdoselective(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
		int OnSetActive(void);
		INT_PTR OnWizardFinish(void);

		
		void EnableTreeControls(BOOL bEnable=TRUE);

		CButton m_chkCheckExisting;
		CCheckedFeedTreeCtrl m_tree;
	}; // class CWatchPropPageChannels
	CWatchPropPageChannels m_WatchPropPageChannels;

	BEGIN_MSG_MAP(CWatchPropSheet)
		CHAIN_MSG_MAP(CGNPropertySheet<CWatchPropSheet>)
	END_MSG_MAP()

	CWatchPropSheet(_U_STRINGorID title = (LPCTSTR) NULL, UINT uStartPage = 0, HWND hWndParent = NULL )
	               : CGNPropertySheet<CWatchPropSheet>(title, uStartPage, hWndParent)
	{
		m_psh.dwFlags |= (PSH_NOAPPLYNOW | PSH_WIZARD );
		m_WatchPropPageName.SetNewsWatch(&m_NewsWatch);
		m_WatchPropPageChannels.SetNewsWatch(&m_NewsWatch);

		// Add property pages, give them access to common data.
		AddPage ( m_WatchPropPageName );
		AddPage ( m_WatchPropPageChannels );

	} // constructor

public:
	CNewsWatch m_NewsWatch;

}; // class CWatchPropSheet

