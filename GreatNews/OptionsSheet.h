#pragma once

#include <atldlgs.h>  // for CGNPropertySheet<>
#include <atlddx.h>   // for DDX
#include "GNPropertySheet.h"
#include "GreatNewsConfig.h"
#include "MyDDX.h"
#include "GNResourceManager.h"

class COptionsSheet: public CGNPropertySheet<COptionsSheet>
{
	class COptionsPageGeneral: public CPropertyPageImpl<COptionsPageGeneral>
	                           , public CMyWinDataExchange<COptionsPageGeneral> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:
			bool m_bAutoUpdate;
			bool m_bRemoveFromTaskBar;
			bool m_bCloseToTray;
			INT_PTR m_cleanupReminder;
			int m_nStartupPage;

			enum {IDD = IDD_OPTION_GENERAL};

			BEGIN_MSG_MAP(COptionsPageGeneral)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				COMMAND_ID_HANDLER(IDC_SHOWBLANKSCREEN, OnRefreshControls)
				COMMAND_ID_HANDLER(IDC_SHOWWELCOME, OnRefreshControls)
				COMMAND_ID_HANDLER(IDC_SHOWTOP10, OnRefreshControls)
				COMMAND_ID_HANDLER(IDC_SHOWLABEL, OnRefreshControls)
				COMMAND_ID_HANDLER(IDC_SHOWWATCH, OnRefreshControls)
				COMMAND_ID_HANDLER(IDS_UPDATECHANNELEVERY, OnRefreshControls)
				COMMAND_ID_HANDLER(IDC_BTNDEFAULTREADER, OnDefaultReader)
				CHAIN_MSG_MAP(CPropertyPageImpl<COptionsPageGeneral>)
			END_MSG_MAP()

			BEGIN_DDX_MAP(COptionsPageGeneral)
				DDX_CHECK(IDC_UPDATECHANNELATSTARTUP, m_bAutoUpdate)
				DDX_CHECK(IDC_REMOVEFROMTASKBAR, m_bRemoveFromTaskBar)
				DDX_CHECK(IDC_CHKCLOSETOTRAY, m_bCloseToTray)
				DDX_CHECK(IDS_UPDATECHANNELEVERY, g_GreatNewsConfig.m_bAutoUpdateChannels)
				DDX_CHECK(IDC_DELTEMPWHENEXIT, g_GreatNewsConfig.m_bCleanTempFilesUponExit)
				DDX_RADIO(IDC_SHOWBLANKSCREEN, m_nStartupPage)
				DDX_COMBO_INDEX(IDC_AUTOUPDATE_FREQ, g_GreatNewsConfig.m_nAutoUpdateFreq)
				DDX_COMBO_INDEX(IDC_CLEANUPREMINDER, m_cleanupReminder)
				DDX_COMBO_ITEMDATA(IDC_ALLLABELS, g_GreatNewsConfig.m_nStartupLabel)
				DDX_COMBO_ITEMDATA(IDC_ALLWATCHES, g_GreatNewsConfig.m_nStartupWatch)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			LRESULT OnDefaultReader(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
			LRESULT OnRefreshControls(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
		
			COptionsPageGeneral(void);
			void EnableControls();
			void PopulateLabels();
			void PopulateWatches();

	}; // class COptionsPageGeneral
	COptionsPageGeneral m_generalPage;


	class COptionPagePager: public CPropertyPageImpl<COptionPagePager>
	                           , public CWinDataExchange<COptionPagePager> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:
			int m_nPageSize;
			int m_nSecondsToMarkItemRead;

			enum {IDD = IDD_OPTION_PAGE};

			BEGIN_MSG_MAP(COptionPagePager)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				COMMAND_ID_HANDLER(IDC_NOTIFYINACTIVECHANNELS, OnNotifyInactive)
				CHAIN_MSG_MAP(CPropertyPageImpl<COptionPagePager>)
			END_MSG_MAP()

			BEGIN_DDX_MAP(COptionPagePager)
				DDX_INT_RANGE(IDC_TXTNUMOFITEMS,m_nPageSize,1,999)
				DDX_INT_RANGE(IDC_ITEM_READ_SECONDS,m_nSecondsToMarkItemRead,0,999)
				DDX_CHECK(IDC_AUTOUPDATECHANNEL, g_GreatNewsConfig.m_bUpdateWhenClick)
				DDX_CHECK(IDC_MARKCHANNELREADWHENSWITCHING, g_GreatNewsConfig.m_bMarkChannelReadWhenSwitching)
				DDX_RADIO(IDC_STAYONSAMEPAGE, g_GreatNewsConfig.m_nNextPageOpenUnread)
				DDX_RADIO(IDC_MARKREADINTERACTIVELY, g_GreatNewsConfig.m_nMarkReadAutomatically)
				DDX_CHECK(IDC_NOTIFYINACTIVECHANNELS, g_GreatNewsConfig.m_bNotifyDinosaurChannels)
				DDX_INT(IDC_DINOSOURDAYS, g_GreatNewsConfig.m_nDinosaurDays)
				DDX_INT(IDC_EDTNEWITEMNOTIFYSECONDS, g_GreatNewsConfig.m_nNewItemNotifySeconds)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			LRESULT OnNotifyInactive(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
			void SetupControls();
	
			COptionPagePager(void);
	
	}; // class COptionPagePager
	COptionPagePager m_pagerPage;

	class COptionPageUsability: public CPropertyPageImpl<COptionPageUsability>
	                           , public CWinDataExchange<COptionPageUsability> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:
			enum {IDD = IDD_OPTION_USABILITY};

			BEGIN_MSG_MAP(COptionPageUsability)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				CHAIN_MSG_MAP(CPropertyPageImpl<COptionPageUsability>)
			END_MSG_MAP()

			BEGIN_DDX_MAP(COptionPageUsability)
				DDX_CHECK(IDC_CHKCLOSENEWSLIST,g_GreatNewsConfig.m_bAutoCloseNewsList)
				DDX_CHECK(IDC_CHKSETFILTERTOUNREAD,g_GreatNewsConfig.m_bAutoSetFilterToUnread)
				DDX_CHECK(IDC_MARK_LASTCHANNEL_READ,g_GreatNewsConfig.m_bOpenNextAutoMarkLastChannelAsRead)
				DDX_CHECK(IDC_SEARCH_OPEN_NEWSLIST,g_GreatNewsConfig.m_bSearchAutoOpenNewsList)
				DDX_CHECK(IDC_BLOCKPOPUP,g_GreatNewsConfig.m_nBlockPopups)
				DDX_CHECK(IDC_SHOW_UPDATEFINISH_TOOLTIPs,g_GreatNewsConfig.m_bShowTooltipAfterUpdate)
			//	DDX_CHECK(IDC_CLICKTODEFAULTBROWSER,g_GreatNewsConfig.m_bOpenRssLinkInDefaultBrowser)
				DDX_CHECK(IDC_DISABLETAB,g_GreatNewsConfig.m_bDisableTabbedBrowsing)
				DDX_CHECK(IDC_DONTCOLLAPSEGROUP,g_GreatNewsConfig.m_bNoAutoGroupCollapse)
				DDX_CHECK(IDC_MARKCHANGEDUNREAD,g_GreatNewsConfig.m_bMarkChangesUnread)

				DDX_RADIO(IDC_RDOEXTERNALBROWSER, g_GreatNewsConfig.m_nDoubleClickingNewsList)
				DDX_RADIO(IDC_OPENRSSLINK, g_GreatNewsConfig.m_nOpenRssLinkInDefaultBrowser)
			END_DDX_MAP()

			COptionPageUsability(void);
			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	}; // class COptionPageUsability
	COptionPageUsability m_usabilityPage;

	class COptionPageFeatures: public CPropertyPageImpl<COptionPageFeatures>
	                           , public CWinDataExchange<COptionPageFeatures> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:
			enum {IDD = IDD_OPTION_FEATURES};

			BEGIN_MSG_MAP(COptionPageFeatures)
				COMMAND_ID_HANDLER(IDC_CONFIGBLOGTHIS, OnConfigBlogThis)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				CHAIN_MSG_MAP(CPropertyPageImpl<COptionPageFeatures>)
			END_MSG_MAP()

			BEGIN_DDX_MAP(COptionPageFeatures)
			END_DDX_MAP()

			COptionPageFeatures(void);
			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			LRESULT OnConfigBlogThis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

			void SetupControls();
			void ReadControls();
	}; // class COptionPageFeatures
	COptionPageFeatures m_featurePage;

	class COptionPageConnection: public CPropertyPageImpl<COptionPageConnection>
	                           , public CWinDataExchange<COptionPageConnection> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:
			enum {IDD = IDD_OPTION_CONNECTION};

			BEGIN_MSG_MAP(COptionPageConnection)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				CHAIN_MSG_MAP(CPropertyPageImpl<COptionPageConnection>)
				COMMAND_ID_HANDLER(IDC_USE_IE_PROXY, OnProxyChange)
				COMMAND_ID_HANDLER(IDC_GN_PROXY, OnProxyChange)
			END_MSG_MAP()

			BEGIN_DDX_MAP(COptionPageConnection)
				DDX_RADIO(IDC_USE_IE_PROXY, g_GreatNewsConfig.m_nProxyToUse)
				DDX_TEXT(IDC_PROXY_URL, g_GreatNewsConfig.m_proxyURL)
				DDX_INT(IDC_PROXY_PORT, g_GreatNewsConfig.m_proxyPort)
				DDX_TEXT(IDC_PROXY_USER, g_GreatNewsConfig.m_proxyUserName)
				DDX_TEXT(IDC_PROXY_PASSWORD, g_GreatNewsConfig.m_proxyPassword)
				DDX_TEXT(IDC_PROXY_BYPASS, g_GreatNewsConfig.m_proxyBypass)
			END_DDX_MAP()

			COptionPageConnection(void);
			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
			LRESULT OnProxyChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

			void SetupControls();
	}; // class COptionPageConnection
	COptionPageConnection m_connectionPage;

	BEGIN_MSG_MAP(COptionsSheet)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		CHAIN_MSG_MAP(CGNPropertySheet<COptionsSheet>)
	END_MSG_MAP()

	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK)
		{
			if(!OnOK())
			{
				bHandled = true;
				return 0;
			}
		}
		bHandled = false;

		return 0;
	}

	BOOL OnOK();

	COptionsSheet(_U_STRINGorID title = _T("Options"), UINT uStartPage = 0, HWND hWndParent = NULL )
	             : CGNPropertySheet<COptionsSheet>(title, uStartPage, hWndParent)
	{
		m_psh.dwFlags |= PSH_NOAPPLYNOW;

		AddPage ( m_generalPage );
		AddPage ( m_pagerPage );
		AddPage ( m_usabilityPage );
		AddPage ( m_featurePage );
		AddPage ( m_connectionPage );

	} // constructor

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
		pResMngr->ApplyLanguageToWindow(m_hWnd, _T("OptionsDialog"));

		return 0;		
	}

}; // class COptionsSheet

