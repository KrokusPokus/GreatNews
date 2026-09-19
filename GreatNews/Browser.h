/*
Filename: BROWSER.H
Description: Defines an interface for a web browser control, including event sinking.
Date: 08/09/2003

Copyright (c) 2003 by Gilad Novik.  (Web: http://gilad.gsetup.com, Email: gilad@gsetup.com)
All rights reserved.

Copyright / Usage Details
-------------------------
You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of the module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#ifndef _BROWSER_H
#define _BROWSER_H

#include <ExDispid.h>
#include <ComDef.h>
#include <mshtml.h>
#include <wininet.h>
#include <map>
#include "mshtmdid.h"

#pragma warning(disable:4192)
#pragma warning(disable:4049)
#import <mshtml.tlb> rename("TranslateAccelerator", "mshtmlTranslateAccelerator")


class CWebBrowser2EventsBase
{
protected:
	static _ATL_FUNC_INFO StatusTextChangeStruct;
	static _ATL_FUNC_INFO TitleChangeStruct;
	static _ATL_FUNC_INFO PropertyChangeStruct;
	static _ATL_FUNC_INFO OnQuitStruct;
	static _ATL_FUNC_INFO OnToolBarStruct;
	static _ATL_FUNC_INFO OnMenuBarStruct;
	static _ATL_FUNC_INFO OnStatusBarStruct;
	static _ATL_FUNC_INFO OnFullScreenStruct;
	static _ATL_FUNC_INFO OnTheaterModeStruct;
	static _ATL_FUNC_INFO DownloadBeginStruct;
	static _ATL_FUNC_INFO DownloadCompleteStruct;
	static _ATL_FUNC_INFO NewWindow2Struct; 
	static _ATL_FUNC_INFO NewWindow3Struct; 
	static _ATL_FUNC_INFO CommandStateChangeStruct;
	static _ATL_FUNC_INFO BeforeNavigate2Struct;
	static _ATL_FUNC_INFO ProgressChangeStruct;
	static _ATL_FUNC_INFO NavigateComplete2Struct;
	static _ATL_FUNC_INFO DocumentComplete2Struct;
	static _ATL_FUNC_INFO OnVisibleStruct;
	static _ATL_FUNC_INFO SetSecureLockIconStruct;
	static _ATL_FUNC_INFO NavigateErrorStruct;
	static _ATL_FUNC_INFO PrivacyImpactedStateChangeStruct;
	//static _ATL_FUNC_INFO FileDownloadStruct;   // Unsafe to use
	//static _ATL_FUNC_INFO WindowClosingStruct;
};


class CHTMLDocumentEventsBase
{
protected:
	static _ATL_FUNC_INFO OnMouseUpStruct;
	static _ATL_FUNC_INFO OnMouseDownStruct;
};

template<class T,UINT nID=0>
class CWebBrowser2 : public CWebBrowser2EventsBase, CHTMLDocumentEventsBase,
					 public IDispEventSimpleImpl<nID, CWebBrowser2<T,nID>, &DIID_DWebBrowserEvents2>,
					 // public IDispEventSimpleImpl<nID, CWebBrowser2, &DIID_HTMLDocumentEvents2>,
				 	 public IDocHostUIHandler,
					 public IOleCommandTarget
{
	typedef CWebBrowser2<T, nID> thisClass;
	typedef IDispEventSimpleImpl<nID, CWebBrowser2, &DIID_DWebBrowserEvents2> WebBrowserEvents2_DispEventSimpleImpl;
	typedef IDispEventSimpleImpl<nID, CWebBrowser2, &DIID_HTMLDocumentEvents2> HTMLDocumentEvents2_DispEventSimpleImpl;

#ifndef DISPID_NEWWINDOW3
#define DISPID_NEWWINDOW3           273
#endif

public:
	enum RefreshConstants
	{
		REFRESH_NORMAL = 0,
		REFRESH_IFEXPIRED = 1,
		REFRESH_CONTINUE = 2,
		REFRESH_COMPLETELY = 3
	};
	
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()
		
	BEGIN_SINK_MAP(thisClass)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_STATUSTEXTCHANGE, __StatusTextChange, &StatusTextChangeStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_PROGRESSCHANGE, __ProgressChange, &ProgressChangeStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_COMMANDSTATECHANGE, __CommandStateChange, &CommandStateChangeStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_DOWNLOADBEGIN, __DownloadBegin, &DownloadBeginStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_DOWNLOADCOMPLETE, __DownloadComplete, &DownloadCompleteStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_TITLECHANGE, __TitleChange, &TitleChangeStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, __NavigateComplete2, &NavigateComplete2Struct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, __BeforeNavigate2, &BeforeNavigate2Struct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_PROPERTYCHANGE, __PropertyChange, &PropertyChangeStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_NEWWINDOW2, __NewWindow2, &NewWindow2Struct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_NEWWINDOW3, __NewWindow3, &NewWindow3Struct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, __DocumentComplete, &DocumentComplete2Struct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_ONQUIT, __OnQuit, &OnQuitStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_ONVISIBLE, __OnVisible, &OnVisibleStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_ONTOOLBAR, __OnToolBar, &OnToolBarStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_ONMENUBAR, __OnMenuBar, &OnMenuBarStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_ONSTATUSBAR, __OnStatusBar, &OnStatusBarStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_ONFULLSCREEN, __OnFullScreen, &OnFullScreenStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_ONTHEATERMODE, __OnTheaterMode, &OnTheaterModeStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_SETSECURELOCKICON, __SetSecureLockIcon, &SetSecureLockIconStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_NAVIGATEERROR, __NavigateError, &NavigateErrorStruct)
		SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_PRIVACYIMPACTEDSTATECHANGE, __PrivacyImpactedStateChange, &PrivacyImpactedStateChangeStruct)
		//SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_WINDOWCLOSING, __WindowClosing, &WindowClosingStruct)
		//SINK_ENTRY_INFO(nID, DIID_DWebBrowserEvents2, DISPID_FILEDOWNLOAD, __FileDownload, &FileDownloadStruct)

		//SINK_ENTRY_INFO(nID, DIID_HTMLDocumentEvents2, DISPID_MOUSEUP, __OnMouseUp, &OnMouseUpStruct)
		//SINK_ENTRY_INFO(nID, DIID_HTMLDocumentEvents2, DISPID_MOUSEDOWN, __OnMouseDown, &OnMouseDownStruct)
	END_SINK_MAP()
		
	CWebBrowser2()
	{
		m_bState[back]=m_bState[forward]=FALSE;
		m_bUseMozilla = false;
	}
	
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		LRESULT lResult=pT->DefWindowProc();
		HRESULT hResult=pT->QueryControl(IID_IWebBrowser2, (void**)&m_pBrowser);
		if (SUCCEEDED(hResult))
		{
			if (FAILED(WebBrowserEvents2_DispEventSimpleImpl::DispEventAdvise(m_pBrowser,&DIID_DWebBrowserEvents2)))
			{
				ATLASSERT(FALSE);
				m_pBrowser.Release();
			}
		}
		return lResult;
	}
	
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (m_pBrowser)
		{
			WebBrowserEvents2_DispEventSimpleImpl::DispEventUnadvise(m_pBrowser, &DIID_DWebBrowserEvents2);
			m_pBrowser.Release();
		}
		bHandled=FALSE;
		return 0;
	}
	
	void OnSetSecureLockIcon(long nSecureLockIcon) { }
	BOOL OnNavigateError(IDispatch* pDisp, const CString& szURL, const CString& szTargetFrameName, LONG nStatusCode)
	{   // Return TRUE to cancel
		return FALSE;
	}
	void OnPrivacyImpactedStateChange(BOOL bImpacted) { }
	void OnStatusTextChange(const CString& szText) { }
	void OnProgressChange(long nProgress, long nProgressMax) { }
	void OnCommandStateChange(long nCommand, BOOL bEnable)
	{
		switch (nCommand)
		{
		case CSC_NAVIGATEBACK:
			m_bState[back]=bEnable;
			break;
		case CSC_NAVIGATEFORWARD:
			m_bState[forward]=bEnable;
			break;
		}
	}
	void OnDownloadBegin() { }
	void OnDownloadComplete() { }
	void OnTitleChange(const CString& szTitle) { }
	void OnNavigateComplete2(IDispatch* pDisp, const CString& szURL) { }
	BOOL OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders)
	{
		AtlTrace(_T("OnBeforeNavigate2\n"));
		//m_spDefaultDocHostUIHandler = (IUnknown*)NULL;
		//m_spDefaultOleCommandTarget = (IUnknown*)NULL;

		//IDispatchPtr spDoc = GetDocument();
		//HRESULT hr;
		//if(spDoc!=NULL &&
		//	((HTMLDocumentEvents2_DispEventSimpleImpl*)this)->m_dwEventCookie != 0xFEFEFEFE &&
		//	FAILED(hr=HTMLDocumentEvents2_DispEventSimpleImpl::DispEventUnadvise(spDoc,&DIID_HTMLDocumentEvents2)))
		//{
		//	ATLASSERT(FALSE);
		//}

		return FALSE;
	}
	void OnPropertyChange(const CString& szProperty) { }
	BOOL OnNewWindow2(IDispatch** ppDisp)
	{   // Return TRUE to cancel
		return FALSE;
	}
	BOOL OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
	{   // Return TRUE to cancel
		return FALSE;
	}
	void OnDocumentComplete(IDispatch* pDisp, const CString& szURL)
	{
		//IDispatchPtr spDoc = GetDocument();
		//HRESULT hr;
		//if(spDoc!=NULL &&
		//	((HTMLDocumentEvents2_DispEventSimpleImpl*)this)->m_dwEventCookie == 0xFEFEFEFE &&
		//	FAILED(hr=HTMLDocumentEvents2_DispEventSimpleImpl::DispEventAdvise(spDoc,&DIID_HTMLDocumentEvents2)))
		//{
		//	ATLASSERT(FALSE);
		//}

		CComPtr<IDispatch> spDisp;
		HRESULT hr = m_pBrowser->get_Document(&spDisp);
		if (SUCCEEDED(hr) && spDisp)
		{
			// If this is not an HTML document (e.g., it's a Word doc or a PDF), don't sink.
			CComQIPtr<IHTMLDocument2, &IID_IHTMLDocument2> spHTML(spDisp);
			if (spHTML)
			{
				// Get pointers to default interfaces
				CComQIPtr<IOleObject, &IID_IOleObject> spOleObject(spDisp);
				if (spOleObject)
				{
					CComPtr<IOleClientSite> spClientSite;
					hr = spOleObject->GetClientSite(&spClientSite);
					if (SUCCEEDED(hr) && spClientSite)
					{
						m_spDefaultDocHostUIHandler = spClientSite;
						m_spDefaultOleCommandTarget = spClientSite;
					}
				}

				// Set this class to be the IDocHostUIHandler
				CComQIPtr<ICustomDoc, &IID_ICustomDoc> spCustomDoc(spDisp);
				if (spCustomDoc)
					spCustomDoc->SetUIHandler(this);
			}
		}
	}
	void OnQuit() { }
	void OnVisible(BOOL bVisible) { }
	void OnToolBar(BOOL bToolBar) { }
	void OnMenuBar(BOOL bMenuBar) { }
	void OnStatusBar(BOOL bStatusBar) { }
	void OnFullScreen(BOOL bFullScreen) { }
	void OnTheaterMode(BOOL bTheaterMode) { }
	/*
	BOOL OnWindowClosing(BOOL bChildWindow)
	{   // Return TRUE to cancel
		return FALSE;
	}
	*/
	/*
	BOOL OnFileDownload()
	{   // Return TRUE to cancel
		return FALSE;
	}
	*/
	
	void OnMouseUp(long button, bool ctrl)
	{
	}

	void OnMouseDown(long button, bool ctrl, bool bUserClickedLink, const CString& url)
	{
	}


	// Properties
	__declspec(property(get=GetAddressBar,put=PutAddressBar)) BOOL AddressBar;
	__declspec(property(get=GetApplication)) IDispatchPtr Application;
	__declspec(property(get=GetBusy)) BOOL Busy;
	__declspec(property(get=GetFullName)) CString FullName;
	__declspec(property(get=GetLocationName)) CString LocationName;
	__declspec(property(get=GetLocationURL)) CString LocationURL;
	__declspec(property(get=GetType)) CString Type;
	__declspec(property(get=GetRegisterAsBrowser,put=PutRegisterAsBrowser)) BOOL RegisterAsBrowser;
	__declspec(property(get=GetRegisterAsDropTarget,put=PutRegisterAsDropTarget)) BOOL RegisterAsDropTarget;
	__declspec(property(get=GetTheaterMode,put=PutTheaterMode)) BOOL TheaterMode;
	__declspec(property(get=GetVisible,put=PutVisible)) BOOL Visible;
	__declspec(property(get=GetMenuBar,put=PutMenuBar)) BOOL MenuBar;
	__declspec(property(get=GetToolBar,put=PutToolBar)) BOOL ToolBar;
	__declspec(property(get=GetOffline,put=PutOffline)) BOOL Offline;
	__declspec(property(get=GetSilent,put=PutSilent)) BOOL Silent;
	__declspec(property(get=GetFullScreen,put=PutFullScreen)) BOOL FullScreen;
	__declspec(property(get=GetStatusBar,put=PutStatusBar)) BOOL StatusBar;
	__declspec(property(get=GetLeft,put=PutLeft)) LONG Left;
	__declspec(property(get=GetTop,put=PutTop)) LONG Top;
	__declspec(property(get=GetWidth,put=PutWidth)) LONG Width;
	__declspec(property(get=GetHeight,put=PutHeight)) LONG Height;
	__declspec(property(get=GetDocument)) IDispatchPtr Document;

	BOOL CanBack() const
	{
		return m_bState[back];
	}

	BOOL CanForward() const
	{
		return m_bState[forward];
	}
	
	void Quit()
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->Quit();
	}
	
	void PutAddressBar(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_AddressBar(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetAddressBar()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_AddressBar(&bResult);
		return bResult ? TRUE : FALSE;
	}
	IDispatchPtr GetApplication()
	{
		ATLASSERT(m_pBrowser);
		IDispatchPtr pDispatch;
		m_pBrowser->get_Application(&pDispatch);
		return pDispatch;
	}
	BOOL GetBusy()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_Busy(&bResult);
		return bResult ? TRUE : FALSE;
	}
	CString GetFullName() const
	{
		ATLASSERT(m_pBrowser);
		CComBSTR szResult;
		m_pBrowser->get_FullName(&szResult);
		return szResult;
	}
	CString GetLocationName() const
	{
		ATLASSERT(m_pBrowser);
		CComBSTR szResult;
		m_pBrowser->get_LocationName(&szResult);
		return (BSTR)szResult;
	}
	CString GetType() const
	{
		ATLASSERT(m_pBrowser);
		CComBSTR szResult;
		m_pBrowser->get_Type(&szResult);
		return (BSTR)szResult;
	}
	CString GetLocationURL() const
	{
		ATLASSERT(m_pBrowser);
		CComBSTR szResult;
		m_pBrowser->get_LocationURL(&szResult);
		return (BSTR)szResult;
	}
	void PutRegisterAsBrowser(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_RegisterAsBrowser(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetRegisterAsBrowser()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_RegisterAsBrowser(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutRegisterAsDropTarget(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_RegisterAsDropTarget(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetRegisterAsDropTarget()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_RegisterAsDropTarget(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutTheaterMode(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_TheaterMode(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetTheaterMode()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_TheaterMode(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutVisible(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_Visible(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetVisible()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_Visible(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutMenuBar(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_MenuBar(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetMenuBar()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_MenuBar(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutToolBar(int nNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_ToolBar(nNewValue);
	}
	BOOL GetToolBar()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_ToolBar(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutOffline(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_Offline(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetOffline()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_Offline(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutSilent(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_Silent(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetSilent()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_Silent(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutFullScreen(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_FullScreen(bNewValue ? VARIANT_TRUE : VARIANT_FALSE);
	}
	BOOL GetFullScreen()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_FullScreen(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutStatusBar(BOOL bNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_StatusBar(bNewValue ? VARIANT_TRUE : VARIANT_FALSE); 
	}
	BOOL GetStatusBar()
	{
		ATLASSERT(m_pBrowser);
		VARIANT_BOOL bResult;
		m_pBrowser->get_StatusBar(&bResult);
		return bResult ? TRUE : FALSE;
	}
	void PutLeft(LONG nNewValue)
	{
		ATLASSERT(m_spBrowser);
		m_pBrowser->put_Left(nNewValue);
	}
	LONG GetLeft()
	{
		ATLASSERT(m_pBrowser);
		LONG nResult;
		m_pBrowser->get_Left(&bResult);
		return nResult;
	}
	void PutTop(LONG nNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_Top(nNewValue);
	}
	LONG GetTop()
	{
		ATLASSERT(m_pBrowser);
		LONG nResult;
		m_pBrowser->get_Top(&bResult);
		return nResult;
	}
	void PutWidth(LONG nNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_Width(nNewValue);
	}
	LONG GetWidth()
	{
		ATLASSERT(m_pBrowser);
		LONG nResult;
		m_pBrowser->get_Width(&bResult);
		return nResult;
	}
	void PutHeight(LONG nNewValue)
	{
		ATLASSERT(m_pBrowser);
		m_pBrowser->put_Height(nNewValue);
	}
	LONG GetHeight()
	{
		ATLASSERT(m_pBrowser);
		LONG nResult;
		m_pBrowser->get_Height(&bResult);
		return nResult;
	}
	IDispatchPtr GetDocument()
	{
		ATLASSERT(m_pBrowser);
		IDispatchPtr pDispatch;
		m_pBrowser->get_Document(&pDispatch);
		return pDispatch;
	}

	HRESULT GetDispatch(IDispatch** ppDisp)
	{
		ATLASSERT(m_pBrowser);
		return m_pBrowser->QueryInterface(IID_IDispatch, (void**)ppDisp);
	}

	HRESULT PutProperty(LPCTSTR szProperty,const VARIANT& vtValue)
	{
		ATLASSERT(m_pBrowser);
		USES_CONVERSION;
		return m_pBrowser->PutProperty(CComBSTR(szProperty),vtValue);
	}
	CComVariant GetProperty(LPCTSTR szProperty)
	{
		ATLASSERT(m_pBrowser);
		CComDispatchDriver pDriver(m_pBrowser);
		ATLASSERT(pDriver);
		CComVariant vtResult;
		pDriver.GetPropertyByName(CComBSTR(szProperty),&vtResult);
		return vtResult;
	}
	
	// Methods
	HRESULT ClientToWindow(LPPOINT pPoint)
	{
		ATLASSERT(m_pBrowser);
		ATLASSERT(pPoint);
		return m_pBrowser->ClientToWindow(&(pPoint->x),&(pPoint->y));
	}
	HRESULT ExecWB(OLECMDID nCmd,OLECMDEXECOPT nCmdOptions,VARIANT* pvInput=NULL,VARIANT* pvOutput=NULL)
	{
		ATLASSERT(m_pBrowser);
		return m_pBrowser->ExecWB(nCmd,nCmdOptions,pvInput,pvOutput);
	}
	HRESULT GoBack()
	{
		ATLASSERT(m_pBrowser);
		return m_pBrowser->GoBack();
	}
	HRESULT GoForward()
	{
		ATLASSERT(m_pBrowser);
		return m_pBrowser->GoForward();
	}
	HRESULT GoHome()
	{
		ATLASSERT(m_pBrowser);
		return m_pBrowser->GoHome();
	}
	HRESULT GoSearch()
	{
		ATLASSERT(m_pBrowser);
		return m_pBrowser->GoSearch();
	}
	HRESULT Navigate(LPCTSTR szURL,DWORD dwFlags=0,LPCTSTR szTargetFrameName=NULL,LPCVOID pPostData=NULL,DWORD dwPostDataLength=0,LPCTSTR szHeaders=NULL)
	{
		USES_CONVERSION;
		ATLASSERT(m_pBrowser);
		ATLASSERT(szURL);
		CComVariant vtTargetFrameName,vtPostData,vtHeaders;
		if (szTargetFrameName)
			vtTargetFrameName=szTargetFrameName;
		if (pPostData && dwPostDataLength>0)
		{
			vtPostData.ChangeType(VT_ARRAY|VT_UI1);
			SAFEARRAYBOUND Bound;
			Bound.cElements=dwPostDataLength;
			Bound.lLbound=0;
			vtPostData.parray=SafeArrayCreate(VT_UI1,1,&Bound);
			ATLASSERT(vtPostData.parray);
			if (vtPostData.parray==NULL)
				return E_OUTOFMEMORY;
			LPVOID pData;
			SafeArrayAccessData(vtPostData.parray,&pData);
			CopyMemory(pData,pPostData,dwPostDataLength);
			SafeArrayUnaccessData(vtPostData.parray);
		}
		if (szHeaders)
			vtHeaders=szHeaders;
		return m_pBrowser->Navigate(T2BSTR(szURL),&CComVariant((LONG)dwFlags),&vtTargetFrameName,&vtPostData,&vtHeaders);
	}
	HRESULT Navigate2(LPCTSTR szURL,DWORD dwFlags=0,LPCTSTR szTargetFrameName=NULL,LPCVOID pPostData=NULL,DWORD dwPostDataLength=0,LPCTSTR szHeaders=NULL)
	{
		ATLASSERT(m_pBrowser);
		ATLASSERT(szURL);
		CComVariant vtTargetFrameName,vtPostData,vtHeaders;
		if (szTargetFrameName)
			vtTargetFrameName=szTargetFrameName;
		if (pPostData && dwPostDataLength>0)
		{
			vtPostData.ChangeType(VT_ARRAY|VT_UI1);
			SAFEARRAYBOUND Bound;
			Bound.cElements=dwPostDataLength;
			Bound.lLbound=0;
			vtPostData.parray=SafeArrayCreate(VT_UI1,1,&Bound);
			ATLASSERT(vtPostData.parray);
			if (vtPostData.parray==NULL)
				return E_OUTOFMEMORY;
			LPVOID pData=NULL;
			SafeArrayAccessData(vtPostData.parray,&pData);
			ATLASSERT(pData);
			CopyMemory(pData,pPostData,dwPostDataLength);
			SafeArrayUnaccessData(vtPostData.parray);
		}
		if (szHeaders)
			vtHeaders=szHeaders;
		return m_pBrowser->Navigate2(&CComVariant(szURL),&CComVariant((LONG)dwFlags),&vtTargetFrameName,&vtPostData,&vtHeaders);
	}
	HRESULT Refresh()
	{
		ATLASSERT(m_pBrowser);
		return m_pBrowser->Refresh();
	}
	HRESULT Refresh2(LONG nLevel)
	{
		ATLASSERT(m_pBrowser);
		// return m_pBrowser->Refresh2(CComVariant(nLevel));
		return m_pBrowser->Refresh2(&CComVariant(nLevel));
	}
	HRESULT Stop()
	{
		ATLASSERT(m_pBrowser);
		return m_pBrowser->Stop();
	}
	OLECMDF QueryStatusWB(OLECMDID nCmd)
	{
		ATLASSERT(m_pBrowser);
		OLECMDF nResult;
		m_pBrowser->QueryStatusWB(nCmd,&nResult);
		return nResult;
	}
	
	HRESULT LoadFromResource(UINT nID) 
	{
		TCHAR szFilename[MAX_PATH];
		GetModuleFileName(_Module.GetModuleInstance(),szFilename,sizeof(szFilename)/sizeof(TCHAR));
		TCHAR szURL[4096];
		StringCchPrintf(szURL,sizeof(szURL)/sizeof(TCHAR), _T("res://%s/%d"),szFilename,nID);
		return Navigate(szURL);
	}
	HRESULT LoadFromResource(LPCTSTR szID) 
	{
		TCHAR szFilename[MAX_PATH];
		GetModuleFileName(_Module.GetModuleInstance(),szFilename,sizeof(szFilename)/sizeof(TCHAR));
		TCHAR szURL[4096];
		StringCchPrintf(szURL,sizeof(szURL)/sizeof(TCHAR),_T("res://%s/%s"),szFilename,szID);
		return Navigate(szURL);
	}

	//
	// discover rss feeds on this page
	//
	size_t DiscoverFeeds(std::map<CString, CString>& rssFeeds)
	{
		rssFeeds.clear();
		std::map<CString, CString> discoveredFeeds;
		try
		{
			//
			// Retrieve the document object.
			//
			MSHTML::IHTMLDocument2Ptr spDoc = GetDocument();
			if(spDoc == NULL)
				return 0;
			CString base = GetLocationURL();

			MSHTML::IHTMLElementCollectionPtr spElements = spDoc->Getall();
			DiscoverFeedsHelp(discoveredFeeds, spElements, m_bUseMozilla);

			// Then we have to patch up for sourceforge.net if we use IE6's engine
			//if(!m_bUseMozilla && base.Find(_T("sourceforge.net")) > 0)
			//{
			//	MSHTML::IHTMLElementCollectionPtr spElements = spDoc->Getall();
			//	MSHTML::IHTMLElementCollectionPtr spAllAnchors = spElements->tags(_bstr_t(_T("A")));
			//	DiscoverFeedsHelp(rssFeeds, spAllAnchors, m_bUseMozilla);
			//}

			//
			//	Convert relative path to absolute
			//
	#define MAXURL 2048
			TCHAR szUrl[MAXURL];
			for(std::map<CString, CString>::iterator it=discoveredFeeds.begin(); it != discoveredFeeds.end(); ++it)
			{
				DWORD dwSize = MAXURL;
				CString& url = it->second;
				if(InternetCombineUrl (base,url,szUrl,&dwSize,ICU_BROWSER_MODE))
				{
					rssFeeds[it->first] = szUrl;
				}
			}

		}
		catch(const _com_error& e)
		{
			AtlTrace(_T("Error detecting feeds[%s]\n"), e.Description());
		}
		catch(...)
		{
			AtlTrace(_T("Error detecting feeds due to unknown reason\n"));
		}

		return rssFeeds.size();
	}

protected:
	enum
	{
		back = 0,
		forward
	};
	CComPtr<IWebBrowser2> m_pBrowser;
	BOOL m_bState[2];
	bool m_bUseMozilla;
	
private:
	void DiscoverFeedsHelp(std::map<CString, CString>& rssFeeds, MSHTML::IHTMLElementCollectionPtr spElements, bool bUseMozilla)
	{
		// check all <a> and <link> elements for rss feeds
		long count= spElements->Getlength();
		for (long i = 0; i < count; ++i) 
		{
			MSHTML::IHTMLElementPtr spElement = spElements->item(i, 0L);
			if(spElement == NULL)
				continue;

			_bstr_t tag = spElement->tagName;
			if(_tcsicmp((LPCTSTR)tag,_T("A")) == 0)
			{
				_bstr_t href = spElement->getAttribute(_T("href"),0);
				_bstr_t title = spElement->getAttribute(_T("title"),0);
				CString url = (LPCTSTR)href;
				if(!CGNUtil::IsFeedUrl(url) && !CGNUtil::IsOpmlUrl(url))
					continue;

				// we found a feed, save it
				CString feedTitle = (LPCTSTR)title;
				if(feedTitle.GetLength())
					rssFeeds[feedTitle] = url;
				else
					rssFeeds[url]=url;
			}
			else if(_tcsicmp((LPCTSTR)tag,_T("LINK")) == 0)
			{
				_bstr_t rel = spElement->getAttribute("rel",0);
				_bstr_t type = spElement->getAttribute("type",0);
				_bstr_t title = spElement->getAttribute("title",0);

				if(rel == _bstr_t(_T("alternate")) &&
					(type == _bstr_t(_T("application/rss+xml"))
					 || type == _bstr_t(_T("application/atom+xml"))
					 || type == _bstr_t(_T("application/xml"))
					 || type == _bstr_t(_T("text/xml"))))
				{
					_bstr_t href = spElement->getAttribute(_T("href"),0);
					CString url  = (LPCTSTR)href;
					if(!url.GetLength())
						continue;

					CString feedTitle = (LPCTSTR)title;
					if(feedTitle.GetLength())
						rssFeeds[feedTitle] = url;
					else
						rssFeeds[url] = url;
				}
			}
		}
	}

	void __stdcall __SetSecureLockIcon(long nSecureLockIcon)
	{
		T* pT = static_cast<T*>(this);
		pT->OnSetSecureLockIcon(nSecureLockIcon);
	}
	
	void __stdcall __NavigateError(IDispatch* pDisp, VARIANT* pvURL, VARIANT* pvTargetFrameName, VARIANT* pvStatusCode, VARIANT_BOOL* pbCancel)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(V_VT(pvURL) == VT_BSTR);
		ATLASSERT(V_VT(pvTargetFrameName) == VT_BSTR);
		ATLASSERT(V_VT(pvStatusCode) == (VT_I4));
		ATLASSERT(pbCancel != NULL);
		*pbCancel=pT->OnNavigateError(pDisp,V_BSTR(pvURL),V_BSTR(pvTargetFrameName),V_I4(pvStatusCode));
	}
	
	void __stdcall __PrivacyImpactedStateChange(VARIANT_BOOL bImpacted)
	{
		T* pT = static_cast<T*>(this);
		pT->OnPrivacyImpactedStateChange(bImpacted);
	}
	
	void __stdcall __StatusTextChange(BSTR szText)
	{
		T* pT = static_cast<T*>(this);
		pT->OnStatusTextChange(szText);
	}
	
	void __stdcall __ProgressChange(long nProgress, long nProgressMax)
	{
		T* pT = static_cast<T*>(this);
		pT->OnProgressChange(nProgress, nProgressMax);
	}
	
	void __stdcall __CommandStateChange(long nCommand, VARIANT_BOOL bEnable)
	{
		T* pT = static_cast<T*>(this);
		pT->OnCommandStateChange(nCommand, (bEnable==VARIANT_TRUE) ? TRUE : FALSE);
	}
	
	void __stdcall __DownloadBegin()
	{
		T* pT = static_cast<T*>(this);
		pT->OnDownloadBegin();
	}
	
	void __stdcall __DownloadComplete()
	{
		T* pT = static_cast<T*>(this);
		pT->OnDownloadComplete();
	}
	
	void __stdcall __TitleChange(BSTR szText)
	{
		T* pT = static_cast<T*>(this);
		pT->OnTitleChange(szText);
	}
	
	void __stdcall __NavigateComplete2(IDispatch* pDisp, VARIANT* pvURL)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(V_VT(pvURL) == VT_BSTR);
		pT->OnNavigateComplete2(pDisp, V_BSTR(pvURL));
	}
	
	void __stdcall __BeforeNavigate2(IDispatch* pDisp, VARIANT* pvURL, VARIANT* pvFlags, VARIANT* pvTargetFrameName, VARIANT* pvPostData, VARIANT* pvHeaders, VARIANT_BOOL* pbCancel)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(V_VT(pvURL) == VT_BSTR);
		ATLASSERT(V_VT(pvTargetFrameName) == VT_BSTR);
		ATLASSERT(V_VT(pvPostData) == (VT_VARIANT | VT_BYREF));
		ATLASSERT(V_VT(pvHeaders) == VT_BSTR);
		ATLASSERT(pbCancel != NULL);
		
		VARIANT* vtPostedData = V_VARIANTREF(pvPostData);
		CSimpleArray<BYTE> pArray;
		if (V_VT(vtPostedData) & VT_ARRAY)
		{
			ATLASSERT(V_ARRAY(vtPostedData)->cDims == 1 && V_ARRAY(vtPostedData)->cbElements == 1);
			long nLowBound=0,nUpperBound=0;
			SafeArrayGetLBound(V_ARRAY(vtPostedData), 1, &nLowBound);
			SafeArrayGetUBound(V_ARRAY(vtPostedData), 1, &nUpperBound);
			DWORD dwSize=nUpperBound+1-nLowBound;
			LPVOID pData=NULL;
			SafeArrayAccessData(V_ARRAY(vtPostedData),&pData);
			ATLASSERT(pData);
			
			pArray.m_nSize=pArray.m_nAllocSize=dwSize;
			pArray.m_aT=(BYTE*)malloc(dwSize);
			ATLASSERT(pArray.m_aT);
			CopyMemory(pArray.GetData(), pData, dwSize);
			SafeArrayUnaccessData(V_ARRAY(vtPostedData));
		}
		*pbCancel=pT->OnBeforeNavigate2(pDisp, V_BSTR(pvURL), V_I4(pvFlags), V_BSTR(pvTargetFrameName), pArray, V_BSTR(pvHeaders)) ? VARIANT_TRUE : VARIANT_FALSE;
	}
	
	void __stdcall __PropertyChange(BSTR szProperty)
	{
		T* pT = static_cast<T*>(this);
		pT->OnPropertyChange(szProperty);
	}
	
	void __stdcall __NewWindow2(IDispatch** ppDisp, VARIANT_BOOL* pbCancel)
	{
		T* pT = static_cast<T*>(this);
		*pbCancel = pT->OnNewWindow2(ppDisp) ? VARIANT_TRUE : VARIANT_FALSE;
	}

	void __stdcall __NewWindow3(IDispatch** ppDisp, VARIANT_BOOL* pbCancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
	{
		T* pT = static_cast<T*>(this);
		*pbCancel = pT->OnNewWindow3(ppDisp, dwFlags, bstrUrlContext, bstrUrl) ? VARIANT_TRUE : VARIANT_FALSE;
	}
	
	void __stdcall __DocumentComplete(IDispatch* pDisp, VARIANT* pvURL)
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(V_VT(pvURL) == VT_BSTR);
		pT->OnDocumentComplete(pDisp, V_BSTR(pvURL));
	}
	
	void __stdcall __OnQuit()
	{
		T* pT = static_cast<T*>(this);
		pT->OnQuit();
	}
	
	void __stdcall __OnVisible(VARIANT_BOOL bVisible)
	{
		T* pT = static_cast<T*>(this);
		pT->OnVisible(bVisible == VARIANT_TRUE ? TRUE : FALSE);
	}
	
	void __stdcall __OnToolBar(VARIANT_BOOL bToolBar)
	{
		T* pT = static_cast<T*>(this);
		pT->OnToolBar(bToolBar == VARIANT_TRUE ? TRUE : FALSE);
	}
	
	void __stdcall __OnMenuBar(VARIANT_BOOL bMenuBar)
	{
		T* pT = static_cast<T*>(this);
		pT->OnMenuBar(bMenuBar == VARIANT_TRUE ? TRUE : FALSE);
	}
	
	void __stdcall __OnStatusBar(VARIANT_BOOL bStatusBar)
	{
		T* pT = static_cast<T*>(this);
		pT->OnStatusBar(bStatusBar == VARIANT_TRUE ? TRUE : FALSE);
	}
	
	void __stdcall __OnFullScreen(VARIANT_BOOL bFullScreen)
	{
		T* pT = static_cast<T*>(this);
		pT->OnFullScreen(bFullScreen == VARIANT_TRUE ? TRUE : FALSE);
	}
	
	void __stdcall __OnTheaterMode(VARIANT_BOOL bTheaterMode)
	{
		T* pT = static_cast<T*>(this);
		pT->OnTheaterMode(bTheaterMode == VARIANT_TRUE ? TRUE : FALSE);
	}
	
	/*
	void __stdcall __FileDownload(VARIANT_BOOL* pbCancel)
	{
		T* pT = static_cast<T*>(this);
		*pbCancel = pT->OnFileDownload() ? VARIANT_TRUE : VARIANT_FALSE;
	}
	*/
	/*
	void __stdcall __WindowClosing(VARIANT_BOOL bChildWindow,VARIANT_BOOL* pbCancel)
	{
		T* pT = static_cast<T*>(this);
		*pbCancel=pT->OnWindowClosing(bChildWindow ? TRUE : FALSE);
	}
	*/

	void __stdcall __OnMouseUp(IDispatch* pHtmlEventObj)
	{
		AtlTrace(_T("mouse up\n"));

		CComQIPtr<IHTMLEventObj> spHtmlEventObj = pHtmlEventObj;

		long button;
		spHtmlEventObj->get_button(&button);
		VARIANT_BOOL ctrl;
		spHtmlEventObj->get_ctrlKey(&ctrl);

		T* pT = static_cast<T*>(this);
		pT->OnMouseUp(button, (ctrl==VARIANT_TRUE));
	}

	void __stdcall __OnMouseDown(IDispatch* pHtmlEventObj)
	{
		AtlTrace(_T("mouse down\n"));

		CComQIPtr<IHTMLEventObj> spHtmlEventObj = pHtmlEventObj;
		if(spHtmlEventObj == NULL)
		{
			AtlTrace(_T("__OnMouseDown Failed\n"));
			return;
		}

		// grab some parameters first
		long button;
		VARIANT_BOOL ctrl;
		if(FAILED(spHtmlEventObj->get_button(&button)))
		{
			AtlTrace(_T("__OnMouseDown Failed\n"));
			return;
		}
		if(FAILED(spHtmlEventObj->get_ctrlKey(&ctrl)))
		{
			AtlTrace(_T("__OnMouseDown Failed\n"));
			return;
		}

		if(button == 0)
		{
			// this is not a real mouse click, probably simulated by script
			AtlTrace(_T("fake OnMouseDown discarded\n"));
			return;
		}

		// check if user clicked on a link
		bool bUserClickedLink = false;
		CString strUrl;
		CComPtr<IHTMLElement> spElem;
		HRESULT hr = spHtmlEventObj->get_srcElement(&spElem);
		if (SUCCEEDED(hr) && spElem)
		{
			CComBSTR bsTagName;

			while (1)
			{
				spElem->get_tagName(&bsTagName);
				bsTagName.ToUpper();

				if (bsTagName == L"BODY")
					break;	// did not click a link

				if (bsTagName == L"A" ||
					bsTagName == L"AREA" ||
					bsTagName == L"INPUT")
				{
					// grab the url if clicked on a link
					CComVariant v;
					spElem->getAttribute(_T("HREF"), 0, &v);
					if(v.vt == VT_BSTR)
						strUrl = v.bstrVal;

					ATLTRACE(_T("HTMLDocumentEvents::onclick fired -- user clicked a link\n"));
					bUserClickedLink = true;
					break;
				}

				CComPtr<IHTMLElement> spParentElem;
				hr = spElem->get_parentElement(&spParentElem);
				if (FAILED(hr) || !spParentElem)
					break;
				spElem = spParentElem;
			}
		}

		// notify listeners
		T* pT = static_cast<T*>(this);
		pT->OnMouseDown(button, (ctrl==VARIANT_TRUE), bUserClickedLink, strUrl);
	}



	//
	// IDocHostUIHandler
	//
	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT FAR* ppt, IUnknown FAR* pcmdTarget, IDispatch FAR* pdispReserved)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->ShowContextMenu(dwID, ppt, pcmdTarget, pdispReserved);
		return S_FALSE;
	}

	STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject FAR* pActiveObject,
					  IOleCommandTarget FAR* pCommandTarget,
                      IOleInPlaceFrame  FAR* pFrame,
                      IOleInPlaceUIWindow FAR* pDoc)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->ShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);
		return S_FALSE;
	}

	STDMETHOD(GetHostInfo)(DOCHOSTUIINFO FAR *pInfo)
	{
		HRESULT hr = S_OK;
		if (m_spDefaultDocHostUIHandler)
			hr = m_spDefaultDocHostUIHandler->GetHostInfo(pInfo);
		if(hr == S_OK)
		{
			pInfo->dwFlags |= DOCHOSTUIFLAG_ACTIVATE_CLIENTHIT_ONLY|DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE|DOCHOSTUIFLAG_NO3DBORDER|DOCHOSTUIFLAG_THEME;
			pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT; 
		}
		return hr;
	}

	STDMETHOD(HideUI)(void)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->HideUI();
		return S_OK;
	}

	STDMETHOD(UpdateUI)(void)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->UpdateUI();
		return S_OK;
	}

	STDMETHOD(EnableModeless)(BOOL fEnable)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->EnableModeless(fEnable);
		return S_OK;
	}

	STDMETHOD(OnDocWindowActivate)(BOOL fActivate)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->OnDocWindowActivate(fActivate);
		return S_OK;
	}

	STDMETHOD(OnFrameWindowActivate)(BOOL fActivate)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->OnFrameWindowActivate(fActivate);
		return S_OK;
	}

	STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow FAR* pUIWindow, BOOL fFrameWindow)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->ResizeBorder(prcBorder, pUIWindow, fFrameWindow);
		return S_OK;
	}

	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID FAR* pguidCmdGroup, DWORD nCmdID)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
		return E_NOTIMPL;
	}

	STDMETHOD(GetOptionKeyPath)(LPOLESTR FAR* pchKey, DWORD dw)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->GetOptionKeyPath(pchKey, dw);
		return E_FAIL;
	}

	STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->GetDropTarget(pDropTarget, ppDropTarget);
		return S_OK;
	}

	STDMETHOD(GetExternal)(IDispatch** ppDispatch)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->GetExternal(ppDispatch);
		return S_FALSE;
	}

	STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
		return S_FALSE;
	}

	STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet)
	{
		if (m_spDefaultDocHostUIHandler)
			return m_spDefaultDocHostUIHandler->FilterDataObject(pDO, ppDORet);
		return S_FALSE;
	}

	//
	// IOleCommandTarget
	//
	STDMETHOD(QueryStatus)(
		/*[in]*/ const GUID *pguidCmdGroup, 
		/*[in]*/ ULONG cCmds,
		/*[in,out][size_is(cCmds)]*/ OLECMD *prgCmds,
		/*[in,out]*/ OLECMDTEXT *pCmdText)
	{
		if(m_spDefaultOleCommandTarget)
			return m_spDefaultOleCommandTarget->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
		return S_OK;
	}

	STDMETHOD(Exec)(
		/*[in]*/ const GUID *pguidCmdGroup,
		/*[in]*/ DWORD nCmdID,
		/*[in]*/ DWORD nCmdExecOpt,
		/*[in]*/ VARIANTARG *pvaIn,
		/*[in,out]*/ VARIANTARG *pvaOut)
	{
		if (nCmdID == OLECMDID_SHOWSCRIPTERROR)
		{
			// Don't show the error dialog, but
			// continue running scripts on the page.
			(*pvaOut).vt = VT_BOOL;
			(*pvaOut).boolVal = VARIANT_TRUE;

			T* pT = static_cast<T*>(this);
			pT->OnStatusTextChange(_T("Script Error."));

			return S_OK;
		}

		if(m_spDefaultOleCommandTarget)
			return m_spDefaultOleCommandTarget->Exec(pguidCmdGroup, nCmdID, nCmdExecOpt, pvaIn, pvaOut);
		return S_OK;
	}

	//
	//	IUnknown implementation
	//
	STDMETHODIMP_(ULONG) AddRef()
	{
		return 1;
	}

	STDMETHODIMP_(ULONG) Release()
	{
		return 1;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
	{

		HRESULT hr = E_NOINTERFACE;
		
		if (ppvObject != NULL)
		{
			*ppvObject = NULL;

			if (IID_IUnknown == riid)
				*ppvObject = static_cast<IDocHostUIHandler*>(this);
			else if (IID_IOleCommandTarget == riid)
				*ppvObject = static_cast<IOleCommandTarget*>(this);
			else if (IID_IDocHostUIHandler == riid)
				*ppvObject = static_cast<IDocHostUIHandler*>(this);

			if (*ppvObject != NULL)
			{
				hr = S_OK;
				((LPUNKNOWN)*ppvObject)->AddRef();
			}

		}
		else
		{
			hr = E_POINTER;
		}
		
		return hr;

	}


private:
	// Default interface pointers
	CComQIPtr<IDocHostUIHandler, &IID_IDocHostUIHandler> m_spDefaultDocHostUIHandler;
	CComQIPtr<IOleCommandTarget, &IID_IOleCommandTarget> m_spDefaultOleCommandTarget;
};

__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::StatusTextChangeStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BSTR}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::TitleChangeStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BSTR}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::PropertyChangeStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BSTR}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::DownloadBeginStruct = {CC_STDCALL, VT_EMPTY, 0, {NULL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::DownloadCompleteStruct = {CC_STDCALL, VT_EMPTY, 0, {NULL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::OnQuitStruct = {CC_STDCALL, VT_EMPTY, 0, {NULL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::NewWindow2Struct = {CC_STDCALL, VT_EMPTY, 2, {VT_BYREF|VT_BOOL,VT_BYREF|VT_DISPATCH}}; 
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::NewWindow3Struct = {CC_STDCALL, VT_EMPTY, 5, {VT_BYREF|VT_BOOL,VT_BYREF|VT_DISPATCH, VT_I4, VT_BSTR, VT_BSTR}}; 
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::CommandStateChangeStruct = {CC_STDCALL, VT_EMPTY, 2, {VT_I4,VT_BOOL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::BeforeNavigate2Struct = {CC_STDCALL, VT_EMPTY, 7, {VT_DISPATCH,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_BOOL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::ProgressChangeStruct = {CC_STDCALL, VT_EMPTY, 2, {VT_I4,VT_I4}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::NavigateComplete2Struct = {CC_STDCALL, VT_EMPTY, 2, {VT_DISPATCH, VT_BYREF|VT_VARIANT}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::DocumentComplete2Struct = {CC_STDCALL, VT_EMPTY, 2, {VT_DISPATCH, VT_BYREF|VT_VARIANT}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::OnVisibleStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BOOL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::OnToolBarStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BOOL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::OnMenuBarStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BOOL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::OnStatusBarStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BOOL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::OnFullScreenStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BOOL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::OnTheaterModeStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BOOL}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::SetSecureLockIconStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_I4}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::NavigateErrorStruct = {CC_STDCALL, VT_EMPTY, 5, {VT_BYREF|VT_BOOL,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_DISPATCH}};
__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::PrivacyImpactedStateChangeStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BOOL}};
//__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::FileDownloadStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BYREF|VT_BOOL}};
//__declspec(selectany) _ATL_FUNC_INFO CWebBrowser2EventsBase::WindowClosingStruct = {CC_STDCALL, VT_EMPTY, 2, {VT_BYREF|VT_BOOL,VT_BOOL}};

//__declspec(selectany) _ATL_FUNC_INFO CHTMLDocumentEventsBase::OnMouseUpStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BYREF|VT_DISPATCH}};
//__declspec(selectany) _ATL_FUNC_INFO CHTMLDocumentEventsBase::OnMouseDownStruct = {CC_STDCALL, VT_EMPTY, 1, {VT_BYREF|VT_DISPATCH}};

#endif
