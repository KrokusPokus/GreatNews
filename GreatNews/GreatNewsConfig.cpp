#include "StdAfx.h"
#include "CmdLineParser.h"
#include "GNUtil.h"
#include "GMTimeLib.h"
#include "version.h"
#include "FeedManagerLib.h"
#include "GreatNewsConfig.h"
#include "FileUtil.h"
#include "GNResourceManager.h"
#include "resource.h"
#include "AsyncDownloadManager.h"
#include "feedManager.h"
#include "CppSQLite3.h"
#include "UpgradeMngr.h"
#include <vector>

CGreatNewsConfig g_GreatNewsConfig;

TCHAR* CGreatNewsConfig::AutoUpdateString[] =
{
    _T("15 min"),
    _T("30 min"),
	_T("1 hour"),
	_T("2 hours"),
	_T("3 hours"),
	_T("4 hours"),
	NULL
};

int CGreatNewsConfig::AutoUpdateSec[] =
{
    15*60,
	30*60,
	60*60,
	120*60,
	180*60,
	240*60,
	0
};

TCHAR* CGreatNewsConfig::CleanupRemindString[] =
{
    _T("1 week"),
    _T("2 weeks"),
    _T("1 month"),
    _T("2 months"),
    _T("3 months"),
    _T("6 months"),
    _T("never"),
    NULL
};

int CGreatNewsConfig::CleanupRemindDays[] =
{
    7,
    14,
    30,
    60,
    90,
    180,
    -1
};

TCHAR* CGreatNewsConfig::CleanupMaxAgeString[] =
{
    _T("All"),
    _T("1 week"),
    _T("2 weeks"),
    _T("1 month"),
    _T("3 months"),
    _T("6 months"),
    NULL
};

int CGreatNewsConfig::CleanupMaxAgeDays[] =
{
    0,
    7,
    14,
    30,
    90,
    180,
    -1
};



CGreatNewsConfig::CGreatNewsConfig(void):
	m_updateFrequency(OneHour),
	m_bAutoCloseNewsList(true),
	m_bAutoSetFilterToUnread(true),
	m_bBypassTTLChecking(false),
	m_bNoChannelGroupView(false),
	m_nSelectedItemFeatures(0),
	m_bDisableTabbedBrowsing(false),
	m_bUseMozillaEngine(false),
	m_nProxyToUse(0),
	m_proxyPort(80),
	m_bNotifyDinosaurChannels(true),
	m_nDinosaurDays(60),
	m_nNewItemNotifySeconds(60),
	m_nStatsNumOfChannels(10),
	m_bStatsExcludeDisabled(false),
	m_nDoubleClickingNewsList(0),
	m_nMouseWheelSensitivity(5),
	m_bHighlightNewsWatch(true),
	m_bIgnoreDbVersion(false),
	m_bKeepOriginalLink(false),
	m_bWorkOffline(false),
	m_maxFeedSizeInK(1000),
	m_nStartupLabel(-1),
	m_nStartupWatch(-1),
	m_bMarkChangesUnread(false),
	m_bCleanTempFilesUponExit(true),
	m_bLegacyBrowser(false),
	m_bInternetZone(true),
	m_bMultipleInstance(false),
	m_bShowTimeIn24Hr(false)
{
	m_searchTargets.Add(_T("Search Google"));
	m_searchTargetURLs.Add(_T("http://www.google.com/search?q=%s"));

	m_searchTargets.Add(_T("Google Group"));
	m_searchTargetURLs.Add(_T("http://groups.google.com/groups?q=%s"));

	m_searchTargets.Add(_T("Yahoo"));
	m_searchTargetURLs.Add(_T("http://search.yahoo.com/search?p=%s"));

	m_searchTargets.Add(_T("MSN"));
	m_searchTargetURLs.Add(_T("http://search.msn.com/results.aspx?q=%s"));

	m_searchTargets.Add(_T("Feedster"));
	m_searchTargetURLs.Add(_T("http://www.feedster.com/search.php?q=%s"));

	m_searchTargets.Add(_T("Bing"));
	m_searchTargetURLs.Add(_T("http://www.bing.com/search?q=%s&format=rss"));
}

CGreatNewsConfig::~CGreatNewsConfig(void)
{
}

bool CGreatNewsConfig::Init(LPCTSTR cmdLine)
{
	if(cmdLine != NULL)
	{
		CCmdLineParser parser(cmdLine);
		if(parser.HasKey(_T("db")))
			m_dbFileName = parser.GetVal(_T("db"));

		if(parser.HasKey(_T("feed")))
		{
			m_newFeedUrl = parser.GetVal(_T("feed"));
			m_newFeedUrl.Replace(_T("feed://http://"), _T("feed://")); // do some fix
			m_newFeedUrl.Replace(_T("http://"), _T("feed://")); // do some fix
			m_newFeedUrl.Replace(_T("feed:feed://"), _T("feed://")); // do some fix
		}

		if(parser.HasKey(_T("IgnoreDbVersion")))
			m_bIgnoreDbVersion = true;

		// for firefox 2.0
		CString value=cmdLine;
		if(value.Find(_T("http")) == 0)
			m_newFeedUrl = value;
	}

	m_appDir = CalculateAppDir();
	m_homeDir = CalculateHomeDir();

	// if no db name is set, use the default db name
	if(m_dbFileName.GetLength() == 0)
		m_dbFileName = m_homeDir + _T("\\")+DEFAULT_DBNAME;

	if(!InitializeNewInstallation(m_homeDir, m_dbFileName))
		return false;

	CString configFile =  m_homeDir + _T("\\")+DEFAULT_CONFIG;
	m_iniFile.SetFilename(configFile);

	LoadConfig();
	CGNUtil::m_podCastingExtensions = m_podcastingEnclosuresTypes;

	LoadSearchChannelDefinition();

	// init resource manager
	CGNResourceManager* pResMngr = CGNSingleton<CGNResourceManager>::Instance();
	pResMngr->Init(GetLanguageDir());
	pResMngr->SetLanguage(GetLanguage());

	// check db file existance
	if(!::PathFileExists(m_dbFileName))
	{
		CString msg;
		msg.Format(ResManagerPtr->GetString(IDS_DATAFILENOTFOUND), (LPCTSTR)m_dbFileName);
		::MessageBox(NULL, msg, _T("GreatNews"), MB_OK|MB_ICONSTOP);
		return false;
	}

	//FeedManagerLib::Init(g_GreatNewsConfig.m_dbFileName, true);
	CFeedManager::Init(g_GreatNewsConfig.m_dbFileName);

	// check db version
	if(!m_bIgnoreDbVersion)
	{
		CString dbVer = FeedManagerLib::GetDbVersion();
		if(dbVer != DB_VERSION_LONG_STR)
		{
			if (::MessageBox(NULL,
				_T("The database file needs to be upgraded\n")
				_T("to work with this version of GreatNews.\n\n")
				_T("It won't be compatible with older versions\n")
				_T("of GreatNews after this upgrade.\n\n")
				_T("Apply the upgrade now?"),
				_T("GreatNews"),
				MB_TASKMODAL|MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK)
			{
				return false;
			}

			CUpgradeMngr upgradeManager;
			CString err, msg;
			if (!upgradeManager.Upgrade(g_GreatNewsConfig.m_dbFileName, err))
				{
					msg.Format(_T("Database update failed:\n%s"), err);
					::MessageBox(NULL, msg, _T("GreatNews"), MB_OK|MB_ICONERROR);
					return false;
				}
			else
				{
					msg.Format(_T("Database has been updated to version %s"), DB_VERSION_LONG_STR);
					::MessageBox(NULL, msg, _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
				}
		}
	}

	SetupProxy();

	return true;
}

CString CGreatNewsConfig::GetConfigFileName()
{
	return m_iniFile.m_szFilename;
}

CString CGreatNewsConfig::GetHomeDir()
{
	return m_homeDir;
}

CString CGreatNewsConfig::GetPluginDir()
{
	return m_appDir+_T("\\Plugins");
}

CString CGreatNewsConfig::GetMediaDir()
{
	return m_appDir+_T("\\Media");
}

CString CGreatNewsConfig::GetFaviconDir()
{
	return m_homeDir+_T("\\Favicons");
}

CString CGreatNewsConfig::CalculateAppDir()
{
    CString path;
    TCHAR buffAppPath[MAX_PATH] = _T("");

	GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();
	path = path.Left(path.ReverseFind('\\'));

    return path;
}

CString CGreatNewsConfig::CalculateHomeDir()
{
    CString path;
    TCHAR buffAppPath[MAX_PATH] = _T("");
    
    // PFAD aus APPDATA auslesen
    if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buffAppPath))) {
        path = buffAppPath;
        path += _T("\\GreatNews");
		::CreateDirectory(path, NULL);
    } else {
        // Fallback: Pfad der Executable ermitteln
        GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
        path.ReleaseBuffer();
        path = path.Left(path.ReverseFind('\\'));
    }
    return path;
}

CString CGreatNewsConfig::GetDefaultChannelOPML()
{
	return CalculateAppDir()+"\\channels.opml";
}

CString CGreatNewsConfig::GetLanguage()
{
	CString language;
	m_iniFile.GetString(_T("Environment"), _T("Language"),language,_T("gn_eng.ini"));
	return language;
}

CString CGreatNewsConfig::GetLanguageDir()
{
	return m_appDir+_T("\\Language");
}

void CGreatNewsConfig::SaveLanguage(LPCTSTR languagePackName)
{
	m_iniFile.PutString(_T("Environment"), _T("Language"), languagePackName);
}

CString CGreatNewsConfig::GetWindowPlacement()
{
	CString place;
	m_iniFile.GetString(_T("Environment"), _T("WinPlacement"),place,_T("0 1 (-1,-1) (-1,-1) (32,22,885,712)"));
	return place;
}

void CGreatNewsConfig::SetWindowPlacement(const CString& place)
{
	m_iniFile.PutString(_T("Environment"), _T("WinPlacement"), place);
}

bool CGreatNewsConfig::GetNewsListClosed()
{
	bool b;
	m_iniFile.GetBool(_T("Environment"), _T("NewsListClosed"), b, true);
	return b;
}

void CGreatNewsConfig::SetNewsListClosed(bool bClosed)
{
	m_iniFile.PutBool(_T("Environment"), _T("NewsListClosed"), bClosed);
}

bool CGreatNewsConfig::GetChannelTreeClosed()
{
	bool b;
	m_iniFile.GetBool(_T("Environment"), _T("ChannelTreeClosed"), b, false);
	return b;
}

void CGreatNewsConfig::SetChannelTreeClosed(bool bClosed)
{
	m_iniFile.PutBool(_T("Environment"), _T("ChannelTreeClosed"), bClosed);
}

int CGreatNewsConfig::GetNewsListPos()
{
	int nPos;
	m_iniFile.GetInt(_T("Environment"), _T("NewsListPos"), nPos, 150);
	return nPos;
}

void CGreatNewsConfig::SetNewsListPos(int pos)
{
	m_iniFile.PutInt(_T("Environment"), _T("NewsListPos"), pos);
}

int CGreatNewsConfig::GetChannelTreePos()
{
	int nPos;
	m_iniFile.GetInt(_T("Environment"), _T("ChannelTreePos"), nPos, 220);
	return nPos;
}
void CGreatNewsConfig::SetChannelTreePos(int pos)
{
	m_iniFile.PutInt(_T("Environment"), _T("ChannelTreePos"), pos);
}

int CGreatNewsConfig::GetDateFilterSelection()
{
	int n;
	m_iniFile.GetInt(_T("Environment"), _T("DateFilter"), n, 4);
	return n;
}

void CGreatNewsConfig::SetDateFilterSelection(int n)
{
	m_iniFile.PutInt(_T("Environment"), _T("DateFilter"), n);
}

int CGreatNewsConfig::GetStatusFilterSelection()
{
	int n;
	m_iniFile.GetInt(_T("Environment"), _T("StatusFilter"), n, 0);
	return n;
}

void CGreatNewsConfig::SetStatusFilterSelection(int n)
{
	m_iniFile.PutInt(_T("Environment"), _T("StatusFilter"), n);
}

bool CGreatNewsConfig::GetCloseToTray()
{
	bool b = false;
	m_iniFile.GetBool(_T("Options"), _T("CloseToTray"), b, false);
	return b;
}

void CGreatNewsConfig::SetCloseToTray(bool bToTray)
{
	m_iniFile.PutBool(_T("Options"), _T("CloseToTray"), bToTray);
}

bool CGreatNewsConfig::GetRemoveFromTaskBar()
{
	bool b;
	m_iniFile.GetBool(_T("Options"), _T("RemoveFromTaskBar"), b, false);
	return b;
}

void CGreatNewsConfig::SetRemoveFromTaskBar(bool bRemove)
{
	m_iniFile.PutBool(_T("Options"), _T("RemoveFromTaskBar"), bRemove);
}

bool CGreatNewsConfig::GetAutoUpdateAtStartup()
{
	bool b;
	m_iniFile.GetBool(_T("Options"), _T("AutoUpdate"), b, false);
	return b;
}

void CGreatNewsConfig::SetAutoUpdateAtStartup(bool bUpdate)
{
	m_iniFile.PutBool(_T("Options"), _T("AutoUpdate"), bUpdate);
}

int CGreatNewsConfig::GetAutoUpdateFreqSec() 
{
	return AutoUpdateSec[m_nAutoUpdateFreq];
}


int CGreatNewsConfig::GetCleanupReminderDays()
{
	return CleanupRemindDays[GetCleanupReminder()];
}

int CGreatNewsConfig::GetCleanupReminder()
{
	int n;
	m_iniFile.GetInt(_T("Options"), _T("CleanupReminder"), n, 2);
	n = max(0, min((sizeof(CleanupRemindDays)/sizeof(int))-1, n));

	return n;
}

void CGreatNewsConfig::SetCleanupReminder(INT_PTR n)
{
	m_iniFile.PutInt(_T("Options"), _T("CleanupReminder"), (int)n);
}

CString CGreatNewsConfig::GetCurrentStyle()
{
	CString v;
	m_iniFile.GetString(_T("Options"), _T("Style"), v, _T("Newspaper.css"));
	return v;
}

int CGreatNewsConfig::GetStartupScreen()
{
	return m_nStartupScreen;
}

void CGreatNewsConfig::SetStartupScreen(int n)
{
	m_nStartupScreen = n;
}

void CGreatNewsConfig::SetCurrentStyle(const CString& style)
{
	m_iniFile.PutString(_T("Options"), _T("Style"), style);
	m_currentStyleHTML.Empty();
}

void CGreatNewsConfig::ProcessGreatNewsTags(CString& str)
{
	// process %MEDIA% tags
	CString dir = g_GreatNewsConfig.GetMediaDir()+"/Media";
	dir.Replace(_T("\\"),_T("/"));
	str.Replace(_T("%MEDIA%"), dir);
}

LPCTSTR CGreatNewsConfig::GetCurrentStyleString()
{
	if(m_currentStyleHTML.GetLength()==0)
	{
		CString cssFileName = g_GreatNewsConfig.GetMediaDir()+"\\"+GetCurrentStyle();
		CGNUtil::ReadStringFromFile(cssFileName, m_currentStyleHTML);

		ProcessGreatNewsTags(m_currentStyleHTML);
	}

	return m_currentStyleHTML;
}

CString CGreatNewsConfig::GetStyleFileContent(const CString& styleFileName)
{
	CString content;
	CString cssFileName = g_GreatNewsConfig.GetMediaDir()+"\\"+styleFileName+".css";
	CGNUtil::ReadStringFromFile(cssFileName, content);

	ProcessGreatNewsTags(content);

	return content;
}


LPCTSTR CGreatNewsConfig::GetBuiltInStyle()
{
	if(m_builtInStyle.GetLength()==0)
	{
		CString cssFileName = g_GreatNewsConfig.GetMediaDir()+"\\builtin_style.tpl";
		CGNUtil::ReadStringFromFile(cssFileName, m_builtInStyle);
		ProcessGreatNewsTags(m_builtInStyle);
	}

	return m_builtInStyle;
}

void CGreatNewsConfig::LoadConfig()
{
	m_iniFile.GetBool(_T("Usability"), _T("AutoCloseNewsList"), m_bAutoCloseNewsList, false);
	m_iniFile.GetBool(_T("Usability"), _T("AutoSetFilterToUnread"), m_bAutoSetFilterToUnread, false);
	m_iniFile.GetBool(_T("Usability"), _T("AutoMarkLastChannelAsRead"), m_bOpenNextAutoMarkLastChannelAsRead, true);
	m_iniFile.GetBool(_T("Usability"), _T("SearchAutoOpenNewsList"), m_bSearchAutoOpenNewsList, true);
	m_iniFile.GetBool(_T("Usability"), _T("ShowTooltipAfterUpdate"), m_bShowTooltipAfterUpdate, false);
	m_iniFile.GetInt(_T("Usability"), _T("BlockPopups"), m_nBlockPopups, 1);
	m_nBlockPopups = min(1, max(0,m_nBlockPopups));
	m_iniFile.GetInt(_T("Usability"), _T("MiddleClickToNewWindow"), m_nMiddleClickToNewWindow, 0);
	m_iniFile.GetInt(_T("Usability"), _T("OpenRssLinkInDefaultBrowser"), m_nOpenRssLinkInDefaultBrowser, 0);
	m_iniFile.GetBool(_T("Usability"), _T("DisableTabbedBrowsing"), m_bDisableTabbedBrowsing, false);
	m_iniFile.GetBool(_T("Usability"), _T("NoAutoGroupCollapsing"), m_bNoAutoGroupCollapse, false);
	m_iniFile.GetBool(_T("Usability"), _T("MarkChangesUnread"), m_bMarkChangesUnread, false);

	m_iniFile.GetBool(_T("Options"), _T("MarkChannelReadWhenSwitching"), m_bMarkChannelReadWhenSwitching, true);
	m_iniFile.GetBool(_T("Options"), _T("AutoUpdateChannels"), m_bAutoUpdateChannels, true);
	//m_iniFile.GetInt(_T("Options"), _T("AutoUpdateFreq"), m_nAutoUpdateFreq, 1);
	int _i;
	m_iniFile.GetInt(_T("Options"), _T("AutoUpdateFreq"), _i, 1);
	m_nAutoUpdateFreq = _i;

	m_iniFile.GetInt(_T("Options"), _T("StartupScreen"), m_nStartupScreen, 1);
	m_nStartupScreen = max(0, min(4, m_nStartupScreen));
	m_iniFile.GetInt(_T("Options"), _T("StartupLabel"), m_nStartupLabel, -1);
	m_iniFile.GetInt(_T("Options"), _T("StartupWatch"), m_nStartupWatch, -1);
	m_iniFile.GetBool(_T("Options"), _T("CleanTempFilesUponExit"), m_bCleanTempFilesUponExit, true);

	m_iniFile.GetBool(_T("Options"), _T("UpdateChannelWhenClick"), m_bUpdateWhenClick, true);
	m_iniFile.GetInt(_T("Options"), _T("PageSize"), m_nPageSize, 10);
	m_nPageSize = min(999, max(1,m_nPageSize));

	m_iniFile.GetInt(_T("Options"), _T("NextPageOpenUnread"), m_nNextPageOpenUnread, 1);
	m_nNextPageOpenUnread = min(1, max(0,m_nNextPageOpenUnread));

	m_iniFile.GetInt(_T("Options"), _T("MarkReadAutomatically"), m_nMarkReadAutomatically, 0);
	m_nMarkReadAutomatically = min(1, max(0,m_nMarkReadAutomatically));

	m_iniFile.GetInt(_T("Options"), _T("SecondsToMarkItemRead"), m_nSecondsToMarkItemRead, 2);
	m_nSecondsToMarkItemRead = min(999, max(0,m_nSecondsToMarkItemRead));

	m_iniFile.GetInt(_T("Options"), _T("SelectedItemFeatures"), m_nSelectedItemFeatures, 
			//CNewsItem::LabelThis|CNewsItem::Arthur|CNewsItem::Delicious);
			CNewsItem::LabelThis|CNewsItem::Delicious|CNewsItem::TrackComments);
	m_iniFile.GetBool(_T("Options"), _T("HighlightNewsWatch"), m_bHighlightNewsWatch, true);
	m_iniFile.GetString(_T("Options"), _T("PodcastingEnclosuresTypes"), m_podcastingEnclosuresTypes, 
			_T(".mp3;.mp4;.aac;.ogg;.wmv;.wma;.avi;.wav;.mpeg;.mpg;.mov;.pdf;.flac;.m4a;.mpc;.ape;.asf;.webm;.flv;.m4v;.ogm;.dvi;.qt;"));
	m_iniFile.GetBool(_T("Options"), _T("NotifyDinosaurChannels"), m_bNotifyDinosaurChannels, true);
	m_iniFile.GetInt(_T("Options"), _T("DinosaurDays"), m_nDinosaurDays, 60);
	m_iniFile.GetInt(_T("Options"), _T("NewItemPopupSeconds"), m_nNewItemNotifySeconds, 60);
	m_iniFile.GetInt(_T("Options"), _T("DoubleClickingNewsList"), m_nDoubleClickingNewsList, 0);
	m_iniFile.GetBool(_T("Options"), _T("UseOriginalLink"), m_bKeepOriginalLink, false);
	CNewsFeedParser::m_bKeepOriginalLink = m_bKeepOriginalLink;
	m_iniFile.GetBool(_T("Options"), _T("WorkOffline"), m_bWorkOffline, false);
	m_iniFile.GetInt(_T("Options"), _T("ReadingPanePosition"), m_nReadingPanePos, 0);

	// connection related options
	m_iniFile.GetInt(_T("Options"), _T("ProxyToUse"), m_nProxyToUse, 0);
	m_iniFile.GetString(_T("Options"), _T("ProxyServerUrl"), m_proxyURL);
	m_iniFile.GetInt(_T("Options"), _T("ProxyPort"), m_proxyPort, 80);
	m_iniFile.GetString(_T("Options"), _T("ProxyServerUser"), m_proxyUserName);
	m_iniFile.GetString(_T("Options"), _T("ProxyServerPassword"), m_proxyPassword);
	m_iniFile.GetString(_T("Options"), _T("ProxyServerBypass"), m_proxyBypass, _T("<local>"));

	// statistics related options
	m_iniFile.GetInt(_T("Statistics"), _T("NumOfChannels"), m_nStatsNumOfChannels,10);
	m_iniFile.GetBool(_T("Statistics"), _T("ExcludeDisabledChannels"), m_bStatsExcludeDisabled, false);

	// advanced options
	m_iniFile.GetBool(_T("Advanced"), _T("BypassTTL"), m_bBypassTTLChecking, false);
	m_iniFile.GetBool(_T("Advanced"), _T("NoChannelGroupView"), m_bNoChannelGroupView, false);
	m_iniFile.GetBool(_T("Advanced"), _T("UseMozillaEngine"), m_bUseMozillaEngine, false);
	m_iniFile.GetInt(_T("Advanced"), _T("MouseWheelSensitivity"), m_nMouseWheelSensitivity, 5);
	m_iniFile.GetBool(_T("Advanced"), _T("NoFavicons"), m_bNoFavicons, false);
	m_iniFile.GetString(_T("Advanced"), _T("ExternalBrowser"), CGNUtil::m_browserPath, _T(""));
	m_iniFile.GetInt(_T("Advanced"), _T("MaxFeedSizeInK"), m_maxFeedSizeInK, 1000);
	m_iniFile.GetString(_T("Advanced"), _T("TempDirectory"), m_tempDirectory, _T(""));
	m_iniFile.GetBool(_T("Advanced"), _T("LegacyBrowser"), m_bLegacyBrowser, false);
	m_iniFile.GetBool(_T("Advanced"), _T("InternetZone"), m_bInternetZone, true);
	m_iniFile.GetBool(_T("Advanced"), _T("MultipleInstance"), m_bMultipleInstance, false);
	m_iniFile.GetBool(_T("Advanced"), _T("ShowTimeIn24Hr"), m_bShowTimeIn24Hr, false);
	CGMTimeHelper::m_bShowTimeIn24Hr = m_bShowTimeIn24Hr;
}


void CGreatNewsConfig::SaveConfig()
{
	m_iniFile.PutBool(_T("Usability"), _T("AutoCloseNewsList"), m_bAutoCloseNewsList);
	m_iniFile.PutBool(_T("Usability"), _T("AutoSetFilterToUnread"), m_bAutoSetFilterToUnread);
	m_iniFile.PutBool(_T("Usability"), _T("AutoMarkLastChannelAsRead"), m_bOpenNextAutoMarkLastChannelAsRead);
	m_iniFile.PutBool(_T("Usability"), _T("SearchAutoOpenNewsList"), m_bSearchAutoOpenNewsList);
	m_iniFile.PutBool(_T("Usability"), _T("ShowTooltipAfterUpdate"), m_bShowTooltipAfterUpdate);
	m_iniFile.PutInt(_T("Usability"), _T("MiddleClickToNewWindow"), m_nMiddleClickToNewWindow);
	m_iniFile.PutInt(_T("Usability"), _T("BlockPopups"), m_nBlockPopups);
	m_iniFile.PutInt(_T("Usability"), _T("OpenRssLinkInDefaultBrowser"), m_nOpenRssLinkInDefaultBrowser);
	m_iniFile.PutBool(_T("Usability"), _T("DisableTabbedBrowsing"), m_bDisableTabbedBrowsing);
	m_iniFile.PutBool(_T("Usability"), _T("NoAutoGroupCollapsing"), m_bNoAutoGroupCollapse);
	m_iniFile.PutBool(_T("Usability"), _T("MarkChangesUnread"), m_bMarkChangesUnread);

	m_iniFile.PutBool(_T("Options"), _T("MarkChannelReadWhenSwitching"), m_bMarkChannelReadWhenSwitching);
	m_iniFile.PutBool(_T("Options"), _T("UpdateChannelWhenClick"), m_bUpdateWhenClick);
	m_iniFile.PutInt(_T("Options"), _T("SecondsToMarkItemRead"), m_nSecondsToMarkItemRead);
	m_iniFile.PutInt(_T("Options"), _T("PageSize"), m_nPageSize);
	m_iniFile.PutInt(_T("Options"), _T("NextPageOpenUnread"), m_nNextPageOpenUnread);
	m_iniFile.PutInt(_T("Options"), _T("MarkReadAutomatically"), m_nMarkReadAutomatically);
	m_iniFile.PutInt(_T("Options"), _T("SelectedItemFeatures"), m_nSelectedItemFeatures);
	m_iniFile.PutBool(_T("Options"), _T("HighlightNewsWatch"), m_bHighlightNewsWatch);
	m_iniFile.PutString(_T("Options"), _T("PodcastingEnclosuresTypes"), m_podcastingEnclosuresTypes); 

	m_iniFile.PutBool(_T("Options"), _T("NotifyDinosaurChannels"), m_bNotifyDinosaurChannels);
	m_iniFile.PutInt(_T("Options"), _T("DinosaurDays"), m_nDinosaurDays);
	m_iniFile.PutInt(_T("Options"), _T("NewItemPopupSeconds"), m_nNewItemNotifySeconds);
	m_iniFile.PutInt(_T("Options"), _T("DoubleClickingNewsList"), m_nDoubleClickingNewsList);
	m_iniFile.PutBool(_T("Options"), _T("UseOriginalLink"), m_bKeepOriginalLink);
	m_iniFile.PutBool(_T("Options"), _T("WorkOffline"), m_bWorkOffline);
	m_iniFile.PutInt(_T("Options"), _T("ReadingPanePosition"), m_nReadingPanePos);

	m_iniFile.PutBool(_T("Options"), _T("AutoUpdateChannels"), m_bAutoUpdateChannels);
	m_iniFile.PutInt(_T("Options"), _T("AutoUpdateFreq"), (int)m_nAutoUpdateFreq);
	m_iniFile.PutInt(_T("Options"), _T("StartupScreen"), m_nStartupScreen);
	m_iniFile.PutInt(_T("Options"), _T("StartupLabel"), m_nStartupLabel);
	m_iniFile.PutInt(_T("Options"), _T("StartupWatch"), m_nStartupWatch);
	m_iniFile.PutBool(_T("Options"), _T("CleanTempFilesUponExit"), m_bCleanTempFilesUponExit);

	// connection related options
	m_iniFile.PutInt(_T("Options"), _T("ProxyToUse"), m_nProxyToUse);
	m_iniFile.PutString(_T("Options"), _T("ProxyServerUrl"), m_proxyURL);
	m_iniFile.PutInt(_T("Options"), _T("ProxyPort"), m_proxyPort);
	m_iniFile.PutString(_T("Options"), _T("ProxyServerUser"), m_proxyUserName);
	m_iniFile.PutString(_T("Options"), _T("ProxyServerPassword"), m_proxyPassword);
	m_iniFile.PutString(_T("Options"), _T("ProxyServerBypass"), m_proxyBypass);

	// statistics related options
	m_iniFile.PutInt(_T("Statistics"), _T("NumOfChannels"), m_nStatsNumOfChannels);
	m_iniFile.PutBool(_T("Statistics"), _T("ExcludeDisabledChannels"), m_bStatsExcludeDisabled);
}


// advanced options
int CGreatNewsConfig::GetChannelUpdateTimeOut()
{
	int n = 180;
	m_iniFile.GetInt(_T("Advanced"), _T("ChannelUpdateTimeout"), n, 180);
	n = min(3600, max(10,n));
	return n;
}

int CGreatNewsConfig::GetNumOfConcurrentUpdate()
{
	int n = 10;
	m_iniFile.GetInt(_T("Advanced"), _T("NumOfConcurrentUpdate"), n, 10);
	n = min(30, max(1,n));

	return n;
}


CTime CGreatNewsConfig::GetCleanupLastRun()
{
	CString time;
	CppSQLite3DB db;
	CppSQLite3Query q;

	CFeedManager::OpenDatabase(db);

	q = db.execQuery(_T("select value from configuration where key = 'cleanup_last_run'"));
	if(!q.eof())
	{
		time = q.getStringField(0);
	}
	q.finalize();

	if(time.GetLength() == 0)
	{
		SetCleanupLastRunNow();
		return CGMTimeHelper::GetCurrentSysTime();
	}
	else
	{
		return CGMTimeHelper::StringToCTime(time);
	}
}

void CGreatNewsConfig::SetCleanupLastRunNow()
{
	CppSQLite3DB db;
	CString time, sql;

	CFeedManager::OpenDatabase(db);
	time = CGMTimeHelper::FormatSqlDate(CGMTimeHelper::GetCurrentSysTime());
	sql.Format(_T("update configuration set value = '%s' where key = 'cleanup_last_run'"), (LPCTSTR)time);
	db.execDML(sql);
	db.close();
}


int CGreatNewsConfig::GetColWidthTitle()
{
	int n;
	m_iniFile.GetInt(_T("Environment"), _T("ColWidthTitle"), n, 400);
	return min(800,max(n,1));
}

void CGreatNewsConfig::SetColWidthTitle(int n)
{
	m_iniFile.PutInt(_T("Environment"), _T("ColWidthTitle"), n);
}

int CGreatNewsConfig::GetColWidthDate()
{
	int n;
	m_iniFile.GetInt(_T("Environment"), _T("ColWidthDate"), n, 130);
	return min(200,max(n,1));
}

void CGreatNewsConfig::SetColWidthDate(int n)
{
	m_iniFile.PutInt(_T("Environment"), _T("ColWidthDate"), n);
}

int CGreatNewsConfig::GetColWidthAuthor()
{
	int n;
	m_iniFile.GetInt(_T("Environment"), _T("ColWidthAuthor"), n, 150);
	return min(400,max(n,1));
}

void CGreatNewsConfig::SetColWidthAuthor(int n)
{
	m_iniFile.PutInt(_T("Environment"), _T("ColWidthAuthor"), n);
}

int CGreatNewsConfig::GetColWidthChannel()
{
	int n;
	m_iniFile.GetInt(_T("Environment"), _T("ColWidthChannel"), n, 120);
	return min(400,max(n,1));
}

void CGreatNewsConfig::SetColWidthChannel(int n)
{
	m_iniFile.PutInt(_T("Environment"), _T("ColWidthChannel"), n);
}

bool CGreatNewsConfig::GetCleanupKeepItems()
{
	bool b;
	m_iniFile.GetBool(_T("Options"), _T("CleanupKeepItems"), b, true);
	return b;
}

void CGreatNewsConfig::SetCleanupKeepItems(bool bKeep)
{
	m_iniFile.PutInt(_T("Options"), _T("CleanupKeepItems"), bKeep);
}

int CGreatNewsConfig::GetCleanupNumOfItemsToKeep()
{
	int n = 200;
	m_iniFile.GetInt(_T("Options"), _T("CleanupNumOfItemsToKeep"), n, 200);
	return n;
}

void CGreatNewsConfig::SetCleanupNumOfItemsToKeep(int num)
{
	m_iniFile.PutInt(_T("Options"), _T("CleanupNumOfItemsToKeep"), num);
}

int CGreatNewsConfig::GetCleanupDefault()
{
	int n = 1;
	m_iniFile.GetInt(_T("Options"), _T("CleanupDefault"), n, 1);
	return n;
}
void CGreatNewsConfig::SetCleanupDefault(INT_PTR n)
{
	m_iniFile.PutInt(_T("Options"), _T("CleanupDefault"), (int)n);
}

bool CGreatNewsConfig::GetCleanupDeleteUnread()
{
	bool b = false;
	m_iniFile.GetBool(_T("Options"), _T("CleanupDeleteUnread"), b, false);
	return b;
}

void CGreatNewsConfig::SetCleanupDeleteUnread(bool b)
{
	m_iniFile.PutBool(_T("Options"), _T("CleanupDeleteUnread"), b);
}

bool CGreatNewsConfig::GetCleanupDeleteTemp()
{
	bool b = false;
	m_iniFile.GetBool(_T("Options"), _T("CleanupDeleteTempFiles"), b, false);
	return b;
}

void CGreatNewsConfig::SetCleanupDeleteTemp(bool b)
{
	m_iniFile.PutBool(_T("Options"), _T("CleanupDeleteTempFiles"), b);
}

bool CGreatNewsConfig::GetCleanupDeleteLabeled()
{
	bool b = false;
	m_iniFile.GetBool(_T("Options"), _T("CleanupDeleteLabeled"), b, false);
	return b;
}
void CGreatNewsConfig::SetCleanupDeleteLabeled(bool bLabeled)
{
	m_iniFile.PutBool(_T("Options"), _T("CleanupDeleteLabeled"), bLabeled);
}

bool CGreatNewsConfig::GetCleanupCompress()
{
	bool b = false;
	m_iniFile.GetBool(_T("Options"), _T("CleanupCompress"), b, false);
	return b;
}

void CGreatNewsConfig::SetCleanupCompress(bool bCompress)
{
	m_iniFile.PutBool(_T("Options"), _T("CleanupCompress"), bCompress);
}



void CGreatNewsConfig::RegisterFeedProtocolHandler()
{
	CComPtr<IRegistrar> spRegistrar;
	spRegistrar.CoCreateInstance(CLSID_Registrar);
	ATLASSERT(spRegistrar != NULL);
	TCHAR szModule[_MAX_PATH];
	GetModuleFileName(NULL, szModule, _MAX_PATH);
	spRegistrar->AddReplacement(_T("MODULE"), szModule);

	HRESULT hr = spRegistrar->ResourceRegister(szModule,IDR_MAINREGISTRY,OLESTR("REGISTRY"));

	return;
}

void CGreatNewsConfig::LoadSearchChannelDefinition()
{
	std::vector<CString> keys;
	m_iniFile.EnumSections(keys, _T("SearchChannelDefinition"));

	// if no search channel definition was found, we populate it with default ones
	if(keys.empty())
	{
		m_iniFile.PutString(_T("SearchChannelDefinition"), 
							_T("Yahoo! - Company News via RSS"),
							_T("http://finance.yahoo.com/rss/headline?s=%KEYWORD%"));
		m_iniFile.PutString(_T("SearchChannelDefinition"), 
							_T("Feedster"),
							_T("http://www.feedster.com/search.php?q=%KEYWORD%&sort=date&content=full&type=rss&limit=20"));
		m_iniFile.PutString(_T("SearchChannelDefinition"), 
							_T("MSN"), 
							_T("http://search.msn.com/results.aspx?q=%KEYWORD%&format=rss"));
		m_iniFile.PutString(_T("SearchChannelDefinition"), 
							_T("Flickr"),
							_T("http://www.flickr.com/services/feeds/photos_public.gne?tags=%KEYWORD%&format=rss_200"));
		m_iniFile.PutString(_T("SearchChannelDefinition"), 
							_T("Google Blog Search"),
							_T("http://blogsearch.google.com/blogsearch_feeds?q=%KEYWORD%&btnG=Search+Blogs&num=50&output=rss"));
		m_iniFile.PutString(_T("SearchChannelDefinition"), 
							_T("Google Blog Search By Date"),
							_T("http://blogsearch.google.com/blogsearch_feeds?q=%KEYWORD%&btnG=Search+Blogs&num=50&output=rss&scoring=d"));
		m_iniFile.PutString(_T("SearchChannelDefinition"), 
							_T("Bing"),
							_T("http://www.bing.com/search?q=%KEYWORD%&format=rss"));
		m_iniFile.Flush();

		m_iniFile.EnumSections(keys, _T("SearchChannelDefinition"));
	}

	SearchChannelDefVector& defs = CSearchChannelDef::m_searchChannelDefs;
	for(std::vector<CString>::iterator it = keys.begin(); it != keys.end(); ++it)
	{
		CString value;
		m_iniFile.GetString(_T("SearchChannelDefinition"), *it, value);
		defs.push_back(	CSearchChannelDef(*it, value) );
	}
}

bool CGreatNewsConfig::InitializeNewInstallation(const CString& homeDir, const CString& dbFileName)
{
	if(CFileUtil::FileExist(homeDir+_T("\\newsfeed.db")))
		return true;

	try
	{
		CppSQLite3DB db;
		db.open(dbFileName);
		db.execDML(_T("BEGIN EXCLUSIVE TRANSACTION;"));

		// tables
		db.execDML(_T("CREATE TABLE feed_group (feed_group_id INTEGER PRIMARY KEY,\n")
					_T("	name VARCHAR(255) NOT NULL UNIQUE,\n")
					_T("	description VARCHAR(255),\n")
					_T("	parent_group INTEGER,\n")
					_T("	root_group INTEGER,\n")
					_T("	group_type INTEGER NOT NULL DEFAULT '1',\n")
					_T("	expanded INTEGER,\n")
					_T("	options1 INTEGER,\n")
					_T("	options2 INTEGER,\n")
					_T("	options3 VARCHAR(254),\n")
					_T("	unread_count INTEGER);"));
		db.execDML(_T("CREATE TABLE news_item (\n")
					_T("	news_id INTEGER PRIMARY KEY,\n")
					_T("	feed_id INTEGER NOT NULL,\n")
					_T("	url VARCHAR(255) NOT NULL,\n")
					_T("	title VARCHAR(255),\n")
					_T("	description BLOB,\n")
					_T("	author VARCHAR(50),\n")
					_T("	published DATETIME,\n")
					_T("	retrieved DATETIME NOT NULL,\n")
					_T("	readtime DATETIME,\n")
					_T("	mark INTEGER,\n")
					_T("	item_guid VARCHAR(230),\n")
					_T("	watched INTEGER,\n")
					_T("	options1 INTEGER,\n")
					_T("	options2 INTEGER,\n")
					_T("	options3 VARCHAR(254),\n")
					_T("	pod_url VARCHAR,\n")
					_T("	commentCnt INTEGER,\n")
					_T("	commentRss VARCHAR(254),\n")
					_T("	commentUrl VARCHAR(254),\n")
					_T("	commentTrack INTEGER,\n")
					_T("	contentCode INTEGER,\n")
					_T("	content_code INTEGER);"));
		db.execDML(_T("CREATE TABLE news_watch (\n")
					_T("	watch_id INTEGER PRIMARY KEY,\n")
					_T("	name VARCHAR(100) NOT NULL UNIQUE,\n")
					_T("	criteria VARCHAR(255) NOT NULL,\n")
					_T("	selective_feed INTEGER NOT NULL DEFAULT '1',\n")
					_T("	match_case INTEGER NOT NULL DEFAULT '0',\n")
					_T("	whole_word INTEGER NOT NULL DEFAULT '1',\n")
					_T("	all_words INTEGER NOT NULL DEFAULT '1',\n")
					_T("	watch_flag INTEGER NOT NULL DEFAULT '0',\n")
					_T("	disabled INTEGER,\n")
					_T("	front_color INTEGER,\n")
					_T("	bk_color INTEGER,\n")
					_T("	display_order INTEGER NOT NULL,\n")
					_T("	options1 INTEGER,\n")
					_T("	options2 INTEGER,\n")
					_T("	options3 VARCHAR(254),\n")
					_T("	unread_count INTEGER);"));
		db.execDML(_T("CREATE TABLE news_watch_channel (\n")
					_T("	watch_id INTEGER NOT NULL,\n")
					_T("	feed_id INTEGER NOT NULL,\n")
					_T("	options1 INTEGER,\n")
					_T("	options2 INTEGER);"));
		db.execDML(_T("CREATE TABLE watch_item (\n")
					_T("	watch_id INTEGER NOT NULL,\n")
					_T("	news_id INTEGER NOT NULL,\n")
					_T("	options1 INTEGER,\n")
					_T("	options2 INTEGER);"));
		db.execDML(_T("CREATE TABLE tag_info (\n")
					_T("	tag_id INTEGER PRIMARY KEY,\n")
					_T("	tag_name VARCHAR(100) NOT NULL UNIQUE,\n")
					_T("	options1 INTEGER,\n")
					_T("	options2 INTEGER,\n")
					_T("	options3 VARCHAR(254),\n")
					_T("	unread_count INTEGER);"));
		db.execDML(_T("CREATE TABLE item_tag (\n")
					_T("	tag_id INTEGER NOT NULL,\n")
					_T("	news_id INTEGER NOT NULL,\n")
					_T("	options1 INTEGER,\n")
					_T("	options2 INTEGER);"));
		db.execDML(_T("CREATE TABLE configuration (\n")
					_T("	key VARCHAR(50) NOT NULL UNIQUE,\n")
					_T("	value VARCHAR(250));"));
		db.execDML(_T("CREATE TABLE blog_tool_type (\n")
					_T("	tool_type_id INTEGER PRIMARY KEY,\n")
					_T("	tool_type_name VARCHAR(100) NOT NULL UNIQUE,\n")
					_T("	default_url VARCHAR(255) NOT NULL);"));
		db.execDML(_T("CREATE TABLE blog_tool (\n")
					_T("	tool_id INTEGER PRIMARY KEY,\n")
					_T("	tool_name VARCHAR(100) NOT NULL UNIQUE,\n")
					_T("	tool_type VARCHAR(50) NOT NULL,\n")
					_T("	tool_url VARCHAR(255) NOT NULL);"));
		db.execDML(_T("CREATE TABLE feed (\n")
					_T("	feed_id INTEGER PRIMARY KEY AUTOINCREMENT,\n")
					_T("	feed_group_id INTEGER NOT NULL,\n")
					_T("	url VARCHAR(250) NOT NULL,\n")
					_T("	name VARCHAR(50) NOT NULL UNIQUE,\n")
					_T("	description VARCHAR(250),\n")
					_T("	image VARCHAR(250),\n")
					_T("	website VARCHAR(250),\n")
					_T("	last_updated DATETIME,\n")
					_T("	language VARCHAR(20),\n")
					_T("	usage INTEGER,\n")
					_T("	disabled INTEGER,\n")
					_T("	options1 INTEGER,\n")
					_T("	options2 INTEGER,\n")
					_T("	options3 VARCHAR(254),\n")
					_T("	unread_count INTEGER,\n")
					_T("	hd_last_mod VARCHAR,\n")
					_T("	hd_etag VARCHAR,\n")
					_T("	update_freq INTEGER,\n")
					_T("	store_dur INTEGER,\n")
					_T("	notify_newitem INTEGER,\n")
					_T("	limit_item INTEGER,\n")
					_T("	limit_item_number INTEGER,\n")
					_T("	keep_inactive INTEGER,\n")
					_T("	contentCode INTEGER,\n")
					_T("	parent_feed_id INTEGER,\n")
					_T("	news_id INTEGER,\n")
					_T("	use_login INTEGER,\n")
					_T("	login_name VARCHAR,\n")
					_T("	login_pwd VARCHAR,\n")
					_T("	priority INTEGER);"));

		// indices
		db.execDML(_T("CREATE INDEX idxGuid ON news_item (item_guid ASC);"));
		db.execDML(_T("CREATE INDEX idxNewsID ON watch_item (news_id);"));
		db.execDML(_T("CREATE INDEX idxWatchID ON watch_item (watch_id);"));
		db.execDML(_T("CREATE INDEX idxTagId on item_tag (tag_id);"));
		db.execDML(_T("CREATE INDEX idxItemId on item_tag (news_id);"));
		db.execDML(_T("CREATE INDEX idxFeedItem ON news_item (feed_id, retrieved, readtime);"));

		// triggers
		db.execDML(_T("CREATE TRIGGER OnDelGroup AFTER DELETE on feed_group\n")
					_T("BEGIN\n")
					_T("	DELETE FROM feed WHERE feed.feed_group_id = OLD.feed_group_id;\n")
					_T("END;"));
		db.execDML(_T("CREATE TRIGGER OnDelTagINfo AFTER DELETE on tag_INfo\n")
					_T("BEGIN\n")
					_T("	DELETE FROM item_tag WHERE tag_id = OLD.tag_id;\n")
					_T("END;"));
		db.execDML(_T("CREATE TRIGGER tgrDELETENews AFTER DELETE ON news_item\n")
					_T("BEGIN\n")
					_T("	DELETE FROM watch_item WHERE news_id = OLD.news_id;\n")
					_T("	DELETE FROM item_tag WHERE news_id = OLD.news_id;\n")
					_T("END;"));
		db.execDML(_T("CREATE TRIGGER tgrDELETEWatch AFTER DELETE on news_watch\n")
					_T("BEGIN\n")
					_T("	DELETE FROM news_watch_channel WHERE watch_id = OLD.watch_id;\n")
					_T("	DELETE FROM watch_item WHERE watch_id = OLD.watch_id;\n")
					_T("END;"));
		db.execDML(_T("CREATE TRIGGER tgrDELETEWatchItem AFTER DELETE on watch_item\n")
					_T("BEGIN\n")
					_T("	UPDATE news_item SET watched = watched - 1 WHERE news_id = OLD.news_id;\n")
					_T("END;"));
		db.execDML(_T("CREATE TRIGGER tgrDELETETagItem AFTER DELETE on item_tag\n")
					_T("BEGIN\n")
					_T("	UPDATE news_item SET mark = mark - 1 WHERE news_id = OLD.news_id;\n")
					_T("END;"));
		db.execDML(_T("CREATE TRIGGER tgrDELETENews2 AFTER DELETE ON news_item FOR EACH ROW WHEN OLD.commentTrack > 0\n")
					_T("BEGIN\n")
					_T("	DELETE FROM news_item WHERE news_item.feed_id IN (SELECT feed_id FROM feed WHERE feed.news_id = OLD.news_id);\n")
					_T("	DELETE FROM feed WHERE feed.news_id = OLD.news_id;\n")
					_T("END;"));
		db.execDML(_T("CREATE TRIGGER OnDelFeed AFTER DELETE ON feed\n")
					_T("BEGIN\n")
					_T("	DELETE FROM news_item WHERE news_item.feed_id = OLD.feed_id;\n")
					_T("	DELETE FROM news_watch_channel WHERE feed_id = OLD.feed_id;\n")
					_T("END;"));
		db.execDML(_T("CREATE TRIGGER OnDelFeed2 AFTER DELETE ON feed FOR EACH ROW WHEN OLD.news_id > 0\n")
					_T("BEGIN\n")
					_T("	UPDATE news_item SET commentTrack = NULL WHERE news_item.news_id = OLD.news_id;\n")
					_T("END;"));

		// data
		db.execDML(_T("INSERT INTO feed_group VALUES(1,'All News Feeds',NULL,NULL,NULL,1,NULL,NULL,NULL,NULL,0);"));
		db.execDML(_T("INSERT INTO feed_group VALUES(2,'Hardware & Gadget','',1,NULL,1,NULL,NULL,0,NULL,0);"));
		db.execDML(_T("INSERT INTO feed_group VALUES(3,'Top Stories','',1,NULL,1,NULL,NULL,0,NULL,0);"));
		db.execDML(_T("INSERT INTO feed_group VALUES(4,'Wired News','',1,NULL,1,NULL,NULL,0,NULL,0);"));
		db.execDML(_T("INSERT INTO feed_group VALUES(5,'Blogs','',1,NULL,1,NULL,0,1,'',0);"));
		//db.execDML(_T("INSERT INTO feed_group VALUES(6,'Chinese Bloggers','',1,NULL,1,NULL,0,1,'',0);"));
		db.execDML(_T("INSERT INTO feed_group VALUES(7,'Living','',1,NULL,1,NULL,0,0,'',NULL);"));
		db.execDML(_T("INSERT INTO news_watch VALUES(1,'Dell HP','Dell HP',1,1,0,0,1,NULL,0,8454143,0,NULL,NULL,NULL,0);"));
		db.execDML(_T("INSERT INTO news_watch VALUES(2,'eBook','eBook',0,0,0,0,1,NULL,0,14804223,1,NULL,NULL,NULL,0);"));
		db.execDML(_T("INSERT INTO news_watch_channel VALUES(1,3,NULL,NULL);"));
		db.execDML(_T("INSERT INTO tag_info VALUES(1,'Friend',NULL,NULL,NULL,0);"));
		db.execDML(_T("INSERT INTO tag_info VALUES(2,'Job',NULL,NULL,NULL,0);"));
		db.execDML(_T("INSERT INTO configuration VALUES('db_version',") DB_VERSION_SHORT_STR _T(");"));
		db.execDML(_T("INSERT INTO configuration VALUES('last_start',1157296650);"));
		db.execDML(_T("INSERT INTO configuration VALUES('last_shutdown',1157296701);"));
		db.execDML(_T("INSERT INTO blog_tool_type VALUES(1,'<Generic>','http://<yoursite.com>/post?title=%TITLE%&url=%URL%&text=%TEXT%');"));
		db.execDML(_T("INSERT INTO blog_tool_type VALUES(2,'WordPress','http://<yoursite.com>/blog/wp-admin/bookmarklet.php?popupurl=%LINK%&popuptitle=%TITLE%&text=%TEXT%');"));
		db.execDML(_T("INSERT INTO blog_tool_type VALUES(3,'TypePad','https://www.typepad.com/t/app/weblog/post?is_qp=1&qp_show=tb,ca,ac,ap,cb,ex,tm,kw&__mode=edit_entry&qp_title=%TITLE%&qp_href=%LINK%&qp_text=%TEXT%');"));
		db.execDML(_T("INSERT INTO blog_tool_type VALUES(4,'MovableType','http://<yoursite.com>/cgi-bin/mt.cgi?is_bm=1&bm_show=&__mode=view&_type=entry&title=%TITLE%&link_title=%TITLE%&link_href=%LINK%&text=%TEXT%');"));
		db.execDML(_T("INSERT INTO blog_tool_type VALUES(5,'Blogger','http://www.blogger.com/blog-this.g?&n=%TITLE%&t=&u=%LINK%');"));
		db.execDML(_T("INSERT INTO blog_tool_type VALUES(6,'Drupal','http://<yoursite.com>/drupal/node/add/blog?edit[title]=%TITLE%&edit[body]=%TEXT%');"));
		db.execDML(_T("INSERT INTO feed VALUES(1,1,'http://www.curiostudio.com/ro-rss.xml','GreatNews Change Logs','The Journal of GreatNews','','http://www.curiostudio.com/blog',NULL,'en',20,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,1,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(2,2,'http://www.anandtech.com/rss/articlefeed.aspx','AnandTech Article Channel','This channel features the latest computer hardware related articles.','','http://www.anandtech.com',NULL,'en-us',15,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(3,2,'http://www.brighthand.com/rss.xml','Brighthand','Brighthand.com is a website dedicated to bringing the latest news, reviews and pricing for all types of handheld devices','','http://www.brighthand.com',NULL,'',25,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(4,2,'http://www.engadget.com/rss.xml','Engadget','Engadget','http://www.engadget.com/media/feedlogo.gif','http://www.engadget.com',NULL,'en-us',9,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(5,2,'http://www.gizmodo.net/index.xml','Gizmodo','Gizmodo, the gadget guide. So much in love with shiny new toys, it''s unnatural.','','http://gizmodo.com/',NULL,'',19,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(7,2,'http://www.pocketpcthoughts.com/xml','Pocket PC Thoughts','Daily News, Views, Rants and Raves','http://www.pocketpcthoughts.com/assets/Aurora/xml/rss_logo.gif','http://www.pocketpcthoughts.com/',NULL,'en-us',17,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(8,2,'http://www.tomshardware.com/articles.xml','Tom''s Hardware Guide: Articles','Tom''s Hardware','http://images.tomshardware.com/images/logos/tomshardware.gif','http://www.tomshardware.com/',NULL,'en-us',19,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(10,2,'http://www.infosyncworld.com/feed/rss/index.php','infoSync World','Reporting from the digital frontier.','http://www.infosyncworld.net/gfx/button/button_isw_150_70_00.gif','http://www.infosyncworld.com/',NULL,'en-us',13,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(11,3,'http://rss.cnn.com/rss/cnn_topstories.rss','CNN.com','CNN.com delivers up-to-the-minute news and information on the latest top stories, weather, entertainment, politics and more.','http://i.cnn.net/cnn/.element/img/1.0/logo/cnn.logo.rss.gif','http://www.cnn.com/rssclick/?section=cnn_topstories',NULL,'en-us',9,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(12,3,'http://rss.cnn.com/rss/cnn_mostpopular.rss','CNN.com - Most Popular','CNN.com delivers up-to-the-minute news and information on the latest top stories, weather, entertainment, politics and more.','','http://www.cnn.com/rssclick/mostpopular/?section=cnn_mostpopular',NULL,'en-us',7,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(13,3,'http://rss.news.yahoo.com/rss/topstories','Yahoo! News: Top Stories','Top Stories','http://us.i1.yimg.com/us.yimg.com/i/us/nws/th/main_142b.gif','http://news.yahoo.com/i/716',NULL,'en-us',7,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(14,4,'http://www.wired.com/news/feeds/rss2/0,2610,3,00.xml','Wired News: Technology','Get the latest headlines from Wired News, the net''s leading chronicle of how technology affects our lives, culture and work.','http://ly.lygo.com/ly/wired/news/images/netcenterb.gif','http://www.wired.com/',NULL,'en_US',14,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(15,4,'http://www.wired.com/news/feeds/rss2/0,2610,,00.xml','Wired News: Top Stories','Get the latest headlines from Wired News, the net''s leading chronicle of how technology affects our lives, culture and work.','http://ly.lygo.com/ly/wired/news/images/netcenterb.gif','http://www.wired.com/',NULL,'en_US',12,0,NULL,NULL,'',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(17,1,'http://dwlt.net/tapestry/dilbert.rdf','Dilbert','Unofficial Dilbert RSS Feed by TapestryComics.com','','http://www.dilbert.com/',NULL,'',40,0,NULL,NULL,'Simple',0,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(18,1,'Search://Flickr/Dog','Dogs on Flickr','A feed of dog - Everyone''s Tagged Photos','http://www.flickr.com/images/buddyicon.jpg','http://www.flickr.com/photos/tags/dog/',NULL,'',8,0,NULL,NULL,'',0,NULL,NULL,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(19,2,'http://www.mobileread.com/feeds/front_rss20.xml','MobileRead Networks','MobileRead Networks - the resource for mobile geeks seeking information and advice for keeping their gadgets happy.','','http://www.mobileread.com',NULL,'en',12,0,NULL,NULL,'',NULL,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(20,5,'http://feeds.feedburner.com/TheRssBlog','The RSS Blog','Randy Charles Morin blogs about RSS, OPML and the XML platform.','http://www.kbcafe.com/rss/logo.jpg','http://www.kbcafe.com/rss/',NULL,'en-us',13,0,NULL,NULL,'',NULL,NULL,NULL,0,0,0,0,200,0,NULL,NULL,NULL,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(23,1,'http://www.googlesightseeing.com/feed/','Google Sightseeing','Why bother seeing the world for real?','','http://www.googlesightseeing.com',NULL,'en',10,0,NULL,NULL,'',0,NULL,NULL,0,0,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(31,7,'http://www.parenting-weblog.com/index.rss','The Parenting Weblog','Parenting, Family, and Child Health','','http://www.parenting-weblog.com/',NULL,'en-US',12,0,NULL,NULL,'',NULL,NULL,NULL,0,0,0,0,200,0,NULL,0,0,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(34,7,'http://www.outdoor-weblog.com/index.rss','The Outdoor Weblog','Your ultimate guide to outdoor sports','','http://www.outdoor-weblog.com/',NULL,'en-US',8,0,NULL,NULL,'',NULL,NULL,NULL,0,0,0,0,200,0,NULL,0,0,0,'','',NULL);"));
		db.execDML(_T("INSERT INTO feed VALUES(36,5,'http://feeds.feedburner.com/Converstations','Converstations','Using Blogs as a Platform for Conversations that Bring Business and Customer Together.','http://mikesansone.typepad.com/conv_smal.png','http://www.converstations.com/',NULL,'en-US',2,0,NULL,NULL,'',NULL,NULL,NULL,0,0,0,0,200,0,NULL,0,0,0,'','',NULL);"));

		db.execDML(_T("COMMIT TRANSACTION;"));
		db.close();
	}
	catch(...)
	{
		return false;
	}

	return true;
}

bool CGreatNewsConfig::SetupProxy()
{
	INTERNET_PROXY_INFO ipi = {0};
	DWORD dwSize2 = sizeof(ipi);

	// If running under Wine, suppress error messages - Its Wininet.dll implementation is incomplete.
	bool bWine = false;
	static const char * (CDECL *pwine_get_version)(void);
	HMODULE hntdll = GetModuleHandle(_T("ntdll.dll"));
	if (hntdll)
	{
		pwine_get_version = (const char *(__cdecl *)(void))GetProcAddress(hntdll, "wine_get_version");
		if (pwine_get_version)
			bWine = true;
	}
	
	if(m_nProxyToUse == 0)	// use default proxy settings
	{
		ipi.dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG ;
		if(!InternetSetOption(NULL, INTERNET_OPTION_PROXY, &ipi, dwSize2))
			if (!bWine)
				MessageBox(NULL, CGNUtil::GetLastErrorMsg(_T("Failed to change proxy settings to default.")), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
	}
	else
	{
		m_proxyURL.Trim();
		if(m_proxyURL.GetLength())
		{
			m_combinedProxyURL.Format(_T("%s:%d"), m_proxyURL, m_proxyPort);
			ipi.dwAccessType = INTERNET_OPEN_TYPE_PROXY ;
			ipi.lpszProxy = (LPCTSTR)m_combinedProxyURL;
			if(m_proxyBypass.GetLength())
				ipi.lpszProxyBypass = (LPCTSTR)m_proxyBypass;
			if(!InternetSetOption(NULL, INTERNET_OPTION_PROXY, &ipi, dwSize2))
				if (!bWine)
					MessageBox(NULL, CGNUtil::GetLastErrorMsg(_T("Failed to change proxy settings.")), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
		}

		if(m_proxyUserName.GetLength())
		{
			if(!InternetSetOption(NULL, INTERNET_OPTION_PROXY_USERNAME , (LPVOID)(LPCTSTR)m_proxyUserName, m_proxyUserName.GetLength()))
				if (!bWine)
					MessageBox(NULL, CGNUtil::GetLastErrorMsg(_T("Failed to change proxy user name.")), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);

			if(m_proxyPassword.GetLength())
			{
				if(!InternetSetOption(NULL, INTERNET_OPTION_PROXY_PASSWORD , (LPVOID)(LPCTSTR)m_proxyPassword, m_proxyPassword.GetLength()))
					if (!bWine)
						MessageBox(NULL, CGNUtil::GetLastErrorMsg(_T("Failed to change proxy password.")), _T("GreatNews"), MB_OK|MB_ICONINFORMATION);
			}
		}


	}

	CGNSingleton<CAsyncDownloadManager>::Instance()->Init(m_nProxyToUse == 1,
															m_combinedProxyURL,
															m_proxyBypass,
															m_proxyUserName,
															m_proxyPassword);

	return true;
}



int CGreatNewsConfig::GetNewsListSortColumn()
{
	int n = -1;
	m_iniFile.GetInt(_T("Environment"), _T("NewsListSortColumn"), n, -1);
	return n;
}

bool CGreatNewsConfig::GetNewsListSortAscending()
{
	bool b = false;
	m_iniFile.GetBool(_T("Environment"), _T("NewsListSortAscending"), b, false);
	return b;
}

void CGreatNewsConfig::SetNewsListSortColumn(int col)
{
	m_iniFile.PutInt(_T("Environment"), _T("NewsListSortColumn"), col);
}

void CGreatNewsConfig::SetNewsListSortAscending(bool bAscending)
{
	m_iniFile.PutBool(_T("Environment"), _T("NewsListSortAscending"), bAscending);
}
