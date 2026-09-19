// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <atlbase.h>

#if (_ATL_VER >= 0x0700)
#include <atlstr.h>
#include <atltypes.h>
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_NO_UNION_CLASSES
#endif

#include <atlapp.h>
#include <atlmisc.h>

#include <atlwin.h>
#include <atlhttp.h>
#include <atlenc.h>

// TODO: reference additional headers your program requires here
