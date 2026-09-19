// DragDrop.h

#ifndef _DRAG_DROP_OBJ_H_
#define _DRAG_DROP_OBJ_H_  1

#define MAX_DROP_FORMATS      6  // Unless we need more...

#define DRAGDROP   0
#define DRAGENTER  1
#define DRAGOVER   2
#define DRAGLEAVE  3

template <class T, const IID* piid> 
class CUnknown : public T
{
public:
	CUnknown() { m_lRefCount = 1; }
	virtual ~CUnknown() { }

	ULONG   __stdcall AddRef ()  {  return InterlockedIncrement(&m_lRefCount);  } 
	ULONG   __stdcall Release()
	{
		long lCount = InterlockedDecrement(&m_lRefCount);
		if(lCount == 0) 
			delete this;
		return lCount; 
	}

	STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject)
	{
		// check to see what interface has been requested
		if(IsEqualIID(iid, *piid) || IsEqualIID(iid, IID_IUnknown)) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		}
		else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

protected:
	LONG		  m_lRefCount;		// Reference count for this COM interface
};

//////////////////////////////////////////////////////////////////////////
class CEnumFormatEtc : public CUnknown<IEnumFORMATETC, &IID_IEnumFORMATETC>
{
public:
	CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats)
	{
		m_nIndex      = 0;
		m_nNumFormats = nNumFormats;
		m_pFormatEtc  = new FORMATETC[nNumFormats];

		// copy the FORMATETC structures
		for(int i = 0; i < nNumFormats; ++i)
			DeepCopyFormatEtc(&m_pFormatEtc[i], &pFormatEtc[i]);
	}

	~CEnumFormatEtc()
	{
		if(m_pFormatEtc) {
			for(ULONG i = 0; i < m_nNumFormats; ++i) {
				if(m_pFormatEtc[i].ptd)
					CoTaskMemFree(m_pFormatEtc[i].ptd);
			}
			delete[] m_pFormatEtc;
		}
	}

	// IEnumFormatEtc members
	STDMETHOD(Next)(ULONG celt, FORMATETC *pFormatEtc, ULONG *pceltFetched)
	{
		ULONG copied  = 0;
		if(celt == 0 || pFormatEtc == 0)  // validate arguments
			return E_INVALIDARG;

		// copy FORMATETC structures into caller's buffer
		while(m_nIndex < m_nNumFormats && copied < celt) {
			DeepCopyFormatEtc(&pFormatEtc[copied], &m_pFormatEtc[m_nIndex]);
			copied++;
			m_nIndex++;
		}

		if(pceltFetched != 0) 
			*pceltFetched = copied;

		return (copied == celt) ? S_OK : S_FALSE;
	}

	STDMETHOD(Skip)(ULONG celt)
	{
		m_nIndex += celt;
		return (m_nIndex <= m_nNumFormats) ? S_OK : S_FALSE;
	}

	STDMETHOD(Reset)()
	{
		m_nIndex = 0;
		return S_OK;
	}

	STDMETHOD(Clone)(IEnumFORMATETC ** ppEnumFormatEtc)
	{
		HRESULT hResult = CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);
		if(hResult == S_OK) {   // manually set the index state
			((CEnumFormatEtc*) *ppEnumFormatEtc)->m_nIndex = m_nIndex;
		}
		return hResult;
	}

	static HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc)
	{
		if(nNumFormats == 0 || pFormatEtc == 0 || ppEnumFormatEtc == 0)
			return E_INVALIDARG;

		*ppEnumFormatEtc = new CEnumFormatEtc(pFormatEtc, nNumFormats);
		return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
	}

	static void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source)
	{
		// copy the source FORMATETC into dest
		*dest = *source;
		if(source->ptd)  {
			// allocate memory & copy the DVTARGETDEVICE if necessary
			dest->ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
			*(dest->ptd) = *(source->ptd);
		}
	}

private:
	ULONG		  m_nIndex;			// current enumerator index
	ULONG		  m_nNumFormats;	// number of FORMATETC members
	FORMATETC *m_pFormatEtc;	// array of FORMATETC objects
};

//////////////////////////////////////////////////////////////////////////
class CDataObject : public CUnknown<IDataObject, &IID_IDataObject>
{
public:
	FORMATETC  *m_pFormatEtc;
	STGMEDIUM  *m_pStgMedium;
	long	      m_nNumFormats;

	CDataObject()
	{
		m_nNumFormats = 0;
		Initialize();
	}

	CDataObject(UINT unFmt)
	{
		m_nNumFormats = 1;
		Initialize();
		// Set up the default
		m_pFormatEtc[0].cfFormat = (CLIPFORMAT)unFmt;
		m_pFormatEtc[0].ptd      = 0;
		m_pFormatEtc[0].dwAspect = DVASPECT_CONTENT;
		m_pFormatEtc[0].lindex   = -1;
		m_pFormatEtc[0].tymed    = TYMED_HGLOBAL;

		m_pStgMedium[0].tymed   = TYMED_HGLOBAL; 
		m_pStgMedium[0].hGlobal = 0;
		m_pStgMedium[0].pUnkForRelease = 0;
	}

	bool Initialize()
	{
		m_pFormatEtc  = new FORMATETC[MAX_DROP_FORMATS];
		m_pStgMedium  = new STGMEDIUM[MAX_DROP_FORMATS];

		// Fill in defaults for the structures
		for(int lIdx = 0; lIdx < MAX_DROP_FORMATS; ++lIdx) {
			m_pFormatEtc[lIdx].cfFormat = 0;
			m_pFormatEtc[lIdx].ptd      = 0;
			m_pFormatEtc[lIdx].dwAspect = DVASPECT_CONTENT;
			m_pFormatEtc[lIdx].lindex   = -1;
			m_pFormatEtc[lIdx].tymed    = TYMED_HGLOBAL;

			m_pStgMedium[lIdx].tymed   = TYMED_HGLOBAL; 
			m_pStgMedium[lIdx].hGlobal = 0;
			m_pStgMedium[lIdx].pUnkForRelease = 0;
		}
		return true;
	}

	~CDataObject()
	{
		FreeData();
		if(m_pFormatEtc) delete[] m_pFormatEtc;
		if(m_pStgMedium) delete[] m_pStgMedium;
	}

	//////////////// IUnknown members End /////////////////////////////////
	void SetHGlobal(HGLOBAL h, long lFmt=0)    
	{ 
		m_pStgMedium[lFmt].hGlobal = h;                 
	}

	///////////////////////////////////////////////////////////////////////
	void FreeData()          
	{ 
		for(long lIdx=0; lIdx < m_nNumFormats; ++lIdx) {
			if(m_pStgMedium[lIdx].hGlobal !=0) {
				GlobalFree(m_pStgMedium[lIdx].hGlobal);  
				m_pStgMedium[lIdx].hGlobal = 0;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////
	bool SetFormatData(UINT cfFmt, void *pData, long lBufLen)
	{
		long lIdx = LookupFormat(cfFmt, true); // true = Adds if needed & it can
		if(lIdx==-1) 
			return false;

		HGLOBAL hData = GlobalAlloc(GHND, lBufLen+1);
		if(hData==0) return false;

		BYTE *pPtr  = (BYTE*)GlobalLock(hData);
		memcpy(pPtr, pData, lBufLen);
		pPtr[lBufLen] = '\0';
		GlobalUnlock(hData);
		m_pStgMedium[lIdx].hGlobal = hData;  
		return true;
	}

	///////////////////////////////////////////////////////////////////////
	bool SetFormatData(UINT cfFmt, HGLOBAL hData)
	{
		long lIdx = LookupFormat(cfFmt, true); // true = Adds if needed & it can
		if(lIdx==-1) 
			return false;
		m_pStgMedium[lIdx].hGlobal = hData;  
		return true;
	}

	bool SetFormat(long lIdx, UINT unFmt)
	{
		if(lIdx > m_nNumFormats) return false;
		m_pFormatEtc[lIdx].cfFormat = (CLIPFORMAT)unFmt;
		return true;
	}

	static HGLOBAL DupMem(HGLOBAL hMem)
	{
		DWORD  len     = (DWORD) GlobalSize(hMem);
		void  *pSource = GlobalLock(hMem);

		void *pDest = GlobalAlloc(GMEM_FIXED, len);
		memcpy(pDest, pSource, len);
		GlobalUnlock(hMem);
		return pDest;
	}

	// IDataObject members
	STDMETHOD(GetData)(FORMATETC *pFormatEtc,  STGMEDIUM *pMedium)
	{
		// try to match the requested FORMATETC with one of our supported formats
		long lIdx = LookupFormatEtc(pFormatEtc);
		if(lIdx == -1)
			return DV_E_FORMATETC;

		// found a match! transfer the data into the supplied storage-medium
		pMedium->tymed			    = m_pFormatEtc[lIdx].tymed;
		pMedium->pUnkForRelease  = 0;
		switch(m_pFormatEtc[lIdx].tymed)
		{
		case TYMED_HGLOBAL:
			pMedium->hGlobal = DupMem(m_pStgMedium[lIdx].hGlobal);
			break;
		default:
			return DV_E_FORMATETC;
		}
		return S_OK;
	}

	STDMETHOD(GetDataHere)(FORMATETC* /*pFormatEtc*/,  STGMEDIUM* /*pMedium*/)
	{
		// GetDataHere is only required for IStream and IStorage mediums
		// It is an error to call GetDataHere for things like HGLOBAL and other clipboard formats
		return DATA_E_FORMATETC;
	}

	STDMETHOD(QueryGetData)(FORMATETC *pFormatEtc)
	{
		return (LookupFormatEtc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
	}

	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* /*pFormatEct*/,  FORMATETC *pFormatEtcOut)
	{
		// Apparently we have to set this field to NULL even though we don't do anything else
		pFormatEtcOut->ptd = NULL;
		return E_NOTIMPL;
	}   

	STDMETHOD(SetData)(FORMATETC* /*pFmt*/, STGMEDIUM* /*pMed*/,  BOOL /*bRel*/)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
	{
		if(dwDirection == DATADIR_GET) {
			// for Win2k+ you can use the SHCreateStdEnumFmtEtc API call, however
			// to support all Windows platforms we need to implement IEnumFormatEtc ourselves.
			return CEnumFormatEtc::CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);
		}
		return E_NOTIMPL;
	}

	STDMETHOD(DAdvise)(FORMATETC*, DWORD, IAdviseSink*, DWORD*)  {  return OLE_E_ADVISENOTSUPPORTED;  }
	STDMETHOD(DUnadvise)(DWORD)              { return OLE_E_ADVISENOTSUPPORTED; }
	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **)   { return OLE_E_ADVISENOTSUPPORTED; }

private:

	long LookupFormatEtc(FORMATETC *pFormatEtc)
	{
		for(long l = 0; l < m_nNumFormats; ++l)  {
			if((pFormatEtc->tymed    &  m_pFormatEtc[l].tymed)   &&
				pFormatEtc->cfFormat == m_pFormatEtc[l].cfFormat && 
				pFormatEtc->dwAspect == m_pFormatEtc[l].dwAspect)
			{
				return l;
			}
		}
		return -1;
	}

	long LookupFormat(UINT cfFormat, bool /*bAdd*/) 
	{
		for(long l = 0; l < m_nNumFormats; ++l)  {
			if(cfFormat == m_pFormatEtc[l].cfFormat)
				return l;
		}
		if(m_nNumFormats < MAX_DROP_FORMATS) {
			long lIdx = m_nNumFormats;
			SetFormat(lIdx, cfFormat);
			m_nNumFormats++;
			return lIdx;
		}
		return -1;
	}

};


//////////////////////////////////////////////////////////////////////////
class CDropSource : public CUnknown<IDropSource, &IID_IDropSource>
{
public:
	CDataObject  *m_pDataObj;
	CPoint        m_ptDrag;
	DWORD         m_grfInitKeyState;	// Initial mouse btn state   

	CDropSource()
	{
		Initialize();   
	}

	virtual ~CDropSource() 
	{ 
		if(m_pDataObj)
			m_pDataObj->Release();
	}

	void Initialize()
	{
		m_pDataObj = new CDataObject();
		m_ptDrag.SetPoint(-1,-1);
	}

	inline CPoint GetDragPt() { return m_ptDrag; }

	STDMETHOD(QueryContinueDrag)(BOOL bEscape, DWORD grfKeyState)
	{
		//	Called by OLE whenever Escape/Control/Shift/Mouse buttons have changed	
		if(bEscape)
			return DRAGDROP_S_CANCEL;	
		if((grfKeyState & MK_LBUTTON) == 0)
			return DRAGDROP_S_DROP;
		return S_OK;  
	}

	STDMETHOD(GiveFeedback)(DWORD /*dwEffect*/)  
	{
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}

	/////////////////////////////////////////////////////////////
	bool SetFormatData(UINT cfFormat, void *pData, long lBufLen)
	{
		if(!m_pDataObj) {
			m_pDataObj = new CDataObject();
			if(!m_pDataObj)
				return false;
		}
		return m_pDataObj->SetFormatData(cfFormat, pData, lBufLen);
	}

	/////////////////////////////////////////////////////////////
	bool SetFormatData(UINT cfFormat, HGLOBAL hData)
	{
		if(!m_pDataObj) {
			m_pDataObj = new CDataObject();
			if(!m_pDataObj)
				return false;
		}
		return m_pDataObj->SetFormatData(cfFormat, hData);
	}


	/////////////////////////////////////////////////////////////
	HRESULT DragDrop(HWND hWnd, DWORD dwValidEffects, DWORD &dwEffect)
	{
		HRESULT hResult = 0;
		::GetCursorPos(&m_ptDrag);
		::ScreenToClient(hWnd, &m_ptDrag);

		hResult = DoDragDrop(m_pDataObj, this, dwValidEffects, &dwEffect);
		if(m_pDataObj)
			m_pDataObj->FreeData();

		return hResult;
	}

	/////////////////////////////////////////////////////////////
	// Call this before ::DoDragDrop() to determine whether a drag operation should be started.
	bool CheckBeginDrag(HWND hWnd)
	{
		bool	bDragging = FALSE;

		// set start point
		::GetCursorPos(&m_ptDrag);
		// No need to convert ScreentoClient since we are comparing with screen coordinates below

		// set initial key state
		m_grfInitKeyState = 0;
		if (::GetKeyState(VK_LBUTTON) < 0)
			m_grfInitKeyState |= MK_LBUTTON;
		if (::GetKeyState(VK_RBUTTON) < 0)
			m_grfInitKeyState |= MK_RBUTTON;
		//DWORD dwLastTick = GetTickCount();
		::SetCapture(hWnd);

		while (!bDragging) {
			// some applications steal capture away at random times
			if (::GetCapture() != hWnd)
				break;

			// peek for next input message
			MSG msg;
			if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
				PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			{
				// check for button cancellation (any button down will cancel)
				if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP ||
					msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN)
					break;

				// check for keyboard cancellation
				if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
					break;

				// check for drag start transition
				bDragging = (msg.pt.x < m_ptDrag.x - DD_DEFDRAGMINDIST || msg.pt.x > m_ptDrag.x + DD_DEFDRAGMINDIST ||
					msg.pt.y < m_ptDrag.y - DD_DEFDRAGMINDIST || msg.pt.y > m_ptDrag.y + DD_DEFDRAGMINDIST);
			}

			// if the user sits here long enough, we eventually start the drag
			//if (GetTickCount() - dwLastTick > DD_DEFDRAGDELAY)
			// bDragging = TRUE;
		}
		ReleaseCapture();

		// Convert now for app usage.
		::ScreenToClient(hWnd, &m_ptDrag);

		return bDragging;
	}


};


//////////////////////////////////////////////////////////////////////////
// no longer a template, just derive you class from droptargetex
class CDropTargetEx : public CUnknown<IDropTarget, &IID_IDropTarget>
{
public:
	//_TParent    *m_pParent;
	HWND	       m_hDropWnd;
	bool         m_bValidData;
	IDataObject *m_pDataObject;
	CPoint       m_ptDrop;

	std::vector<UINT>  m_Formats;

	/////////////////////////////////////////////////////////////
	CDropTargetEx()
	{
		Initialize();
	}

	void Initialize()
	{
		m_hDropWnd = 0;
		m_bValidData = false;
		m_ptDrop.SetPoint(-1, -1);
		OleInitialize(NULL);
		CoLockObjectExternal(this, TRUE, FALSE);  // acquire a strong lock
	}

	~CDropTargetEx()
	{
		if(m_hDropWnd)
			UnregisterDropWindow();
	}

	bool AcceptFormat(UINT cfFormat)
	{
		m_Formats.push_back(cfFormat);
		return true;
	}

	inline CPoint GetDropPt() { return m_ptDrop; }

	bool RegisterDropWindow(HWND hWnd)
	{
		//m_pParent  = pParent;
		m_hDropWnd = hWnd;
		HRESULT hr = RegisterDragDrop(hWnd, this);
		if(FAILED(hr)) {
			ATLTRACE(_T("CDropTargetEx RegisterDragDrop failed:0x%x\n"), hr);
			return false;
		}
		return true;
	}

	/////////////////////////////////////////////////////////////
	void UnregisterDropWindow()
	{
		if(m_hDropWnd) {
			RevokeDragDrop(m_hDropWnd);   // remove drag+drop
			CoLockObjectExternal(this, FALSE, TRUE);  // remove the strong lock
			m_hDropWnd = 0;
		}
	}

	/////////////////////////////////////////////////////////////
	STDMETHOD(DragEnter)(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		m_pDataObject = pDataObject;
		m_bValidData = QueryDataObject(pDataObject);   // does it have data we want?
		m_ptDrop.SetPoint(pt.x, pt.y);
		ScreenToClient(m_hDropWnd, &m_ptDrop);
		*pdwEffect = DROPEFFECT_NONE;
		if(m_bValidData) {
			// get the dropeffect from the parent
			*pdwEffect = DragNotify(DRAGENTER, pDataObject, m_ptDrop);
			// modify the dropeffect based on keyboard state
			*pdwEffect = DropEffect(grfKeyState, m_ptDrop, *pdwEffect);
			SetFocus(m_hDropWnd);
		}
		AtlTrace(_T("DragEnter\n"));

		return S_OK;
	}


	/////////////////////////////////////////////////////////////
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		m_ptDrop.SetPoint(pt.x, pt.y);
		ScreenToClient(m_hDropWnd, &m_ptDrop);
		*pdwEffect = DROPEFFECT_NONE;
		if(m_bValidData) {
			// get the dropeffect from the parent
			*pdwEffect = DragNotify(DRAGOVER, m_pDataObject, m_ptDrop);
			// modify the dropeffect based on keyboard state
			*pdwEffect = DropEffect(grfKeyState, m_ptDrop, *pdwEffect);
			//SetFocus(m_hDropWnd);
		}
		AtlTrace(_T("DragOver\n"));
		return S_OK;
	}

	STDMETHOD(DragLeave)() 
	{ 
		//if(m_bValidData)
		//     DragNotify(DRAGLEAVE, m_pDataObject, m_ptDrop);
		AtlTrace(_T("DragLeave\n"));

		return S_OK; 
	}

	STDMETHOD(Drop)(IDataObject *pDataObject, DWORD /*grfKeyState*/, POINTL pt, DWORD *pdwEffect)
	{
		AtlTrace(_T("Drop\n"));

		m_ptDrop.SetPoint(pt.x, pt.y);
		ScreenToClient(m_hDropWnd, &m_ptDrop);
		*pdwEffect = DROPEFFECT_NONE;
		if(m_bValidData) 
			DragNotify(DRAGDROP, pDataObject, m_ptDrop);
		return S_OK;
	}

	/////////////////////////////////////////////////////////////
	DWORD DragNotify(long lFlag, IDataObject *pDataObject, CPoint pt)
	{
		DWORD dwRet=0;
		STGMEDIUM stgmed;
		FORMATETC fmtetc  = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		for(long l = 0; l < (long) m_Formats.size(); ++l) {
			fmtetc.cfFormat   = (CLIPFORMAT)m_Formats[l];
			// See if the dataobject contains the data type we want (stored as a HGLOBAL)
			if(pDataObject->QueryGetData(&fmtetc) == S_OK) { // Oh Joy! there be data sign!
				if(pDataObject->GetData(&fmtetc, &stgmed) == S_OK) {
					// we asked for the data as a HGLOBAL, so access it appropriately
					void *pData = GlobalLock(stgmed.hGlobal);
					if (lFlag == DRAGDROP)
						OnDragDrop(fmtetc.cfFormat, pData, pt);
					else if (lFlag == DRAGENTER)
						dwRet = OnDragEnter(fmtetc.cfFormat, pData, pt);
					else if (lFlag == DRAGOVER)
						dwRet = OnDragOver(fmtetc.cfFormat, pData, pt);
					else if (lFlag == DRAGLEAVE)
						OnDragLeave(fmtetc.cfFormat, pData, pt);
					GlobalUnlock(stgmed.hGlobal);
					ReleaseStgMedium(&stgmed);  // release the data using the COM API
				}
			}
		}
		return dwRet;
	}

	// overload these to do something in your class
	virtual DWORD OnDragEnter(UINT unFmt, void* pDrop, CPoint pt) { return IsValidDrop(unFmt, pDrop, pt); }
	virtual DWORD OnDragOver (UINT unFmt, void* pDrop, CPoint pt) { return IsValidDrop(unFmt, pDrop, pt); }
	virtual void  OnDragLeave(UINT /*unFmt*/, void* /*pDrop*/, CPoint /*pt*/)  {  }
	virtual void  OnDragDrop (UINT /*unFmt*/, void* /*pDrop*/, CPoint /*pt*/)  {  }
	virtual DWORD IsValidDrop(UINT /*unFmt*/, void* /*pDrop*/, CPoint /*pt*/)  {  return DROPEFFECT_NONE;  }

private:

	/////////////////////////////////////////////////////////////
	DWORD DropEffect(DWORD grfKeyState, CPoint /*pt*/, DWORD dwAllowed)
	{
		DWORD dwEffect = 0;
		// 1. check "pt" -> do we allow a drop at the specified coordinates?
		// 2. work out that the drop-effect should be based on grfKeyState
		if(grfKeyState & MK_CONTROL)  
			dwEffect = dwAllowed & DROPEFFECT_COPY;
		else if(grfKeyState & MK_SHIFT) 
			dwEffect = dwAllowed & DROPEFFECT_MOVE;

		// 3. no key-modifiers were specified (or drop effect not allowed), so
		//    base the effect on those allowed by the dropsource
		if(dwEffect == 0) {
			if(dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
			if(dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
			if(dwAllowed & DROPEFFECT_LINK) dwEffect = DROPEFFECT_LINK;
		}
		return dwEffect;
	}

	/////////////////////////////////////////////////////////////
	bool QueryDataObject(IDataObject *pDataObject)
	{
		FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		for(long l = 0; l < (long) m_Formats.size(); ++l) {
			fmtetc.cfFormat   = (CLIPFORMAT) m_Formats[l];
			if(pDataObject->QueryGetData(&fmtetc) == S_OK)
				return true;
		}
		return false;
	}
};


#endif