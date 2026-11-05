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
      if( ::IsRectEmpty(&rcNormalPosition) ) return FALSE;

      // The whole desktop, which may span several monitors and have negative coordinates
      RECT rcScreen = {
         ::GetSystemMetrics(SM_XVIRTUALSCREEN),
         ::GetSystemMetrics(SM_YVIRTUALSCREEN),
         ::GetSystemMetrics(SM_XVIRTUALSCREEN) + ::GetSystemMetrics(SM_CXVIRTUALSCREEN),
         ::GetSystemMetrics(SM_YVIRTUALSCREEN) + ::GetSystemMetrics(SM_CYVIRTUALSCREEN) };
      if( ::IsRectEmpty(&rcScreen) )
         ::SetRect(&rcScreen, 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));

      // rcNormalPosition is given in workspace coordinates (relative to the work area of the
      // primary monitor), so convert it to screen coordinates for the comparison.
      RECT rcWork = { 0, 0, 0, 0 };
      ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
      RECT rcScreenPos = rcNormalPosition;
      ::OffsetRect(&rcScreenPos, rcWork.left, rcWork.top);

      RECT rcTemp;
      if( !::IntersectRect(&rcTemp, &rcScreenPos, &rcScreen) ) return FALSE;

      // Don't hand the whole thing to SetWindowPlacement() on a window that has never been
      // shown: it sets position, size, restore rect and show state in one go, which leaves
      // the window with inconsistent frame/restore geometry on Wine (window 1-2px too tall,
      // caption not draggable, restore rect wrong). Instead show the window the ordinary way
      // first and only then place the normal rectangle explicitly: sizing a window that has
      // never been shown to exactly the work area height makes KWin (Wine) flag it as
      // "maximized vertically" when it is mapped.
      ::ShowWindow(hWnd, SW_SHOWNORMAL);
      ::SetWindowPos(hWnd, NULL, rcScreenPos.left, rcScreenPos.top,
         rcScreenPos.right - rcScreenPos.left, rcScreenPos.bottom - rcScreenPos.top,
         SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

      if( showCmd == SW_SHOWMAXIMIZED )
      {
         // Window is now at its normal position, so the restore rect is correct when maximizing.
         WINDOWPLACEMENT wp = { sizeof(wp) };
         if( ::GetWindowPlacement(hWnd, &wp) )
         {
            wp.showCmd = SW_SHOWMAXIMIZED;
            wp.flags = 0;
            wp.ptMaxPosition.x = wp.ptMaxPosition.y = -1;
            return ::SetWindowPlacement(hWnd, &wp);
         }
      }
      // minimized/normal -> start in normal state (already shown above)
      return TRUE;
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

