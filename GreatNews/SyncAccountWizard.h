#if !defined(WTL_SYNCACCOUNTWIZARD_H_A5A3C73E_158C_41F0_A5959B4FEFFC30D4_DEFINED)
#define WTL_SYNCACCOUNTWIZARD_H_A5A3C73E_158C_41F0_A5959B4FEFFC30D4_DEFINED

#include <atldlgs.h>  // for CGNPropertySheet<>
#include <atlddx.h>   // for DDX
#include "GNPropertySheet.h"
#include "BloglinesService.h"
#include "CheckedFeedTree.h"

class CSyncAccountWizard: public CGNPropertySheet<CSyncAccountWizard>
{

	struct CData
	{
		CData() : m_cntGroupImported(0), m_cntGroupSkipped(0), m_cntGroupFailed(0),
			m_cntFeedImported(0), m_cntFeedSkipped(0), m_cntFeedFailed(0),
			m_bDontMark(false) {}

		MSXML2::IXMLDOMDocumentPtr m_spBloglinesOPML;

		CString                      m_csSyncAccount;
		CString                      m_csSyncPassword;
		CString                      m_csSyncConfirmPassword;
		bool m_bDontMark;

		int m_cntGroupImported;
		int m_cntGroupSkipped;
		int m_cntGroupFailed;
		int m_cntFeedImported;
		int m_cntFeedSkipped;
		int m_cntFeedFailed;

	} m_Data;


	class CAccountPage: public CPropertyPageImpl<CAccountPage>
	                                , public CWinDataExchange<CAccountPage> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:

			enum {IDD = IDD_SYNC_ACCOUNT};

			CData *m_pData;

			BEGIN_MSG_MAP(CAccountPage)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				CHAIN_MSG_MAP(CPropertyPageImpl<CAccountPage>)
			END_MSG_MAP()

			BEGIN_DDX_MAP(CAccountPage)
				DDX_TEXT(IDC_SYNC_ACCOUNT, m_pData->m_csSyncAccount)
				DDX_TEXT(IDC_SYNC_PASSWORD, m_pData->m_csSyncPassword)
				DDX_TEXT(IDC_SYNC_CONFIRM_PASSWORD, m_pData->m_csSyncConfirmPassword)
			END_DDX_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		
			CAccountPage(void);

		int OnSetActive(void);
		int OnKillActive(void);
	
	}; // class CAccountPage
	CAccountPage m_accountPage;


	class CChannelPage: public CPropertyPageImpl<CChannelPage>
	                                , public CWinDataExchange<CChannelPage> //  DDX implementation, call DoDataExchange() where relevant.
	{
		public:

			enum {IDD = IDD_SYNC_CHANNELS};

			CData *m_pData;

			BEGIN_DDX_MAP(CChannelPage)
				DDX_CHECK(IDC_DONTMARKBLOGLINESREAD, m_pData->m_bDontMark)
			END_DDX_MAP()

			BEGIN_MSG_MAP(CChannelPage)
				MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
				COMMAND_ID_HANDLER(IDC_BTNSELECTALL, OnBtnselectall)
				COMMAND_ID_HANDLER(IDC_BTNUNSELECTALL, OnBtnunselectall)
				CHAIN_MSG_MAP(CPropertyPageImpl<CChannelPage>)
				REFLECT_NOTIFICATIONS()
			END_MSG_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		
			LRESULT OnBtnselectall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
			LRESULT OnBtnunselectall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
			CChannelPage(void);

		int OnSetActive(void);
		INT_PTR OnWizardFinish(void);

	private:
		void ImportFeedGroup(FeedGroupPtr group);
		void ImportNewsFeed(NewsFeedPtr feed);
		CCheckedFeedTreeCtrl m_treeSelect;
		
	}; // class CChannelPage
	CChannelPage m_channelPage;

	BEGIN_MSG_MAP(CSyncAccountWizard)
		CHAIN_MSG_MAP(CGNPropertySheet<CSyncAccountWizard>)
	END_MSG_MAP()


	CSyncAccountWizard(_U_STRINGorID title = (LPCTSTR) NULL, UINT uStartPage = 0, HWND hWndParent = NULL )
	                  : CGNPropertySheet<CSyncAccountWizard>(title, uStartPage, hWndParent)
	{
		m_psh.dwFlags |= (PSH_NOAPPLYNOW | PSH_WIZARD);

		// Add property pages, give them access to common data.
		AddPage ( m_accountPage ); m_accountPage.m_pData = &m_Data;
		AddPage ( m_channelPage ); m_channelPage.m_pData = &m_Data;
	} // constructor
}; // class CSyncAccountWizard

#endif // !defined(WTL_SYNCACCOUNTWIZARD_H_A5A3C73E_158C_41F0_A5959B4FEFFC30D4_DEFINED)
