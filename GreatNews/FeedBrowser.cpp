#include "StdAfx.h"
#include <atlpath.h>
#include <atlutil.h>
#include "FeedBrowser.h"
#include "GNUtil.h"

#pragma warning(disable:4192)
#pragma warning(disable:4049)
#import <mshtml.tlb> rename("TranslateAccelerator", "mshtmlTranslateAccelerator")

#include <strsafe.h>

#include <Initguid.h>
DEFINE_GUID(CGID_IWebBrowser,0xED016940L,0xBD5B,0x11cf,0xBA,0x4E,0x00,0xC0,0x4F,0xD7,0x08,0x16);

#define HTML_FLAG_FINDWHOLEWORD 2
#define HTML_FLAG_FINDMATCHCASE 4

CFeedBrowser::CFeedBrowser(CFeedBrowserAdvisor& advisor) :
	m_advisor(advisor)
{
}

BOOL CFeedBrowser::PreTranslateMessage(MSG* pMsg)
{
	if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
	   (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
		return FALSE;

	// give HTML page a chance to translate this message
	return (BOOL)SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg);
}


void CFeedBrowser::OnNavigateComplete2(IDispatch* pDisp, const CString& szURL)
{
	if(BrowsingInternet())
	{
		m_advisor.OnUrl(GetLocationURL());
	}
	else
	{
		m_advisor.OnUrl(_T(""));
	}

	if(BrowsingInternet() && DiscoverFeeds())
	{
		m_advisor.OnFoundFeed(m_discoveredFeeds.size());
	}
}

void CFeedBrowser::OnProgressChange(long nProgress, long nProgressMax)
{
	m_advisor.OnProgressChange(nProgress, nProgressMax);
}

bool CFeedBrowser::BrowsingInternet()
{
	CString url = GetLocationURL();
	return CGNUtil::IsInternetUrl(url);
}

void CFeedBrowser::OnDocumentComplete(IDispatch* pDisp, const CString& szURL)
{
	baseClass::OnDocumentComplete(pDisp, szURL);

	if(m_strHighlight.GetLength())
	{
		HighLight(m_strHighlight);
		m_strHighlight.Empty();
	}
}


BOOL CFeedBrowser::OnBeforeNavigate2(IDispatch* pDisp, const CString& szURL, DWORD dwFlags, const CString& szTargetFrameName, CSimpleArray<BYTE>& pPostedData, const CString& szHeaders)
{
	Silent = (!CGNUtil::IsInternetUrl(szURL));

	BOOL bCancel = m_advisor.OnBeforeNavigate2(pDisp, szURL, dwFlags, szTargetFrameName, pPostedData, szHeaders);

	if(!bCancel)
		baseClass::OnBeforeNavigate2(pDisp, szURL, dwFlags, szTargetFrameName, pPostedData, szHeaders);

	return bCancel;
}

void CFeedBrowser::OnStatusTextChange(const CString& szText)
{
	m_advisor.OnStatusTextChange(szText);
}

void CFeedBrowser::OnTitleChange(const CString& szTitle)
{
	m_advisor.OnTitleChange(szTitle);
}


HRESULT CFeedBrowser::HighLight(const CString& highlightText, bool bHighLight, const CString& highlightAttr)
{
    // Retrieve the document object.
	CComPtr<IDispatch> pHtmlDocDispatch = NULL;
    HRESULT hr = m_pBrowser->get_Document(&pHtmlDocDispatch);
	if(FAILED(hr))
		return hr;
    CComQIPtr<IHTMLDocument2>doc(pHtmlDocDispatch);
    if (doc == NULL)
        return E_INVALIDARG;

	CComQIPtr<IMarkupServices> pMS;
	CComQIPtr<IMarkupContainer> pMC;
	CComPtr<IMarkupPointer> ptrBegin, ptrEnd;

	_bstr_t attr(highlightAttr);
	_bstr_t textToFind(highlightText);

	pMS = doc;
	pMC = doc;
	if(pMS && pMC)
	{
		pMS->CreateMarkupPointer(&ptrBegin);
		pMS->CreateMarkupPointer(&ptrEnd);

		ptrBegin->SetGravity(POINTER_GRAVITY_Right);
		ptrEnd->SetGravity(POINTER_GRAVITY_Left);

		ptrBegin->MoveToContainer(pMC, TRUE);
		ptrEnd->MoveToContainer(pMC, FALSE);

		while(TRUE)
		{ // Find text
			HRESULT hr = ptrBegin->FindText(textToFind, 0, ptrEnd, NULL);

			if (S_FALSE == hr) break;// did not find the text

			IHTMLElementPtr pFontEl;
			if (bHighLight)
			{
				hr = pMS->CreateElement(TAGID_FONT, attr, &pFontEl); // create FONT element with attributes for selection
				hr = pMS->InsertElement( pFontEl, ptrBegin, ptrEnd); // Insert created element to context
				ptrBegin->MoveToPointer(ptrEnd); // Continue searching
			}
			else
			{
				// Remove last created element in the context 
				hr = ptrBegin->CurrentScope( &pFontEl);
				hr = pMS->RemoveElement(pFontEl);
				ptrBegin->MoveToPointer(ptrEnd); // Continue searching 
			}
		}

		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

BOOL CFeedBrowser::OnNewWindow2(IDispatch** ppDisp)
{
	return m_advisor.OnNewWindow2(ppDisp);
}
BOOL CFeedBrowser::OnNewWindow3(IDispatch** ppDisp, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
	return m_advisor.OnNewWindow3(ppDisp, dwFlags, bstrUrlContext, bstrUrl);
}


HRESULT CFeedBrowser::LoadWebBrowserFromStream(IStream* pStream)
{
	HRESULT hr;
	CComPtr<IDispatch> pHtmlDoc = NULL;
	CComPtr<IPersistStreamInit> pPersistStreamInit = NULL;

    // Retrieve the document object.
	hr = m_pBrowser->get_Document(&pHtmlDoc);
	if(FAILED(hr))
		return hr;

    // Query for IPersistStreamInit.
    hr = pHtmlDoc->QueryInterface( IID_IPersistStreamInit,  (void**)&pPersistStreamInit );
    if ( SUCCEEDED(hr) )
    {
        // Initialize the document.
        hr = pPersistStreamInit->InitNew();
        if ( SUCCEEDED(hr) )
        {
            // Load the contents of the stream.
            hr = pPersistStreamInit->Load( pStream );

			//CComQIPtr<IHTMLDocument2>doc(pHtmlDoc);
			//if (doc != NULL)
			//{
			//	doc->put_charset(CComBSTR(_T("utf-8")));
			//}
        }
    }

	return S_OK;
}

HRESULT CFeedBrowser::LoadFromString(const CString& szHTML)
{
	//int u16Len = szHTML.GetLength();
	//int u8Len = (u16Len+1)*sizeof(wchar_t);
	//char* pBuffer = new char[u8Len+1];
	//int n = WideCharToMultiByte(CP_UTF8,
	//							0,
	//							(LPCTSTR)szHTML,
	//							u16Len,
	//							pBuffer,
	//							u8Len,
	//							" ",
	//							0);
	//ATLASSERT(n<=u8Len);
	//pBuffer[n]=0;

	size_t cchLength = szHTML.GetLength()*sizeof(TCHAR);
    HGLOBAL hHTMLText = GlobalAlloc( GPTR, cchLength + 2 + 2 );

    if ( hHTMLText )
    {
		BYTE* p = (BYTE*)hHTMLText;
		*p = (BYTE)0xFF;
		*(p+1)=(BYTE)0xFE;
		StringCchCopy((TCHAR*)(p+2), cchLength, (LPCTSTR)szHTML);

        //memcpy((void*)hHTMLText, (void*)pBuffer, n+1);
		//delete[] pBuffer;

		CComPtr<IStream> pStream;
        HRESULT hr = CreateStreamOnHGlobal( hHTMLText, TRUE, &pStream );
        if ( SUCCEEDED(hr) )
        {
            // Call the helper function to load the browser from the stream.
            LoadWebBrowserFromStream( pStream  );
			pStream = NULL;
        }
		else
		{
			GlobalFree( hHTMLText );
		}
    }
	else
	{
		//delete[] pBuffer;
	}

	return S_OK;
}

/*
HRESULT CFeedBrowser::HtmlFindNext(const CString& strFind,BOOL bForward,BOOL bMatchCase,BOOL bWholeWordOnly)
{
    static BSTR bstrSpanWhole = L"Textedit";
    static BSTR bstrSpanChar  = L"Character";

	CComPtr<IDispatch> pHtmlDocDispatch = NULL;
    // Retrieve the document object.
    HRESULT hr = m_pBrowser->get_Document(&pHtmlDocDispatch);
	if(FAILED(hr))
		return hr;

	CComBSTR bstrFind = (LPCTSTR)strFind;
	CComBSTR bstrReplace = _T("<font style=\"background-color:yellow\">");
	bstrReplace += bstrFind;
	bstrReplace += _T("</font>");

    CComQIPtr<IHTMLDocument2>piDoc(pHtmlDocDispatch);
    CComPtr<IHTMLSelectionObject>piSel;
    CComPtr<IHTMLTxtRange>piRange;
    CComPtr<IDispatch>pdispRange;

    BSTR bstrBookMark=NULL;
    CComBSTR bstrFindLocal;
    VARIANT_BOOL b=VARIANT_FALSE;
    long lFlag=0;

    if (piDoc == NULL)
        return E_INVALIDARG;

    if (S_OK != piDoc->get_selection(&piSel))
        return S_FALSE;

    if (S_OK != (hr=piSel->createRange(&pdispRange)) || (pdispRange == NULL))
        return hr;

    if (S_OK != pdispRange->QueryInterface(&piRange))
        return S_FALSE;

    if (S_OK == piRange->get_text(&bstrFindLocal) && (bstrFindLocal))
    {
        piRange->select();
        piRange->collapse((VARIANT_BOOL)(bForward) ? false : true);

        if (bForward)
            piRange->moveEnd(bstrSpanWhole, 1, &lFlag);
        else
            piRange->moveStart(bstrSpanWhole, -1, &lFlag);

        piRange->getBookmark(&bstrBookMark);
    }

    if (bstrFind != NULL)
        bstrFindLocal = bstrFind;

    if (!bstrFindLocal)
        return S_FALSE;

    //////////////////
    if (!bForward)
    {
        CComBSTR bstrT;
        piRange->expand(bstrSpanWhole, &b);
        piRange->collapse((VARIANT_BOOL)false);
        piRange->moveStart(bstrSpanChar, -1, &lFlag);
        piRange->get_text(&bstrT);
        piRange->moveEnd(bstrSpanChar, 1, &lFlag);
        piRange->put_text(bstrT);
    }
    //////////////////

    piRange->expand(bstrSpanWhole, &b);
    lFlag=0;

    if (bstrBookMark)
        piRange->moveToBookmark(bstrBookMark, &b);
    if (bMatchCase)
        lFlag |= HTML_FLAG_FINDMATCHCASE;
    if (bWholeWordOnly)
        lFlag |= HTML_FLAG_FINDWHOLEWORD;

	CComBSTR rCmd = _T("Backcolor");
    while ((S_OK == piRange->findText(bstrFindLocal,((bForward) ? 0 : 0x80000000),lFlag, &b))
			&& (b))
    {
        //piRange->scrollIntoView((VARIANT_BOOL)(bForward) ? false : true);
        // piRange->select();
		CComVariant v = _T("yellow");
		piRange->execCommand(rCmd,VARIANT_FALSE,v,NULL);
		piRange->setEndPoint(CComBSTR(_T("StartToEnd")),piRange);
    } 
	
	return S_OK;
}
*/

HRESULT CFeedBrowser::ExecCmdTarget(DWORD nCmdID)
{
	CComPtr<IDispatch> pHtmlDocDispatch = NULL;
    // Retrieve the document object.
    HRESULT hr = m_pBrowser->get_Document(&pHtmlDocDispatch);
	if(FAILED(hr))
		return hr;
	
	CComQIPtr<IOleCommandTarget> pCmdTarget(pHtmlDocDispatch);
	if(!pCmdTarget)
		return E_FAIL;

	return pCmdTarget->Exec(&CGID_IWebBrowser, nCmdID, 0,
                            NULL, NULL);

}

size_t CFeedBrowser::DiscoverFeeds()
{
	m_discoveredFeeds.clear();
	return baseClass::DiscoverFeeds(m_discoveredFeeds);
}

void CFeedBrowser::OnMouseDown(long button, bool ctrl, bool bUserClickedLink, const CString& url)
{
	m_advisor.OnMouseDown(button, ctrl,bUserClickedLink, url);
}

long GetOffsetX(MSHTML::IHTMLElementPtr spElement)
{
	long x = (long)spElement->offsetLeft;
	MSHTML::IHTMLElementPtr spParent;// = spElement->parentElement; //offsetParent;
	if(spParent)
		x += GetOffsetX(spParent);

	return x;
}

long GetOffsetY(MSHTML::IHTMLElementPtr spElement)
{
	MSHTML::IHTMLElementPtr spParent; // = spElement->offsetParent;
	long y = (long)spElement->offsetTop;
	if(spParent)
		y += GetOffsetY(spParent);

	return y;
}

CString CFeedBrowser::GetUrlUnderMouse()
{
	DWORD dwPos = GetMessagePos();
	CPoint pt( GET_X_LPARAM( dwPos ), GET_Y_LPARAM ( dwPos ) );
	HWND hwndMouseOver = WindowFromPoint(pt);
	if(hwndMouseOver == NULL || !IsChild(hwndMouseOver))
		return _T("");

	CPoint pt_screen = pt;
	
	::ScreenToClient(hwndMouseOver, &pt);
	
	CPoint pt_client = pt;

	// --- HighDPI fix ---
    UINT dpiX = 96, dpiY = 96;
    HDC hdc = ::GetDC(hwndMouseOver);
    if (hdc)
    {
        dpiX = ::GetDeviceCaps(hdc, LOGPIXELSX);
        dpiY = ::GetDeviceCaps(hdc, LOGPIXELSY);
        ::ReleaseDC(hwndMouseOver, hdc);
    }

    if (dpiX > 0 && dpiY > 0 && (dpiX != 96 || dpiY != 96))
    {
        pt.x = ::MulDiv(pt.x, 96, dpiX);
        pt.y = ::MulDiv(pt.y, 96, dpiY);
    }

//* Debugging */	CString msg;
//* Debugging */	msg.Format(_T("pt_screen: %d/%d    pt_client: %d/%d    pt_client_fixed: %d/%d"), pt_screen.x, pt_screen.y, pt_client.x, pt_client.y, pt.x, pt.y);
//* Debugging */	::MessageBox(NULL, msg, _T("GetUrlUnderMouse"), MB_OK | MB_ICONINFORMATION);
    // --------------------------------------
	
	return GetUrlUnderPoint(pt);
}

CString CFeedBrowser::GetUrlUnderPoint(POINT pt)
{
	if(m_pBrowser == NULL)
		return _T("");

	try
	{
		MSHTML::IHTMLDocument2Ptr spDoc = GetDocument();
		if(spDoc != NULL)
		{
			if(m_bUseMozilla)
			{
				MSHTML::IHTMLElement2Ptr bodyElement = spDoc->body;
				if(bodyElement != NULL)
				{
					// adjust to scollbar position
					pt.y += bodyElement->scrollTop;
					pt.x += bodyElement->scrollLeft;
				}

				MSHTML::IHTMLElementCollectionPtr allElements = spDoc->all;
				for(long i = 0; i < allElements->length; ++i)
				{
					MSHTML::IHTMLElementPtr spElement = allElements->item(i,i);
					_bstr_t tag = spElement->tagName;
					if(_tcscmp((LPCTSTR)tag,_T("A")) != 0)
						continue;
					_bstr_t href = spElement->getAttribute(_T("href"),0);

					long x = GetOffsetX(spElement);
					long y = GetOffsetY(spElement);
					CRect rcElement(CPoint(x, y),
									CSize(spElement->offsetWidth, spElement->offsetHeight));
					if(rcElement.PtInRect(pt))
					{
						return (LPCTSTR)CGNUtil::FixUrl((LPCTSTR)href);
					}
				}
			}
			else	// IE
			{
				IDispatchPtr spDispatch = spDoc->elementFromPoint(pt.x, pt.y);
				MSHTML::IHTMLElementPtr spElement = spDispatch;
				while(spElement)
				{
					_bstr_t tagName = spElement->GettagName();
					if (tagName == _bstr_t(_T("A")))
					{
						_bstr_t bstrUrl = spElement->toString();
						MSHTML::IHTMLAnchorElementPtr spAnchor = spElement;
						if(spAnchor)
							spAnchor->focus();
						return (LPCTSTR)CGNUtil::FixUrl((LPCTSTR)bstrUrl);
					}

					spElement = spElement->parentElement;
				}
			}
		}
	}
	catch(_com_error& e)
	{
		HRESULT hr = e.Error();
	}
	catch(...)
	{
	}

	return _T("");
}

HWND CFeedBrowser::Create(HWND parent, bool bUseMozilla)
{
	static const CLSID CLSID_MozillaBrowser=
											{ 0x1339B54C, 0x3453, 0x11D2,
											{ 0x93, 0xB9, 0x00, 0x00,
											0x00, 0x00, 0x00, 0x00 } };
	m_bUseMozilla = bUseMozilla;

	CString clsID=_T("about:blank");
	if(m_bUseMozilla)
	{
		LPOLESTR pstrCLSID_CAxOwnControl = NULL;
		StringFromCLSID( CLSID_MozillaBrowser, &pstrCLSID_CAxOwnControl );
		clsID = pstrCLSID_CAxOwnControl;
		CoTaskMemFree(pstrCLSID_CAxOwnControl);
	}

	DWORD dwIEStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN |WS_CLIPSIBLINGS;
	if(!bUseMozilla)
		dwIEStyle |= WS_HSCROLL | WS_VSCROLL;
	HWND hwndBrowser = CWindowImpl<CFeedBrowser, CAxWindow>::Create(parent, rcDefault, clsID, dwIEStyle, WS_EX_CLIENTEDGE);
	if(::IsWindow(hwndBrowser))
	{
		//CComPtr<IAxWinAmbientDispatchEx> ptrWinAmbientDisp;
		//QueryHost(&ptrWinAmbientDisp);
		//HRESULT hr = ptrWinAmbientDisp->SetAmbientDispatch((IDispatch*)(&m_pobjAmbientDisp)); 
		//ATLASSERT(hr == S_OK);

		//IOleObjectPtr poc = m_pBrowser.p;
		//if(poc)
		//{
		//	CComPtr<IOleClientSite> oldSite;
		//	poc->GetClientSite(&oldSite);
		//	m_pobjAmbientDisp.m_oldClientSite = oldSite.p;
		//	poc->SetClientSite((IOleClientSite*)&m_pobjAmbientDisp);
		//}
	}

	return hwndBrowser;
}

void CFeedBrowser::SetBrowserProperties(int newProperites)
{
	m_pobjAmbientDisp.m_dwBrowserProperites = newProperites;

	try
	{
		IOleControlPtr poc = GetDocument();
		if(poc != NULL)
		{
			poc->OnAmbientPropertyChange(DISPID_AMBIENT_DLCONTROL);
		}
	}
	catch(...)
	{
	}
}

bool CFeedBrowser::IsAtPageBottom(bool bLegacyBrowser)
{
	if(bLegacyBrowser)
	{
		MSHTML::IHTMLDocument2Ptr spDoc = GetDocument();
		if(spDoc == NULL)
			return false;

		MSHTML::IHTMLElement2Ptr bodyElement = spDoc->body;
		if(bodyElement == NULL)
			return false;

		int scrollHeight = bodyElement->scrollHeight;
		int scrollTop = bodyElement->scrollTop;
		int clientHeight = bodyElement->clientHeight;
		int clientTop = bodyElement->clientTop;

		bodyElement = NULL;
		spDoc = NULL;

		return (scrollHeight <= clientHeight+scrollTop) ;
	}
	else
	{
		MSHTML::IHTMLDocument3Ptr spDoc = GetDocument();
		if(spDoc == NULL)
			return false;

		MSHTML::IHTMLElement2Ptr docElement = spDoc->documentElement;
		if(docElement == NULL)
			return false;

		int scrollTop = docElement->scrollTop;
		int scrollHeight = docElement->scrollHeight;
		int clientHeight = docElement->clientHeight;
		return (scrollHeight <= scrollTop+clientHeight);
	}
}

bool CFeedBrowser::IsAtPageTop(bool bLegacyBrowser)
{
	if(bLegacyBrowser)
	{
		MSHTML::IHTMLDocument2Ptr spDoc = GetDocument();
		if(spDoc == NULL)
			return false;

		MSHTML::IHTMLElement2Ptr bodyElement = spDoc->body;
		if(bodyElement == NULL)
			return false;

		int scrollTop = bodyElement->scrollTop;

		bodyElement = NULL;
		spDoc = NULL;

		return (scrollTop==0) ;
	}
	else
	{
		MSHTML::IHTMLDocument3Ptr spDoc = GetDocument();
		if(spDoc == NULL)
			return false;

		MSHTML::IHTMLElement2Ptr docElement = spDoc->documentElement;
		if(docElement == NULL)
			return false;

		int scrollTop = docElement->scrollTop;
		return (scrollTop == 0);
	}
}

bool CFeedBrowser::HasActiveElement()
{
	try
	{
		MSHTML::IHTMLDocument2Ptr spDoc = GetDocument();
		if(spDoc == NULL)
			return false;
		
		return (spDoc->activeElement != NULL);
	}
	catch(...)
	{
	}

	return false;
}

HWND CFeedBrowser::GetIEWindow()
{
	HWND wnd = NULL;

	MSHTML::IHTMLDocument2Ptr spDoc = GetDocument();
	if(spDoc == NULL)
		return NULL;

	CComQIPtr<IOleWindow> spOleWin = spDoc.GetInterfacePtr();
	if(spOleWin)
		spOleWin->GetWindow(&wnd);
	
	return wnd;
}

void CFeedBrowser::OnSetFocus(HWND hOldWnd)
{
	try
	{
		MSHTML::IHTMLDocument2Ptr spDoc = GetDocument();
		if(spDoc == NULL)
			return ;

		MSHTML::IHTMLElement2Ptr body = spDoc->body;
		if(body)
			body->focus();
	}
	catch(...)
	{
	}
}

