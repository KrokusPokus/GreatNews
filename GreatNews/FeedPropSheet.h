#pragma once

#include "GNPropertySheet.h"
#include "NewsFeed.h"
#include "FeedGroup.h"
#include "FeedTreeCtrlBase.h"
#include "myddx.h"

class CFeedPropSheet: public CGNPropertySheet<CFeedPropSheet>
{
public:
	CNewsFeed m_newsFeed;
	CString m_title;
	bool m_bReloadTree;
	bool m_bWizardMode;

	class CData
	{
	public:
		CData(CNewsFeed& feed, bool& bReload, bool& bWizardMode) 
			: m_feed(feed), m_bReloadTree(bReload), m_bWizardMode(bWizardMode){}

		CNewsFeed& m_feed;
		bool& m_bReloadTree;
		bool& m_bWizardMode;
	};

	class CFeedPageUrl: public CPropertyPageImpl<CFeedPageUrl>
                        , public CMyWinDataExchange<CFeedPageUrl> //  DDX implementation, call DoDataExchange() where relevant.
						, public CData
	{

		//  CButton                     idcChkquerychannel;
		//  CEdit                       idcEdit1;
		
		public:
		enum {IDD = IDD_CHANNEL_URL};

		BEGIN_MSG_MAP(CFeedPageUrl)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDC_VALIDATE, OnValidate)
			COMMAND_ID_HANDLER(IDC_DETACHBLOGLINES, OnDetach)
			COMMAND_ID_HANDLER(IDC_CHKREQUIRELOGIN, OnRequireLogin)
			CHAIN_MSG_MAP(CPropertyPageImpl<CFeedPageUrl>)
		END_MSG_MAP()

		BEGIN_DDX_MAP(CFeedPageUrl)
			DDX_TEXT(IDC_EDTURL, m_feed.m_url)
			DDX_CHECK(IDC_CHKQUERYCHANNEL, m_bQuery)
			DDX_CHECK(IDC_CHKDISABLE, m_feed.m_bDisabled)
			DDX_CHECK(IDC_CHKREQUIRELOGIN, m_feed.m_bUseLogin)
			DDX_TEXT(IDC_TXTLOGINNAME, m_feed.m_loginName)
			DDX_TEXT(IDC_TXTLOGINPWD, m_feed.m_loginPassword)
		END_DDX_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnValidate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnDetach(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnRequireLogin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
		CFeedPageUrl(CNewsFeed& feed, bool& bReload, bool& bWizardMode);

		int OnSetActive(void);
		int OnKillActive(void);
		
		bool m_bQuery;

	private:
		void SetupControls();
		CString AutoDetectFeed(const CString& url);


	}; // class CFeedPageUrl
	CFeedPageUrl m_pageUrl;


	class CFeedPageProperties: public CPropertyPageImpl<CFeedPageProperties>
                        , public CMyWinDataExchange<CFeedPageProperties> //  DDX implementation, call DoDataExchange() where relevant.
						, public CData
	{
	public:
		CString m_strTTL;

		enum {IDD = IDD_CHANNEL_PROPERTIES};

		BEGIN_MSG_MAP(CFeedPageProperties)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			CHAIN_MSG_MAP(CPropertyPageImpl<CFeedPageProperties>)
		END_MSG_MAP()

		BEGIN_DDX_MAP(CFeedPageProperties)
			DDX_TEXT(IDC_TITLE, m_feed.m_title)
			DDX_TEXT(IDC_DESCRIPTION,m_feed.m_description)
			DDX_TEXT(IDC_WEBSITE,m_feed.m_website)
			DDX_TEXT(IDC_TXT_TTL, m_strTTL)
		END_DDX_MAP()
	
		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		
		CFeedPageProperties(CNewsFeed& feed, bool& bReload, bool& bWizardMode);

		int OnSetActive(void);
		int OnKillActive(void);
		
	}; // class CFeedPageProperties
	CFeedPageProperties m_pageProperties;

	class CFeedPageSettings: public CPropertyPageImpl<CFeedPageSettings>
                        , public CMyWinDataExchange<CFeedPageSettings> //  DDX implementation, call DoDataExchange() where relevant.
						, public CData
	{
	public:
		enum {IDD = IDD_CHANNEL_SETTINGS};

		BEGIN_MSG_MAP(CFeedPageSettings)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDC_CHKCLEANUPITEM, OnLimitItems)
			CHAIN_MSG_MAP(CPropertyPageImpl<CFeedPageSettings>)
		END_MSG_MAP()

		BEGIN_DDX_MAP(CFeedPageSettings)
			DDX_COMBO_INDEX(IDC_UPDATE_FREQ,m_feed.m_updateFreq)
			DDX_COMBO_INDEX(IDC_CBOCLEANUP,m_feed.m_cleanupMaxAge)
			DDX_CHECK(IDC_NOTIFYNEWITEMS, m_feed.m_notifyNewItem)
			DDX_CHECK(IDC_CHKCLEANUPITEM, m_feed.m_bLimitItems)
			DDX_INT(IDC_EDTCLEANUPITEM, m_feed.m_maxNumOfItems)
			DDX_CHECK(IDC_KEEPINACTIVE, m_feed.m_bKeepInactive)
		END_DDX_MAP()
	
		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		
		CFeedPageSettings(CNewsFeed& feed, bool& bReload, bool& bWizardMode);

		int OnSetActive(void);
		int OnKillActive(void);
		void SetupControls();
		LRESULT OnLimitItems(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
	}; // class CFeedPageSettings
	CFeedPageSettings m_pageSettings;

	class CFeedPageGroup: public CPropertyPageImpl<CFeedPageGroup>
                        , public CMyWinDataExchange<CFeedPageGroup> //  DDX implementation, call DoDataExchange() where relevant.
						, public CData
	{
	public:

		enum {IDD = IDD_CHANNEL_GROUP};

		BEGIN_MSG_MAP(CFeedPageGroup)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDC_NEWGROUP, OnNewGroup)
			CHAIN_MSG_MAP(CPropertyPageImpl<CFeedPageGroup>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnNewGroup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
		CFeedPageGroup(CNewsFeed& feed, bool& bReload, bool& bWizardMode);

		int OnSetActive(void);
		int OnKillActive(void);
		INT_PTR OnWizardFinish(void);
	
	public:
		CFeedTreeCtrlBase m_treeGroup;
	}; // class CFeedPageGroup
	CFeedPageGroup m_pageGroup;

	BEGIN_MSG_MAP(CFeedPropSheet)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CGNPropertySheet<CFeedPropSheet>)
	END_MSG_MAP()


	CFeedPropSheet(_U_STRINGorID title , NewsFeedPtr newsFeed=NULL, UINT uStartPage = 0, HWND hWndParent = NULL )
	              : CGNPropertySheet<CFeedPropSheet>(title, uStartPage, hWndParent),
					m_pageUrl(m_newsFeed, m_bReloadTree, m_bWizardMode),
					m_pageProperties(m_newsFeed, m_bReloadTree, m_bWizardMode),
					m_pageGroup(m_newsFeed, m_bReloadTree, m_bWizardMode),
					m_pageSettings(m_newsFeed, m_bReloadTree, m_bWizardMode)

	{
		m_bReloadTree = false;
		if(newsFeed!=NULL)
			m_newsFeed = *newsFeed;

		// m_title = title.m_lpstr;
		m_psh.dwFlags |= PSH_NOAPPLYNOW;
		if(m_newsFeed.m_id==0) // new feed?
		{
			m_psh.dwFlags |= PSH_WIZARD;
			m_bWizardMode = true;
		}
		else
			m_bWizardMode = false;

		// Add property pages, give them access to common data.
		AddPage ( m_pageUrl );
		AddPage ( m_pageProperties );
		AddPage ( m_pageGroup );

		if(m_newsFeed.m_id!=0) // existing feed?
		{
			AddPage(m_pageSettings);
		}
	} // constructor

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{

		return 0;		
	}

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


}; // class CFeedPropSheet

