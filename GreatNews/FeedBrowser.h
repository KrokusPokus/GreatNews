#pragma once

#include <set>
#include "loki\SmartPtr.h"
#include "Browser.h"
//#include "GNCommand.h"
#include "FeedManagerLib.h"
#include "FeedBrowserAdvisor.h"

#define HTMLID_FIND 1
#define HTMLID_VIEWSOURCE 2
#define HTMLID_OPTIONS 3


class CAmbientDispatch : public IDispatch, IOleClientSite
{
public:
	CAmbientDispatch()
	{
		m_dwBrowserProperites = 0;
	}


public:
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
				*ppvObject = (IUnknown*)(IDispatch*)(this);
			else if (IID_IDispatch == riid)
				*ppvObject = static_cast<IDispatch*>(this);
			else if (IID_IOleClientSite == riid)
				*ppvObject = static_cast<IOleClientSite*>(this);

			if (*ppvObject != NULL)
			{
				hr = S_OK;
			}

		}
		else
		{
			hr = E_POINTER;
		}
		
		return hr;
	}

public:
        // IDispatch method
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
        { return E_NOTIMPL; }
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
        { return E_NOTIMPL; }
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames,
                            UINT cNames, LCID lcid, DISPID* rgDispId)
        { return E_NOTIMPL; }

	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
      LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
      EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		if (dispidMember == DISPID_AMBIENT_DLCONTROL)
		{
			if (!pvarResult)
				return E_POINTER;

			pvarResult->vt = VT_I4;
			pvarResult->lVal =  m_dwBrowserProperites;
			return S_OK;
		}
		else
		{
			return DISP_E_MEMBERNOTFOUND;
		}
	}

// IOleClientSite Methods
public:
   STDMETHOD(SaveObject)(void)
	{
		if(m_oldClientSite)
			return m_oldClientSite->SaveObject();
		else
			ATLTRACENOTIMPL(_T("IOleClientSite::SaveObject"));
	}

   STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
	{
		if(m_oldClientSite)
			return m_oldClientSite->GetMoniker(dwAssign, dwWhichMoniker,ppmk);
		else
			ATLTRACENOTIMPL(_T("IOleClientSite::GetMoniker"));
	}

   STDMETHOD(GetContainer)(IOleContainer **ppContainer)
	{
		if(m_oldClientSite)
			return m_oldClientSite->GetContainer(ppContainer);
		else
			ATLTRACENOTIMPL(_T("IOleClientSite::GetContainer"));
	}

   STDMETHOD(ShowObject)(void)
	{
		if(m_oldClientSite)
			return m_oldClientSite->ShowObject();
		else
			ATLTRACENOTIMPL(_T("IOleClientSite::ShowObject"));
	}

   STDMETHOD(OnShowWindow)(BOOL fShow)
	{
		if(m_oldClientSite)
			return m_oldClientSite->OnShowWindow(fShow);
		else
			ATLTRACENOTIMPL(_T("IOleClientSite::OnShowWindow"));
	}

   STDMETHOD(RequestNewObjectLayout)(void)
	{
		if(m_oldClientSite)
			return m_oldClientSite->RequestNewObjectLayout();
		else
			ATLTRACENOTIMPL(_T("IOleClientSite::RequestNewObjectLayout"));
	}

public:
	int m_dwBrowserProperites;
	IOleClientSitePtr m_oldClientSite;
};


class CFeedBrowser : public CWindowImpl<CFeedBrowser, CAxWindow>,
					 public CWebBrowser2<CFeedBrowser>
{
#define SAFE_BROWSER_PROPERTIES		DLCTL_NO_SCRIPTS|DLCTL_NO_DLACTIVEXCTLS|DLCTL_NO_RUNACTIVEXCTLS 
#define DEFAULT_BROWSER_PROPERTIES		0

	typedef CWebBrowser2<CFeedBrowser> baseClass;
public:
	DECLARE_WND_SUPERCLASS(_T("GN_Browser"), CAxWindow::GetWndClassName())
	CFeedBrowser(CFeedBrowserAdvisor& advisor);

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CFeedBrowser)
		MSG_WM_SETFOCUS(OnSetFocus)
		CHAIN_MSG_MAP(CWebBrowser2<CFeedBrowser>)
	END_MSG_MAP()

	void OnSetFocus(HWND hOldWnd);
	void OnMouseDown(long button, bool ctrl, bool bUserClickedLink, const CString& url);
	BOOL OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
	BOOL OnNewWindow2(IDispatch** ppDisp);
	void OnNavigateComplete2(IDispatch* pDisp, const CString& szURL);
	void OnDocumentComplete(IDispatch* pDisp, const CString& szURL);
	BOOL OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders);
	void OnStatusTextChange(const CString& szText);
	void OnTitleChange(const CString& szTitle);
	void OnProgressChange(long nProgress, long nProgressMax);

	// HRESULT HtmlFindNext(const CString& strFind, BOOL bForward=TRUE, BOOL bMatchCase=FALSE, BOOL bWholeWordOnly=FALSE);
	HRESULT HighLight(const CString& highlightText, bool bHighLight=true, const CString& highlightAttr=_T("COLOR=#0000ff STYLE=\"background-color:#ffff00\""));
	HRESULT LoadFromString(const CString& szHTML);
	HRESULT ExecCmdTarget(DWORD nCmdID);
	CString GetUrlUnderPoint(POINT pt);
	CString	GetUrlUnderMouse();
	bool IsAtPageBottom(bool bLegacyBrowser);
	bool IsAtPageTop(bool bLegacyBrowser);
	bool CanScroll();
	bool HasActiveElement();

	HWND GetIEWindow();

public:
	void SetHighlightString(const CString& str) {m_strHighlight=str;}
	bool BrowsingInternet();
	size_t DiscoverFeeds();
	HWND Create(HWND parent, bool bUseMozilla);
	void SetBrowserProperties(int newProperites=SAFE_BROWSER_PROPERTIES);

public:
	std::map<CString, CString> m_discoveredFeeds;

protected:
	HRESULT LoadWebBrowserFromStream(IStream* pStream);

protected:
	CAmbientDispatch m_pobjAmbientDisp; 
	CFeedBrowserAdvisor& m_advisor;
	CString m_strHighlight;
};

typedef Loki::SmartPtr<CFeedBrowser, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> FeedBrowserPtr;

