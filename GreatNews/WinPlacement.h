#pragma once

/////////////////////////////////////////////////////////////////////////////
// CWindowPlacement

class CWindowPlacement : public WINDOWPLACEMENT
{
public:
   CWindowPlacement()
   {
      ::ZeroMemory( (WINDOWPLACEMENT*) this, sizeof(WINDOWPLACEMENT) );
      length = sizeof(WINDOWPLACEMENT);
      showCmd = SW_NORMAL;
   }
   CWindowPlacement(HWND hWnd)
   {
      length = sizeof(WINDOWPLACEMENT);
      GetPosData(hWnd);
   }
   BOOL GetPosData(HWND hWnd)
   {
      ATLASSERT(::IsWindow(hWnd));
      return ::GetWindowPlacement(hWnd, this);
   }
   BOOL GetPosData(LPCTSTR pstr)
   {
      ATLASSERT(!::IsBadStringPtr(pstr, -1));
      if( ::lstrlen(pstr) == 0 ) return FALSE;
      flags = _GetInt(pstr);
      showCmd = _GetInt(pstr);
      ptMinPosition.x = _GetInt(pstr);
      ptMinPosition.y = _GetInt(pstr);
      ptMaxPosition.x = _GetInt(pstr);
      ptMaxPosition.y = _GetInt(pstr);
      rcNormalPosition.left = _GetInt(pstr);
      rcNormalPosition.top = _GetInt(pstr);
      rcNormalPosition.right = _GetInt(pstr);
      rcNormalPosition.bottom = _GetInt(pstr);
      return TRUE;
   }
   BOOL GetPosData(HKEY hReg, LPCTSTR pstrKeyName)
   {
      ATLASSERT(hReg);
      ATLASSERT(!::IsBadStringPtr(pstrKeyName,-1));
      TCHAR szData[80];
      DWORD dwCount = sizeof(szData)/sizeof(TCHAR);
      DWORD dwType = NULL;
      LONG lRes = ::RegQueryValueEx(hReg, pstrKeyName, NULL, &dwType, (LPBYTE)szData, &dwCount);
      if( lRes != ERROR_SUCCESS ) return FALSE;
      return GetPosData(szData);
   }
   BOOL SetPosData(HWND hWnd)
   {
      // NOTE: Do not place this call in the window's own WM_CREATE handler.
      //       Remember to call ShowWindow() if this method fails!
      ATLASSERT(::IsWindow(hWnd));
      ATLASSERT(!::IsRectEmpty(&rcNormalPosition)); // Initialize structures first
      // Make sure it is not out-of-bounds
      RECT rcTemp;
      RECT rcScreen = { 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) };
      if( ::IsRectEmpty(&rcNormalPosition) ) return FALSE;
      if( !::IntersectRect(&rcTemp, &rcNormalPosition, &rcScreen) ) return FALSE;
      // Show it...
      return ::SetWindowPlacement(hWnd, this);
   }
   BOOL SetPosData(LPTSTR pstr, UINT cchMax) const
   {
      ATLASSERT(!::IsBadWritePtr(pstr, cchMax));
      StringCchPrintf(pstr, cchMax, _T("%ld %ld (%ld,%ld) (%ld,%ld) (%ld,%ld,%ld,%ld)"),
         flags,
         showCmd,
         ptMinPosition.x,
         ptMinPosition.y,
         ptMaxPosition.x,
         ptMaxPosition.y,
         rcNormalPosition.left,
         rcNormalPosition.top,
         rcNormalPosition.right,
         rcNormalPosition.bottom);
      return TRUE;
   }
   BOOL SetPosData(CString& str)
   {
		SetPosData(str.GetBuffer(512), 512);
		str.ReleaseBuffer();

		return TRUE;
   }
   BOOL SetPosData(HKEY hReg, LPCTSTR pstrValueName) const
   {
      ATLASSERT(!::IsBadStringPtr(pstrValueName,-1));
      ATLASSERT(hReg);
      TCHAR szData[80];
      if( !SetPosData(szData, (UINT) sizeof(szData)/sizeof(TCHAR)) ) return FALSE;
      return ::RegSetValueEx(hReg, pstrValueName, NULL, REG_SZ, (CONST BYTE*) szData, (::lstrlen(szData)+1)*sizeof(TCHAR)) == ERROR_SUCCESS;
   }
   long _GetInt(LPCTSTR& pstr) const
   {
      // NOTE: 'pstr' argument is "byref"
      bool fNeg = false;
      if( *pstr == '-' ) {
         fNeg = true;
         pstr = ::CharNext(pstr);
      }
      long n = 0;
      while( *pstr >= '0' && *pstr <= '9' ) {
         n = (n * 10L) + (*pstr - '0');
         pstr = ::CharNext(pstr);
      }
      while( *pstr >= ' ' && *pstr < '0' && *pstr != '-' ) pstr = ::CharNext(pstr);
      return fNeg ? -n : n;
   }
};

