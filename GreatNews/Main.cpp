#include "stdafx.h"
#include "resource.h"
#include "AboutDlg.h"
#include "MainFrm.h"
#include "NewsItemCache.h"
#include "IniFile.h"
#include "WinPlacement.h"
#include "UIUtil.h"
#include "GNClipBoard.h"
#include "FeedManagerLib.h"

CAppModule _Module;

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	if(!g_GreatNewsConfig.Init(lpstrCmdLine))
		return -1;
	

	CNewsItem::SetItemFeatures(g_GreatNewsConfig.m_nSelectedItemFeatures, g_GreatNewsConfig.m_bHighlightNewsWatch);
	CDownloadContext::m_maxFeedSizeInK = g_GreatNewsConfig.m_maxFeedSizeInK;
	CNewsFeed::m_bMarkChangesUnread = g_GreatNewsConfig.m_bMarkChangesUnread;

	HWND anotherGreatNews;
	if(NULL != (anotherGreatNews = ::FindWindow(_T("GreatNews_FrameWindowClass"), NULL)))
	{
		if(g_GreatNewsConfig.m_newFeedUrl.GetLength()) // we are launched to subscribe channel?
		{
			CUIUtil::ShowMyWindow(anotherGreatNews);

			// try to pass command line parameter
			CGNClipBoard clipboard;
			if(clipboard.Open(anotherGreatNews))
			{
				USES_CONVERSION;
				clipboard.SetTextData(T2A((LPTSTR)(LPCTSTR)g_GreatNewsConfig.m_newFeedUrl));
				clipboard.SetUnicodeTextData(T2W((LPTSTR)(LPCTSTR)g_GreatNewsConfig.m_newFeedUrl));

				::PostMessage(anotherGreatNews, WM_COMMAND, ID_CHANNEL_ADDCHANNEL,0);
			}
			return 0;
		}
		else
		{
			if(!g_GreatNewsConfig.m_bMultipleInstance)
			{
				// not allow multiple instances
				CUIUtil::ShowMyWindow(anotherGreatNews);
				return 0;
			}
		}
	}

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	bool bShow = true;
	CString placement = g_GreatNewsConfig.GetWindowPlacement();
	if(placement.GetLength())
	{
		CWindowPlacement place;
		if(place.GetPosData(placement) && place.SetPosData(wndMain))
		{
			bShow = false;
		}
	}

	if(nCmdShow != SW_SHOWNORMAL)
		wndMain.ShowWindow(nCmdShow);


	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
#ifdef MULTITHREADED
	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
	HRESULT hRes = ::CoInitialize(NULL);
#endif
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES |ICC_USEREX_CLASSES );	// add flags to support other controls

	// hRes = _Module.Init(NULL, hInstance);
	hRes = _Module.Init(NULL, hInstance, &LIBID_ATLLib);
	ATLASSERT(SUCCEEDED(hRes));

	AtlAxWinInit();

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
