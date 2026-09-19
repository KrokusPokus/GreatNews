#pragma once

#include "IniFile.h"


class CGreatNewsConfig
{
public:

#define DEFAULT_DBNAME _T("newsfeed.db")
#define DEFAULT_CONFIG _T("GreatNews.ini")

	enum UpdateFreqency {
		None,
		FifteenMin,
		HalfHour,
		OneHour
	};
	enum StartupScreen {
		Blank,
		Welcome,
		Top10Visited,
		Label,
		Watch
	};
	enum ReadingPanePos
	{
		Bottom = 0,
		Right = 1
	};

	CGreatNewsConfig(void);
	~CGreatNewsConfig(void);

public:
	bool Init(LPCTSTR cmdLine);
	CString GetHomeDir();
	CString GetPluginDir();
	CString GetMediaDir();
	CString GetFaviconDir();
	CString GetLanguageDir();

	CString GetDefaultChannelOPML();
	CString CalculateAppDir();
	CString CalculateHomeDir();
	CString GetConfigFileName();
	void RegisterFeedProtocolHandler();

	//
	// User Environments
	//
	CString GetLanguage();
	void SaveLanguage(LPCTSTR languagePackName);

	CString GetWindowPlacement();
	void SetWindowPlacement(const CString& place);
	bool GetNewsListClosed();
	void SetNewsListClosed(bool bClosed);
	bool GetChannelTreeClosed();
	void SetChannelTreeClosed(bool bClosed);
	int GetNewsListPos();
	void SetNewsListPos(int pos);
	int GetChannelTreePos();
	void SetChannelTreePos(int pos);
	int GetDateFilterSelection();
	void SetDateFilterSelection(int n);
	int GetStatusFilterSelection();
	void SetStatusFilterSelection(int n);

	int GetColWidthTitle();
	void SetColWidthTitle(int n);
	int GetColWidthDate();
	void SetColWidthDate(int n);
	int GetColWidthAuthor();
	void SetColWidthAuthor(int n);
	int GetColWidthChannel();
	void SetColWidthChannel(int n);

	CTime GetCleanupLastRun();
	void SetCleanupLastRunNow();
	//
	// Options
	//
	bool GetRemoveFromTaskBar();
	void SetRemoveFromTaskBar(bool bRemove);
	bool GetCloseToTray();
	void SetCloseToTray(bool bToTray);

	bool m_bAutoUpdateChannels;
	INT_PTR m_nAutoUpdateFreq;
	bool GetAutoUpdateAtStartup();
	void SetAutoUpdateAtStartup(bool bUpdate);
	int GetAutoUpdateFreqSec();

	int GetCleanupReminder();
	void SetCleanupReminder(INT_PTR n);
	int GetCleanupReminderDays();
	CString GetCurrentStyle();
	void SetCurrentStyle(const CString& style);
	LPCTSTR GetCurrentStyleString();
	LPCTSTR GetBuiltInStyle();
	CString GetStyleFileContent(const CString& styleFileName);
	int GetStartupScreen();
	void SetStartupScreen(int n);

	int GetCleanupDefault();
	void SetCleanupDefault(INT_PTR n);
	bool GetCleanupDeleteUnread();
	void SetCleanupDeleteUnread(bool bUpdate);
	bool GetCleanupDeleteLabeled();
	void SetCleanupDeleteLabeled(bool bLabeled);
	bool GetCleanupDeleteTemp();
	void SetCleanupDeleteTemp(bool bDel);
	bool GetCleanupCompress();
	void SetCleanupCompress(bool bCompress);
	bool GetCleanupKeepItems();
	void SetCleanupKeepItems(bool bKeep);
	int GetCleanupNumOfItemsToKeep();
	void SetCleanupNumOfItemsToKeep(int num);
	//bool GetShowWelcomePage();
	//void SetShowWelcomePage(bool bShow);

	// news list
	int GetNewsListSortColumn();
	bool GetNewsListSortAscending();
	void SetNewsListSortColumn(int col);
	void SetNewsListSortAscending(bool bAscending);

	// advanced options
	int GetChannelUpdateTimeOut();
	int GetNumOfConcurrentUpdate();
	bool m_bBypassTTLChecking;
	bool m_bNoChannelGroupView;
	bool m_bUseMozillaEngine;
	bool m_bNoFavicons;
	CString m_tempDirectory;
	int m_maxFeedSizeInK;
	bool m_bMarkChangesUnread;
	bool m_bLegacyBrowser;
	bool m_bInternetZone;
	bool m_bMultipleInstance;
	bool m_bIgnoreDbVersion;
	bool m_bShowTimeIn24Hr;

	// generic load/save
	void LoadConfig();
	void SaveConfig();

	void LoadSearchChannelDefinition();
	bool SetupProxy();

public:
	static TCHAR* AutoUpdateString[];
	static int AutoUpdateSec[];
	static TCHAR* CleanupRemindString[];
	static int CleanupRemindDays[];
	static TCHAR* CleanupMaxAgeString[];
	static int CleanupMaxAgeDays[];

public:
	CString m_newFeedUrl; // store the feed url sent in from command line
	CString m_appDir;
	CString m_homeDir;
	CString m_dbFileName;
	UpdateFreqency m_updateFrequency;
	bool m_bAutoCloseNewsList;
	bool m_bAutoSetFilterToUnread;
	bool m_bOpenNextAutoMarkLastChannelAsRead;
	bool m_bMarkChannelReadWhenSwitching;
	bool m_bSearchAutoOpenNewsList;
	bool m_bShowTooltipAfterUpdate;
	int m_nOpenRssLinkInDefaultBrowser;	// 0=curren tab 1=defaultbrowser 2=newtab
	int m_nMiddleClickToNewWindow;
	bool m_bUpdateWhenClick;
	int m_nPageSize;
	bool m_bNotifyDinosaurChannels;
	int m_nDinosaurDays;
	int m_nBlockPopups;
	int m_nSecondsToMarkItemRead;
	int m_nNextPageOpenUnread;
	int m_nMarkReadAutomatically;
	int m_nSelectedItemFeatures;
	bool m_bHighlightNewsWatch;	// this option really should be merged into m_nSelectedItemFeatures
	CString m_podcastingEnclosuresTypes;
	bool m_bDisableTabbedBrowsing;
	int m_nNewItemNotifySeconds;
	int m_nDoubleClickingNewsList;
	bool m_bNoAutoGroupCollapse;
	int m_nMouseWheelSensitivity;
	bool m_bKeepOriginalLink;
	bool m_bWorkOffline;
	int m_nStartupScreen;
	int m_nStartupLabel;
	int m_nStartupWatch;
	bool m_bCleanTempFilesUponExit;
	int m_nReadingPanePos;	// 0 = bottom, 1= right;

	// proxies
	int m_nProxyToUse;
	CString m_proxyURL;
	CString m_combinedProxyURL;	
	int m_proxyPort;
	CString m_proxyUserName;
	CString m_proxyPassword;
	CString m_proxyBypass;

	// statistics
	int m_nStatsNumOfChannels;
	bool m_bStatsExcludeDisabled;

	CSimpleArray<CString> m_searchTargets;
	CSimpleArray<CString> m_searchTargetURLs;

public:
	CIniFile m_iniFile;
	CString m_currentStyleHTML;
	CString m_builtInStyle;

public:
	CString m_userName;
	CString m_userEmail;
	CString m_userVersion;

private:
	bool InitializeNewInstallation(const CString& homeDir, const CString& dbFileName);
	void ProcessGreatNewsTags(CString& str);
};

extern CGreatNewsConfig g_GreatNewsConfig;